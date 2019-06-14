#include "TimeCriticalRendering.h"
#include <iostream>

TCR::TCR() {
  n_triangles_rendered = std::vector<int>(60, 0);
  times = std::vector<double>(60, 0);
  total_cost = 0;
  frame_num = 0;
  has_info = false;
}

void TCR::getBestLODs(Map* map, std::map<char,TriangleMesh*> meshes, double fps, double time, std::unordered_set<int> PVS, glm::vec3 player_pos) {
  
  bool done = false;

  total_cost = 0;
  
  std::map<int,int> lod_levels;
  // Calculate min number of triangles
  for (int idx : PVS) {
    int row = idx / map->layout[0].size();
    int col = idx % map->layout[0].size();
    if (map->layout[row][col] == 'w' ) {
      total_cost += 12;
      continue;
    }
    if (map->layout[row][col] == ' ') {
      total_cost += 2;
      continue;
    }

    TriangleMesh* curr = meshes[map->layout[row][col]];
    lod_levels[idx] = 0;
    total_cost += curr->lod.simp_tris[lod_levels[idx]].size() / 3;
  }
   
  // Change TPS here
  tps = 270000000;

  // cout << "Total Cost: " << n_triangles_rendered[0] << endl;
  // cout << "TPS calc: " << tps << endl;
  // cout << "Time last frame: " << time << endl;

  double max_cost = tps / 60;
  int best_cost;
  double benefit_low;
  double benefit_high;
  int cost_low;
  int cost_high;
  double ratio_low;
  double ratio_high;
  double best_ratio;
  int best_idx;

  glm::vec3 real_pos = glm::vec3(floor(-player_pos[0]), floor(player_pos[1]), floor(-player_pos[2]));

  while (!done) {
    best_idx = -1;
    best_ratio = std::numeric_limits<double>::min();

    for (int idx : PVS) {
      if (lod_levels[idx] == 3) continue;

      int row = idx / map->layout[0].size();
      int col = idx % map->layout[0].size();
      if (map->layout[row][col] == 'w' || map->layout[row][col] == ' ') continue;

      TriangleMesh* curr = meshes[map->layout[row][col]];

      double distance = glm::distance(real_pos, glm::vec3(row, 0, col));
      
      cost_low = curr->lod.simp_tris[lod_levels[idx]].size() / 3;
      cost_high = curr->lod.simp_tris[lod_levels[idx]+1].size() / 3;
      
      int lvl;
      // If lod level is 3 then its the original mesh
      if (lod_levels[idx] + 1 == 3) lvl = 12;
      else lvl = lod_levels[idx] + 6;
      benefit_low = (pow(2, lod_levels[idx] + 5) * (1/distance))/ curr->lod.diagonal_bbox;
      benefit_high = (pow(2, lvl) * (1/distance))/ curr->lod.diagonal_bbox;
      
      // With this function it does not work, rat is always negative
      // ratio_low = benefit_low/cost_low;
      // ratio_high = benefit_high/cost_high;
      // double rat = ratio_high - ratio_low;
      
      // This one does work 
      double rat = (benefit_high - benefit_low) / (cost_high - cost_low);
      if (total_cost + cost_high < max_cost && rat > best_ratio) {
        best_ratio = rat;
        best_idx = idx;
        best_cost = cost_high;
      } 
    }

    if (best_idx != -1) {
      lod_levels[best_idx]++;
      total_cost += best_cost;
    } else done = true;

  }

  // Set LOD levels
  for (int idx : PVS) {
    int row = idx / map->layout[0].size();
    int col = idx % map->layout[0].size();
    if (map->layout[row][col] == 'w' || map->layout[row][col] == ' ') {
      continue;
    }
    map->lodLevel[idx] = lod_levels[idx];
  }
  
}