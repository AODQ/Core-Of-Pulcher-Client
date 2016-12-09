/*#include "AOD.h"
#include "LuaScript.h"

#include <fstream>
#include <string>

static int R_Key_Pressed(lua_State* l) {
  uint8_t key = luaL_checkinteger(l, 1);
  return AOD::Input::keys[key];
}

static 

static void Init_AOD_Lua_Functions() {
  auto new_func = [&](int (*f)(lua_State*), std::string nam) {
    lua_pushcfunction(LuaScript::state, f);
    lua_setglobal(LuaScript::state, "R_Key_Pressed");
  };
}

static void Load_Object_List(std::string scriptname) {
  // default object list
  LuaScript::object_list = {
    "Player"
  };

  // load custom objects
  if ( scriptname != "" ) {
    scriptname += "\/ObjectList.txt";
    std::ifstream fil(scriptname);
    try {
      if ( !fil.good ) throw "Could not open file";
      int line_number = 0;
      while ( fil.good ) {
        std::string line;
        std::getline(fil, line);
        auto str_list = Util::Extract_Tokens(line,
                        std::string("\\\"[^\"]\\\""));
        if ( str_list.size != 2 )
          throw ("Invalid format line " + std::to_string(line_number));
        //LuaScript::object_list.push_back(str_list);
        ++ line_number;
      }
    } catch ( std::string err ) {
      AOD::Output("Could not load object list @" + scriptname);
      AOD::Output(" -- " + err);
    }
  }
}

static void Load_Event_List(std::string scriptname) {
  // default event list
  LuaScript::event_list = {
    "Fire"
  };
}

void LuaScript::Initialize(std::string scriptname) {
  state = luaL_newstate();
  luaL_openlibs(state);
  


  int error;
  error = luaL_loadfile(state, "scripts//entry.lua");
  if ( error != 0 ) {
    AOD::Output("Error opening entry.lua");
    return;
  }
  lua_pcall(state, 0, LUA_MULTRET, 0);
}*/