#include "Item.h"
#include "Item_Armour.h"
#include "Item_Health.h"
#include "Item_Powerup.h"

using HFT = HUD::Font_Type;

// ---- health items ----------------------------------------------------------

// -- health shards --
const int  HP_Item_Shard::Health    = 5,
           HP_Item_Shard::Limit     = 200,
           HP_Item_Shard::Resp_time = 15;
const bool HP_Item_Shard::Stack = 1;
// -- health capsules --
const int  HP_Item_Capsule::Health    = 30,
           HP_Item_Capsule::Limit     = 100,
           HP_Item_Capsule::Resp_time = 15;
const bool HP_Item_Capsule::Stack = 0;
// -- health mega --
const int  HP_Item_Mega::Health    = 100,
           HP_Item_Mega::Limit     = 200,
           HP_Item_Mega::Resp_time = 35;
const bool HP_Item_Mega::Stack = 1;
const HFT  HP_Item_Mega::Font_type = HFT::purple;

// ---- armour items ----------------------------------------------------------

// -- armour shards --
const int   Arm_Item_Shard::Armour     = 5,
            Arm_Item_Shard::Limit      = 200,
            Arm_Item_Shard::Resp_time  = 15;
const float Arm_Item_Shard::Protection = -1; // retains protection

// -- armour bronze --
const int   Arm_Item_Bronze::Armour     = 50,
            Arm_Item_Bronze::Limit      = 100,
            Arm_Item_Bronze::Resp_time  = 15;
const float Arm_Item_Bronze::Protection = 0.5f;
const HFT   Arm_Item_Bronze::Font_type  = HFT::bronze;

// -- armour silver --
const int   Arm_Item_Silver::Armour     = 100,
            Arm_Item_Silver::Limit      = 150,
            Arm_Item_Silver::Resp_time  = 25;
const float Arm_Item_Silver::Protection = 2.f/3.f;
const HFT   Arm_Item_Silver::Font_type  = HFT::silver;

// -- armour gold --
const int   Arm_Item_Gold::Armour     = 150,
            Arm_Item_Gold::Limit      = 200,
            Arm_Item_Gold::Resp_time  = 25;
const float Arm_Item_Gold::Protection = 3.f/4.f;
const HFT   Arm_Item_Gold::Font_type  = HFT::gold;

// ---- powerup items ---------------------------------------------------------

// -- powerup strength --
const int   Powerup_Item_Strength::Effect_time    = 30;
const float Powerup_Item_Strength::Resp_time      = 120;
const HFT   Powerup_Item_Strength::Font_type      = HFT::blue;

// -- powerup velocity --
const int   Powerup_Item_Velocity::Effect_time    = 30;
const float Powerup_Item_Velocity::Resp_time      = 120;
const HFT   Powerup_Item_Velocity::Font_type      = HFT::green;

// -- powerup camouflage --
const int   Powerup_Item_Camouflage::Effect_time  = 30;
const float Powerup_Item_Camouflage::Resp_time    = 120;
const HFT   Powerup_Item_Camouflage::Font_type    = HFT::purple;

// -- powerup immortality --
const int   Powerup_Item_Immortality::Effect_time = 30;
const float Powerup_Item_Immortality::Resp_time   = 120;
const HFT   Powerup_Item_Immortality::Font_type   = HFT::bronze;

// -- powerup time master --
const int   Powerup_Item_Time::Effect_time        = 15;
const float Powerup_Item_Time::Resp_time          = 120;
const HFT   Powerup_Item_Time::Font_type          = HFT::red;