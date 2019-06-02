#ifndef _OCT_INCLUDE
#define _OCT_INCLUDE

#include <vector>
#include <set>
#include <eigen3/Eigen/Dense>
#include "TriangleMesh.h"

#define MAX_DEPTH 12

using namespace std;

struct Vertex {
  glm::vec3 coords;
  int idx;
};

struct node {
  bool is_leaf;
  vector<node*> children;
  vector<Vertex> verts;
};

struct Cluster {
  vector<Vertex> verts;
  Eigen::Vector4d v_bar;
  int v_bar_idx;
};

class Octree {
  public:
    Octree();
    Octree(glm::vec3, double, double, double);
    void insert(glm::vec3, int);
    vector<Cluster*> cluster(int);
    int nNodesAtLevel(int);
    int depth;
    void free();
  private:
    node* root;
    double root_length_x;
    double root_length_y;
    double root_length_z;
    glm::vec3 root_origin;
};
#endif
