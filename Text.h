/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef TEXT_H_
#define TEXT_H_
#pragma once

#include <SDL.H>
#include <glew/glew.h>
#include <SDL_opengl.h>
//#include <windows.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gl/GL.h>
#include <gl/GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <string>

#include "AOD.h"
#include "Utility.h"
#include "Vector.h"

namespace AOD_Engine {
  class Font {
    FT_Face face;
    GLuint char_texture[128];
    GLuint char_lists;
    int width;
  public:
    static FT_Library FTLib;

    Font(std::string file, int siz);
    ~Font();

    FT_Face R_Face();
    GLuint R_Character(char);
    GLuint R_Character_List();
    inline int R_Width() { return width; }

    static void Init();
    static Font* Load_Font(std::string fil, int siz);
  };

  extern std::map<std::pair<std::string, int>, Font*> fonts;
}

namespace AOD {
  class Text {
    Vector position;
    std::string string,
                font_name;
    int pt_size;
    std::string font;
    AOD_Engine::Font* ft_font;

    bool uses_default_font;
    static std::string default_font;
    static int default_pt_size;

    void Refresh_Message();

    bool visible;
  public:
    Text(int pos_x, int pos_y, std::string);
    Text(Vector pos,           std::string);
    ~Text();

    void Set_Position(Vector);
    void Set_Position(float, float);
    void Set_String(std::string);
    void Set_Font(std::string, int pt_Size);
    void Set_Colour(int r, int g, int b);
    void Set_Visible(bool);
    void Set_To_Default();

    Vector R_Position();
    std::string& R_Str();
    std::string R_Font();
    int         R_Pt_Size();
    AOD_Engine::Font* R_FT_Font();
    bool R_Visible();

    static void Set_Default_Font(std::string, int pt_size);
    static std::string R_Default_Font();
  };
}
#endif