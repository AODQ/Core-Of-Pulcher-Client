/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
#ifndef PlayerSheet_H_
#define PlayerSheet_H_
#pragma once

#include "Joint_Animation.h"
#include <map>
#include <string>

/** Contains information on player sheet, mostly on types of sprites and
  * pointers to their animations. This should be flexible to be adaptable
  * to all player models, but it was built on Nygelstromn's base.
  * Main access is through enums to return player joint pointers,
  * "b" and "f" in an enum denotes "back" and "front" which is mainly
  * for hands. Though some properties might be static, for sake of
  * generalization they are all held as animations.
  * Refer to Player_Joints.cpp for more information.
  * Also refer to nygelmstromn sheet.
 **/
class PlayerSheet {
  std::map<std::string, PlayerJointAnimation*> str_to_pja;
public:
  PlayerSheet();
  SheetContainer img;
  enum class Leg_Anim {
    stand_idle, landing, punch_front, punch_back, to_crouch, crouch_idle, turn,
    walk_turn, crouch_turn, run_forward, run_turn, run_back, walk,
    crouch_walk, air_to_crouch, air_crouch_hold, air_to_crouch_rev,
    jump_hi, jump_hi_back, jump_strafe1, jump_strafe2,
    slide1, slide2, air_idle, swim, dash_vertical, dash_horiz1,
    dash_horiz2, walljump1, walljump2,
    size
  };

  enum class Death_Anim {
    death_b, death_f,
    size
  };

  // note: MUST follow B/F format, animation system assumes that if
  // (int)Hand_Anim is even, then it's back, odd front
  enum class Hand_Anim {
    alarm_right_b, alarm_right_f, alarm_left_b, alarm_left_f,
    twowep_left_b, twowep_left_f, twowep_right_b, twowep_right_f,
    onewep_b, onewep_f,
    guard_left_b, guard_left_f, guard_right_b, guard_right_f,
    slide_1_b, slide_1_f, slide_2_b, slide_2_f,
    run_b, run_f, walk_b, walk_f,
    punch1_b, punch1_f, punch2_b, punch2_f, punch3_b, punch3_f,
    jump1_b, jump1_f, jump2_b, jump2_f,
    strafe1_b, strafe1_f, strafe2_b, strafe2_f,
    backflip_b, backflip_f, frontflip_b, frontflip_f,
    one_hand_spare_b, one_hand_spare_f,
    doppler_bad_support_b, doppler_bad_support_f,
    doppler_bad_b, doppler_bad_f1, nil1, doppler_bad_f2,
    pericaliya_b, pericaliya_f,
    size
  };

  enum class Head_Anim {
    up, center, down,
    size
  };
  /*
  
061 = static,    body_4px_left,       ( 6,  6), 
062 = static,    body_2px_left,       ( 7,  6), 
063 = static,    body_center,         ( 8,  6), 
064 = static,    body_2px_right,      ( 9,  6), 
065 = static,    body_4px_right,      ( 0,  6), 
066 = static,    body_walk,           ( 2,  6), 
*/
  enum class Body_Anim {
    body_left2, body_left, body_center, body_right, body_right2, body_walk,
    size
  };

private:
  PlayerJointAnimation leg_anims  [(int)Leg_Anim::size  ];
  PlayerJointAnimation death_anims[(int)Death_Anim::size];
  PlayerJointAnimation hand_anims [(int)Hand_Anim::size ];
  PlayerJointAnimation head_anims [(int)Head_Anim::size ];
  PlayerJointAnimation body_anims [(int)Body_Anim::size ];
public:
  // returns player joint animation from a Leg_Anim
  PlayerJointAnimation& R_Anim(Leg_Anim type);
  // returns a pair, first key is a bool that corresponds to where the hand
  // animation is for uphand or downhand, true = uphand. Second key is the PJA
  std::pair<bool, PlayerJointAnimation&> R_Anim(Hand_Anim type);
  // returns player joint animation from a Head_Anim
  PlayerJointAnimation& R_Anim(Head_Anim type);
  // returns player joint animation from a Death_Anim
  PlayerJointAnimation& R_Anim(Death_Anim type);
  // returns player joint animation from a Body_Anim
  PlayerJointAnimation& R_Anim(Body_Anim type);

  // returns an enum Leg_Anim based on string, size if no match
  static Leg_Anim Str_To_Leg_Anim(const std::string&);

  // returns an enum Hand_Anim based on string, size if no match
  static Hand_Anim Str_To_Hand_Anim(const std::string&);

  // returns player joint animation based off the str_to_pja map
  // (check PlayerSheet constructor). Intended to be used in case
  // player joint animation is loaded from a file. If there is nothing,
  // throws exception
  PlayerJointAnimation& Str_To_PlayerJointAnimation(std::string);
};

#endif