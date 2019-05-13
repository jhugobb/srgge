#include "Map.h"

Map::Map() {

}

void Map::addRow(int size) {
  std::vector<char> v(size, ' ');
  layout.push_back(v);
}

std::set<char> Map::getTypesOfMesh() {
  return std::set<char>(types);
}
void Map::addWall(int i, int j) {
  layout[i][j] = 'w';
  types.insert('w');
}

void Map::addFloor(int i, int j) {
  layout[i][j] = 'f';
  types.insert('f');
}