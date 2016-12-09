/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
#include "Joint_Animation.h"
#include "Images.h"
#include "Player.h"
/*
// ---- player joint/animations -----------------------------------------------
// ---- constructors ----------------------------------------------------------
namespace PSH = Pl_Sprite_Handler;
using PLBody  = PSH::Body;
PSH::Joint::Joint(Player* p, Type t) : player(p), type(t) {
  obj = new AOD::Object();
  obj->Set_Image_Size({64, 64});
}
PSH::Joint::~Joint() {
  AOD::Remove(obj);
}
void PSH::Joint::Reset() {
  anim_handler.Reset();
}
static float joint_hand_offx = 37-32,
             joint_hand_offy = 22-32;
PSH::Joint_Head::Joint_Head  (Player* p) : Joint(p, Type::head ) {
  AOD::Add(obj, 11);
}
PSH::Joint_Hand::Joint_Hand(Player* p) : Joint(p, Type::uphand) {
  AOD::Add(obj, 12);
  obj->Set_Origin({joint_hand_offx, joint_hand_offy});
}
PSH::Joint_DnHand::Joint_DnHand(Player* p) : Joint(p, Type::dnhand) {
  AOD::Add(obj, 9);
  obj->Set_Origin({joint_hand_offx, joint_hand_offy});
}
PSH::Joint_Lower::Joint_Lower(Player* p) : Joint(p, Type::lleg ) {
  AOD::Add(obj, 9);
  obj->Set_Origin({25-32, 19-32});
  turn_timer = 0.0f;
}

// ---- body ------------------------------------------------------------------
PLBody::Body(Player* p) : player(p), uphand(p), dnhand(p), lower(p), head(p) {
  obj = new AOD::Object();
  obj->Set_Image_Size({64, 64});
  AOD::Add(obj,10);
  prev_angle = 0;
  prev_dir = 0;
}
PLBody::~Body() {
  AOD::Remove(obj);
}
void PLBody::Update(AOD::Vector pos, float angle) {
  static float head_x  = -3, head_y  =  7,
               hand_x  =  1, hand_y  =  1,
               lower_x =  4, lower_y = 10;
  float head_off_x, head_off_y, hand_off_x, hand_off_y;
  head_off_x = head_off_y = hand_off_x = hand_off_y = 0;

  obj->Set_Position(pos + AOD::Vector({6, 1}));
  bool dir = std::cos(angle) < 0;
  switch ( player->R_Anim_Legs() ) {
    case PSheet::Anim_Legs::walk_turn:
      dir = player->R_Velocity().x > 0;
    break;
    case PSheet::Anim_Legs::walk:
      dir = player->R_Velocity().x > 0;
    default:
      if ( dir != prev_dir ) {
         lower.Turn();
      }
    break;
  }
  // --- do leg and grab all_off (effects all other joints & body)
  float langle = 0;
  if ( player->R_Anim_Legs() == PSheet::Anim_Legs::slide1 ||
       player->R_Anim_Legs() == PSheet::Anim_Legs::slide2 )
    langle = player->R_Dir() == Player::Direction::Right ? 90 : -90;
  AOD::Vector all_off = lower._Update(obj->R_Position() +
        AOD::Vector{lower_x, lower_y} +
        AOD::Vector{(dir?0:lower.R_Flip_Off_X()), 0}, langle, dir);

  // --- set body ( no flip yet )
  obj->Add_Position( all_off );
  obj->Set_Sprite(*Images_Player::nygel.body_center->R_Frame_SR(0));
  if ( player->R_Anim_Legs() == PSheet::Anim_Legs::walk ||
       player->R_Anim_Legs() == PSheet::Anim_Legs::walk_turn ) {
    head_off_x = dir ? -3 : 3;
    head_off_y = -1;
    obj->Set_Sprite(*Images_Player::nygel.body_walk->R_Frame_SR(0));
  }
  // --- set head
  head.Update(obj->R_Position() +
        AOD::Vector{head_x + head_off_x, head_y + head_off_y}, angle, dir);
  lower.Update(obj->R_Position() +
        AOD::Vector{lower_x, lower_y}, langle, dir);
  // --- set hands
  if ( player->R_Sel_Weapon() == Weapon::Fists ) {
    using ALegs = PSheet::Anim_Legs;
    switch ( player->R_Anim_Legs() ) {
      case ALegs::jump_strafe1: case ALegs::jump_strafe2:
      case ALegs::run_back: case ALegs::run_forward:
      case ALegs::run_turn: case ALegs::punch_back: case ALegs::punch_front:
        // --- todo ---
      break;
    }
  }
  uphand.Update(obj->R_Position() +
         AOD::Vector{hand_x + hand_off_x, hand_y + hand_off_y}, angle, dir);
  dnhand.Update(obj->R_Position() +
         AOD::Vector{hand_x - hand_off_x, hand_y + hand_off_y}, angle, dir);

  // body flip
  if ( dir ) {
    obj->Flip_X();
    obj->Add_Position({12, 0});
  }
  prev_angle = angle;
  prev_dir   = dir;
}
void PSH::Body::Reset_Lower() {
  lower.Reset();
}

void PSH::Body::Reset_Hands() {
  uphand.Reset();
  dnhand.Reset();
}

// ---- head ------------------------------------------------------------------
void PSH::Joint_Head::Update(AOD::Vector mpos, float angle, bool dir) {
  static float joint_head_offx = 37-32,
               joint_head_offy = 22-32;
  obj->Set_Position(mpos + AOD::Vector(joint_head_offx,
                                          joint_head_offy));
  float s = std::sin(angle);
  if ( s < -.666 )
    obj->Set_Sprite( *Images_Player::nygel.head_up->R_Frame_SR(0) );
  else if ( s > .666 )
    obj->Set_Sprite( *Images_Player::nygel.head_down->R_Frame_SR(0) );
  else
    obj->Set_Sprite( *Images_Player::nygel.head_mid->R_Frame_SR(0) );
  if ( dir ) {
    obj->Flip_X();
    obj->Add_Position({8, 0});
  }
};

// ---- hands -----------------------------------------------------------------
using PAH   = PSheet::Anim_Hands;

void Set_Hand_Anim(PAH anim_hand, PSheet& sheet, bool& dir, float& angle,
        Anim_Handler& anim_handler, bool& flipx, bool& flipy, bool arm) {
  Animation* anim;
  
  static auto static_set = [&](PAH left, PAH right = PAH::size)->void {
    anim_handler.Set(sheet.hand_anims[(int)(
        (right == PAH::size) ? left : (dir ? left : right))].get()
    );
  };

  static std::map<PAH, PAH[2]> anim_set_type = {
    { PAH::hold_wep_two, {PAH::twowep_right_f, PAH::twowep_left_f} },
    { PAH::hold_wep_one, {PAH::onewep_f,       PAH::onewep_f     } },
    { PAH::guard,   {PAH::guard_right_f,       PAH::guard_left_f } },
    { PAH::slide_1, {PAH::slide_1_f,           PAH::alarm_left_f } },
    { PAH::slide_2, {PAH::slide_2_f,           PAH::alarm_left_f } },
    { PAH::alarmed, {PAH::alarm_right_f,       PAH::alarm_left_f } },
    { PAH::jump,    {PAH::run_f,               PAH::run_f        } },
    { PAH::run,     
  };

  switch ( anim_hand ) {
    case PAH::alarmed:
      angle = 0;
      angle = 0;
    break;
    case PAH::run:
      anim_handler.Set(sheet.hands_run_f.get());
      anim_handler.Update();
      angle = 0;
      flipy = 0;
      flipx = 1;
      /*if ( dir )
        mpos.x += 9;
    break;
    case PAH::walk:
      anim_handler.Set(sheet.hands_walk_f.get());
      anim_handler.Update();
      angle = 0;
      flipy = 0;
      flipx = 1;
    break;
    case PAH::punch1: case PAH::punch2: case PAH::punch3: {
      /*if ( dir ) {
        int ind = (int)player->R_Anim_Hands() - (int)PAH::punch1;
        anim = (dir ? sheet.hands_punch_f[ind] :
                      sheet.hands_static[ (int)PAH::alarm_right_f ]);
        anim_handler.Set(anim);
        anim_handler.Update();
      } 
    } break;
    case PAH::jump: {
      Animation* anim = (dir ? sheet.hands_jump_right_f :
                                sheet.hands_jump_left_f );
      anim_handler.Set(anim);
      anim_handler.Update();
      angle = 0;
      flipx = 1;
      flipy = 0;
    } break;
    case PAH::strafe1:
      anim_handler.Set(sheet.hands_strafe1_f);
      anim_handler.Update();
      flipy = 0;
      flipx = 1;
      angle = 0;
    break;
    case PAH::strafe2:
      anim_handler.Set(sheet.hands_strafe2_f);
      anim_handler.Update();
      flipy = 0;
      flipx = 1;
      angle = 0;
    break;
    case PAH::backflip:
      anim_handler.Set(sheet.hands_backflip_f);
      anim_handler.Update();
      angle = 0;
    break;
    case PAH::frontflip:
      anim = sheet.hands_frontflip_f;
      anim_handler.Set(anim);
      anim_handler.Update();
      angle = 0;
    break;
  }
}
void Up_Translate_Rotate(SheetRect* srect, AOD::Object* obj, bool flipx,
         bool flipy, bool dir, AOD::Vector mpos, float joint_offx,
         float joint_offy, float angle) {
  if ( srect )
    obj->Set_Sprite(*srect);
  obj->Set_Position(mpos);
  obj->Set_Origin({joint_hand_offx, joint_hand_offy});
  if ( flipy && dir ) {
    obj->Flip_Y();
    obj->Set_Origin({joint_hand_offx, -joint_hand_offy});
    obj->Add_Position({0,-20});
  }
  if ( flipx && dir ) {
    obj->Flip_X();
    obj->Set_Origin({-joint_hand_offx, joint_hand_offy});
    obj->Add_Position({0,0});
  }
  obj->Set_Rotation(angle);
}
void Up_Add_Offset(AOD::Vector& mpos, bool dir, PAH anim_hand) {
  static const AOD::Vector offset[2][(int)PSheet::Anim_Hands::size] = {
    { // ---- left  ----
//  hold_wep_two, hold_wep_one, alarmed, guard,  slide_1, slide_2,
      {0,0},        {0,0},        {0,0},   {0,0},  {0,0},   {0,0},
//  run,   walk,  punch1, punch2, punch3, jump,  strafe1, strafe2,
      {0,0}, {1,0}, {0,0},  {0,0},  {0,0},  {0,0}, {0,0},   {0,0},
//  backflip, frontflip
      {0,0},    {0,0}
    },
    { // ---- right ----
//  hold_wep_two, hold_wep_one, alarmed, guard,  slide_1, slide_2,
      {0,0},        {0,0},        {0,0},   {0,0},  {0,0},   {0,0},
//  run,   walk,  punch1, punch2, punch3, jump,  strafe1, strafe2,
      {9,0}, {9,1}, {0,0},  {0,0},  {0,0},  {0,0}, {9,0},   {10,0},
//  backflip, frontflip
      {0,0},    {0,0}
    }
  };
  mpos += offset[dir][(int)anim_hand];
}


void PSH::Joint_Hand::Update(AOD::Vector mpos, float angle, bool dir) {
  // initial var dec
  auto& sheet = Images_Player::nygel;
  using PAH = PSheet::Static_Hand;
  using PAH   = PSheet::Anim_Hands;
  bool flipy = 1, flipx = 0;
  // get anim hand and set sheet
  PSheet::Anim_Hands anim_hand = player->R_Anim_Hands();

  Up_Add_Offset(mpos, dir, anim_hand);

  auto* srect = anim_handler.R_SR();

}

void Dn_Set_Anim(PAH anim_hand, PSheet& sheet, bool& dir, float& angle,
                 Anim_Handler& anim_handler, bool& flipx, bool& flipy) {
  Animation* anim;

  static auto static_set = [&](PAH left, PAH right = PAH::size)->void {
    anim_handler.Set(sheet.hand_anims[(int)(
        (right == PAH::size) ? left : (dir ? left : right))].get()
    );
  };

  switch ( anim_hand ) {
    case PAH::hold_wep_two:
      static_set(PAH::twowep_left_b, PAH::twowep_right_b);
    break;
    case PAH::hold_wep_one:
      static_set(PAH::onewep_b);
    break;
    case PAH::alarmed:
      static_set(PAH::alarm_left_b, PAH::alarm_right_b);
    break;
    case PAH::guard:
      static_set(PAH::guard_left_b, PAH::guard_right_b);
    break;
    case PAH::slide_1:
      static_set(PAH::alarm_right_b, PAH::slide_1_b);
    break;
    case PAH::slide_2:
      static_set(PAH::alarm_right_b, PAH::slide_2_b);
    break;
    case PAH::run:
      anim_handler.Set(sheet.hands_run_b);
      anim_handler.Update();
      angle = 0;
      flipy = 0;
      flipx = 1;
    break;
    case PAH::walk:
      static_set(sheet.hands_walk_b);
      anim_handler.Update();
      angle = 0;
      flipy = 0;
      flipx = 1;
    break;
    case PAH::jump: {
      static_set(PAH::hands_jump_left_b, hands_jump_right_b);
      anim_handler.Update();
      angle = 0;
      flipx = 1;
      flipy = 0;
    } break;
    case PAH::strafe1:
      static_set(sheet.hands_strafe1_b);
      anim_handler.Update();
      flipy = 0;
      flipx = 1;
      angle = 0;
    break;
    case PAH::strafe2:
      static_set(sheet.hands_strafe2_b);
      anim_handler.Update();
      flipy = 0;
      flipx = 1;
      angle = 0;
    break;
    case PAH::backflip:
      anim_handler.Set(sheet.hands_backflip_b);
      anim_handler.Update();
      angle = 0;
    break;
    case PAH::frontflip:
      anim_handler.Set(sheet.hands_frontflip_b);
      anim_handler.Update();
      angle = 0;
    break;
  }
}

void Dn_Translate_Rotate(SheetRect* srect, AOD::Object* obj, bool flipx,
        bool flipy, bool dir, AOD::Vector mpos, float joint_offx,
        float joint_offy, float angle) {
  if ( srect )
    obj->Set_Sprite(*srect);
  obj->Set_Position(mpos);
  obj->Set_Origin({joint_offx,  joint_offy});
  if ( flipy && dir ) {
    obj->Flip_Y();
    obj->Set_Origin({joint_offx, -joint_offy});
    obj->Add_Position({0,-20});
  }
  if ( flipx && dir ) {
    obj->Flip_X();
    obj->Set_Origin({-joint_offx, joint_offy});
    obj->Add_Position({35,0});
  }
  obj->Set_Rotation(angle);
}

void Dn_Add_Offset(AOD::Vector& mpos, bool dir, PAH anim_hand) {
  static const AOD::Vector offset[2][(int)PSheet::Anim_Hands::size] = {
    { // ---- left ----
  //  hold_wep_two, hold_wep_one, alarmed, guard,  slide_1, slide_2,
      {0,0},        {0,0},        {0,0},   {0,0},  {0,0},   {0,0},
  //  run,   walk,   punch1, punch2, punch3, jump,  strafe1, strafe2,
      {0,0}, {-1,0}, {0,0},  {0,0},  {0,0},  {0,0}, {0,0},   {0,0},
  //  backflip, frontflip
      {0,0},    {0,0}
    },
    { // ---- right ----
  //  hold_wep_two, hold_wep_one, alarmed, guard,  slide_1, slide_2,
      {0,0},        {0,0},        {0,0},   {0,0},  {0,0},   {0,0},
  //  run,     walk,    punch1, punch2, punch3, jump,  strafe1, strafe2,
      {-25,0}, {-25,0}, {0,0},  {0,0},  {0,0},  {0,0}, {-23,2}, {-22,2},
  //  backflip, frontflip
      {0,0},    {0,0}
    }
  };
  mpos += offset[dir][(int)anim_hand];
}

void PSH::Joint_DnHand::Update(AOD::Vector mpos, float angle, bool dir) {
  PSheet::Anim_Hands anim_hand = player->R_Anim_Hands();
  bool flipx, flipy;
  Dn_Set_Anim(anim_hand, Images_Player::nygel, dir, angle, anim_handler,
              flipx, flipy);
  Dn_Add_Offset(mpos, dir, anim_hand);
  Dn_Translate_Rotate(anim_handler.R_SR(), obj, flipx, flipy, dir, mpos,
                        joint_hand_offx, joint_hand_offy, angle);
}
// ---- legs ------------------------------------------------------------------
void PSH::Joint_Lower::Turn() {
  switch ( player->R_Anim_Legs() ) {
    case PSheet::Anim_Legs::stand_idle:
      turn_timer = 75.f;
    break;
    case PSheet::Anim_Legs::walk:
      player->Set_Anim_Legs(PSheet::Anim_Legs::walk_turn);
    break;
    case PSheet::Anim_Legs::run_back:
    case PSheet::Anim_Legs::run_forward:
      player->Set_Anim_Legs(PSheet::Anim_Legs::run_turn);
    break;
    case PSheet::Anim_Legs::crouch_walk: case PSheet::Anim_Legs::crouch_idle:
      player->Set_Anim_Legs(PSheet::Anim_Legs::crouch_turn);
    break;
  }
}
int PSH::Joint_Lower::R_Flip_Off_X() {
  if ( turn_timer > 0 ) return 0;
  return 0;
}

using PAL = PSheet::Anim_Legs;

void Lower_Update_Anim(Player* player, Anim_Handler& anim_handler, bool dir,
                       PSheet& sheet) {
  if ( anim_handler.done ) {
    switch ( player->R_Anim_Legs() ) {
      case PAL::walk_turn:
        player->Set_Anim_Legs(PSheet::Anim_Legs::walk);
      break;
      case PAL::run_turn:
        if ( dir )
          player->Set_Anim_Legs(PSheet::Anim_Legs::run_forward);
        else
          player->Set_Anim_Legs(PSheet::Anim_Legs::run_back);
      break;
      case PAL::crouch_turn:
        player->Set_Anim_Legs(PSheet::Anim_Legs::crouch_idle);
      break;
      case PAL::landing:
        player->Set_Anim_Legs(PSheet::Anim_Legs::stand_idle);
      break;
      case PAL::to_crouch:
        if ( player->R_Key_Left() || player->R_Key_Right() )
          player->Set_Anim_Legs(PSheet::Anim_Legs::crouch_walk);
        else
          player->Set_Anim_Legs(PSheet::Anim_Legs::crouch_idle);
      break;
    }
  }
  anim_handler.Set(sheet.leg_anims[player->R_Anim_Legs()]);
  anim_handler.Update();
}

void Lower_Set_Angle(PAL lower, float& angle) {
  switch ( lower ) {
    case PAL::slide1: case PAL::slide2: break;
    default:
      angle = 0;
  }
}

void Lower_Add_Preoffset(PAL lower, AOD::Vector& offset,
                         Anim_Handler& anim_handler) {
  switch ( lower ) {
    case PAL::stand_idle:
      offset = {0,
        anim_handler.type.animation->R_Frame(anim_handler.timer) - 4};
    break;
    case PAL::landing:
      offset += {0,
        anim_handler.type.animation->R_Frame(anim_handler.timer)*2 - 3};
    break;
    case PAL::to_crouch:
      offset += {0,
        anim_handler.type.animation->R_Frame(anim_handler.timer)*4 - 9};
    break;
    case PAL::run_back: case PAL::run_forward:
    case PAL::run_turn: case PAL::walk: case PAL::walk_turn:
      offset += {0, -3};
    break;
    case PAL::crouch_idle: case PAL::crouch_turn:
    case PAL::crouch_walk:
      offset += {0,  3};
    break;
  }
}

void Lower_Translate_Rotate(SheetRect* srect, AOD::Object* obj, bool dir,
                    float angle, AOD::Vector mpos, AOD::Vector offset) {
  if ( srect )// --- post-add offset handler
    obj->Set_Sprite(*srect);
  if ( dir != obj->R_Flipped_X() )
    obj->Flip_X();
  obj->Set_Rotation(angle);
  obj->Set_Position(mpos + offset);
  if ( dir ) {
    obj->Add_Position({4, 0});
  }
}

void Lower_Add_Postoffset(PAL lower, AOD::Vector& offset, bool dir) {
  switch ( lower ) {
    case PAL::crouch_idle: case PAL::crouch_turn:
    case PAL::crouch_walk:
      offset += {0, -2};
    break;
    default:
      offset += {0, -4};
    break;
  }
}

bool Lower_Update_Turn(float& turn_timer, AOD::Object* obj, PSheet& sheet,
                       AOD::Vector mpos) {
  if ( turn_timer > 0.0f ) {
    turn_timer -= AOD::R_MS();
    obj->Set_Sprite(sheet.legs_turn->R_Frame(turn_timer));
    obj->Set_Position(mpos);
    return true;
  }
  return false;
}

AOD::Vector PSH::Joint_Lower::_Update(AOD::Vector mpos, float angle, bool dir) {
  auto& sheet = Images_Player::nygel;
  AOD::Vector offset = {0, 0};
  PAL anim = player->R_Anim_Legs();
  if ( Lower_Update_Turn(turn_timer, obj, sheet, mpos) )
    return offset;
  Lower_Update_Anim(player, anim_handler, dir, sheet);
  Lower_Set_Angle(anim, angle);
  Lower_Add_Preoffset(anim, offset, anim_handler);
  Lower_Translate_Rotate(anim_handler.R_SR(), obj, dir,
                                      angle, mpos, offset);
  Lower_Add_Postoffset(anim, offset, dir);
  return offset;
}
*/


/* -- update sprite --

  if ( !in_air &&  anim_legs != ALeg::landing ) {
    if ( crouching ) {
      // sliding occurs when on the ground, not already sliding and
      // your speed is over crouch_walk
      if ( !in_air && Player_Consts::max_crouch_speed*1.2 ) {
        // Set in slide state if not already
        if ( anim_legs != ALeg::slide1 && anim_legs != ALeg::slide2 ) {
          anim_legs = Util::R_Rand(0, 100)>50 ?
                        ALeg::slide1 : ALeg::slide2;
        }
        return;
      }

      if ( anim_legs != ALeg::crouch_idle && anim_legs != ALeg::crouch_turn &&
           anim_legs != ALeg::crouch_walk && anim_legs != ALeg::slide1      &&
           anim_legs != ALeg::slide2 )
      {
        anim_legs = ALeg::to_crouch;
      }
      if ( anim_legs != ALeg::crouch_turn ) {
        if ( abs(velocity.x) < 0.2 ) {
          anim_legs = ALeg::crouch_idle;
        } else {
          anim_legs = ALeg::crouch_walk;
        }
      }
    } else { // not crouching
      if ( abs(velocity.x) < 0.2 ) {
        anim_legs = ALeg::stand_idle;
        anim_hands = PSheet::Anim_Hands::guard;
      } else if ( abs(velocity.x) < 2.0 ) {
        if ( anim_legs != ALeg::walk_turn )
          anim_legs = ALeg::walk;
        anim_hands = PSheet::Anim_Hands::walk;
      } else {
        anim_hands = PSheet::Anim_Hands::run;
        if ( anim_legs != ALeg::run_turn )
          if ( key_left ) {
            if ( direction == Direction::Right )
              anim_legs = ALeg::run_back;
            else
              anim_legs = ALeg::run_forward;
          } else if ( key_right ) {
            if ( direction == Direction::Left )
              anim_legs = ALeg::run_back;
            else
              anim_legs = ALeg::run_forward;
          }
      }
    }
  } else { // in air
    
  }
  */