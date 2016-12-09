#ifndef PULNETOBJ_H_
#define PULNETOBJ_H_
#pragma once
// similar to server's NetObject_Types.h
#include "NetObj.h"
#include <stdint.h>

enum PulObjs {
  player,
  match_timer,
  size
};

void Add_NetObj_Pul(AODP::NetObj*, uint16_t type, uint16_t flags);

#endif
