/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "stdafx.h"
#include "AOD.h"


AOD::Vector AOD_Engine::Camera::position;
AOD::Vector AOD_Engine::Camera::size;

void AOD::Camera::Set_Position(const Vector& pos) {
  AOD_Engine::Camera::position = pos;
}
void AOD::Camera::Set_Position(float x, float y) {
  AOD_Engine::Camera::position = Vector(x,y);
}
void AOD::Camera::Set_Size(const Vector& siz) {
  if ( siz.x <= 0 || siz.y <= 0 ) {
    AOD_Engine::Debug_Output("Error: Camera size dimensions <= 0");
    return;
  }
  AOD_Engine::Camera::size = siz;
}
void AOD::Camera::Set_Size(float x, float y) {
  if ( x <= 0 || y <= 0 ) {
    AOD_Engine::Debug_Output("Error: Camera size dimensions <= 0");
    return;
  }
  AOD_Engine::Camera::size = Vector(x,y);
}

AOD::Vector AOD::Camera::R_Size()     { return AOD_Engine::Camera::size; }
AOD::Vector AOD::Camera::R_Position() { return AOD_Engine::Camera::position; }