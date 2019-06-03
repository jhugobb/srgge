#include <eigen3/Eigen/Dense>
#include <iostream>
#include <limits>
#include <ctime>
#include <queue>
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

  map<int, vector<Triangle*>> neighbors;
  for (unsigned int i = 0; i < verts.size(); i++) {
    neighbors[i] = vector<Triangle*>();
  }

  vector<Triangle*> tri_structs;

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

  std::cout << "Starting calculation of Qs" << endl;
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

  bool useOctree = true;

  double min_x, min_y, min_z, max_x, max_y, max_z;
  min_x = min_y = min_z = numeric_limits<double>::max();
  max_x = max_y = max_z = numeric_limits<double>::min();
  for (unsigned int i = 0; i < verts.size(); i++) {
    glm::vec3 v = verts[i];
    if (v[0] < min_x) min_x = v[0];
    if (v[1] < min_y) min_y = v[1];
    if (v[2] < min_z) min_z = v[2];
    
    if (v[0] > max_x) max_x = v[0];
    if (v[1] > max_y) max_y = v[1];
    if (v[2] > max_z) max_z = v[2];
  }
  /* OCTREE DECIDER
   * ------------------------------------------------------------------------------
   * */
  Octree octree;
  if (useOctree) {
    glm::vec3 origin(min_x, min_y, min_z);
    //glm::vec3 origin(-1, -1, -1);
    octree = Octree(origin, max_x-min_x, max_y-min_y, max_z-min_z);
    clock_t begin = std::clock();
    for (unsigned int i = 0; i < verts.size(); i++) {
      octree.insert(verts[i], i);
    }
    clock_t end = std::clock();
    cout << "insertion" << endl;
    cout << double(end-begin) / CLOCKS_PER_SEC << endl;
    //int result = octree.nNodesAtLevel(6);
    // Cluster level 4 is low bound
    // Cluster level 5 seems good mid 
    for (unsigned int i = 4; i < 9; i+=2) {
      begin = clock();
      vector<Cluster*> clusters = octree.cluster(i);
      map<int, Cluster*> representatives;
      int count = 0;
      for (Cluster* c : clusters) {
        count += c->verts.size();
      }
      assert(count == verts.size());
      int k = 0;
      for (Cluster* c : clusters) {
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
      for (int i = 0; i < triangles.size(); i+=3) {
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
      
      cout << "Final vert count " << simp_v.size() << endl;
      cout << "Final tri count " << simp_t.size()/3 << endl;
    }
    // simp_vertices = verts;
    // simp_tris = triangles;
  } else {
    // int lvl1 = 64; //cbrt(verts.size());
    // vector<vector<vector<Cell>>> grid(lvl1, vector<vector<Cell>>(lvl1, vector<Cell>(lvl1, Cell())));
    // for (unsigned int i = 0; i < verts.size(); i++) {
    //   glm::vec3 v = verts[i];
    //   int x_idx=0, y_idx=0, z_idx=0;
    //   x_idx = floor((v[0]- min_x) / (max_x - min_x) * lvl1);
    //   y_idx = floor((v[1]- min_y) / (max_y - min_y) * lvl1);
    //   z_idx = floor((v[2]- min_z) / (max_z - min_z) * lvl1);
    //   if (x_idx == lvl1) x_idx--;
    //   if (y_idx == lvl1) y_idx--;
    //   if (z_idx == lvl1) z_idx--;
    //   GridPos gp;
    //   gp.x = x_idx;
    //   gp.y = y_idx;
    //   gp.z = z_idx;
    //   gridPos[i] = gp;
    //   grid[x_idx][y_idx][z_idx].verts.push_back(i);
    // }
    // vector<Eigen::Vector4d> v_bars;
    // int idx = 0;
    // for (unsigned int x = 0; x < lvl1; x++) {
    //   for (unsigned int y = 0; y < lvl1; y++) {
    //     for (unsigned int z = 0; z < lvl1; z++) {
    //       Eigen::Matrix4d Qtotal;
    //       Qtotal << 0,0,0,0,
    //                 0,0,0,0,
    //                 0,0,0,0,
    //                 0,0,0,0;
    //       Eigen::Vector4d pos(0,0,0,0);
    //       for (int i : grid[x][y][z].verts) {
    //         Qtotal += Qs[i];
    //         pos += Eigen::Vector4d(verts[i][0], verts[i][1], verts[i][2], 0);
    //       }
    //       pos/=grid[x][y][z].verts.size();
    //       pos[3] = 1;
    //       Qtotal(3,0) = 0;
    //       Qtotal(3,1) = 0;
    //       Qtotal(3,2) = 0;
    //       Qtotal(3,3) = 1;
    //       Eigen::Vector4d b(0,0,0,1);
    //       Eigen::Vector4d v_bar;
    //       if (Qtotal.determinant() == 0) {
    //         v_bar = pos;
    //       } else v_bar = Qtotal.fullPivLu().solve(b);
    //       v_bars.push_back(v_bar);
    //       grid[x][y][z].v_bar_idx = idx;
    //       idx++;
    //       //cout << v_bar[0] << " " << v_bar[1] << " " << v_bar[2] << " " << v_bar[3] << endl;
    //     }
    //   }
    // }

    // for (int i = 0; i < triangles.size(); i+=3) {
    //   int v1 = triangles[i];
    //   int v2 = triangles[i+1];
    //   int v3 = triangles[i+2];
    //   GridPos gp1 = gridPos[v1];
    //   GridPos gp2 = gridPos[v2];
    //   GridPos gp3 = gridPos[v3];
    //   Cell c1 = grid[gp1.x][gp1.y][gp1.z];
    //   Cell c2 = grid[gp2.x][gp2.y][gp2.z];
    //   Cell c3 = grid[gp3.x][gp3.y][gp3.z];
    //   if (c1.v_bar_idx == c2.v_bar_idx || c1.v_bar_idx == c3.v_bar_idx || c2.v_bar_idx == c3.v_bar_idx) {
    //     continue;
    //   }
    //   simp_tris.push_back(c1.v_bar_idx);
    //   simp_tris.push_back(c2.v_bar_idx);
    //   simp_tris.push_back(c3.v_bar_idx);
    // }
    // int i = 0;
    // for (unsigned int x = 0; x < lvl1; x++) {
    //   for (unsigned int y = 0; y < lvl1; y++) {
    //     for (unsigned int z = 0; z < lvl1; z++) {
    //     if (grid[x][y][z].verts.size() != 0) {
    //       i++;
    //     }
    //     Eigen::Vector4d pos = v_bars[grid[x][y][z].v_bar_idx];
    //     glm::vec3 new_pos(-pos[0]/pos[3], -pos[1]/pos[3], -pos[2]/pos[3]);
    //     simp_vertices.push_back(new_pos);
    //     }
    //   }
    // }
    // cout << "size: " << lvl1*lvl1*lvl1 << endl;
    // cout << i << endl;
  }

  // cout << "we got here" << endl;
  // simp_vertices = vector<glm::vec3>(verts);
  // for (unsigned int i = 0; i < tris.size(); i++) {
  //   simp_tris.push_back(tris[i].v[0]);
  //   simp_tris.push_back(tris[i].v[1]);
  //   simp_tris.push_back(tris[i].v[2]);
  // }
  // cout << "and here as well" << endl;
  simp_tris.push_back(triangles);
  simp_vertices.push_back(verts);
  Qs.clear();
  for (unsigned int i = 0; i < tri_structs.size(); i++) {
    free(tri_structs[i]);
  }
  neighbors.clear();
  octree.free();
}