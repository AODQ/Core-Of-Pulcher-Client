#ifndef PLAYER_JOINTS_H_
#define PLAYER_JOINTS_H_

#include "Animation.h"
#include "PlayerSheet.h"

class Player;

namespace Pl_Sprite_Handler {
  // --- animation/obj handling ---
  class Body;
  class Joint {
  public:
    enum class Type {
      uphand, dnhand, lleg, rleg, head, size
    };
  protected:
    Player* player;
    const Type type;
    AOD::Object* obj;
    Anim_Handler anim;
    void Update_Base(AOD::Vector pos, float angle);
  public:
    Joint(Player* p, Type t);
    ~Joint();
    virtual void Update(AOD::Vector& position, float angle, bool dir) = 0;
    inline const Anim_Handler& R_Anim() const { return anim; }
    inline void Set_Anim_Time(float t, bool flip = 0) {
      anim.timer = flip ? 1.0f - t : t;
    }
    void Reset();
  };
  
  class Joint_Hand : public Joint {
    PlayerSheet::Hand_Anim type;
  public:
    Joint_Hand(Player* player, Type type);
    void Update(AOD::Vector& position, float angle, bool dir);
    // will only set animation if animation has changed
    void Set_Anim(Animation* a, PlayerSheet::Hand_Anim t);
    PlayerSheet::Hand_Anim R_Anim_Type();
  };
  class Joint_Lower : public Joint {
    float turn_timer;
    PlayerSheet::Leg_Anim type; 
  public:
    Joint_Lower(Player* player);
    void Update(AOD::Vector& position, float angle, bool dir);
    // will only set animation if animation has changed
    void Set_Anim(Animation* a, PlayerSheet::Leg_Anim t);
    int R_Flip_Off_X();
    void Turn();
    PlayerSheet::Leg_Anim R_Anim_Type();
  };
  class Joint_Head : public Joint {
    PlayerSheet::Head_Anim type;
  public:
    Joint_Head(Player* player);
    void Update(AOD::Vector& position, float angle, bool dir);
    // will only set animation if animation has changed
    void Set_Anim(Animation* a, PlayerSheet::Head_Anim t);
  };

  class Body {
    Player* player;
    AOD::Object* obj;
    float prev_angle;
    bool prev_dir;
    PlayerSheet::Body_Anim type;
  public:
    Joint_Hand uphand;
    Joint_Hand dnhand;
    Joint_Head  head;
    Joint_Lower lower;
    Body(Player* player);
    ~Body();
    void Update(AOD::Vector position, float angle);
    // will only set animation if animation has changed
    void Set_Anim(Animation* a, PlayerSheet::Body_Anim t);
    void Reset_Lower();
    void Reset_Hands();
  };
}

#endif