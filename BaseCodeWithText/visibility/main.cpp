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

int main(int argc, char** argv) {

  Map* map = new Map();
  parse(map, "../../maps/3.map");
  
  default_random_engine gen;
  uniform_real_distribution<double> dist(0.0,1.0);

  unsigned int map_size = map->layout.size();
  unsigned int row_size;
  
  double x;
  double y;
  double angle;

  Eigen::Vector2d v;
  Eigen::Vector2d u;

  
  std::map<int, unordered_set<int>> PVSs;
  cout << "map size: " << map_size << endl;
  for (unsigned int i = 0; i < map_size; i++) {
    row_size = map->layout[i].size();
    //cout << "row size: " << row_size << endl;
    cout << "i " << i << endl;
    for (unsigned int j = 0; j < row_size; j++) {
      unordered_set<int> PVS;
      PVS.emplace(i*row_size + j-1);
      //cout << " j " << j << endl;
      for (int k = 0; k < RAYS_PER_CELL; k++) {
        x = i + dist(gen) - 0.5;
        y = j + dist(gen) - 0.5;
        angle = dist(gen) * 2 * M_PI;

        int step_x = (cos(angle) >= 0) ? 1 : -1;
        int step_y = (sin(angle) >= 0) ? 1 : -1;

        double near_x = (step_x >= 0) ? (i + 1) - x : x - i;	
        double near_y = (step_y >= 0) ? (j + 1) - y : y - j;

        v = Eigen::Vector2d(cos(angle),sin(angle));
        //v.normalize();

        double ray_step_to_vside = (v[0] != 0) ? near_x / v[0] : std::numeric_limits<double>::max();
        double ray_step_to_hside = (v[1] != 0) ? near_y / v[1] : std::numeric_limits<double>::max();

        double dx = (v[0] != 0) ? 1.0 / v[0] : std::numeric_limits<double>::max();
        double dy = (v[1] != 0) ? 1.0 / v[1] : std::numeric_limits<double>::max();

        if (std::abs(ray_step_to_vside) < std::abs(ray_step_to_hside)) {
          ray_step_to_vside = ray_step_to_vside + dx;
          x += step_x;
        } else {
          ray_step_to_hside = ray_step_to_hside + dy;
          y += step_y;
        }
        while (x < map_size && x >= 0 && y < row_size && y >= 0) {

          unsigned int cell_pos_x = floor(x);
          unsigned int cell_pos_y = floor(y);
          //cout << cell_pos_x << " " << cell_pos_y<< endl;

          PVS.emplace(cell_pos_x * row_size + cell_pos_y-1);
          if (map->layout[cell_pos_x][cell_pos_y] == 'w') {
            break;
          }

          if (std::abs(ray_step_to_vside) < std::abs(ray_step_to_hside)) {
            ray_step_to_vside = ray_step_to_vside + dx;
            x += step_x;
          } else {
            ray_step_to_hside = ray_step_to_hside + dy;
            y += step_y;
          }
        }
      }
      PVSs[i*row_size + j] = PVS;
    }
  }

  ofstream output("../results/3.vis", ios::out);
  if (output.is_open()) {
    for (unsigned int i = 0; i < PVSs.size(); i++) {
      unordered_set<int> PVS = PVSs[i];
      for (int idx : PVS) {
        output << idx;
        output << " ";
      } 
      output << endl;
    }
    output.close();
  }
  return 0;
}