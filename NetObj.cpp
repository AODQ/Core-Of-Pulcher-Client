#include "AOD.h"
#include "NetObj.h"
#include <map>

namespace EngNet = AODP::Network;
std::map<uint16_t, AODP::NetObj*> EngNet::net_objs;

AODP::NetObj::NetObj(NetFlagType flg, uint16_t _ot, AOD::Object* o) {
  obj = o;
  net_flags = flg;
  obj_type = _ot;
  dt = 20;
  dt_rot = 0.0f;
  dt_pos = {0.0f, 0.0f};
}

AODP::NetObj::NetObj(NetFlagType flg, NetObj_Type _ot, AOD::Object* _obj) {
  obj = _obj;
  net_flags = std::bitset<3U>(flg);
  obj_type = static_cast<int>(_ot);
  dt = 20;
}

AODP::NetObj::~NetObj() {}

void AODP::NetObj::Set_Sync(AOD::Vector npos, float nrot) {
  /* We could set current position to goal position, however i believe this
     would cause too many unnecessary 'teleports' so I'll just assume that
     this is OK, I might have to come back to this and make some force
     set position if the client falls behind from the server*/
  o_rot = dt_rot;
  o_pos = dt_pos;
  dt_pos = npos;
  dt_rot = nrot;
  -- dt;
}

void AODP::NetObj::Update_Sync() {
  ++ dt;
  float f = (float)(dt)/(60.f/15.f);
  // do lerp of position
  if ( f >= 1 ) {
    f = 1;
  }
  AOD::Output(std::to_string(f));
  obj->Set_Position({dt_pos.x + f * (o_pos.x - dt_pos.x),
                     dt_pos.y + f * (o_pos.y - dt_pos.y)});
  obj->Set_Rotation(dt_rot + f * (o_rot - dt_rot));
}

uint16_t net_object_cnt = 0;

void EngNet::Add_NetObj(AODP::NetObj* obj, uint16_t netID) {
  // TODO: Assert that t  obj->Set_NetID(netID);
  auto key = EngNet::net_objs.find(netID);
  if ( key != EngNet::net_objs.end() && key->second != nullptr ) {
    AOD::Output("Trying to overwrite NetObj ID: " + std::to_string(netID));
    AOD::Output(" -- Will deallocate object from AOD");
    Rem_NetObj(netID);
  }
  ++ net_object_cnt;
  EngNet::net_objs[netID] = obj;
}

void EngNet::Rem_NetObj(uint16_t netID) {
  auto& key = EngNet::net_objs.find(netID);
  if ( key == EngNet::net_objs.end() || key->second == nullptr ) {
    AOD::Output("Trying to remove nonexistent NetObj of ID: " +
                  std::to_string(netID));
  } else {
    -- net_object_cnt;
    auto*& ptr = EngNet::net_objs[netID];
    AOD::Remove( ptr->R_AODObj() );
    delete ptr;
    ptr = nullptr;
  }
}

void EngNet::Update_NetObj_Sync(){
  auto cnt = net_object_cnt;
  if ( cnt <= 0 ) return;
  for ( auto& n_i : net_objs ) {
    if ( n_i.second != nullptr ) {
      n_i.second->Update_Sync();
      if ( --cnt <= 0 ) return;
    }
  }
}

int EngNet::R_True_Amt_Netobjs() {
  int cnt = 0;
  for ( auto& obj : net_objs )
    if ( obj.second != nullptr )
      ++ cnt;
  return cnt;
}

void EngNet::Call_Func_On_Every_Netobj(
                  std::function<void(const AODP::NetObj*)> fn) {
  for ( auto& obj : net_objs )
    if ( obj.second != nullptr )
      fn(obj.second);
}

