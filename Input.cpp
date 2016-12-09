/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "stdafx.h"
#include "Client_Vars.h"
#include "Input.h"

namespace EngInp = AOD_Engine::Input;
namespace AODInp = AOD::Input;

unsigned char EngInp::mouse;
int EngInp::mouse_x,
    EngInp::mouse_y;


void EngInp::Refresh_Input() {
  AODInp::keys = (Uint8*)SDL_GetKeyboardState(nullptr);
  mouse = SDL_GetMouseState(&mouse_x,&mouse_y);
  AODInp::keys[ MOUSEBIND::MOUSELEFT   ] = AODInp::R_LMB();
  AODInp::keys[ MOUSEBIND::MOUSERIGHT  ] = AODInp::R_RMB();
  AODInp::keys[ MOUSEBIND::MOUSEMIDDLE ] = AODInp::R_MMB();
  AODInp::keys[ MOUSEBIND::MOUSEX1     ] = AODInp::R_MX1();
  AODInp::keys[ MOUSEBIND::MOUSEX2     ] = AODInp::R_MX2();
  // Let AOD handle mouse wheel
}

Uint8* AODInp::keys = nullptr;

bool AODInp::R_LMB() { return EngInp::mouse&SDL_BUTTON(SDL_BUTTON_LEFT  ); }
bool AODInp::R_RMB() { return EngInp::mouse&SDL_BUTTON(SDL_BUTTON_RIGHT ); }
bool AODInp::R_MMB() { return EngInp::mouse&SDL_BUTTON(SDL_BUTTON_MIDDLE); }
bool AODInp::R_MX1() { return EngInp::mouse&SDL_BUTTON(SDL_BUTTON_X1    ); }
bool AODInp::R_MX2() { return EngInp::mouse&SDL_BUTTON(SDL_BUTTON_X2    ); }

float AODInp::R_Mouse_X(bool camoffset) {
  return AOD_Engine::Input::mouse_x + (camoffset ?
          Camera::R_Position().x - Camera::R_Size().x/2 : 0);
}
float AODInp::R_Mouse_Y(bool camoffset) {
  return AOD_Engine::Input::mouse_y + (camoffset ?
          Camera::R_Position().y - Camera::R_Size().y/2 : 0);
}