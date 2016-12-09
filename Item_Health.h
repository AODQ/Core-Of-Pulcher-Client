#ifndef ITEM_HEALTH_H_
#define ITEM_HEALTH_H_
#pragma once

#include "Item.h"

class Player;

class HP_Item : public AOD::Object, public Item {
public:
  HP_Item();
  virtual void Apply_Effect(Player*) = 0;    
  virtual const HUD::Font_Type R_Font_Type();
  void Update();
};

class HP_Item_Shard : public HP_Item {
public:
  HP_Item_Shard();
  void Apply_Effect(Player*);
  void Update();
  static const int Health, Limit, Resp_time;
  static const bool Stack; // can be used passed 100 hp
};

class HP_Item_Capsule : public HP_Item {
public:
  HP_Item_Capsule();
  void Apply_Effect(Player*);
  void Update();
  static const int Health, Limit, Resp_time;
  static const bool Stack; // can be used passed 100 hp
};

class HP_Item_Mega : public HP_Item {
public:
  HP_Item_Mega();
  void Apply_Effect(Player*);
  void Update();
  static const HUD::Font_Type Font_type;
  static const int Health, Limit, Resp_time;
  static const bool Stack; // can be used passed 100 hp
};

#endif