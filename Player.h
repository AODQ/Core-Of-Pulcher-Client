/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef __PLAYER_H_
#define __PLAYER_H_
#pragma once

#include <map>
#include <string>

#include "NetObj.h"
#include "Player_Joints.h"
#include "Item.h"
#include "Weapons.h"

class Player;

enum class PlayerStatus {
  dead, alive, spectator, nil, size
};
enum class PlayerTeam {
  none, blue, red, size
};
class Player_Info {
public:
  enum class Team { none, blue, red };
  std::string name;
  PlayerStatus status;
  PlayerTeam team;
  const uint8_t uid;

  Player_Info(unsigned int uid, std::string name = "Coward",
                            PlayerStatus = PlayerStatus::nil);

  Player* pl_handle;
};
extern const std::string playerstatus_str[],
                         playerteam_str[];

// returns the status in the form of a 
// string (useful for networking)
std::string R_Status_Str(Player_Info*);

// returns status of player (dead, alive, etc) in form of string
std::string R_Player_Status_Str(Player_Info*);

// return status of team in form of string
std::string R_Team_Status_Str(Player_Info*);

namespace Player_Consts {
  extern float max_ground_speed,
               max_crouch_speed,
               ground_friction, // multiplicative loss, no move on ground
               air_friction,    // multiplicative loss, no move in air
               accel_ground,
               accel_air,
               max_air_vel_x,
               max_air_vel_y,
               gravity,
               jump_force,
               dash_force,
               dash_timer;

  extern const std::vector<float*> floats_map;
};

class Tile_Base;

class Player : public AOD::PolyObj {
public:
  enum class Sprite_Status {
    stand, run, crouch, slide, jump, wall_jump
  };
  enum class Angle {
    Up, Zero, Down, size
  };
  enum class Direction {
    Left, Right, size
  };
private:
  const Player_Info* pl_info;
  int health, armour;
  AOD::Vector velocity_per_second;
  Item_Type armour_type;
  float angle, prev_angle;
  Weapon_Status weapon_status[9];
  int weapon_ammo[9];
  Weapon sel_weapon;
  int mega_stack;

  bool key_up, key_down, key_left, key_right, key_prim,
       key_sec, key_crouch, key_dash,
       key_jump;

  Direction direction, prev_direction;
  bool in_air, wall_jumping, in_liquid;
  // ---- dash
  bool dashing,
       dashing_free;
  bool air_dash_up  [(int)Direction::size],
       air_dash_down[(int)Direction::size];
  // ---- wjmp
  AOD::Vector wjmp_old_vel;
  Direction wjmp_last_dir;
  // ---- jump
  bool jumping, // key jump is down AND has let go since last jump
       jumping_free; // key jump has been let go of
  Direction jumping_leg;
  // ---- crouch/sliding
  float   slide_timer;
  Direction _slide_dir;
  bool crouching, sliding;
  // ---- timers
  float timer_fall_jump;
  float timer_dash[(int)Direction::size][(int)Angle::size];

  float timer_keep_wj_vel, timer_wj_last_dir;

  Tile_Base* Tile_Collision();

  Angle wjmp_angle, dash_angle;
  Direction wjmp_dir, dash_dir;

  // ---- utility functions
  std::vector<AOD::Collision_Info> Tile_Collision(AOD::Vector velocity);
  std::vector<std::pair<Tile_Base*, AOD::Collision_Info>>
                                          R_Coll_Tiles(AOD::Vector vel);
  // ---- graphical/joints
  Pl_Sprite_Handler::Body sprite_body;

  // ---- update functions
  void Update_Pre_Var_Set();
  void Update_Crouch();
  void Update_Horiz_Key_Input();
  void Update_Dash();
  void Update_Friction();
  void Update_Jump();
  void Update_Wall_Jump();
  void Update_Gravity();
  void Update_Velocity();
  void Update_Sprite();

  // -- graphical utility functions
  // checks if crouching or walking/running/idle (and their turns)
  void Gfx_To_Ground_Movement();
  // checks if uncrouching or crouching turn/move
  void Gfx_To_Ground_Crouch_Movement();
  // checks if landing on ground
  void Gfx_To_Ground_Check();
  // checks if air idle or jumping
  void Gfx_To_Air_Check();
public:
  Player(Player_Info*);
  void Update();

  inline Pl_Sprite_Handler::Body& R_Joint_Body() { return sprite_body; }

  void Set_Health(int);
  void Set_In_Liquid(bool);
  void Set_Oxygen(int);
  void Set_Crouching ( bool );

  void Set_Key_Up( bool );
  void Set_Key_Down( bool );
  void Set_Key_Left( bool );
  void Set_Key_Right( bool );
  void Set_Key_Sec( bool );
  void Set_Key_Prim( bool );
  void Set_Key_Crouch( bool );
  void Set_Key_Dash( bool );
  void Set_Key_Jump( bool );

  const Player_Info* R_Pl_Info();
  
  bool R_Key_Up() const;
  bool R_Key_Down() const;
  bool R_Key_Left() const;
  bool R_Key_Right() const;
  bool R_Key_Sec() const;
  bool R_Key_Prim() const;
  bool R_Key_Crouch() const;
  bool R_Key_Dash() const;
  bool R_Key_Jump() const;
  bool R_Wjmp() const;

  inline bool R_In_Air() const { return in_air; }
  inline Direction R_Dir() const { return direction; };

  Angle R_Dash_Angle() const;
  Direction R_Dash_Dir() const;
  Angle R_WJmp_Angle() const;
  Direction R_WJmp_Dir() const;

  int R_Health() const;
  int R_Armour() const;
  Item_Type R_Armour_Type() const;
  Weapon_Status R_Weapon_Status(Weapon) const;
  inline Weapon R_Sel_Weapon() const { return sel_weapon; }
  int R_Ammo_Curr_Weapon() const;
  int R_Mega_Stack() const;

  inline float R_Aiming_Angle() { return 0.0f; }

  Pl_Sprite_Handler::Body R_Sprite_Body();
  
  void Set_Anim(PlayerSheet::Hand_Anim a);
  void Set_Anim(PlayerSheet::Leg_Anim  a);
  void Set_Anim(PlayerSheet::Body_Anim a);
  void Set_Anim(PlayerSheet::Head_Anim a);
};


#endif
