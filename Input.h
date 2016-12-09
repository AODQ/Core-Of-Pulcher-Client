/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 

#ifndef INPUT_H_
#define INPUT_H_
#include "SDL.h"


namespace AOD_Engine {
  namespace Input {
    extern unsigned char mouse;
    extern int mouse_x, mouse_y;
    // doesn't do SDL_PumpEvents
    void Refresh_Input();
  }
}

namespace AOD {
  namespace Input {
    bool R_LMB();
    bool R_RMB();
    bool R_MMB();
    //bool R_MWU();
    //bool R_MWD();
    bool R_MX1();
    bool R_MX2();

    float R_Mouse_X(bool cam_offset = 0);
    float R_Mouse_Y(bool cam_offset = 0);

    extern Uint8* keys;
  }
}
#endif