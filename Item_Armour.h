#ifndef ITEM_ARMOUR_H_
#define ITEM_ARMOUR_H_
#pragma once

#include "AOD.h"
#include "HUD.h"
#include "Item.h"

class Player;

class Arm_Item : public AOD::Object, public Item {
  float timer; // anims
public:
  Arm_Item();
  virtual void Apply_Effect(Player*) = 0;
  virtual const HUD::Font_Type R_Font_Type();
  void Update();
};

class Arm_Item_Shard : public Arm_Item {
public:
  Arm_Item_Shard();
  void Apply_Effect(Player*);
  void Update();

  static const int Armour, Limit, Resp_time;
  static const HUD::Font_Type Font_type;
  static const float Protection;
};

class Arm_Item_Bronze : public Arm_Item {
public:
  Arm_Item_Bronze();
  void Apply_Effect(Player*);
  void Update();

  static const int Armour, Limit, Resp_time;
  static const float Protection;
  static const HUD::Font_Type Font_type;
};

class Arm_Item_Silver : public Arm_Item {
public:
  Arm_Item_Silver();
  void Apply_Effect(Player*);
  void Update();

  static const int Armour, Limit, Resp_time;
  static const float Protection;
  static const HUD::Font_Type Font_type;
};

class Arm_Item_Gold : public Arm_Item {
public:
  Arm_Item_Gold();
  void Apply_Effect(Player*);
  void Update();

  static const int Armour, Limit, Resp_time;
  static const float Protection;
  static const HUD::Font_Type Font_type;
};

#endif