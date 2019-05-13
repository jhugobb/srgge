#ifndef _MAP_INCLUDE
#define _MAP_INCLUDE

#include <vector>
#include <set>

class Map {

  public:
    Map();
    void addWall(int, int);
    void addFloor(int, int);
    void addRow(int);
    void addBlank(int, int);
    std::set<char> getTypesOfMesh();
    std::vector<std::vector<char>> layout;

  private:
    std::set<char> types;
};
#endif
