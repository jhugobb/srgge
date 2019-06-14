#include "Octree.h"
#include <iostream>

using namespace std;

Octree::Octree() {}

Octree::Octree(glm::vec3 origin, double length_x, double length_y, double length_z) {
  root_length_x = length_x;
  root_length_y = length_y;
  root_length_z = length_z;
  root_origin = origin;
  root = NULL;
  depth = 0;
}

node* ins(Vertex v, node* root, glm::vec3 origin, double length_x, double length_y, double length_z, int dep, Octree* o) {
  if (o->depth < dep) o->depth = dep;
  double half_x, half_y, half_z;
  glm::vec3 new_origin;
  glm::vec3 center;
  glm::vec3 dir; 
  int i;
  if (root == NULL) {
    root = new node;
    root->is_leaf = true;
    root->children = vector<node*>(8, NULL); 
    root->verts = vector<Vertex>();
    root->verts.push_back(v);
    return root;
  }
  if (root->is_leaf && dep < MAX_DEPTH) {
    root->is_leaf = false;
    for (Vertex v2 : root->verts) {
      half_x = length_x/2;
      half_y = length_y/2;
      half_z = length_z/2;
      new_origin = origin;
      center = origin + glm::vec3(half_x, half_y, half_z);
      dir = v2.coords - center;
      i = 0;
      if (dir.x >= 0) {i++; new_origin.x += half_x;}
      if (dir.y >= 0) {i+=2; new_origin.y += half_y;}
      if (dir.z >= 0) {i+=4; new_origin.z += half_z;}
      root->children[i] = ins(v2, root->children[i], new_origin, half_x, half_y, half_z, dep+1, o);
    }
    half_x = length_x/2;
    half_y = length_y/2;
    half_z = length_z/2;
    new_origin = origin;
    center = origin + glm::vec3(half_x, half_y, half_z);
    dir = v.coords - center;
    i = 0;
    if (dir.x >= 0) {i++; new_origin.x += half_x;}
    if (dir.y >= 0) {i+=2; new_origin.y += half_y;}
    if (dir.z >= 0) {i+=4; new_origin.z += half_z;}
    root->children[i] = ins(v, root->children[i], new_origin, half_x, half_y, half_z, dep+1, o);
    root->verts.clear();
    return root;
  }
  if (!root->is_leaf) {
    half_x = length_x/2;
    half_y = length_y/2;
    half_z = length_z/2;
    new_origin = origin;
    center = origin + glm::vec3(half_x, half_y, half_z);
    dir = v.coords - center;
    i = 0;
    if (dir.x >= 0) {i++; new_origin.x += half_x;}
    if (dir.y >= 0) {i+=2; new_origin.y += half_y;}
    if (dir.z >= 0) {i+=4; new_origin.z += half_z;}
    
    root->children[i] = ins(v, root->children[i], new_origin, half_x, half_y, half_z, dep+1, o);
    return root;
  }
  if (root->is_leaf && dep >= MAX_DEPTH) root->verts.push_back(v);
  return root;
}

void Octree::insert(glm::vec3 vert, int idx) {
  Vertex v;
  v.coords = vert;
  v.idx = idx;
  root = ins(v, root, root_origin, root_length_x, root_length_y, root_length_z, 0, this);
}

bool getClust(node* root, Cluster* c) {
  if (root == NULL) return false; 
  if (root->is_leaf) {
    for (Vertex v : root->verts) c->verts.push_back(v);
    return true;
  } else {
    for (node* child : root->children) {
      getClust(child, c);
    }
    return true;
  }
}

vector<Cluster*> clust(node* root, int level) {
  vector<Cluster*> cs = vector<Cluster*>();
  if (root == NULL) return cs;
  if (level == 0) {
    if (root->is_leaf) {
      Cluster* c = new Cluster;
      c->verts = vector<Vertex>();
      c->verts.insert(c->verts.end(), root->verts.begin(), root->verts.end());
      cs.push_back(c);
    } else {
      for (node* child : root->children) {
        Cluster* c = new Cluster;
        c->verts = vector<Vertex>();
        bool succ = getClust(child, c);
        if (succ) cs.push_back(c);
        else free(c);
      }
    }
    return cs;
  } else {
    if (root->is_leaf) {
      Cluster* c = new Cluster;
      c->verts = vector<Vertex>();
      c->verts.insert(c->verts.end(), root->verts.begin(), root->verts.end());
      cs.push_back(c);
    } else {
      for (node* child : root->children) {
        vector<Cluster*> child_cs = clust(child, level-1);
        cs.insert(cs.end(), child_cs.begin(), child_cs.end());
      }
    }
    return cs;
  }
}

vector<Cluster*> Octree::cluster(int level) {
  vector<Cluster*> result;
  result = clust(root, level);
  return result;
}

int count(node* root, int level) {
  int result = 0;
  if (root == NULL) return 0;
  if (level == 0) return 1;
  else {
    for (node* child : root->children) {
      result += count(child, level-1);
    }
    return result;
  }
}

int Octree::nNodesAtLevel(int level) {
  return count(root, level);
}

void free_oct(node* root) {
  if (root == NULL) return;
  if (root->is_leaf) {
    root->verts.clear();
  }
  for (node* child : root->children) {
    free_oct(child);
    free(child);
  }
}

void Octree::free() {
  free_oct(root);
  std::free(root);
}