/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef SERVER_CONSTS_H_
#define SERVER_CONSTS_H_
#pragma once

#include <vector>
#include <string>

#include "AOD.h"
#include "Item.h"

// server defined
#define CLIENT_VERSION 0001
#define CLIENT_MS_DT 15
#define PORT_NUMBER 9111
#define MAX_PACKET_LEN 1024

namespace CV {
  enum class Sound_Type {
    master, effect, hitnotif, announcer, 
    music_master, music_menu, music_dm, music_ctf,
    music_race, size
  };
  // settings
  extern std::string username;
  extern int screen_width;
  extern int screen_height;
  extern std::string map_name;
  extern unsigned int volume[(int)Sound_Type::size];
  using Keybind = std::pair<int, std::string>;
  extern std::vector<Keybind> keybinds;

  void Load_Config();
}

enum MOUSEBIND
     { MOUSELEFT = 300, MOUSERIGHT = 301, MOUSEMIDDLE = 302,
       MWHEELUP  = 303, MWHEELDOWN = 304,
       MOUSEX1   = 305, MOUSEX2    = 306 };

#define KEY_JUMP     1
#define KEY_DASH     2
#define KEY_DOWN     4
#define KEY_LEFT     8
#define KEY_RIGHT   16
#define KEY_CROUCH  32
#define KEY_PRIM    64
#define KEY_SEC    128

#define KEY_MISC_ANGLE_UP    2
#define KEY_MISC_ANGLE_ZERO  4
#define KEY_MISC_ANGLE_DOWN  8
#define KEY_MISC_DIR_LEFT   16
#define KEY_MISC_DIR_RIGHT  32

#endif