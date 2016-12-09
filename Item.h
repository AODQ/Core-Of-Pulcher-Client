#ifndef ITEM_H_
#define ITEM_H_
#pragma once

class Player;

enum class Item_Type {
  pu_strength, pu_immort, pu_vel, pu_camo, pu_time,
  arm_gold, arm_silver, arm_bronze, arm_shard, arm_nil,
  hp_mega, hp_capsule, hp_shard, hp_nil, size
};

class Item {
  const Item_Type type;
  float timer; // mainly for animations
public:
  Item(Item_Type);
  Item_Type R_Type();
  virtual void Apply_Effect(Player*) = 0;
};

#endif
