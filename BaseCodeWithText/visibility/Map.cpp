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
  layout[i][j] = ' ';
  types.insert(' ');
}

void Map::addBunny(int i, int j) {
  layout[i][j] = 'b';
  types.insert('b');
}

void Map::addArmadillo(int i, int j) {
  layout[i][j] = 'a';
  types.insert('a');
}

void Map::addDragon(int i, int j) {
  layout[i][j] = 'd';
  types.insert('d');
}

void Map::addFrog(int i, int j) {
  layout[i][j] = 'r';
  types.insert('r');
}

void Map::addMax(int i, int j) {
  layout[i][j] = 'm';
  types.insert('m');
}

void Map::addLucy(int i, int j) {
  layout[i][j] = 'l';
  types.insert('l');
}

void Map::addHorse(int i, int j) {
  layout[i][j] = 'h';
  types.insert('h');
}

void Map::addMoai(int i, int j) {
  layout[i][j] = 'o';
  types.insert('o');
}