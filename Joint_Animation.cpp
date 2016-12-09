#include "Images.h"
#include "Joint_Animation.h"

using PJA = PlayerJointAnimation;

Animation::SRect_Vec PJA::Create_Anim(int x, int endx, int y, 
                                SheetContainer& img, std::string anim_name) {
  -- x; -- y;
  auto C_Sheet_Rect = [&](int X, int Y) -> SheetRect {
    float x = X, y = Y;
    AOD::Vector ul = { x   *64,  y   *64},
                lr = {(x+1)*64, (y+1)*64};
    SheetRect sr = SheetRect(img, ul, lr);
    sr.width  = 64;
    sr.height = 64;
    return sr;
  };
  //endx += 2;
  Animation::SRect_Vec vec;
  if ( x < endx ) {
    for ( ; x < endx; ++ x )
      vec.push_back(C_Sheet_Rect(x, y));
  } else {
    endx -= 2;
    for ( ; x > endx; -- x )
      vec.push_back(C_Sheet_Rect(x, y));
  }
  return vec;
}

PJA::PlayerJointAnimation() : Animation({}, 0) {
  offset = {0, 0};
}

PJA::PlayerJointAnimation(int x, int ex, int y, SheetContainer& img,
      Animation::Type type, AOD::Vector _offset, float speed)
   : Animation(Create_Anim(x, ex, y, img, ""), speed, type),
     offset(_offset)
 {}

void PJA::Reconstruct(int x, int ex, int y, SheetContainer& img,
      Animation::Type type, AOD::Vector _offset, float speed,
      std::string anim_name) {
  Animation::Reconstruct(Create_Anim(x, ex, y, img, anim_name), speed, type);
  offset = _offset;
}

