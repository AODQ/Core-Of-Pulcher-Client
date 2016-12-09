
/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef DATA_H_
#define DATA_H_
#pragma once

#include "AOD.h"
#include "HUD.h"
#include "Images.h"
#include "PlayerSheet.h"
#include "Weapons.h"

#include <string>
#include <vector>

// image/sound data that should be loaded at either
// server connect or start of game

// ---- images ----------------------------------------------------------------
class SheetContainer;

namespace Images_Misc {
  extern SheetContainer pulcher_logo,
                         text_cursor,
                         console_background;
  void Load();
  void Unload();
}

namespace Images_Player {
  // nygelstromn player sheet
  extern PlayerSheet nygel;
  extern SheetContainer debug;

  void Refresh_Player_Image_Data();

  void Load();
  void Unload();
}

namespace Images_HUD {
  // ---- fonts
  // HUD { Font_Size, Font_Type, Font_Char }
  extern SheetRect hchar[][8][14];

  // ---- weapons
  // Weapons_Status, Weapon (weapons.h)
  extern SheetRect wep[][10];

  // ---- notifications
  // HUD::Notif_Type
  extern SheetRect notif[];

  // ---- icons
  //extern SheetContainer icon_small, icon_small2, icon_large;
  // HUD { Icon_Size, Icon_Type }
  extern SheetRect icon[][20];

  // ---- layout
  // HUD { Panel_Img_Type, Panel_Type }
  extern SheetContainer panel[][3];
  // ---- misc
  //extern SheetContainer accel_image;
  // HUD::Accel_Type
  extern SheetRect accel[];

  void Load();
  void Unload();
}


// ---- sounds ----------------------------------------------------------------

namespace Sounds {
  extern Mix_Music* loading_screen_music[];
  extern const int loading_screen_music_size;

  void Load();
  void Unload();
}
// ---- general ---------------------------------------------------------------

void Load_All_Resources();
void Unload_All_Resources();

extern std::vector<std::vector<std::string>> loading_hints;

void Load_Hints();


#endif