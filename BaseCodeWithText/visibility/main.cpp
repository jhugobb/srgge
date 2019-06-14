#include <iostream>
#include "Map.h"
#include <string>
#include <fstream>
#include <random>
#include <cmath>
#include <map>
#include <unordered_set>

#include <eigen3/Eigen/Dense>

#define RAYS_PER_CELL 300000
#define CELL_SIZE 1

using namespace std;

void parse(Map* map, string filename) {
  ifstream file(filename, ios::in);
  string line; 
  unsigned int size;
  unsigned int i = 0;
  while (getline(file, line)) {
    size = line.size(); 
    map->addRow(size);
    for (unsigned int j= 0; j< size; j++) {
      switch (line[j]) {
        case 'w':
          map->addWall(i,j);
          break;
        case ' ':
          map->addFloor(i,j);
          break;
				case 'b':
					map->addBunny(i,j);
					break;
				case 'a':
					map->addArmadillo(i,j);
					break;
				case 'm':
					map->addMax(i,j);
					break;
				case 'o':
					map->addMoai(i,j);
					break;
				case 'h':
					map->addHorse(i,j);
					break;
				case 'l':
					map->addLucy(i,j);
					break;
				case 'r':
					map->addFrog(i,j);
					break;
				case 'd':
					map->addDragon(i,j);
					break;
        default:
          break;
      }
    }
    i++;
  }
	file.close();
}

int main(int argc, char* argv[]) {

  Map* map = new Map();
  if (argc < 3) {
    cout << "Usage ./vis /path/to/map /path/to/vis" << endl;
    return 0;
  }
  parse(map, string(argv[1]));
  
  default_random_engine gen;
  uniform_real_distribution<double> dist(0.0,1.0);

  unsigned int map_size = map->layout.size();
  unsigned int row_size;
  
  double x;
  double y;
  double angle;

  Eigen::Vector2d v;
  
  std::map<int, unordered_set<int>> PVSs;
  cout << "map size: " << map_size << endl;
  for (unsigned int i = 0; i < map_size; i++) {
    row_size = map->layout[i].size();
    cout << "i " << i << endl;
    
    for (unsigned int j = 0; j < row_size; j++) {
      unordered_set<int> PVS;
      PVS.emplace(i*row_size + j);

      for (int k = 0; k < RAYS_PER_CELL; k++) {
        x = i + dist(gen) - 0.5;
        y = j + dist(gen) - 0.5;
        angle = dist(gen) * 2 * M_PI;

        // Check direction of ray
        int step_x = (cos(angle) >= 0) ? 1 : -1;
        int step_y = (sin(angle) >= 0) ? 1 : -1;

        // Calculate the closest component to the next grid border
        double near_x = (step_x >= 0) ? (i + 1) - x : x - i;	
        double near_y = (step_y >= 0) ? (j + 1) - y : y - j;

        v = Eigen::Vector2d(cos(angle),sin(angle));

        double dist_v = (v[0] != 0) ? near_x / v[0] : numeric_limits<double>::max();
        double dist_h = (v[1] != 0) ? near_y / v[1] : numeric_limits<double>::max();

        double step_ray_x = (v[0] != 0) ? 1.0 / v[0] : numeric_limits<double>::max();
        double step_ray_y = (v[1] != 0) ? 1.0 / v[1] : numeric_limits<double>::max();

        // Update step because we already added the current cell
        if (abs(dist_v) < abs(dist_h)) {
          dist_v = dist_v + step_ray_x;
          x += step_x;
        } else {
          dist_h = dist_h + step_ray_y;
          y += step_y;
        }
        while (x < map_size && x >= 0 && y < row_size && y >= 0) {

          unsigned int cell_pos_x = floor(x);
          unsigned int cell_pos_y = floor(y);

          PVS.emplace(cell_pos_x * row_size + cell_pos_y);
          if (map->layout[cell_pos_x][cell_pos_y] == 'w') {
            break;
          }
          // Update step
          if (abs(dist_v) < abs(dist_h)) {
            dist_v = dist_v + step_ray_x;
            x += step_x;
          } else {
            dist_h = dist_h + step_ray_y;
            y += step_y;
          }
        }
      }
      PVSs[i*row_size + j] = PVS;
    }
  }
  // Write to file
  ofstream output(string(argv[2]), ios::out);
  if (output.is_open()) {
    for (unsigned int i = 0; i < PVSs.size(); i++) {
      unordered_set<int> PVS = PVSs[i];
      for (int istep_ray_x : PVS) {
        output << istep_ray_x;
        output << " ";
      } 
      output << endl;
    }
    output.close();
  }
  return 0;
}