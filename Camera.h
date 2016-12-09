/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef CAMERA_H_
#define CAMERA_H_
#pragma once
#include "Vector.h"

namespace AOD_Engine {
  namespace Camera {
    extern AOD::Vector position;
    extern AOD::Vector size;
  }
}

namespace AOD {
  namespace Camera {
    void Set_Position(const Vector& pos);
    void Set_Position(float, float);
    void Set_Size    (const Vector& siz);
    void Set_Size    (float, float);

    AOD::Vector R_Position();
    AOD::Vector R_Size();
  }
}
#endif