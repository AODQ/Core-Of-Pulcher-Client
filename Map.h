/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef MAP_H_
#define MAPH_
#pragma once

#include "Images.h"
#include <utility>
#include <vector>

class Background;
class Spawner;
class Game_Manager;


class Tile_Sheet_Container { // helper-tool for Map
public:
  enum edge_dir { W, A, S, D };
  std::vector<std::pair<int,char*>> edges;
  SheetContainer sheet;
  std::string file_name;
  Tile_Sheet_Container();
};


class Tile_Base : public AOD::PolyObj {
public:
  Tile_Base();

  int layer;

  // spritesheet, offsets of tile in image, used for edges
  int sheet_index, sheet_off_w, sheet_off_h, tile_index; // offsets for the sheet
  bool flip_x, flip_y; // flip x or y tile ?
  bool visible;
  bool collideable, affect_player, triggered_by_player, step_down_platform;
};


class Map {
  using Vec3D_TB = std::vector<std::vector<std::vector<Tile_Base>>>;
  
  inline bool Is_Valid(int, int);

  std::vector<Tile_Sheet_Container> tile_sheets;
  std::vector<Background> backgrounds;
  //std::vector<Tile_Base*> map_tiles_ref;
  
  Vec3D_TB map_tiles;
  
  //std::vector<Spawner > spawners;
  //std::vector<Spawner*> pl_spawns;
  friend Game_Manager;
public:
  Map(const char* fil_name);

  // failure/successful (bool)
  // int pos x, y, width, height
  // Returns vector containing all tiles within specified area
  std::vector<Tile_Base*> R_Tile_Vec(int,int,int,int);
  //std::vector<Tile_Base*>* R_Spawn_Pts() { return &spawn_tiles; }
  std::vector<Tile_Base*>& R_Tiles_Ref();
    
  int R_Height();
  int R_Width();
  int R_Layer_Size(int x,int y);
  Tile_Base* R_Tile(int x,int y,int l);

  std::vector<Tile_Sheet_Container>& R_Tile_Sheets();

  //std::vector<Spawner >* R_Spawners();
  //std::vector<Spawner*>* R_Player_Spawners();
};

class Sheet {
public:
  int width, height;
  std::vector< std::pair<int, AOD::PolyObj>> tile_polys;
};

#endif