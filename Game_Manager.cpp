/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Game_Manager.h"

#include "AOD.h"
#include "Client_Vars.h"
#include "Data.h"
#include "HUD.h"
#include "Images.h"
#include "Network.h"
#include "NetEvent_Handler.h"
#include "Map.h"

#include "Player.h"

#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>

Game_Manager* game_manager = new Game_Manager();

std::string loading_state_names[(int)Loading_State::size] = {
  "connecting to server", "unpack header", "loading map data",
  "extracting sheet data", "extracting background data",
  "extracting script data", "loading header data", "loading sheets",
  "loading backgrounds", "loading scripts", "finalizing data loading",
  "loading tile info", "generating spawners", "sending done loading",
  "Loading finished: click to continue!", "Error, check console for details."
};

Game_Manager::Game_Manager() {  
  state_id = Program_State::in_menu;
  state.menu_state = Menu_State::Intro;
  main_thread_loader_fil = "";
  main_thread_loader_done.store(1);
  main_thread_music_done.store(1);
  main_thread_sound_done.store(1);
}

std::mutex image_main_thread_mutex, poll_image_mutex;

bool Game_Manager::Load_Image_Main_Thread(std::string file) {
  std::lock_guard<std::mutex> image_main_thread_guard(image_main_thread_mutex);
  if ( main_thread_loader_fil == "" ) {
    main_thread_loader_fil = file;
    main_thread_loader_done.store(0);
   } else
    return 0;
  return 1;
}

bool Game_Manager::Poll_Image_Loaded(SheetContainer & t) {
  std::lock_guard<std::mutex> poll_image_guard(poll_image_mutex);
  if ( main_thread_loader_done.load() ) {
    t = main_thread_loader_sc;
    main_thread_loader_fil = "";
    return 1;
  }
  return 0;
}

static std::mutex main_thread_sound_mutex;

void Game_Manager::Play_Sound_Main_Thread(Mix_Chunk* sound,CV::Sound_Type st) {
  std::lock_guard<std::mutex> main_thread_sound_lock(main_thread_sound_mutex);
  while ( !main_thread_sound_done ); // wait for main thread to finish using
  main_thread_sound_done.store(0); // stop main thread from using
  sounds_to_play.push_back( std::pair<Mix_Chunk*, CV::Sound_Type>(sound, st));
  main_thread_sound_done.store(1);
}

static std::mutex main_thread_music_mutex;

void Game_Manager::Play_Sound_Main_Thread(Mix_Music* music,CV::Sound_Type st) {
  std::lock_guard<std::mutex> main_thread_music_lock(main_thread_music_mutex);
  while ( !main_thread_music_done ); // wait for main thread to finish
  main_thread_music_done.store(0); // stop main thread from using
  music_to_play = music;
  music_type    = st;
  main_thread_sound_done.store(1);
}

// ---- object handlers -------------------------------------------------------


// objects

template <>
void Game_Manager::Rem_Obj<AOD::Object>(AOD::Object* t) {
  AOD::Remove(t);
  for ( int i = misc.size()-1; i != -1; -- i )
    if ( misc[i] == t )
      misc.erase(misc.begin() + i,
                 misc.begin() + i + 1);
}
template<>
void Game_Manager::Add_Obj<AOD::Object>(AOD::Object* t, int layer) {
  AOD::Add(t, layer);
  misc.push_back(t);
}
template <>
void Game_Manager::Rem_All<AOD::Object>() {
  while ( misc.size() != 0 ) {
    AOD::Remove(misc[misc.size()-1]);
    misc.pop_back();
  }
}
template <>
int Game_Manager::R_Vec_Size<AOD::Object>() {
  return misc.size();
}
template <>
AOD::Object* Game_Manager::R_Vec_Elem<AOD::Object>(int i) {
  if ( i < misc.size() )
    return misc[i];
  return nullptr;
}

// texts

template <>
void Game_Manager::Rem_Obj<AOD::Text>(AOD::Text* t) {
  AOD::Remove(t);
  for ( int i = misc_text.size()-1; i != -1; -- i )
    if ( misc_text[i] == t )
      misc_text.erase(misc_text.begin() + i,
                      misc_text.begin() + i + 1);
}
template <>
void Game_Manager::Add_Obj<AOD::Text>(AOD::Text* t, int l) {
  AOD::Add(t);
  misc_text.push_back(t);
};
template <>
void Game_Manager::Rem_All<AOD::Text>() {
  while ( misc_text.size() != 0 ) {
    AOD::Remove(misc_text[misc_text.size()-1]);
    misc_text.pop_back();
  }
}
template <>
int Game_Manager::R_Vec_Size<AOD::Text>() {
  return misc_text.size();
}
template <>
AOD::Text* Game_Manager::R_Vec_Elem<AOD::Text>(int i) {
  if ( i < misc_text.size() )
    return misc_text[i];
  return nullptr;
}

// players

template <>
void Game_Manager::Rem_Obj<Player>(Player* t) {
  AOD::Remove(t);
  for ( int i = this->players.size()-1; i != -1; -- i )
    if ( players[i] == t )
      players.erase(players.begin() + i,
                 players.begin() + i + 1);
}
template<>
void Game_Manager::Add_Obj<Player>(Player* t, int l) {
  AOD::Add(t, l);
  players.push_back(t);
}
template <>
void Game_Manager::Rem_All<Player>() {
  while ( players.size() != 0 ) {
    AOD::Remove(players[players.size() - 1]);
    players.pop_back();
  }
}
template <>
int Game_Manager::R_Vec_Size<Player>() {
  return players.size();
}
template <>
Player* Game_Manager::R_Vec_Elem<Player>(int i) {
  if ( i < players.size() )
    return players.at(i);
  return nullptr;
}

// defaults

template <typename T>
void Game_Manager::Rem_Obj(T* t) {}

template<typename T>
void Game_Manager::Add_Obj(T* t, int l) {}

template <typename T>
void Game_Manager::Rem_All() {}

template <typename T>
int Game_Manager::R_Vec_Size() { return 0; }

template <typename T>
T* Game_Manager::R_Vec_Elem(int i) { return nullptr; }

void Game_Manager::Rem_All_Of_Types() {
  Rem_All<AOD::Object>();
  Rem_All<Player>();
  Rem_All<AOD::Text>();
}


// ---- manager handling ------------------------------------------------------

void Game_Manager::Hard_Refresh_Map() {
  /*for ( int x = 0; x != curr_map->R_Width(); ++ x )
  for ( int y = 0; y != curr_map->R_Height(); ++ y )
  for ( int z = 0; z != curr_map->R_Layer_Size(x, y); ++ z )
    AOD::Add( curr_map->R_Tile(x, y, z) );*/
}

void Game_Manager::Load_Map(std::string map_name) {
  if ( std::ifstream(map_name).good() ) {
    if ( curr_map )
      delete curr_map;
    curr_map = new Map(map_name.c_str());
  } else
    std::cout << "Map file " << map_name << " does not exist\n";
}

void Game_Manager::Add_Player() {

}

bool Game_Manager::Pl_ID_Is_Me(uint32_t uid) {
  return (uint8_t)uid == c_player->uid;
}

const Util::Time& Game_Manager::R_Match_Time() const {
  if ( !match_time ) return Util::Time();
  const auto& timer = match_time->R_Timer();
  /* AOD::Output("1111: " + std::to_string(timer.R_Seconds())); */
  /* AOD::Output("2222: " + std::to_string(timer.R_Seconds())); */
  return timer;
}
void Game_Manager::Set_Match_Time(MatchTimer* mt) {
  /* std::stringstream a; */
  /* a << static_cast<void*>(mt); */
  /* AOD::Output("B MATCH TIMER: " + a.str()); */
  match_time = mt;
  /* a << static_cast<void*>(mt); */
  /* AOD::Output("A MATCH TIMER: " + a.str()); */
}
int Game_Manager::R_Red_Score() const { return match->R_Red_Score(); }
int Game_Manager::R_Blu_Score() const { return match->R_Blu_Score(); }
const Match* Game_Manager::R_Match() const { return match; }

// ----- update handling ------------------------------------------------------

std::mutex state_handler;

void Game_Manager::State_Update(Program_State st, Program_Union_State us) {
  std::lock_guard<std::mutex> lock(state_handler);
  next_state = us;
  next_state_id = st;
  game_manager;
}

using PUS = Program_Union_State;

void Game_Manager::State_Update(Loading_State ls) {
  Program_Union_State p;
  SDL_Delay(1);
  p.load_state = ls;
  State_Update(Program_State::loading, p);
}

void Game_Manager::State_Update(Game_State gs) {
  Program_Union_State p;
  p.game_state = gs;
  State_Update(Program_State::in_game, p);
}

void Game_Manager::State_Update(Menu_State ms) {
  Program_Union_State p;
  p.menu_state = ms;
  State_Update(Program_State::in_menu, p);
}

Program_State Game_Manager::R_State() { return state_id; }
Program_Union_State Game_Manager::R_Union_State() { return state; }
Game_Manager_State_Data::State_Data& Game_Manager::R_State_Data() {
  return state_data;
}

bool Cmp_PUS(Program_State id, PUS& a, PUS& b) {
  switch ( id ) {
    case Program_State::in_game: return a.game_state == b.game_state;
    case Program_State::in_menu: return a.menu_state == b.menu_state;
    case Program_State::loading: return a.load_state == b.load_state;
  }
}

void Game_Manager::Update() {
  RESTART_UPDATE:
  bool something_happened = 0;
  if ( !(something_happened = (next_state_id != state_id)) )
    something_happened = !Cmp_PUS(state_id, next_state, state);
  // check if something happened to state
  if ( something_happened ) {
    auto state_changed = next_state;
    auto state_changed_id = next_state_id;
    Program_State_Revert((state_id != next_state_id));
    Program_State_Update(next_state_id, next_state, state_id != next_state_id);
    if ( next_state_id != state_changed_id ||
        !Cmp_PUS(next_state_id, next_state, state_changed) ) {
      goto RESTART_UPDATE;
    }
  }

  state_id = next_state_id;
  state = next_state;

  // check if an image needs to be loaded
  if ( !main_thread_loader_done ) {
    main_thread_loader_sc = Load_Image(main_thread_loader_fil.c_str());
    main_thread_loader_done.store( 1 );
  }

  // check if sound needs to be played
  if ( !main_thread_sound_done ) {
    main_thread_sound_done.store(0); // stop other threads
    for ( auto i : sounds_to_play )
      AOD::Sounds::Play_Sound(i.first, CV::volume[(int)i.second]);
    main_thread_sound_done.store(1);
  }

  // check if music needs to be played
  if ( !main_thread_music_done ) {
    main_thread_music_done.store(0); // stop main thread from using
    AOD::Sounds::Play_Music(music_to_play, (int)music_type);
    main_thread_music_done.store(1);
  }
  // general update
  using namespace Game_Manager_State_Data;
  switch ( state_id ) {
    case Program_State::in_game:
      switch ( state.game_state ) {
        case Game_State::Dead:
          state_data.gs_dead.spawn_timer -= AOD::R_MS()/1000;
          if ( state_data.gs_dead.spawn_timer < 0 )
           state_data.gs_dead.spawn_timer = 0;
          state_data.gs_dead.spawn_timer_text->Set_String(
            "SPAWN IN " + std::to_string((int)std::ceilf(
                        state_data.gs_dead.spawn_timer)));
        break;
        case Game_State::Alive:
        break;
      }
      // update HUD
      gHUD->Update();
      match->Update();
    break;
    case Program_State::in_menu:

    break;
    case Program_State::loading:
      game_manager->R_Vec_Elem<AOD::Text>(0)->Set_String(
              loading_state_names[(int)state.load_state]);
    break;
  }
  
  if ( AOD::Input::keys[ SDL_SCANCODE_F ] ) {
    Images_Player::Refresh_Player_Image_Data();
  }
}

void Game_Manager::Program_State_Revert(bool maj_change) {
  // ---- major change --------------------------------------------------------
  if ( maj_change ) {
    Rem_All_Of_Types();
    switch ( state_id ) {
      case Program_State::loading:
        // stop music (only if match is in session)
        Mix_FadeOutMusic(20000);
      break;
      case Program_State::in_game:
        // remove HUD
        delete gHUD;
        delete match_time;
        gHUD = nullptr;
        match_time = nullptr;
      break;
    }
  }

  // minor change
  switch ( state_id ) {
    case Program_State::in_game:
      switch ( state.game_state ) {
        case Game_State::Spectator:

        break;
        case Game_State::Dead:
          AOD::Remove(state_data.gs_dead.spawn_timer_text);
        break;
      }
    break;
  }
}


void Game_Manager::Program_State_Update(Program_State state_id,
                       Program_Union_State state, bool change) {
  // ---- major change --------------------------------------------------------
  if ( change ) {
    switch ( state_id ) {
      case Program_State::in_game:
        // check required netobj components exists
        if ( match_time == nullptr )
          AOD::Output("Error: Match timer does not exist");
        AOD::Camera::Set_Position(CV::screen_width /2, CV::screen_height/2);
        // setup HUD
        this->gHUD = new HUD::HUDMain(HUD::Panel_Type::large);
        match = (Match*)(new Match_CTF());
      break;
      case Program_State::in_menu:
      break;
      case Program_State::loading:
        // add loading symbol to top
        AOD::Object* sym = new AOD::Object();
        sym->Set_Position(CV::screen_width/2, 100 + CV::screen_height/8.5f);
        sym->Set_Sprite(Images_Misc::pulcher_logo.texture, 0);
        sym->Set_Size(Images_Misc::pulcher_logo.width,
                      Images_Misc::pulcher_logo.height, 1);
        Add_Obj(sym);
        // current status
        AOD::Text* tex = new AOD::Text(10, CV::screen_height - 30, "");
        tex->Set_Font("DejaVuSansMono.ttf", 12);
        tex->Set_String("");
        Add_Obj(tex);
        // music
        int mus_id = (int)Util::R_Rand(0, Sounds::loading_screen_music_size);
        Play_Sound_Main_Thread(Sounds::loading_screen_music[mus_id],
                                        CV::Sound_Type::music_menu);
      break;
    }
  }

  // ---- minor change --------------------------------------------------------
  using namespace Game_Manager_State_Data;
  switch ( state_id ) {
    case Program_State::in_game:
      switch ( state.game_state ) {
        case Game_State::Dead:
          state_data.gs_dead = GS_Dead();
        break;
        case Game_State::Alive:
        break;
      }
    break;
    case Program_State::in_menu:

    break;
    case Program_State::loading:
      game_manager->R_Vec_Elem<AOD::Text>(0)->Set_String(
              loading_state_names[(int)state.load_state]);
      if ( state.load_state == Loading_State::send_done_loading ) {
        if ( !game_manager->c_player ) {
          AOD::Output("Server connection failed, c_player nil");
          PulNet::Disconnect();
          State_Update(Menu_State::Intro);
        } else {
          AOD::Output("Sending hard client refresh for done loading");
          game_manager->c_player->status = PlayerStatus::spectator;
          PulNet::Send_Hard_Client_Refresh();
        }
      }
    break;
  }
}

Game_Manager_State_Data::GS_Dead::GS_Dead() {
  spawn_timer_text = new AOD::Text(0, 0, "");
  spawn_timer_text->Set_Position(CV::screen_width/2, CV::screen_height/2);
  AOD::Add(spawn_timer_text);
  spawn_timer = 200U;
  spawn_timer_text->Set_String("SPAWN IN " + std::to_string(spawn_timer));
  PulNet::Send_Packet(std::string("SPAWNTIME"));
}

Game_Manager_State_Data::GS_Alive::GS_Alive() {}

Game_Manager_State_Data::State_Data::State_Data() {
  nil = nullptr;
}
