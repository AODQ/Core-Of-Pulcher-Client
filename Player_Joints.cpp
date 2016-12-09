/* Player animation works like this:
 *   Head gets really only three sprites that are rotations of a single obj,
 * there is no rotation done on our end for the head.
 *   Body stays relatively the same the entire way through but we might need
 * to make adjustments to its position/rotation for things like sliding and
 * crouching.
 *   Legs have various animation types that all depend on the player's current
 * actions, though they are relatively simple as the sprites contain both
 * sets of legs rather splitting it up leg-by-leg.
 *   Hands have relatively same deal as legs, except there are two hands we
 * have to deal with on an individual basis. Front (f) and back (b) which can
 * change based off the direction the player is looking in. You could either
 * set two hands to be left/right or the two hands to be front/back, I go
 * with the latter approach (it doesn't really matter either way afaik).
 * Though I believe all hands have a front/back, sometimes they will be
 * overriden by other hands (say if you are holding a weapon and sliding,
 * your slide front/back will be replaced depending on direction)
 *   There is also death animations but they replace the entire joint animation
 * so they are not complex whatsoever.
 */

#include "Joint_Animation.h"
#include "Animation.h"
#include "Player.h"
#include "Player_Joints.h"
#include "Data.h"

using PSheet = PlayerSheet;
namespace ImgPl = Images_Player;

// ---- constructors ----------------------------------------------------------
namespace PSH = Pl_Sprite_Handler;
static const AOD::Vector Joint_hand_off  = { 37-32, 22-32 },
                         Joint_head_off  = { 6, 7 },
                         Joint_lower_off = { -10, -13 };
PSH::Joint::Joint(Player* p, Type t) : player(p), type(t) {
  obj = new AOD::Object();
  obj->Set_Image_Size({64, 64});
  switch ( type ) {
    case Type::dnhand: case Type::uphand:
      obj->Set_Origin({32-36, 32-22});
    break;
    
  }
}
PSH::Joint::~Joint() {
  AOD::Remove(obj);
}
void PSH::Joint::Reset() {
  anim.Reset();
}
PSH::Joint_Head::Joint_Head  (Player* p) : Joint(p, Type::head ) {
  AOD::Add(obj, 11);
}
PSH::Joint_Hand::Joint_Hand(Player* p, Type t) : Joint(p, t) {
  if ( t == Type::dnhand )
    AOD::Add(obj,  9);
  else
    AOD::Add(obj, 12);
  obj->Set_Origin(Joint_hand_off);
}
PSH::Joint_Lower::Joint_Lower(Player* p) : Joint(p, Type::lleg ) {
  AOD::Add(obj, 9);
  obj->Set_Origin(Joint_lower_off);
  turn_timer = 0.0f;
}
// ---- joint base ------------------------------------------------------------
void PSH::Joint::Update_Base(AOD::Vector pos, float angle) {
  obj->Set_Position(pos);
  obj->Set_Rotation(angle);
  obj->Set_Sprite(anim.Update());
}
// ---- body ------------------------------------------------------------------
using PLBody  = PSH::Body;
PLBody::Body(Player* p) : player(p), uphand(p, Joint::Type::uphand),
                          dnhand(p, Joint::Type::dnhand), lower(p), head(p) {
  obj = new AOD::Object();
  AOD::Add(obj, 10);
  obj->Set_Image_Size({64, 64});
  obj->Set_Sprite(Images_Player::nygel.R_Anim(
                  PSheet::Body_Anim::body_center).R_Frame());
  obj->Set_Origin({-2, -4});
  prev_angle = 0;
  prev_dir   = 0;
}

PLBody::~Body() {
  AOD::Remove(obj);
}

void PLBody::Set_Anim(Animation* _anim, PSheet::Body_Anim _type) {
  obj->Set_Sprite(_anim->R_Frame());
  type = _type;
}

void PLBody::Update(AOD::Vector pos, float angle) {
  AOD::Vector lower_off{ 9, 17}, head_off{-3, 7},
              hand_off { -4,  11};
  // -- grab misc values (angle/dir) --
  bool dir = std::cos(angle) > 0.0f;
  float langle = player->R_Aiming_Angle();
  if ( lower.R_Anim_Type() == PlayerSheet::Leg_Anim::slide1 ||
       lower.R_Anim_Type() == PlayerSheet::Leg_Anim::slide2 )
    langle = (player->R_Dir() == Player::Direction::Right? 90 : -90);
  // -- update positions/sprites --
  AOD::Vector mpos = pos + lower_off;
  lower.Update (mpos, langle, dir);
  obj->Set_Position(mpos - AOD::Vector{14, 25});
  angle = std::_Pi - angle;
  { // set body rotation
    float t_angle = angle;
    /*if ( !dir ) {
      if ( angle >= 0 )
        angle = std::fmin(std::_Pi/32.f, angle);
      else
        angle = std::fmax(-std::_Pi/32.f, angle);
    } else {
      if ( angle >= 0 )
        angle = std::fmax(std::_Pi - std::_Pi/32.f, angle);
      else
        angle = std::fmin(-std::_Pi + std::_Pi/32.f, angle);
      angle -= std::_Pi;
    }
    obj->Set_Rotation(angle);*/
    angle = t_angle;
  }
  head.Update  (obj->R_Position() + head_off,  angle, dir);
  dnhand.Update(obj->R_Position() + hand_off,  angle, dir);
  uphand.Update(obj->R_Position() + hand_off,  angle, dir);

  // flip body?
  if ( dir ^ obj->R_Flipped_X() ) {
    obj->Flip_X();
  }
  if ( dir ) obj->Add_Position({12, 0});
  prev_angle = angle;
  prev_dir = dir;
}


// ---- head ------------------------------------------------------------------
void PSH::Joint_Head::Update(AOD::Vector& mpos, float angle, bool dir) {
  Update_Base(mpos + Joint_head_off + AOD::Vector{-2, -18}, 0.0f);
  float s = std::sin(angle);
  obj->Set_Sprite(
    (s < -.666 ? ImgPl::nygel.R_Anim(PSheet::Head_Anim::up).R_Frame() :
     s > 0.666 ? ImgPl::nygel.R_Anim(PSheet::Head_Anim::down).R_Frame() :
                 ImgPl::nygel.R_Anim(PSheet::Head_Anim::center).R_Frame()));
  if ( dir ) {
    obj->Flip_X();
    obj->Add_Position({10, 0});
  }
}

void PSH::Joint_Head::Set_Anim(Animation* _anim, PSheet::Head_Anim _type) {
  if ( anim.animation != _anim ) {
    anim.animation = _anim;
    type = _type;
  }
}

// ---- hands -----------------------------------------------------------------
using PAH = PSheet::Hand_Anim;

void PSH::Joint_Hand::Update(AOD::Vector& mpos, float angle, bool dir) {
  Update_Base(mpos + Joint_hand_off, angle);
  if ( dir ^ obj->R_Flipped_X() )  {
    obj->Flip_X();
    obj->Add_Position({12, 0});
    angle -= std::_Pi;
  }
  obj->Set_Rotation(angle);
}

void PSH::Joint_Hand::Set_Anim(Animation* _anim, PSheet::Hand_Anim _type) {
  if ( anim.animation != _anim ) {
    anim.animation = _anim;
    type = _type;
  }
}

// ---- lower -----------------------------------------------------------------
using PAL = PSheet::Leg_Anim;

AOD::Text* d_text = nullptr;

void PSH::Joint_Lower::Update(AOD::Vector& mpos, float angle, bool dir) {
  if ( d_text == nullptr ) {
    d_text = new AOD::Text(200, 200, "");
    AOD::Add(d_text);
  }
  switch ( R_Anim_Type() ) {
    case PSheet::Leg_Anim::stand_idle:
      switch ( (int)anim.timer ) {
        case 0:
          mpos.y += 0;
        break;
        case 1:
        break;
        case 2:
          mpos.y += 1;
        break;
        case 3:
          mpos.y += 2;
        break;
        case 4:
          mpos.y += 3;
        break;
      }
    break;
  }
  Update_Base(mpos + Joint_lower_off, angle);
  d_text->Set_String("Leg: " + std::to_string(anim.timer));
}

void PSH::Joint_Lower::Set_Anim(Animation* _anim, PSheet::Leg_Anim _type) {
  if ( anim.animation != _anim ) {
    anim.animation = _anim;
    type = _type;
  }
}

PlayerSheet::Leg_Anim PSH::Joint_Lower::R_Anim_Type() {
  return type;
}
