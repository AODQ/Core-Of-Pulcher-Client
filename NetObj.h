#ifndef NETWORKOBJ_H_
#define NETWORKOBJ_H_
#pragma once

#include "AOD.h"
#include "NetObject_Types.h"
#include <cstdint>
#include <bitset>
#include <functional>

// "AODP" denotes a "prototype" AOD or a "pulcher" AOD, either one works.
// basically things that I want to put a more generalized form into the AOD
// library "officially" sometime in the future.
namespace AODP {
  // note that you MUST inherit from AOD::Object if you plan on using this!
  // (or alternatives such as PolyObj).
  class NetObj {
  public:
    enum FLAGS {
      SYNC_POS,
      SYNC_ROT,
      SYNC_ANIM,
      SIZE
    };
    using NetFlagType = std::bitset<FLAGS::SIZE>;
  private:
    NetFlagType net_flags;
    uint_fast8_t dt;
    AOD::Vector dt_pos, o_pos;
    float       dt_rot, o_rot;
    // ID of object on client AND server side (ID on AOD::Object does not
    // synchronize with the server)
    uint16_t netID;
    // Type of object, it does nothing for AOD Engine but it is necessary so
    // so that when you create an object on the client or server, we know
    // what type of object was made and the game itself must handle the
    // rest
    uint16_t obj_type;
    AOD::Object* obj;
  public:
    // You should call Add_NetObj immediately after this if you want to
    // add it to the network
    NetObj(NetFlagType, uint16_t obj_type, AOD::Object* obj);
    NetObj(NetFlagType,  NetObj_Type obj_type, AOD::Object* obj);
    ~NetObj();
    inline void Set_NetID(uint16_t n)  { netID = n;       }
    inline uint16_t R_NetID()    const { return netID;    }
    inline uint16_t R_Obj_Type() const { return obj_type; }
    inline AOD::Object* R_AODObj()     { return obj;      }
    inline void Set_AODObj(AOD::Object* o) { obj = o; }
    void Set_Sync(AOD::Vector npos, float rotation);
    void Update_Sync(); 
  };
  namespace Network {
    extern std::map<uint16_t /*NETID*/, AODP::NetObj*> net_objs;
    // will be implicitly called by network, will deallocate netobj
    void Rem_NetObj(uint16_t netID);
    // Add networked object to known networked objects map. This should only
    // be called by Add_NetObj_From_Server
    void Add_NetObj(AODP::NetObj*, uint16_t netID);
    // Update sync on all objects
    void Update_NetObj_Sync();
    // returns actual amount of net objects
    int R_True_Amt_Netobjs();
    // calls function on each net obj
    void Call_Func_On_Every_Netobj(std::function<void(const AODP::NetObj*)>);
  }
};

#endif
