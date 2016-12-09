/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef IMAGES_H_
#define IMAGES_H_
#pragma once

#include "AOD.h"
#include <memory>
#include <SDL_opengl.h>
#include <vector>

// contains information of image
class SheetContainer {
public:
  GLuint texture;
  int width, height;
  SheetContainer();
  SheetContainer(GLuint,int,int);
  SheetContainer(char*);
};

// A sheet container that will also contain location of obj inside a sheet,
// pixel-based coordinates where origin is {0, 0}. Useful for spritesheets,
// I'm sure there are some other utilities such as image cropping.
class SheetRect : public SheetContainer {
public:
  AOD::Vector ul, lr;
  // nil constructor (no sheet container, ul/lr will both set to {0, 0}
  SheetRect();
  // Creates sheet rect whose image is sheet container, and coordinates
  // are from upper-left (ul) to lower-right (lr), which are relative offsets
  // from the origin {0, 0}
  SheetRect(const SheetContainer&, AOD::Vector ul, AOD::Vector lr);
};

SheetContainer Load_Image(const char* fil);
SheetContainer Load_Image(std::string fil);

#endif