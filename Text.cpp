/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#include <GLew\glew.h>
#include "stdafx.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <fstream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "AOD.h"
#include "Text.h"
#include "Vector.h"

/* EXTERNS */

using AOD_Text = AOD::Text;
std::string AOD_Text::default_font = "";
std::map<std::pair<std::string, int>, AOD_Engine::Font*>    AOD_Engine::fonts;
int AOD_Text::default_pt_size = 12;
FT_Library AOD_Engine::Font::FTLib;


std::string R_FT_Error_String(int code);

/* FONT */

void AOD_Engine::Font::Init() {
  auto comp = FT_Init_FreeType(&FTLib);
  if ( comp ) {
    AOD_Engine::Debug_Output("Could not open FreeType Library: " +
      R_FT_Error_String(comp) + "\n");
    return;
  }
}

AOD_Engine::Font::Font(std::string file, int siz) {
  if ( !std::ifstream(file.c_str()).good() ) {
    AOD_Engine::Debug_Output("Could not load font " + file + ": not found\n");
    return;
  }
  int comp = FT_New_Face(FTLib, file.c_str(), 0, &face);
  if ( comp ) {
    AOD_Engine::Debug_Output("Could not load font " + file + ": " +
      R_FT_Error_String(comp) + '\n');
    return;
  }
  if ( (comp = FT_Set_Pixel_Sizes(face, 0, siz)) ) {
    AOD_Engine::Debug_Output("Could not load font " + file + ": " +
      R_FT_Error_String(comp));
  }

  glGenTextures( 128, char_texture );
  char_lists = glGenLists(128);



  for ( int i = 0; i != 128; ++ i ) {
    FT_UInt index = FT_Get_Char_Index(face, i);
    if ( FT_Load_Glyph(face, index, FT_LOAD_RENDER) ) {
      AOD_Engine::Debug_Output("Could not load char index " +
                                std::to_string(i) + " for font " + file);
      continue;
    }
    if ( FT_Render_Glyph(face->glyph, FT_Render_Mode_::FT_RENDER_MODE_NORMAL)) {
      AOD_Engine::Debug_Output("Failed to render char index " + 
                                std::to_string(i) + " for font " + file);
      continue;
    }

    FT_Glyph glyph;
    if ( FT_Get_Glyph ( face->glyph, &glyph ) ) {
      AOD_Engine::Debug_Output( "Get Glyph failed at char index " +
                                std::to_string(i) + " for font " + file);
      continue;
    }
    if ( FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1) ) {
      AOD_Engine::Debug_Output( "Glyph to bitmap failed at char index " +
                                std::to_string(i) + " for font " + file);
      continue;
    }
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    FT_Bitmap& map = bitmap_glyph->bitmap;

    //map.palette_mode = ILUT_PALETTE_MODE;

    int d = (face->glyph->metrics.height - face->glyph->metrics.horiBearingY)>>6;

    int w = map.width*map.width,
        h = map.rows *map.rows;


    GLubyte* data = new GLubyte[2 * w * h];

    for ( int x = 0; x != w; ++ x )
    for ( int y = 0; y != h; ++ y ) {
      data[2 * (x + y*w)    ] =
      data[2 * (x + y*w) + 1] =
        (x >= map.width || y >= map.rows ) ?
        0 : map.buffer[x + map.width*y];
    }

    glBindTexture(GL_TEXTURE_2D, char_texture[i]);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE_ALPHA,
                  GL_UNSIGNED_BYTE, data );

    delete[] data;
    glNewList(char_lists + i, GL_COMPILE );

      glBindTexture( GL_TEXTURE_2D, char_texture[i] );

      glPushMatrix();
        glTranslatef(face->glyph->bitmap_left, 0, 0);
        float x = (float)map.width/(float)w,
              y = (float)map.rows /(float)h;

        glBegin( GL_QUADS );
          glTexCoord2f( 0.0f, y ); glVertex2f( 0.0f, d );
          glTexCoord2f( x, y ); glVertex2f( map.width, d );
          glTexCoord2f( x, 0.0f); glVertex2f( map.width,-(int)map.rows + d );
          glTexCoord2f( 0.0f, 0.0f ); glVertex2f( 0.0f, -(int)map.rows + d );
        glEnd();
      glPopMatrix();

      glTranslatef(face->glyph->advance.x>>6, 0, 0);

    glEndList();
  }

  FT_Done_Face(face);
};

AOD_Engine::Font::~Font() {
  glDeleteTextures(128, char_texture);
}

FT_Face AOD_Engine::Font::R_Face() { return face; }
GLuint AOD_Engine::Font::R_Character(char c) {
  return char_texture[c];
}
GLuint AOD_Engine::Font::R_Character_List() {
  return char_lists;
}

AOD_Engine::Font* AOD_Engine::Font::Load_Font(std::string str, int pt_size) {
  auto font_pair = std::pair<std::string,int>(str,pt_size);
  if ( AOD_Engine::fonts.find(font_pair) == AOD_Engine::fonts.end() ) {
    AOD_Engine::fonts[font_pair] = new Font(str, pt_size);
  }
  return AOD_Engine::fonts[font_pair];
}

/* TEXT */

AOD_Text::Text(int pos_x, int pos_y, std::string str) {
  position.x = pos_x;
  position.y = pos_y;
  string = str;
  font = "";
  ft_font = nullptr;

  uses_default_font = 1;
  pt_size = 12;
  visible = 1;

  if ( default_font != "" )
    Refresh_Message();
}

AOD_Text::Text(AOD::Vector pos, std::string str) {
  position = pos;
  string = str;
  font = "";
  ft_font = nullptr;

  uses_default_font = 1;
  pt_size = 12;
  visible = 1;

  if ( default_font != "" )
    Refresh_Message();
}

void AOD_Text::Set_Position(AOD::Vector pos) {
  position = pos;
}
void AOD_Text::Set_Position(float x, float y) {
  position.x = x;
  position.y = y;
}
void AOD_Text::Set_String(std::string str) {
  string = str;
  //Refresh_Message();
}
void AOD_Text::Set_Colour(int r, int g, int b) {
  //colour = { r*255, g*255, b*255 };
}
void AOD_Text::Set_Font(std::string str, int pt_siz) {
  AOD_Engine::Font::Load_Font(str, pt_siz);
  font = str;
  pt_size = pt_siz;
  uses_default_font = 0;
  Refresh_Message();
}
void AOD_Text::Set_Visible(bool t) {
  visible = t;
}
void AOD_Text::Set_To_Default() {
  uses_default_font = 1;
  if ( default_font != "" )
    Refresh_Message();
}

void AOD_Text::Set_Default_Font(std::string str, int pt_siz) {
  AOD_Engine::Font::Load_Font(str, pt_siz);
  default_font = str;
  default_pt_size = pt_siz;
}

AOD::Vector AOD_Text::R_Position() { return position; }
std::string AOD_Text::R_Font() {
  if ( ft_font == nullptr ) return default_font;
  else                      return font;
}
int         AOD_Text::R_Pt_Size() {
  if ( uses_default_font   ) return default_pt_size;
  else                       return pt_size;
}
AOD_Engine::Font* AOD_Text::R_FT_Font() { return ft_font; }
std::string& AOD_Text::R_Str() { return string; }
bool AOD_Text::R_Visible() { return visible; }

std::string AOD_Text::R_Default_Font() { return default_font; }

void AOD_Text::Refresh_Message() {
  if ( uses_default_font ) {
    pt_size = default_pt_size;
    font    = default_font;
    ft_font = AOD_Engine::fonts[std::pair<std::string,int>(default_font,
                                                           default_pt_size)];
  } else {
    ft_font = AOD_Engine::fonts[std::pair<std::string,int>(font, pt_size)];
  }
}

AOD_Text::~Text() {

}


std::string R_FT_Error_String(int code) {
  if ( 0x00 == code ) return "no error";
  if ( 0x01 == code ) return "cannot open resource";
  if ( 0x02 == code ) return "unknown file format";
  if ( 0x03 == code ) return "broken file";
  if ( 0x04 == code ) return "invalid FreeType version";
  if ( 0x05 == code ) return "module version is too low";
  if ( 0x06 == code ) return "invalid argument";
  if ( 0x07 == code ) return "unimplemented feature";
  if ( 0x08 == code ) return "broken table";
  if ( 0x09 == code ) return "broken offset within table";
  if ( 0x0A == code ) return "array allocation size too large";

  /* glyph/character errors */

  if ( 0x10 == code ) return "invalid glyph index";
  if ( 0x11 == code ) return "invalid character code";
  if ( 0x12 == code ) return "unsupported glyph image format";
  if ( 0x13 == code ) return "cannot render this glyph format";
  if ( 0x14 == code ) return "invalid outline";
  if ( 0x15 == code ) return "invalid composite glyph";
  if ( 0x16 == code ) return "too many hints";
  if ( 0x17 == code ) return "invalid pixel size";

  /* handle errors */

  if ( 0x20 == code ) return "invalid object handle";
  if ( 0x21 == code ) return "invalid library handle";
  if ( 0x22 == code ) return "invalid module handle";
  if ( 0x23 == code ) return "invalid face handle";
  if ( 0x24 == code ) return "invalid size handle";
  if ( 0x25 == code ) return "invalid glyph slot handle";
  if ( 0x26 == code ) return "invalid charmap handle";
  if ( 0x27 == code ) return "invalid cache manager handle";
  if ( 0x28 == code ) return "invalid stream handle";

  /* driver errors */

  if ( 0x30 == code ) return "too many modules";
  if ( 0x31 == code ) return "too many extensions";

  /* memory errors */

  if ( 0x40 == code ) return "out of memory";
  if ( 0x41 == code ) return "unlisted object";

  /* stream errors */

  if ( 0x51 == code ) return "cannot open stream";
  if ( 0x52 == code ) return "invalid stream seek";
  if ( 0x53 == code ) return "invalid stream skip";
  if ( 0x54 == code ) return "invalid stream read";
  if ( 0x55 == code ) return "invalid stream operation";
  if ( 0x56 == code ) return "invalid frame operation";
  if ( 0x57 == code ) return "nested frame access";
  if ( 0x58 == code ) return "invalid frame read";

  /* raster errors */

  if ( 0x60 == code ) return "raster uninitialized";
  if ( 0x61 == code ) return "raster corrupted";
  if ( 0x62 == code ) return "raster overflow";
  if ( 0x63 == code ) return "negative height while rastering";

  /* cache errors */

  if ( 0x70 == code ) return "too many registered caches";

  /* TrueType and SFNT errors */

  if ( 0x80 == code ) return "invalid opcode";
  if ( 0x81 == code ) return "too few arguments";
  if ( 0x82 == code ) return "stack overflow";
  if ( 0x83 == code ) return "code overflow";
  if ( 0x84 == code ) return "bad argument";
  if ( 0x85 == code ) return "division by zero";
  if ( 0x86 == code ) return "invalid reference";
  if ( 0x87 == code ) return "found debug opcode";
  if ( 0x88 == code ) return "found ENDF opcode in execution stream";
  if ( 0x89 == code ) return "nested DEFS";
  if ( 0x8A == code ) return "invalid code range";
  if ( 0x8B == code ) return "execution context too long";
  if ( 0x8C == code ) return "too many function definitions";
  if ( 0x8D == code ) return "too many instruction definitions";
  if ( 0x8E == code ) return "SFNT font table missing";
  if ( 0x8F == code ) return "horizontal header (hhea) table missing";
  if ( 0x90 == code ) return "locations (loca) table missing";
  if ( 0x91 == code ) return "name table missing";
  if ( 0x92 == code ) return "character map (cmap) table missing";
  if ( 0x93 == code ) return "horizontal metrics (hmtx) table missing";
  if ( 0x94 == code ) return "PostScript (post) table missing";
  if ( 0x95 == code ) return "invalid horizontal metrics";
  if ( 0x96 == code ) return "invalid character map (cmap) format";
  if ( 0x97 == code ) return "invalid ppem value";
  if ( 0x98 == code ) return "invalid vertical metrics";
  if ( 0x99 == code ) return "could not find context";
  if ( 0x9A == code ) return "invalid PostScript (post) table format";
  if ( 0x9B == code ) return "invalid PostScript (post) table";

  /* CFF, CID, and Type 1 errors */

  if ( 0xA0 == code ) return "opcode syntax error";
  if ( 0xA1 == code ) return "argument stack underflow";
  if ( 0xA2 == code ) return "ignore";
  if ( 0xA3 == code ) return "no Unicode glyph name found";


  /* BDF errors */

  if ( 0xB0 == code ) return "`STARTFONT' field missing";
  if ( 0xB1 == code ) return "`FONT' field missing";
  if ( 0xB2 == code ) return "`SIZE' field missing";
  if ( 0xB3 == code ) return "`FONTBOUNDINGBOX' field missing";
  if ( 0xB4 == code ) return "`CHARS' field missing";
  if ( 0xB5 == code ) return "`STARTCHAR' field missing";
  if ( 0xB6 == code ) return "`ENCODING' field missing";
  if ( 0xB7 == code ) return "`BBX' field missing";
  if ( 0xB8 == code ) return "`BBX' too big";
  if ( 0xB9 == code ) return "Font header corrupted or missing fields";
  if ( 0xBA == code ) return "Font glyphs corrupted or missing fields";
}
