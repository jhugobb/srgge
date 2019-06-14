#ifndef _LOD_INCLUDE
#define _LOD_INCLUDE

#include <vector>
#include <set>
#include <map>
#include "Octree.h"

using namespace std;

class TriangleMesh;

class LOD {

  public:
    LOD();
    LOD(TriangleMesh*);
    vector<vector<glm::vec3>> simp_vertices;
    vector<vector<int>> simp_tris;
    double diagonal_bbox;
};
#endif
