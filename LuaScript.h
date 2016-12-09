#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_
#pragma once
extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lua.hpp>
}

namespace LuaScript {
  lua_State* state;

  // Initializes lua state, should run at the loading of a map
  void Initialize(std::string scriptname = "");
  // Checks if error occurred after processing a lua function
  void Check_Error(int error);
  // Terminates lua state, should run at map change or disconnect
  void Terminate();

  std::vector<std::string> object_list, event_list;
};

#endif