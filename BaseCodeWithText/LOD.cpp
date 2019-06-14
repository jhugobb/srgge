#include <eigen3/Eigen/Dense>
#include <iostream>
#include <limits>
#include <ctime>
#include <queue>
#include <omp.h>
#include "TriangleMesh.h"
#include "LOD.h"

using namespace std;

struct GridPos {
  int x;
  int y;
  int z;
};

struct Cell {
  vector<int> verts;
  int v_bar_idx;
};

struct Triangle {
  int v1;
  int v2;
  int v3;
  bool is_calc;
  Eigen::Matrix4d K;
};


LOD::LOD() {}

LOD::LOD(TriangleMesh* tm) {
  simp_tris = vector<vector<int>>();
  simp_vertices = vector<vector<glm::vec3>>();
  vector<glm::vec3> verts = vector<glm::vec3>(tm->getVertices());
  vector<int> triangles = vector<int>(tm->getTriangles());

  cout << "Building V:{F}." << endl;
  map<int, vector<Triangle*>> neighbors;
  for (unsigned int i = 0; i < verts.size(); i++) {
    neighbors[i] = vector<Triangle*>();
  }

  vector<Triangle*> tri_structs;

  // Create triangles
  for (unsigned int i = 0; i < triangles.size(); i+=3) {
    Triangle* t = new Triangle();
    t->v1 = triangles[i];
    t->v2 = triangles[i+1];
    t->v3 = triangles[i+2];
    t->is_calc = false;
    neighbors[t->v1].push_back(t);
    neighbors[t->v2].push_back(t);
    neighbors[t->v3].push_back(t);
    tri_structs.push_back(t);
  }

  std::cout << "Starting calculation of Q matrices." << endl;
  // Calculate per vertex Q
  vector<Eigen::Matrix4d> Qs;
  vector<GridPos> gridPos;
  gridPos.resize(verts.size());
  for (unsigned int i = 0; i < verts.size(); i++) {
    vector<Triangle*> neigh_faces = neighbors[i];
    Eigen::Matrix4d Q;
    Q << 0,0,0,0,
         0,0,0,0,
         0,0,0,0,
         0,0,0,0;

    // For all the triangles that the vertex belongs to
    for (unsigned int j = 0; j < neigh_faces.size(); j++) {
      Eigen::Matrix4d K;
      Triangle* tri = neigh_faces[j];
      if (!tri->is_calc) {

        glm::vec3 v1 = verts[tri->v1];
        glm::vec3 v2 = verts[tri->v2];
        glm::vec3 v3 = verts[tri->v3];

        glm::vec3 n = glm::cross(v3-v1, v2-v1);
        n = glm::normalize(n);
        double a = n[0];
        double b = n[1];
        double c = n[2];
        double d = a*v1[0] + b*v1[1] + c*v1[2];

        K(0,0) = a*a;
        K(0,1) = a*b;
        K(0,2) = a*c;
        K(0,3) = a*d;
        
        K(1,0) = b*a;
        K(1,1) = b*b;
        K(1,2) = b*c;
        K(1,3) = b*d;

        K(2,0) = c*a;
        K(2,1) = c*b;
        K(2,2) = c*c;
        K(2,3) = c*d;

        K(3,0) = d*a;
        K(3,1) = d*b;
        K(3,2) = d*c;
        K(3,3) = d*d;

        tri->K = K;
        tri->is_calc = true;
      }
      Q += tri->K;
    }
    Qs.push_back(Q);
  }

  double min_x, min_y, min_z, max_x, max_y, max_z;
  min_x = min_y = min_z = numeric_limits<double>::max();
  max_x = max_y = max_z = numeric_limits<double>::min();

  // Calculate Bounding box Dimensions
  for (unsigned int i = 0; i < verts.size(); i++) {
    glm::vec3 v = verts[i];
    if (v[0] < min_x) min_x = v[0];
    if (v[1] < min_y) min_y = v[1];
    if (v[2] < min_z) min_z = v[2];
    
    if (v[0] > max_x) max_x = v[0];
    if (v[1] > max_y) max_y = v[1];
    if (v[2] > max_z) max_z = v[2];
  }

  glm::vec3 min(min_x, min_y, min_z);
  glm::vec3 max(max_x, max_y, max_z);
  diagonal_bbox = glm::distance(min, max);

  Octree octree;
  glm::vec3 origin(min_x, min_y, min_z);
  //glm::vec3 origin(-1, -1, -1);
  cout << "Creating octree." << endl;
  octree = Octree(origin, max_x-min_x, max_y-min_y, max_z-min_z);
  clock_t begin = std::clock();
  for (unsigned int i = 0; i < verts.size(); i++) {
    octree.insert(verts[i], i);
  }


  // Generate Clusters
  for (unsigned int i = 5; i < 8; i+=1) {
    begin = clock();
    cout << "Obtaining Cluster level " << i << "." << endl;
    vector<Cluster*> clusters = octree.cluster(i);
    map<int, Cluster*> representatives;
    int count = 0;
    for (Cluster* c : clusters) {
      count += c->verts.size();
    }

    int k = 0;
    unsigned int size = clusters.size();
    cout << "Calculating Representatives' positions." << endl;
    for (unsigned int j = 0; j < size; j++) {
      Cluster* c = clusters[j];
      Eigen::Matrix4d Qtotal;
      Qtotal << 0,0,0,0,
                0,0,0,0,
                0,0,0,0,
                0,0,0,0;
      Eigen::Vector4d pos(0,0,0,0);
      for (Vertex v : c->verts) {
        Qtotal += Qs[v.idx];
        pos += Eigen::Vector4d(verts[v.idx][0], verts[v.idx][1], verts[v.idx][2], 0);
        representatives[v.idx] = c;
      }
      pos/=c->verts.size();
      pos[3] = 1;
      Qtotal(3,0) = 0;
      Qtotal(3,1) = 0;
      Qtotal(3,2) = 0;
      Qtotal(3,3) = 1;
      Eigen::Vector4d b(0,0,0,1);
      Eigen::Vector4d v_bar;
      if (Qtotal.determinant() == 0) {
        v_bar = pos;
      } else v_bar = Qtotal.fullPivLu().solve(b);
      c->v_bar = v_bar;
      c->v_bar_idx = k;
      k++;
    }
    
    vector<int> simp_t;
    // Push vectors of cluster level
    for (unsigned int i = 0; i < triangles.size(); i+=3) {
      int v1 = triangles[i];
      int v2 = triangles[i+1];
      int v3 = triangles[i+2];
      Cluster* c1 = representatives[v1];
      Cluster* c2 = representatives[v2];
      Cluster* c3 = representatives[v3];
      if (c1 == c2 || c1 == c3 || c2 == c3) {
        continue;
      }
      simp_t.push_back(c1->v_bar_idx);
      simp_t.push_back(c2->v_bar_idx);
      simp_t.push_back(c3->v_bar_idx);
    }
    vector<glm::vec3> simp_v;
    for (unsigned int x = 0; x < clusters.size(); x++) {
      Cluster* c = clusters[x];
      Eigen::Vector4d pos = c->v_bar;
      glm::vec3 new_pos(-pos[0]/pos[3], -pos[1]/pos[3], -pos[2]/pos[3]);
      simp_v.push_back(new_pos);
    }
    simp_vertices.push_back(simp_v);
    simp_tris.push_back(simp_t);

    for (unsigned int i = 0; i < clusters.size(); i++) {
      free(clusters[i]);
    }
    clusters.clear();
    representatives.clear();
    
    cout << "Final vert count of Cluster level " << i << ": " << simp_v.size() << endl;
    cout << "Final tri count of Cluster level " << i << ": " << simp_t.size()/3 << endl;
    
  }
  clock_t end = std::clock();
  cout << "Creation of LOD took: ";
  cout << double(end-begin) / CLOCKS_PER_SEC << " sec." <<  endl << endl << endl;

  // Free memory
  simp_tris.push_back(triangles);
  simp_vertices.push_back(verts);
  Qs.clear();
  for (unsigned int i = 0; i < tri_structs.size(); i++) {
    free(tri_structs[i]);
  }
  neighbors.clear();
  octree.free();
}