/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

 /* TODO (memory list thing):
  *  sliding into in_air means switch to strafe jump and rotate back to 0
  */
#include "Data.h"
#include "Player.h"

void Player::Gfx_To_Ground_Movement() {
  using Leg_Anim = PlayerSheet::Leg_Anim;
  if ( crouching ) {
    Set_Anim(Leg_Anim::to_crouch);
    return;
  }
  if ( velocity.x < AOD::To_MS(2) )
    Set_Anim(Leg_Anim::stand_idle);
  else if ( velocity.x < AOD::To_MS(5) )
    Set_Anim(Leg_Anim::walk);
  else
    Set_Anim(Leg_Anim::run_forward);
}

void Player::Gfx_To_Ground_Crouch_Movement() {
  using Leg_Anim = PlayerSheet::Leg_Anim;
  if ( velocity.x < AOD::To_MS(2) )
    Set_Anim(Leg_Anim::crouch_idle);
  else
    Set_Anim(Leg_Anim::crouch_walk);
  if ( prev_direction != direction )
    Set_Anim(Leg_Anim::crouch_turn);
  if ( !crouching )
    Gfx_To_Ground_Movement();
}

void Player::Gfx_To_Ground_Check() {
  using Leg_Anim = PlayerSheet::Leg_Anim;
  if ( !in_air )
    Set_Anim(Leg_Anim::landing);
}

void Player::Gfx_To_Air_Check() {
  using Leg_Anim = PlayerSheet::Leg_Anim;
  if ( jumping )
    if ( key_left || key_right )
      Set_Anim((int)Util::R_Rand(0, 2) ? Leg_Anim::jump_strafe1 :
                                                Leg_Anim::jump_strafe2);
    else
      Set_Anim(Leg_Anim::jump_hi);
}

void Player::Update_Sprite() {
  sprite_body.Update(position, angle);
  using Leg_Anim = PlayerSheet::Leg_Anim;
  using Hand_Anim = PlayerSheet::Hand_Anim;
  auto& lower = sprite_body.lower;
  auto& lower_anim  = lower.R_Anim();
  auto& psheet = Images_Player::nygel;
  Anim_Handler ah;
  // --- set ground-based movement
  switch ( lower.R_Anim_Type() ) {
    case Leg_Anim::stand_idle:  case Leg_Anim::run_back:
    case Leg_Anim::run_forward: case Leg_Anim::walk:
      Gfx_To_Ground_Movement();
    break;
  }
  // --- set leg animation
  switch ( lower.R_Anim_Type() ) {
    // --- walk run ---
    case Leg_Anim::run_back: case Leg_Anim::run_forward:
      if ( prev_direction != direction )
        Set_Anim(Leg_Anim::walk_turn);
      Gfx_To_Air_Check();
    break;
    case Leg_Anim::walk:
      if ( prev_direction != direction )
        Set_Anim(Leg_Anim::walk_turn);
      Gfx_To_Air_Check();
    break;
    // --- crouch ---
    case Leg_Anim::to_crouch:
      if ( !crouching ) // crouch animation only plays on ground
        Gfx_To_Ground_Movement();
      else if ( lower_anim.done )
        Gfx_To_Ground_Crouch_Movement();
      Gfx_To_Air_Check();
    break;
    case Leg_Anim::crouch_idle:
      Gfx_To_Ground_Crouch_Movement();
      Gfx_To_Air_Check();
    break;
    case Leg_Anim::crouch_walk:
      Gfx_To_Ground_Crouch_Movement();
      Gfx_To_Air_Check();
    break;
    case Leg_Anim::crouch_turn:
      if ( lower_anim.done )
        Gfx_To_Ground_Crouch_Movement();
      Gfx_To_Air_Check();
    break;
    // --- slide/dash ---
    case Leg_Anim::slide1: case Leg_Anim::slide2:
      if ( !sliding )
        if ( in_air )
          Set_Anim(Leg_Anim::air_idle);
        else
          Gfx_To_Ground_Movement();
    break;
    case Leg_Anim::dash_horiz1: case Leg_Anim::dash_horiz2:
    case Leg_Anim::dash_vertical:
      Gfx_To_Ground_Check();
    break;
    // --- jump ---
    case Leg_Anim::jump_hi:
      if ( lower_anim.done )
        if ( crouching )
          Set_Anim(Leg_Anim::air_crouch_hold);
        else
          Set_Anim(Leg_Anim::jump_hi_back);
    break;
    // --- air ---
    case Leg_Anim::air_idle:
      if ( crouching )
        Set_Anim(Leg_Anim::air_to_crouch);
      Gfx_To_Ground_Check();
    break;
    case Leg_Anim::air_to_crouch:
      if ( !crouching ) {
        float timer = lower_anim.timer;
        Set_Anim(Leg_Anim::air_to_crouch_rev);
        lower.Set_Anim_Time(timer, true);
      } else if ( lower_anim.done )
        Set_Anim(Leg_Anim::air_crouch_hold);
      Gfx_To_Ground_Check();
    break;
    case Leg_Anim::air_crouch_hold:
      if ( !crouching )
        Set_Anim(Leg_Anim::air_idle);
      Gfx_To_Ground_Check();
    break;
    case Leg_Anim::air_to_crouch_rev:
      if ( crouching ) {
        float timer = lower_anim.timer;
        Set_Anim(Leg_Anim::air_to_crouch);
        lower.Set_Anim_Time(timer, true);
      } else if ( lower_anim.done ) {
        Set_Anim(Leg_Anim::jump_hi);
        lower.Set_Anim_Time(1.0f);
      }
    break;
    // --- misc ---
    case Leg_Anim::landing:
      if ( lower_anim.done )
        Gfx_To_Ground_Movement();
      Gfx_To_Air_Check();
    break;
  }
  
  // --- overrides ---
  
  prev_angle = angle;
  prev_direction = direction;
}

void Player::Set_Anim(PlayerSheet::Hand_Anim a) {
  auto t = Images_Player::nygel.R_Anim(a);
  t.first ? sprite_body.dnhand.Set_Anim(&t.second, a) :
            sprite_body.uphand.Set_Anim(&t.second, a);
}
void Player::Set_Anim(PlayerSheet::Leg_Anim a) {
  sprite_body.lower.Set_Anim(&Images_Player::nygel.R_Anim(a), a);
}
void Player::Set_Anim(PlayerSheet::Body_Anim a) {
  sprite_body.Set_Anim(&Images_Player::nygel.R_Anim(a), a);
}
void Player::Set_Anim(PlayerSheet::Head_Anim a) {
  sprite_body.head.Set_Anim(&Images_Player::nygel.R_Anim(a), a);
}