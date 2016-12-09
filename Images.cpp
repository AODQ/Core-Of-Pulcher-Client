/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "AOD.h"

#include "Images.h"
#include "Joint_Animation.h"
#include "Utility.h"

#include <fstream>
#include <mutex>

#include <SDL_opengl.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <tuple>


SheetContainer::SheetContainer(){
  texture = 0;
  width = height = 0;
}

SheetContainer::SheetContainer(GLuint t, int w, int h) {
  texture = t;
  width = w;
  height = h;
}

SheetContainer::SheetContainer(char* image_name) {
  auto z = Load_Image(image_name);
  texture = z.texture;
  width   = z.width;
  height  = z.height;
}

// ---- animations ------------------------------------------------------------

/*Animation::Animation(const SheetContainer& sc, float ms, Type t) {
  type = t;
  image = sc;
  end = ms;
  dir = 1;
}*/




// ---- sheet rect ------------------------------------------------------------

SheetRect::SheetRect() {}
SheetRect::SheetRect(const SheetContainer& sc, AOD::Vector tul,
                       AOD::Vector tlr) : SheetContainer(sc) {
  ul = tul;
  lr = tlr;
  width  = abs(ul.x - lr.x);
  height = abs(ul.y - lr.y);
  ul.x /= sc.width; lr.y /= sc.height;
  lr.x /= sc.width; ul.y /= sc.height;
  float ty = lr.y;
  lr.y = 1 - ul.y;
  ul.y = 1 - ty;
}


// ---- utility functions -----------------------------------------------------

static std::mutex load_image_mutex;
SheetContainer Load_Image(const char * fil) {
  std::lock_guard<std::mutex> load_image_lock(load_image_mutex);

  ret_assert2(std::ifstream(fil).good(), "File " + std::string(fil) +
              " not found", SheetContainer());
  ILuint IL_ID;
  GLuint GL_ID;
  int width, height;

  ilGenImages(1, &IL_ID);
  ilBindImage(IL_ID);
  if ( ilLoadImage( fil ) == IL_TRUE ) {
    ILinfo ImageInfo;
    iluGetImageInfo(&ImageInfo);
    if ( ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT )
      iluFlipImage();

      if ( !ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE) ) {
        auto t = iluErrorString(ilGetError());
        AOD_Engine::Debug_Output(t);
        return 0;
      }
      glBindTexture(GL_TEXTURE_2D, 0);
      glGenTextures(1, &GL_ID);
      glBindTexture(GL_TEXTURE_2D, GL_ID);
      if ( !glIsTexture(GL_ID) ) {
        AOD::Output("Error generating GL texture");
        return SheetContainer();
      }
      // set texture clamping method
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

      // set texture interpolation method
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        
      width  = ilGetInteger(IL_IMAGE_WIDTH);
      height = ilGetInteger(IL_IMAGE_HEIGHT);

      // texture specs
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
    } else {
      auto t = ilGetError();
      AOD_Engine::Debug_Output("Error loading " + std::string(fil) + ": " +
        iluErrorString(t) + "(" + std::to_string(ilGetError()) + ')');
      return 0;
    }

    ilDeleteImages(1, &IL_ID);
    return SheetContainer(GL_ID, width, height);
}

SheetContainer Load_Image(std::string fil) {
  return Load_Image(fil.c_str());
}