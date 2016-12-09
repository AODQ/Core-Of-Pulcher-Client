#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "Images.h"

/* Can load animations from sheet rects (meaning that you can have several
 * frames from different sheet containers). You can't just add them to the
 * AOD stage and expect it to work, it's more of a storage of an animation
 * than a player. It's intended to be used with the Anim_Handler, but if
 * you want you can keep track of the milliseconds that have
 * elapsed yourself.
 */
class Animation {
public:
  /* animation types:
   * Linear:        0, 1, ..., n (end)
   * Repeat:        0, 1, ..., n, 0, 1, ... (no end)
   * Static:        0 (end)
   * Zigzag:        0, 1, ..., n, n-1, ..., 0 (end)
   * Zigzag_Repeat: 0, 1, ..., n, n-1, ..., 0, 1, ... (no end)
   */
  enum class Type {
    Linear, Repeat, Static, Zigzag, Zigzig_Repeat
  };
  // Returns type based off string: "Linear" "Repeat" "Zigzag" "Zigzag_Repeat"
  // if no match is found, returns linear.
  static Type Str_To_Type(const std::string&);
  using SRect_Vec = std::vector<SheetRect>;
private:
  SRect_Vec frames;
  float ms_per_frame;
  Type type;
public:
  // SRect_Vec: vector of sheet rects that contain each tile
  // ms_per_frame_: milliseconds that should elapse per animation frame (not
  // in game frame)
  Animation(SRect_Vec, float ms_per_frame_, Type = Type::Linear);

  // reconstructs animation based off same parameters as constructor
  void Reconstruct(SRect_Vec, float ms_per_frame_, Type = Type::Linear);
  
  /*void Add_Frame(const SheetRect&);
  void Set_Sheet_Container(const SheetContainer&);
  void Set_MS_Per_Frame(float ms);*/  
  // returns total frames of animation
  inline int  R_Size()           const { return frames.size(); }
  inline Type R_Type()           const { return type;          }
  inline float  R_MS_Per_Frame() const { return ms_per_frame;  }
  // Returns index of frame
  int               R_Frame   (float ms) const;
  // Should only be used for Static types, it will return the 0th sheet rect
  const SheetRect& R_Frame   ()         const;
  const SheetRect& R_Frame_SR(float ms) const;
  // returns sheet container of specified frame, defaults to 0
  const SheetContainer& R_SheetContainer() const;
};

// Just contains background information, does nothing of usefulness yet.
class Background {
public:
  SheetContainer* image;

  bool vertical = 0, horizontal = 0, repeats = 0;
  float speed_x = 0,// background speed is defined by following:
        speed_y = 0;// If  parralax, then 1:1 ratio from map units w/ player
  bool parralax = 0;// If !parralax, then 1 unit moved per 1 frame
  int layer = 0;
};

/* Plays animations from the Animation class, will keep track of the timer,
 * whether the animation is finished, etc.
 */
class Anim_Handler {
public:
  Animation* animation;
  // current direction of animation (1 = forwards, 0 = backwards)
  bool dir = 0;
  bool done = 0;
  float timer = 0.0f;
  Anim_Handler();
  Anim_Handler(Animation* anim);

  // Resets the animation player (direction, timer and done)
  void Reset();
  // Will call Reset
  void Set(Animation* anim);
  const SheetRect& Update();
  // returns ratio of playback in percentage (0.53 = 53%)
  float R_Time_Ratio() const;
};

#endif