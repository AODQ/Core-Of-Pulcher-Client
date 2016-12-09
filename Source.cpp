/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#define NOMINMAX
#include "AOD.h"

#include <fstream>
#include <stdio.h>
#include <tchar.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <thread>

#include "Client_Vars.h"
#include "Game_Manager.h"
#include "Data.h"
#include "Network.h"

#include "Player.h"

// common resolutions:
// 800x600, 1920x1080

void Load_Settings() {
  CV::screen_width = 800;
  CV::screen_height = 600;
};


void Keep_Alive() {
  SDL_Delay(500);
  PulNet::Send_Packet("nil", PulNet::PacketFlag::UDP);
}

void Init() {
  AOD::Console::console_open = false;
  AOD_Engine::Console::console_type = AOD::Console::TYPE_DEBUG_IN;
  Load_Settings();

  AOD::Initialize(CV::screen_width, CV::screen_height, 17, "Pulcher");
  AOD::Camera::Set_Size(AOD::Vector(CV::screen_width, CV::screen_height));

  AOD::Text::Set_Default_Font("DejaVuSansMono.ttf",8);
  AOD::Initialize_Console(1,SDL_SCANCODE_GRAVE, "");
  AOD::Set_BG_Colour(.08,.08,.095);
  
  Load_All_Resources();

  AOD_Engine::Console::cursor->Set_Sprite(
                            Images_Misc::text_cursor.texture);
  AOD_Engine::Console::cursor->Set_Is_Static_Pos(1);
  AOD_Engine::Console::background->Set_Sprite(
                            Images_Misc::console_background.texture);
  AOD_Engine::Console::background->Set_Is_Static_Pos(1);

  CV::Load_Config();

  PulNet::Init();
  game_manager->State_Update(Menu_State::Intro);
  #ifdef PLAYERDEBUG
  game_manager->c_player = game_manager->player_info[0] =
  new Player_Info(0, CV::username);
  game_manager->c_player->pl_handle = new Player(game_manager->c_player);
  game_manager->c_player->pl_handle->Set_Position({ 300, 300 });
  AOD::Add(game_manager->c_player->pl_handle);
  #endif // PLAYERDEBUG

}
   
int _tmain(int argc, _TCHAR* argv[]) {
  Init();
  AOD::Camera::Set_Position(CV::screen_width/2, CV::screen_height/2);
  AOD::Object* test = new AOD::Object;
  std::thread(Keep_Alive);
  AOD::Run();
  return 0;
}