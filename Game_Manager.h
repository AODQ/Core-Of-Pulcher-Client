/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef __Game_Manager_H_
#define __Game_Manager_H_
#pragma once

#include <atomic>
#include <vector>

#include "AOD.h"
#include "Client_Vars.h"
#include "MatchTimer.h"
#include "Images.h"
#include "Match.h"

class Map;
class Player;
class Player_Info;

enum class Program_State {
  in_menu, loading, in_game
};

enum class Loading_State {
  connecting_to_server,
  unpack_header, loading_map_data,
  extracting_sheet_data, extracting_background_data,
  extracting_script_data, loading_header_data,
  loading_sheets, loading_backgrounds,
  loading_scripts, finalizing_loading, loading_tiles,
  generating_spawners, send_done_loading,
  waiting_on_user, error, size
};

extern std::string loading_state_names[(int)Loading_State::size];

enum class Menu_State {
  Intro
};

enum class Game_State {
  Spectator, Dead, Alive
};

union Program_Union_State {
  Loading_State load_state;
  Menu_State    menu_state;
  Game_State    game_state;
};

namespace Game_Manager_State_Data {
  class GS_Dead {
  public:
    AOD::Text* spawn_timer_text;
    float      spawn_timer;
    GS_Dead();
  };

  class GS_Alive {
  public:
    GS_Alive();
  };

  union State_Data {
    GS_Dead gs_dead;
    GS_Alive gs_alive;
    void* nil;
    State_Data();
  };
}
namespace HUD {
  class HUDMain;
}

class Match;
enum class CV::Sound_Type;

class Game_Manager {
  Program_State state_id;
  Program_Union_State state;

  Program_State next_state_id;
  Program_Union_State next_state;

  Match* match = nullptr;
  MatchTimer* match_time;
  HUD::HUDMain* gHUD = nullptr;

  // these objects are ones that aren't game manager or other important
  // objects that need to run every frame. Typically on disconnect or
  // map change and so forth this vector is cleared
  std::vector<Player*> players;
  std::vector<AOD::Object*> misc;
  std::vector<AOD::Text*> misc_text;

  // for loading on main thread
  SheetContainer main_thread_loader_sc;
  std::string main_thread_loader_fil;
  std::vector<std::pair<Mix_Chunk*, CV::Sound_Type>> sounds_to_play;
  Mix_Music* music_to_play;
  CV::Sound_Type music_type;
  std::atomic<bool> main_thread_loader_done, main_thread_sound_done,
                    main_thread_music_done;

  // clears data from old status
  void Program_State_Revert(bool major_change);
  // updates new status
  void Program_State_Update(Program_State state_id, Program_Union_State state,
                            bool change);
  Game_Manager_State_Data::State_Data state_data;
public:
  uint8_t client_timer;

  Game_Manager();
  //Projectile_Manager* projectiles;
  Player_Info* player_info[256] = { nullptr };
  Player_Info* c_player = nullptr;

  // use this to ask main thread to load an image
  bool Load_Image_Main_Thread(std::string file);
  bool Poll_Image_Loaded     (SheetContainer& t);

  // use this to ask man thread to play sound or song
  void Play_Sound_Main_Thread(Mix_Chunk* sound, CV::Sound_Type);
  void Play_Sound_Main_Thread(Mix_Music* music, CV::Sound_Type);

  // --- obj handlers ---------------------------------------------------------

  // will remove from objects held by game manager
  // of type, also will remove from world
  template <typename T>
  void Rem_Obj(T*);
  // will remove all objects held by game manager of
  // template type
  template <typename T>
  void Rem_All();
  // will remove all object held by game manager of all types
  void Rem_All_Of_Types();
  // will add from objects held by game manager
  // of type, also will add from world
  template <typename T>
  void Add_Obj(T*, int layer = 0);
  // return size of vector of type
  template <typename T>
  int R_Vec_Size();
  // return element of vector of type from index
  template <typename T>
  T* R_Vec_Elem(int);
  
  // map
  Map* curr_map = nullptr;

  // Removes all tiles from AOD references.
  // Should only be called once in between
  // gamestate transitions
  void Hard_Refresh_Map();

  void Load_Map(std::string map_name);
  void Add_Player();

  // utils
  void Set_Match_Time(MatchTimer*);
  const Util::Time& R_Match_Time() const;
  int R_Red_Score() const;
  int R_Blu_Score() const;
  const Match* R_Match() const;
  inline HUD::HUDMain* R_HUD() { return gHUD; }
  inline bool Player_Exists(uint32_t uid) {
    return (uid>=0 && uid<256) && (bool)player_info[uid];
  }
  bool Pl_ID_Is_Me(uint32_t);
  // updates

  void State_Update(Program_State, Program_Union_State);
  void State_Update(Loading_State);
  void State_Update(Game_State);
  void State_Update(Menu_State);
  Program_State R_State();
  Program_Union_State R_Union_State();
  Game_Manager_State_Data::State_Data& R_State_Data();
  void Update();
};

extern Game_Manager* game_manager;

#endif
