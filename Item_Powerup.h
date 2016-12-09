#ifndef ITEM_POWERUP_H_
#define ITEM_POWERUP_H_
#pragma once

#include "Item.h"

class Player;

class Powerup_Item : public AOD::Object, public Item {
  float timer;
public:
  Powerup_Item();
  virtual void Apply_Effect(Player*) = 0;
  virtual const HUD::Font_Type R_Font_Type();
  void Update();
};

class Powerup_Item_Strength : public Powerup_Item {
public:
  Powerup_Item_Strength();
  void Apply_Effect(Player*);
  void Update();

  static const int Effect_time;
  static const HUD::Font_Type Font_type;
  static const float Resp_time;
};

class Powerup_Item_Velocity : public Powerup_Item {
public:
  Powerup_Item_Velocity();
  void Apply_Effect(Player*);
  void Update();

  static const int Effect_time;
  static const HUD::Font_Type Font_type;
  static const float Resp_time;
};

class Powerup_Item_Camouflage : public Powerup_Item {
public:
  Powerup_Item_Camouflage();
  void Apply_Effect(Player*);
  void Update();
  // statics

  static const int Effect_time;
  static const HUD::Font_Type Font_type;
  static const float Resp_time;
};

class Powerup_Item_Immortality : public Powerup_Item {
public:
  Powerup_Item_Immortality();
  void Apply_Effect(Player*);
  void Update();
  // statics

  static const int Effect_time;
  static const HUD::Font_Type Font_type;
  static const float Resp_time;
};
class Powerup_Item_Time : public Powerup_Item {
public:
  Powerup_Item_Time();
  void Apply_Effect(Player*);
  void Update();

  static const int Effect_time;
  static const HUD::Font_Type Font_type;
  static const float Resp_time;
};

#endif