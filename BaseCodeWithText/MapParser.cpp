#include <fstream>
#include <string>

#include "Map.h"

using namespace std;

void parse(Map* map, string filename) {
  ifstream file(filename);
  string line; 
  unsigned int size;
  unsigned int i = 0;
  while (getline(file, line)) {
    size = line.size(); 
    map->addRow(size);
    for (int j= 0; j< size; j++) {
      switch (line[j]) {
        case 'w':
          map->addWall(i,j);
          break;
        case 'f':
          map->addFloor(i,j);
          break;
        // TODO: add more cases;
        default:
          break;
      }
    }
    i++;
  }
}