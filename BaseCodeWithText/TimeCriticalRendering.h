#include <map>
#include <unordered_set>

#include "TriangleMesh.h"
#include "Map.h"

using namespace std;

class TCR {
  public:
    TCR();
    void getBestLODs(Map*, map<char, TriangleMesh*>, double, double, unordered_set<int>, glm::vec3);

    double tps;
    //int n_triangles_rendered; // Number of triangles rendered in the last frame;
    int total_cost;
    int frame_num;
    bool has_info;
    vector<int> n_triangles_rendered;
    vector<double> times;
};
