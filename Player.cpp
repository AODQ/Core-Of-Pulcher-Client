/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#include "Console_Handler.h"
#include "Client_Vars.h"
#include "Data.h"
#include "NetObject_Types.h"
#include "Map.h"
#include "Player.h"

#include <map>
#include <sstream>

Player::Player(Player_Info* t):pl_info(t), AOD::PolyObj(), sprite_body(this) {
  wall_jumping = dashing = 0;
  visible = 1;
  armour_type = Item_Type::arm_nil;
  armour = 25;
  health = 25;
  PolyObj::Set_Vertices({{-21.f/2.f, -49.f/2.f}, {-21.f/2.f,  49.f/2.f},
                         { 21.f/2.f,  49.f/2.f}, { 21.f/2.f, -49.f/2.f}});
  Set_Size(21, 49);
  Set_Sprite(Images_Player::debug);
  timer_fall_jump = -1;
  for ( int i = 0; i != (int)Direction::size; ++ i ) {
    air_dash_down[i] = 0;
    air_dash_up[i]   = 0;
    for ( int o = 0; o != (int)Angle::size; ++ o )
      timer_dash[i][o] = 0.0f;
  } 
  timer_keep_wj_vel = -1;
  timer_wj_last_dir = -1.f;
  jumping_free = 0;
  dashing_free = dashing = 0;
  jumping_leg = Direction::Right;
  //crouch_slide_timer = -1.f;
  crouching = 0;
  for ( auto& i : weapon_status )
    i = Weapon_Status::No_Weapon;
  // --- remove ---
  weapon_status[(int)Weapon::Manshredder] = Weapon_Status::Selected;
  // -- set default wep (manshredder)
  weapon_ammo[(int)Weapon::Manshredder]   = 20;
  sel_weapon = Weapon::Manshredder;

  Set_Anim(PlayerSheet::Leg_Anim::stand_idle);
  Set_Anim(PlayerSheet::Hand_Anim::alarm_left_b);
  Set_Anim(PlayerSheet::Hand_Anim::alarm_left_f);
}

void Player::Update() {
  key_crouch = key_left = key_right = key_up = key_down =
  key_jump = key_dash = key_prim = key_sec = 0;
  // key bindings
  if ( !AOD::Console::console_open ) {
    for ( auto& bind : CV::keybinds ) {
      if ( AOD::Input::keys[ bind.first ] ) {
        std::string tstr = bind.second;
        if ( tstr == "jump" )        key_jump   = true;
        else if ( tstr == "crouch" ) key_crouch = true;
        else if ( tstr == "left"   ) key_left   = true;
        else if ( tstr == "right"  ) key_right  = true;
        else if ( tstr == "down"   ) key_down   = true;
        else if ( tstr == "up"     ) key_up     = true;
        else if ( tstr == "jump"   ) key_jump   = true;
        else if ( tstr == "dash"   ) key_dash   = true;
        else if ( tstr == "prim"   ) key_prim   = true;
        else if ( tstr == "sec"    ) key_sec    = true;
        else Handle_Console_Input(tstr);
      }
    }
  }

  AOD::Camera::Set_Position(position);

  angle = std::atan2f(AOD::Input::R_Mouse_Y(1) - (position.y-18),
                      (position.x+10) - AOD::Input::R_Mouse_X(1)) + std::_Pi;
  direction = (std::cos(angle) > 0.0f)? Direction::Left : Direction::Right;
  Update_Velocity();

  Update_Pre_Var_Set();
  Update_Crouch();
  Update_Horiz_Key_Input();
  Update_Dash();
  Update_Jump();
  Update_Wall_Jump();
  Update_Gravity();
  Update_Sprite();

  sprite_body.Update(position, angle);

  // hud updates
  weapon_status[(int)sel_weapon] = Weapon_Status::Selected;
  if ( weapon_ammo[(int)sel_weapon] <= 0 ) {
    weapon_status[(int)sel_weapon] = Weapon_Status::No_Ammo;
  }

  AOD::Input::keys[ MOUSEBIND::MWHEELDOWN ] = 0;
  AOD::Input::keys[ MOUSEBIND::MWHEELUP   ] = 0;
}


bool Player::R_Key_Up               () const { return key_up;       }
bool Player::R_Key_Down             () const { return key_down;     }
bool Player::R_Key_Left             () const { return key_left;     }
bool Player::R_Key_Right            () const { return key_right;    }
bool Player::R_Key_Sec              () const { return key_sec;      }
bool Player::R_Key_Prim             () const { return key_prim;     }
bool Player::R_Key_Crouch           () const { return key_crouch;   }
bool Player::R_Key_Dash             () const { return key_dash;     }
bool Player::R_Key_Jump             () const { return key_jump;     }
bool Player::R_Wjmp                 () const { return wall_jumping; }
Player::Angle Player::R_Dash_Angle  () const { return dash_angle;   }
Player::Direction Player::R_Dash_Dir() const { return dash_dir;     }
Player::Angle Player::R_WJmp_Angle  () const { return wjmp_angle;   }
Player::Direction Player::R_WJmp_Dir() const { return wjmp_dir;     }
int Player::R_Health                () const { return health;       }
int Player::R_Armour                () const { return armour;       }
Item_Type Player::R_Armour_Type     () const { return armour_type;  }
Weapon_Status Player::R_Weapon_Status(Weapon wep) const {
  return weapon_status[(int)wep];
}
int Player::R_Ammo_Curr_Weapon() const { return weapon_ammo[(int)sel_weapon];}
int Player::R_Mega_Stack()       const { return mega_stack; }

void Player::Set_Crouching (bool k) { crouching  = k; }
void Player::Set_Key_Up    (bool k) { key_up     = k; }
void Player::Set_Key_Down  (bool k) { key_down   = k; }
void Player::Set_Key_Left  (bool k) { key_left   = k; }
void Player::Set_Key_Right (bool k) { key_right  = k; }
void Player::Set_Key_Sec   (bool k) { key_sec    = k; }
void Player::Set_Key_Prim  (bool k) { key_prim   = k; }
void Player::Set_Key_Crouch(bool k) { key_crouch = k; }

void Player::Set_Key_Dash(bool key) {
  key_dash = key;
  dash_angle = Angle::Zero;
  if ( key_up )
    dash_angle = Angle::Up;
  else if ( key_down )
    dash_angle = Angle::Down;

  if ( key_left )
    dash_dir = Direction::Left;

  if ( key_right )
    dash_dir = Direction::Right;
}

void Player::Set_Key_Jump(bool key) {
  key_jump = key;
}

// ---- player consts ---------------------------------------------------------
float Player_Consts::max_ground_speed = 8.0f,
      Player_Consts::max_crouch_speed = 2.0f,
      Player_Consts::ground_friction  = .86f,
      Player_Consts::air_friction     = .97f,
      Player_Consts::accel_ground     = 1.5f,
      Player_Consts::accel_air        = 2.5f,
      Player_Consts::max_air_vel_x    = 15.0f,
      Player_Consts::max_air_vel_y    = 13.0f,
      Player_Consts::gravity          = 3.25f,
      Player_Consts::jump_force       = 5.5f,
      Player_Consts::dash_force       = 0.75f,
      Player_Consts::dash_timer       = 2000.f;

const std::vector<float*> Player_Consts::floats_map {
  { &Player_Consts::max_ground_speed },
  { &Player_Consts::max_crouch_speed },
  { &Player_Consts::ground_friction  },
  { &Player_Consts::air_friction     },
  { &Player_Consts::accel_ground     },
  { &Player_Consts::accel_air        },
  { &Player_Consts::max_air_vel_x    },
  { &Player_Consts::max_air_vel_y    },
  { &Player_Consts::gravity          },
  { &Player_Consts::jump_force       },
  { &Player_Consts::dash_force       }
};

// ---- player info -----------------------------------------------------------
const Player_Info* Player::R_Pl_Info() {
  return pl_info;
}

Player_Info::Player_Info(unsigned int _uid, std::string _name,
                PlayerStatus _ps): uid(_uid) {
  name = _name;
  status = _ps;
  team = PlayerTeam::none;
  pl_handle = nullptr;
}

const std::string playerstatus_str[(int)PlayerStatus::size] {
  "dead", "alive", "spectator", "nil"
};

const std::string playerteam_str[(int)PlayerTeam::size] {
  "none", "blue", "red"
};

std::string R_Status_Str(Player_Info* pl) {
  return R_Player_Status_Str(pl) + ' ' + R_Team_Status_Str(pl);
}

std::string R_Player_Status_Str(Player_Info* pl) {
  return playerstatus_str[(int)pl->status];
}

std::string R_Team_Status_Str(Player_Info* pl) {
  return playerteam_str[(int)pl->team];
}
