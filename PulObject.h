/*#ifndef OBJECTLIST_H_
#define OBJECTLIST_H_
#pragma once

#include "AOD.h"

#include <map>
#include <string>
#include <vector>

class ScriptsList {
public:
  std::vector<std::string> list;
};

class PulObj : AOD::Object {
  const std::string& pul_type;  
public:
  PulObj(const std::string& pulcher_type, Type = Type::nil);
  inline const std::string& R_Pul_Type() const { return pul_type; }
};

class PulPolyObj : AOD::Object {

public:
  PulPolyObj(const std::string& pulcher_type);

};

#endif*/