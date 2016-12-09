/* (c) CiggBit. All rights reserved.uint16_t
 * See license.txt for more information
 */
 
#ifndef REALM_H_
#define REALM_H_
#pragma once

#include <vector>

namespace AOD {
  class Object;
  class Text;
}

namespace AOD_Engine {
  class Realm {
    std::vector<std::vector<AOD::Object*>> objects;
    std::vector<AOD::Text*> text;

    std::vector<AOD::Object*> objs_to_rem;

    GLfloat bg_red, bg_blue, bg_green;
  public:
    Realm(int window_width,int window_height,
          char* window_name = "", char* icon = "");
    ~Realm();
    void Reset();

    void __Add(AOD::Object*,int layer = 0);
    void __Add(AOD::Text*);
    void __Remove(AOD::Object*);
    void __Remove(AOD::Text*);

    void Set_BG_Colours(GLfloat r, GLfloat g, GLfloat b);

    void Update();
    void Render();
  };
}

#endif