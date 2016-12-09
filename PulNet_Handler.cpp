#include "PulNet_Handler.h"
#include "Player.h"
#include "AOD.h"
#include "Game_Manager.h"
#include "Network.h"
#include "Utility.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <thread>
#include <vector>

// ------ static utils --------------------------------------------------------
// returns next parameter from a string received from ENET packet
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

static void NetObj_Loop(PulNet::PacketEvent, std::string, char*,
                        std::function<void (std::string, int&)>);

void PulNet::Handle_Packet(enet_uint8* data, size_t len, ENetPeer* peer) {
  // strip event type from data
  PulNet::PacketEvent nevent = static_cast<PulNet::PacketEvent>(data[0]);
  std::vector<uint8_t> vec(data+1, data + len);
  Handle_Packet(nevent, vec, peer);
}

/// MATCH TIMER SENDING TWICE ?????

void Add_NetObj_From_Server(AODP::NetObj* o, NetObj_Type type, uint8_t flag) {
  switch ( type ) {
    case NetObj_Type::player: {
      Player_Info* pl = game_manager->player_info[flag];
      pl->pl_handle = new Player(pl);
      o->Set_AODObj(pl->pl_handle);
      o->Set_NetID(static_cast<uint16_t>(NetObj_Type::player));
    } break;
    case NetObj_Type::match_timer: {
      auto mt = new MatchTimer();
      o->Set_AODObj(mt);
      game_manager->Set_Match_Time(mt);
      o->Set_NetID(static_cast<uint16_t>(NetObj_Type::match_timer));
    } break;
  }
}

void PulNet::Handle_Packet(PulNet::PacketEvent nevent,
                           std::vector<uint8_t>& data, ENetPeer* peer) {
  using namespace PulNet;
  int it;
  switch ( nevent ) {
    case PulNet::PacketEvent::netObj_create:{ // Type ID X Y FLAGS
      if ( game_manager->R_State() == Program_State::loading )
        game_manager->State_Update(Game_State::Spectator);
      int amt = data.size()/15;
      AOD::Output("Creating " + std::to_string(amt) + " netobjs from server");
      AOD::Output("Packet size: " + std::to_string(amt) + " ; " +
                                    std::to_string(data.size()));
      for ( int it = 0; amt > 0; -- amt ) {
        auto    id = Util::Unpack_Num<uint16_t>(data.begin(), it);
        auto  type = Util::Unpack_Num<uint16_t>(data.begin(), it);
        auto     x = Util::Unpack_Num<int16_t> (data.begin(), it);
        auto     y = Util::Unpack_Num<int16_t> (data.begin(), it);
        auto   rot = Util::Unpack_Num<float>   (data.begin(), it);
        auto  anim = Util::Unpack_Num<uint16_t>(data.begin(), it);
        auto flags = Util::Unpack_Num<uint8_t >(data.begin(), it);
        AODP::NetObj* nobj = new AODP::NetObj(0, type, nullptr);
        nobj->Set_NetID(id);
        Add_NetObj_From_Server(nobj, (NetObj_Type)type, flags);
        if ( nobj->R_AODObj() != nullptr ) {
          nobj->R_AODObj()->Set_Position(AOD::Vector(x, y));
          nobj->R_AODObj()->Set_Rotation(rot);
          AOD::Add(nobj->R_AODObj());
        }
        AODP::Network::Add_NetObj(nobj, id);
      }
    }break;
    case PulNet::PacketEvent::netObj_remove:{ // ID
      int amt = data.size();
      int i;
      for ( it = 0, i = 0; amt > 0; -- amt ) {
        auto id = Util::Unpack_Num<uint8_t>(data.begin(), it);
        AODP::Network::Rem_NetObj(id);
      }
    }
    case PulNet::PacketEvent::netObj_refresh:{ // ID X Y ROT ANIM
      auto server_timer = data[data.size()-1];
      data.pop_back();
      auto client_timer = game_manager->client_timer;
      std::string dbg_msg = "Server: " + std::to_string(server_timer) + ", " +
                            "Client: " + std::to_string(client_timer);
      // check not outdated
      if ( server_timer < client_timer ) {
        dbg_msg += "Checked ";
        if ( !(server_timer < 50 && client_timer > 200) ) {
          /* AOD::Output(dbg_msg + " Skipped"); */
          return;
        }
      }
      /* AOD::Output(dbg_msg + " Applied"); */
      AOD::Output("------- refresh ---------");
      game_manager->client_timer = server_timer;
      // interpret refresh
      int amt = data.size()/11;
      for ( int it = 0; amt > 0; -- amt ) {
        auto  id  = Util::Unpack_Num<uint16_t > (data.begin(), it);
        auto   x  = Util::Unpack_Num<int16_t  > (data.begin(), it);
        auto   y  = Util::Unpack_Num<int16_t  > (data.begin(), it);
        auto rot  = Util::Unpack_Num<float    > (data.begin(), it);
        auto anim = Util::Unpack_Num<uint8_t  > (data.begin(), it);
        if ( AODP::Network::net_objs.size() <= id ) {
          AOD::Output("Network trying to refresh invalid net obj ID " +
                      std::to_string(id) + ", size: " +
                      std::to_string(AODP::Network::net_objs.size()));
        } else {
          auto* nobj = AODP::Network::net_objs[id];
          if ( nobj == nullptr ) {
            AOD::Output("Net object ID " + std::to_string(id) + " is null");
          } else
            nobj->Set_Sync({x, y}, rot);
        }
      }
    }break;
    case PulNet::PacketEvent::connection:{
      // send Hard Client Refresh packet
      int it = 0;
      auto c_uid = Util::Unpack_Num<uint8_t>(data.begin(), it);
      game_manager->c_player = new Player_Info(c_uid, CV::username);
      Send_Hard_Client_Refresh();
      CV::map_name = Util::Unpack_Str(data, it);
      // grab server var pairs
      for ( int i = 0; i != Player_Consts::floats_map.size(); ++ i ) {
        auto fl  = Util::Unpack_Num<float>(data.begin(), it);
        *Player_Consts::floats_map[i] = fl;
      }
      // load map
      if ( !std::ifstream(CV::map_name) ) {
        AOD::Output("Map " + CV::map_name + " could not be loaded");
        enet_peer_disconnect(peer, 0);
      }
      std::thread([&](){game_manager->Load_Map(CV::map_name);}).detach();
      SDL_Delay(50);
    }break;
    case PulNet::PacketEvent::hard_client_refresh:{
      int it = 0;
      auto uid = Util::Unpack_Num<uint8_t>(data.begin(), it);
      auto longhand_name = Util::Unpack_Str(data, it);
      auto status  = Util::Unpack_Str(data, it);
      auto& pl = game_manager->player_info[uid];
      if ( pl == nullptr ) {
        AOD::Output("user " + longhand_name + " (" + std::to_string(uid) + ")" +
                    "connected");
        pl = new Player_Info(uid, longhand_name);
        if ( game_manager->c_player->uid == uid )
          game_manager->c_player = pl;
      }
      std::stringstream ss(status);
      std::string playerstatus, teamstatus;
      ss >> playerstatus >> teamstatus;
      pl->team = teamstatus == "red"  ? PlayerTeam::red  :
                 teamstatus == "blue" ? PlayerTeam::blue : PlayerTeam::none;
      pl->status = playerstatus == "dead"      ? PlayerStatus::dead      :
                   playerstatus == "alive"     ? PlayerStatus::alive     :
                   playerstatus == "spectator" ? PlayerStatus::spectator :
                                                 PlayerStatus::nil;
      AOD::Output("Received HCR from user ID " + std::to_string(uid));
      AOD::Output(" -- Team:   " + teamstatus);
      AOD::Output(" -- Status: " + playerstatus);
    }break;
  }
}
  /*
      if ( !std::ifstream(args[0]) ) {
        AOD::Output("Map " + args[0] +
                    " could not be loaded. Disconnecting from server");
        enet_peer_disconnect(server_peer, 0);
        return;
      }
      std::string temp = args[0];
      std::thread([&](){game_manager->Load_Map(temp);}).detach();
      SDL_Delay(50);
    case packet_event::newplayer: {
      int uid = Util::Str_To_T<uint32_t>(args[0]);
      if ( uid != game_manager->c_player->uid ) {
        game_manager->player_info[uid] = new Player_Info(uid, args[1]);
        PlayerStatus status = PlayerStatus::nil;
        if ( args[2] == "dead" ) {
          status = PlayerStatus::dead;
        }
        if ( args[2] == "alive" ) {
          status = PlayerStatus::alive;
          game_manager->player_info[t]->pl_handle =
              new Player(game_manager->player_info[t]);
          AOD::Add(game_manager->player_info[t]->pl_handle);
        }
        if ( args[2] == "spectator" ) {
          status = PlayerStatus::spectator;
        }
        game_manager->player_info[uid]->status = PlayerStatus(status);
      }
    }break;
    case PulNet::PacketEvent::player_join:{
      uint8_t uid = data[0];
      if ( game_manager->Player_Exists(uid) ) {
        game_manager->player_info[uid]->status = PlayerStatus::dead;
        if ( uid = game_manager->c_player->uid )
          game_manager->State_Update(Game_State::Dead);
      }
    }break;
    /*
    case PulEvent::EventType::kick:{
      enet_peer_disconnect(peer, 0);
    }break;
    case PulEvent::EventType::player_spawn:{
      uint8_t uid = data[0];
      uint16_t x = data[1]     + data[2]<<8,
               y = data[3]<<16 + data[4]<<24;
      Player_Info* pl_info = game_manager->player_info[uid];
      if ( pl_info ) {
        pl_info->status = PlayerStatus::alive;
        Player*& pl_handle = pl_info->pl_handle;
        pl_handle = new Player(pl_info);
        pl_handle->Set_Position({x, y});
        game_manager->Add_Obj<Player>(pl_handle);
        
        if ( game_manager->Pl_ID_Is_Me(uid) ) {
          game_manager->State_Update(Game_State::Alive);
        }
      }
    }break;
    case PulEvent::EventType::set_var:{
      std::string m = "";
      int it = 0;
      while ( data[it] != '\0' )
        m.push_back(data[it++]);
      const auto& var_it = Player_Consts::floats_map.find(m);
      if ( var_it == Player_Consts::floats_map.end() ) {
        AOD::Output("Server tried to change unknown " + m);
        return; // dismiss
      }
      *var_it->second = Util::Unpack_Num<float>(data.begin(), it);
    }break;
    case PulEvent::EventType::set_player_name:{
      int uid = data[0];
      
    }break;
  // get first notation
  //AOD::Output("Received packet " + std::string((char*)pack->data));
  /*
  switch ( p_translate_it->second.first ) {
    case packet_event::vimap:
    {
    }
    case packet_event::setID:
      game_manager->c_player = game_manager->player_info[std::stoi(args[0])] =
        new Player_Info(std::stoi(args[0]), CV::username);
    break;
    case packet_event::setvar:
    {
      const auto& float_it = Player_Consts::floats_map.find(args[0]);
      if ( float_it == Player_Consts::floats_map.end() ) {
        AOD::Output("Server tried to change unknown variable: " + args[0]);
        return; // just dismiss it
      }
      // we're on main thread so don't worry about atomics
      try {
        (*float_it->second) = std::stof(args[1]);
      } catch ( ... ) { return; }
      //AOD::Output("Server changed variable " + args[0] + " to " + args[1]);
    }
    break;
    case packet_event::refresh_player:
    {
      std::stringstream data_stream;
      data_stream << data;
      int uid;
      data_stream >> uid;
      float x, y;
      data_stream >> x >> y;
      // check valid
      /*if ( game_manager->player_info[uid] != nullptr &&
           game_manager->player_info[uid]->pl_handle != nullptr ) {
        // set positions/green
        auto& pl = game_manager->player_info[uid]->pl_handle;
        if ( pl )
          pl->Set_Position(x, y);
        data_stream >> x >> y;
        if ( pl )
          pl->Set_Velocity(x, y);
        data_stream >> x;
        if ( pl )
          pl->Set_Crouching(x);
      }
    }
    break;
    }break;
  }*/

void PulNet::Handle_Disconnect(ENetPeer* peer) {
  AOD::Output("You've been disconnected");
  peer->data = nullptr;
}

void PulNet::Handle_Player_Send() {
  /*
  if ( game_manager->c_player && game_manager->c_player->pl_handle ) {
    Player* pl = game_manager->c_player->pl_handle;
    ++PulNet::client_timestamp;
    if ( PulNet::client_timestamp == 0 )
      ++PulNet::client_timestamp;
    uint16_t keys =
      (pl->R_Key_Jump()   ? KEY_JUMP   : 0) +
                            KEY_DASH        +
      (pl->R_Key_Down()   ? KEY_DOWN   : 0) +
      (pl->R_Key_Left()   ? KEY_LEFT   : 0) +
      (pl->R_Key_Right()  ? KEY_RIGHT  : 0) +
      (pl->R_Key_Crouch() ? KEY_CROUCH : 0) +
      (pl->R_Key_Prim()   ? KEY_PRIM   : 0) +
      (pl->R_Key_Sec()    ? KEY_SEC    : 0);

    uint8_t dash = 1;

    if ( pl->R_Key_Dash() ) {
      switch ( pl->R_Dash_Angle() ) {
        case Player::Angle::Down: dash |= KEY_MISC_ANGLE_DOWN; break;
        case Player::Angle::Up:   dash |= KEY_MISC_ANGLE_UP;   break;
        case Player::Angle::Zero: dash |= KEY_MISC_ANGLE_ZERO; break;
      }
      switch ( pl->R_Dash_Dir() ) {
        case Player::Direction::Left:  dash |= KEY_MISC_DIR_LEFT;  break;
        case Player::Direction::Right: dash |= KEY_MISC_DIR_RIGHT; break;
      }
    }

    uint8_t wjmp = 1;

    if ( pl->R_Wjmp() ) {
      switch ( pl->R_WJmp_Dir() ) {
        case Player::Direction::Left:  wjmp |= KEY_MISC_DIR_LEFT;  break;
        case Player::Direction::Right: wjmp |= KEY_MISC_DIR_RIGHT; break;
      }

      switch ( pl->R_WJmp_Angle() ) {
        case Player::Angle::Up:   wjmp |= KEY_MISC_ANGLE_UP;   break;
        case Player::Angle::Zero: wjmp |= KEY_MISC_ANGLE_ZERO; break;
        case Player::Angle::Down: wjmp |= KEY_MISC_ANGLE_DOWN; break;
      }
    }
    std::vector<uint8_t> data;

    Util::Append_Pack_Num(data, PulEvent::EventType::player_refresh);
    Util::Append_Pack_Num(data, PulNet::client_timestamp);
    Util::Append_Pack_Num(data, keys);
    Util::Append_Pack_Num(data, dash);
    Util::Append_Pack_Num(data, wjmp);
    Util::Append_Pack_Num(data, pl->R_Aiming_Angle());

    PulNet::Send_Packet(PulNet::packet_event::client_refresh, data,
                        PulNet::PacketFlag::UDP);
  }*/
}

  
