#ifndef _MAP_INCLUDE
#define _MAP_INCLUDE

#include <vector>
#include <set>
#include <map>

class Map {

  public:
    Map();
    void addWall(int, int);
    void addFloor(int, int);
    void addBunny(int, int);
    void addDragon(int,int);
    void addFrog(int,int);
    void addMax(int,int);
    void addArmadillo(int,int);
    void addLucy(int,int);
    void addHorse(int,int);
    void addMoai(int,int);
    void addRow(int);
    void addBlank(int, int);
    void addPlayer(int, int);
    std::set<char> getTypesOfMesh();
    std::vector<std::vector<char>> layout;
    std::map<int, int> lodLevel;

    int player_i;
    int player_j;

  private:
    std::set<char> types;
};
#endif
