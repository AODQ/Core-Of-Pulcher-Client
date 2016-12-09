/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Console_Handler.h"
#include "Client_Vars.h"
#include "Data.h"
#include "Event.h"
#include "Game_Manager.h"
#include "Network.h"
#include "Player.h"

#include <thread>

#include "AOD.h"


static std::string R_Next_Parameter(std::string& str) {
  std::string arg = "";
  if ( str.size() == 0 ) return "";
  while ( str[0] == ' ' ) str.erase(str.begin(), str.begin() + 1);
  if ( str[0] == '"' ) { // look until end of \"
    str.erase(str.begin(), str.begin()+1);
    while ( str.size() != 0 && str[0] != '"' ) {
      arg += str[0];
      str.erase(str.begin(), str.begin()+1);
    }
    if ( str.size() != 0 ) // remove \"
      str.erase(str.begin(), str.begin()+1);
  } else { // look until end of space
    while ( str.size() != 0 && str[0] != ' ' ) {
      arg += str[0];
      str.erase(str.begin(), str.begin()+1);
    }
  }
  if ( str.size() != 0 ) // remove space if one exists
    str.erase(str.begin(), str.begin()+1);
  return arg;
}

void Handle_Console_Input(std::string& inp) {
  if ( inp == "console" ) return;
  AOD::Output(inp);
  // try to execute command

  // ---- zero argument commands -----------
  std::string arg1 = R_Next_Parameter(inp);
  for ( auto& i : arg1 ) i = tolower(i);

  if ( arg1 == "quit" || arg1 == "exit" )
    exit(0);
  
  if ( arg1 == "spectate" ) {
    
  }

  if ( arg1 == "list_netobjs" ) {
    int tot_netobjs = AODP::Network::R_True_Amt_Netobjs();
    AOD::Output("Total netobjs: " + std::to_string(tot_netobjs));
    AODP::Network::Call_Func_On_Every_Netobj([](const AODP::NetObj* netobj) {
      auto otype = netobj->R_Obj_Type();
      NetObj_Type ntype = static_cast<NetObj_Type>(otype);
      AOD::Output("#"      + std::to_string(netobj->R_NetID())    + "  "   +
                  "Type: " + std::to_string(otype)                + " "    +
                  "( "     + NetObj_Types_To_String(ntype)        + " ) ");
    });
    return;
  }
  
  if ( arg1 == "join" ) {
    if ( game_manager->R_State() == Program_State::in_game &&
         game_manager->R_Union_State().game_state == Game_State::Spectator ){
      std::vector<uint8_t> pkt = {
          (uint8_t)PulNet::PacketEvent::query,
          (uint8_t)PulNet::PacketQueries::join_game,
          (uint8_t)PlayerTeam::none
      };
      PulNet::Send_Packet(pkt);
    }
    return;
  }

  if ( arg1 == "disconnect" ) {
    PulNet::Disconnect();
    return;
  }

  if ( arg1 == "say" ) { // counts as zero arg since takes entire inp
    PulNet::Send_Packet("SAY " + inp);
    return;
  }

  if ( arg1 == "ismusicplaying" ) {
    switch ( Mix_GetMusicType(nullptr) ) {
      case MUS_NONE:
        AOD::Output("No music is playing");
      break;
      case MUS_WAV:
        AOD::Output("Music format WAV playing");
      break;
      case MUS_OGG:
        AOD::Output("Music format OGG playing");
      break;
      case MUS_MP3:
        AOD::Output("Music format MP3 (unsupported!) playing");
      break;
      default:
        AOD::Output("Music format unknown playing");
    }
    return;
  }

  if ( arg1 == "pausemusic" ) {
    Mix_PauseMusic();
    return;
  }

  if ( arg1 == "haltmusic" ) {
    Mix_HaltMusic();
    return;
  }


  if ( arg1 == "continuemusic" ) {
    Mix_ResumeMusic();
    return;
  }

  if ( arg1 == "localhost" ) {
    game_manager->State_Update(Game_State::Spectator);
    game_manager->State_Update(Game_State::Alive);
    return;
  }
  
  // ---- debug one arg commands -----------

  if ( arg1 == "refresh_player_anim" ) {
    Images_Player::Refresh_Player_Image_Data();
    return;
  }
  
  // ---- one argument commands ------------
  std::string arg2 = R_Next_Parameter(inp);

  if ( arg1 == "connect" &&
       game_manager->R_State() == Program_State::in_menu ) {
    // update
    game_manager->State_Update(Loading_State::connecting_to_server);
    std::thread(PulNet::Connect, arg2).detach();
    return;
  }

  if ( arg1 == "load" ) {
    std::thread([&](){game_manager->Load_Map(arg2);}).detach();
  }

  if ( arg1 == "changename" ) {
    if ( arg2 == "" ) return;
    if ( arg2.size() > 10 )
      arg2.resize(10);
    for ( auto& i : arg2 )
      if ( !isalnum(i) ) i = 'X';
    CV::username = arg2;
    AOD::Output("Set username to: " + CV::username);
    std::vector<uint8_t> uvec = { (uint8_t)PulEvent::EventType::change_name };
    for ( auto i : CV::username )
      uvec.push_back(i);
    PulNet::Send_Packet(PulNet::PacketEvent::fire_event, uvec);
    return;
  }

  if ( arg1 == "playmusic" ) {
    auto i = AOD::Sounds::Load_Music("audio\\" + arg2);
    if ( i )
      game_manager->Play_Sound_Main_Thread(i, CV::Sound_Type::music_master);
    //Mix_FreeMusic(i);
    return;
  }

  if ( arg1 == "playsound" ) {
    auto i = AOD::Sounds::Load_Sound("audio\\" + arg2);
    if ( i )
      game_manager->Play_Sound_Main_Thread(i, CV::Sound_Type::effect);
    return;
  }
  
  if ( arg1 == "sethealth" ) {
    
  }

  // ---- debug one argument commands -----
  
  if ( arg1 == "leg" ) {
    try {
      auto anim = PlayerSheet::Str_To_Leg_Anim(arg2);
      if ( anim == PlayerSheet::Leg_Anim::size )
        throw (arg2 + " invalid leg animation");
      auto typ = PlayerSheet::Str_To_Leg_Anim(arg2);
      game_manager->c_player->pl_handle->R_Joint_Body().lower.Set_Anim(
        &Images_Player::nygel.R_Anim(typ), typ
      );
    } catch ( std::string e ) {
      AOD::Output(e);
    }
    return;
  }

  if ( arg1 == "hand" ) {
    try {
      auto anim = PlayerSheet::Str_To_Hand_Anim(arg2);
      if ( anim == PlayerSheet::Hand_Anim::size )
        throw ( arg2 + " invalid hand animation");
      auto typ = PlayerSheet::Str_To_Hand_Anim(arg2);
      game_manager->c_player->pl_handle->Set_Anim(typ);
    } catch ( std::string e ) {
      AOD::Output(e);
    }
    return;
  }

  // ---- two argument commands -----------
  std::string arg3 = R_Next_Parameter(inp);

  AOD::Output(arg1 + " unknown command");
}

void Update_Console_Input_Position() {
  namespace ECons = AOD_Engine::Console;
  ECons::input_after->Set_Position(
    ECons::input->R_Position().x + ECons::input->R_Str().size()*5,
    ECons::input->R_Position().y);
  ECons::cursor->Set_Position(ECons::input_after->R_Position());
  ECons::cursor->Add_Position(1, -3);
}
