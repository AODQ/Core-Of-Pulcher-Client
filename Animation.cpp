#include "Animation.h"

#include <algorithm>

Animation::Animation(SRect_Vec rec,  float ms, Type t) {
  Reconstruct(rec, ms, t);
}

void Animation::Reconstruct(SRect_Vec rec, float ms, Type t) {
  type         = t;
  ms_per_frame = ms;
  frames       = rec;
}

int Animation::R_Frame(float ms) const {
  return std::min<int>(std::max<int>(ms, 0), frames.size()-1);;
}

const SheetRect& Animation::R_Frame() const { return frames[0]; }
const SheetRect& Animation::R_Frame_SR(float ms) const {
  if ( frames.size() != 0 )
    return frames[R_Frame(ms)];
  else {
    return SheetRect();
  }
}



Animation::Type Animation::Str_To_Type(const std::string& str) {
  static std::map<std::string, Animation::Type> anim_type = {
    {"Linear", Type::Linear}, {"Zigzag", Type::Zigzag},
    {"Repeat", Type::Repeat}, {"Zigzag_Repeat", Type::Zigzig_Repeat}
  };

  auto e = anim_type.find(str);
  if ( e == anim_type.end() ) return Animation::Type::Linear;
  return e->second;
}

// ----- anim handler ----
Anim_Handler::Anim_Handler() {
  animation = nullptr;
  Reset();
}
Anim_Handler::Anim_Handler(Animation* anim) {
  Reset();
  animation = anim;
}

void Anim_Handler::Reset() {
  dir = 1;
  timer = 0;
  done = 0;
}

void Anim_Handler::Set(Animation* anim) {
  animation = anim;
  Reset();
}

const SheetRect& Anim_Handler::Update() {
  if ( animation == nullptr )
    return SheetRect();
  if ( animation->R_Type() == Animation::Type::Static )
    return animation->R_Frame();
  // -- add to timer and check all boundaries based on type
  timer += (float)(animation->R_MS_Per_Frame())/AOD::R_MS() * (dir? 1 : -1);
  if ( AOD::Input::keys [ SDL_SCANCODE_RIGHT ] ) {
    timer += 1;
    AOD::Input::keys [ SDL_SCANCODE_RIGHT ] = 0;
  }
  if ( AOD::Input::keys [ SDL_SCANCODE_LEFT ] ) {
    timer -= 1;
    AOD::Input::keys [ SDL_SCANCODE_LEFT ] = 0;
  }
  switch ( animation->R_Type() ) {
    case Animation::Type::Linear:
      if ( timer >= animation->R_Size() ) {
        done = 1;
      }
    break;
    case Animation::Type::Repeat:
      if ( timer >= animation->R_Size() ) {
        timer = 0;
      }
    break;
    case Animation::Type::Zigzag:
      if ( timer >= animation->R_Size() ) {
        timer = animation->R_Size() - 1;
        dir = 0;
      }
      if ( timer < 0 ) {
        timer = 0;
        done = 1;
      }
    break;
    case Animation::Type::Zigzig_Repeat:
      if ( timer >= animation->R_Size() ) {
        timer = animation->R_Size() - 1;
        dir = 0;
      }
      if ( timer < 0 ) {
        timer = 1;
        dir = 1;
      }
    break;
  }
  return animation->R_Frame_SR(timer);
}

float Anim_Handler::R_Time_Ratio() const {
  switch ( animation->R_Type() ) {
    case Animation::Type::Linear: case Animation::Type::Repeat:
      return timer/(float)(animation->R_Size() * animation->R_MS_Per_Frame());
    break;
    case Animation::Type::Static:
      return 1.00f;
    break;
    case Animation::Type::Zigzag: case Animation::Type::Zigzig_Repeat: {
      float a = timer/(float)(
        (animation->R_Size() * animation->R_MS_Per_Frame())*2.f);
      if ( !dir )
        a = 0.5-a + 0.5f;
      return a;
    } break;
  }
  return 0.0f;
}