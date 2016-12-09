#ifndef JOINT_ANIMATION_H_
#define JOINT_ANIMATION_H_
#pragma once

#include "Vector.h"
#include "Animation.h"
#include <vector>

class SheetRect;
class SheetContainer;
// Inheritance of Animation used to make creating player joint animations
// easier.
class PlayerJointAnimation : public Animation {
  static Animation::SRect_Vec Create_Anim(int x, int endx, int y,
                                  SheetContainer& img, std::string anim_name);
  AOD::Vector offset;
public:
  PlayerJointAnimation();
  /* Slices frames from {X, Y} to {EX, Y}, tile based
   * _offsets: relative to the center of the frame (32x32)
   * speed:    seconds per frame
   */ 
  PlayerJointAnimation(int x, int ex, int y, SheetContainer& img,
      Animation::Type type, AOD::Vector offset, float speed);
  // Reconstructs the joint animation, same parameters as the constructor
  void Reconstruct(int x, int ex, int y, SheetContainer& img,
      Animation::Type type, AOD::Vector offset, float speed,
      std::string animation_name /* debug/error purposes */);
};

#endif