#include "NetObj.h"
#include "Game_Manager.h"
#include "Player.h"
#include "PulNetObj.h"


void Add_NetObj_Pul(AODP::NetObj* obj, uint16_t type, uint8_t flags) {
  auto& add = AODP::Network::Add_NetObj;
  if ( (uint16_t)PulObjs::size <= type ) {
    AOD::Output("Server trying to create unknown object " +
                std::to_string(type));
  }
  switch ( (PulObjs)type ) {
    case PulObjs::player: {
      int pid = flags;
      obj->Set_AODObj(new Player(game_manager->player_info[pid]));
    }break;
    case PulObjs::match_timer: {
      obj->Set_AODObj(new MatchTimer());
      AOD::Output("CREATED MATCH TIMER!");
    }break;
    default: {
      obj->Set_AODObj(new AOD::Object());
      obj->R_AODObj()->Set_Image_Size({32, 32});
    }break;
  };
}
