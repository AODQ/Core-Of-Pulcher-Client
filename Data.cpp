/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#include <fstream>
#include <string>

#include "Data.h"
#include "HUD.h"
#include "Images.h"
#include "Joint_Animation.h"
#include "SimpleIni.h"
#include "AOD.h"

// ---- images ----------------------------------------------------------------
template <typename T, typename... Rest>
static void DelImages(T& t, Rest&... rest) {
  glDeleteTextures(1, &t.texture);
  t.height = t.width = 0;
  t.texture = 0;
  DelImages(t, rest...);
}
// -- misc
SheetContainer Images_Misc::pulcher_logo,
                Images_Misc::console_background,
                Images_Misc::text_cursor;
void Images_Misc::Load() {
  pulcher_logo       = Load_Image("Images\\pulcher_logo.png");
  console_background = Load_Image("Images\\console_background.png");
  text_cursor        = Load_Image("Images\\text_cursor.png");
}
void Images_Misc::Unload() {
  DelImages(pulcher_logo, console_background, text_cursor);
}
// ---- player ------------------------------------------------------------
PlayerSheet Images_Player::nygel;
SheetContainer Images_Player::debug;

struct Animation_Data {
  Animation_Data(std::string str) {
    int it = -1;
    auto tokens = Util::Extract_Tokens(str, std::string("[\\w\\.\\d]+"));
    animation = (tokens[0] == "animation");
    anim_name = tokens[1];
    ex_assert(tokens.size() == (animation?7:4), "Incorrect arguments "
                "for token " + anim_name);
    if ( animation ) {
      x     = Util::Str_To_T<int>(tokens[2]);
      ex    = Util::Str_To_T<int>(tokens[3]);
      y     = Util::Str_To_T<int>(tokens[4]);
      speed = Util::Str_To_T<float>(tokens[5]);
      anim_type = Animation::Str_To_Type(tokens[6]);
    } else {
      x = Util::Str_To_T<int>(tokens[2]);
      ex = x + 1;
      y = Util::Str_To_T<int>(tokens[3]);
      anim_type = Animation::Type::Linear;
      speed = 0;
    }
    offx = offy = 0;
  };

  bool animation;
  Animation::Type anim_type;
  std::string anim_name;
  int x, ex, y;
  int offx, offy;
  float speed;

};

void Images_Player::Refresh_Player_Image_Data() {
  std::string func_name = "Refresh_Plater_Image_Data";
  CSimpleIniA ini;
  ini.SetUnicode();
  if ( ini.LoadFile("data\\player_image_data.ini") < 0 ) {
    AOD::Output("Unable to load file player_image_data.ini @"+func_name);
    return;
  }

  { // ---- nygelstromn ---------------------------
    PlayerSheet& sheet = Images_Player::nygel;
    CSimpleIniA::TNamesDepend props;
    ini.GetAllKeys("nygelstromn", props);
    using PJA = PlayerJointAnimation;
    for ( auto& i : props ) {
      auto data = Animation_Data(ini.GetValue("nygelstromn", i.pItem));
      PlayerJointAnimation* pja_ptr;
      try {
        pja_ptr = &sheet.Str_To_PlayerJointAnimation(data.anim_name);
      } catch ( ... ) {
        AOD::Output("Invalid PlayerJointAnimation name: " + data.anim_name);
        continue;
      }
      pja_ptr->Reconstruct(data.x, data.ex, data.y, Images_Player::nygel.img,
          data.anim_type, {data.offx, data.offy}, data.speed, data.anim_name);
    }
  }
}

void Images_Player::Load() {
  debug     = Load_Image("Images\\psheet\\player_hitbox.png");
  nygel.img = Load_Image("Images\\psheet\\nygelstromn_sheet.png");
  auto& img = nygel.img;
  auto& s = nygel;
  Refresh_Player_Image_Data();
};

void Images_Player::Unload();

// ---- HUD
namespace Images_HUD {

  int font_width[(int)  HUD::Font_Size::size] = { 0 };
  SheetRect hchar[(int)HUD::Font_Size::size]
                 [(int)HUD::Font_Type::size]
                 [(int)HUD::Font_Char::size]  {};
  SheetRect wep  [(int)Weapon_Status::size][(int)Weapon::size] {};
  SheetRect notif[(int)HUD::Notif_Type::size] {};
  SheetRect icon[(int) HUD::Icon_Size::size]
                 [(int)HUD::Icon_Type::size]   {};
  SheetRect accel[(int)HUD::Accel_Type::size] {};
  SheetContainer panel[(int)HUD::Panel_Img_Type::size]
                           [(int)HUD::Panel_Type::size];
}
void Images_HUD::Load() {
  using namespace HUD;
  { // fonts
    auto sc_l = Load_Image("images\\hud\\font_18x12.png"),
         sc_s = Load_Image("images\\hud\\font_10x9.png");

    int w, h;
    Font_Size fsiz;
    SheetContainer* sc;
    auto Set_Char =
      [&](int fchar, int ft, float offx, float offy) {
        hchar[(int)fsiz][ft][fchar] = SheetRect(*sc,
          {offx*w,     offy*h},
          {offx*w + w, offy*h + h});
    };

    int xpos[] = { 13, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0, 1, 2, 3 };

    // ------ 18 x 12 --------
    sc = &sc_l;
    w = 18; h = 12;
    fsiz = Font_Size::large;
    for ( int typ = 0; typ != (int)HUD::Font_Type::size; ++ typ ) {
      for ( int ch = 0; ch != (int)HUD::Font_Char::size; ++ ch ) {
        float offx = xpos[ch],
              offy = typ;
        Set_Char(ch, typ, offx, offy);
      }
    };
    // ----- 10 x 9  ---------
    sc = &sc_s;
    w = 10; h = 9;
    fsiz = Font_Size::small;
    for ( int typ = 0; typ != (int)Font_Type::size; ++ typ ) {
      for ( int ch = 0; ch != (int)Font_Char::size; ++ ch ) {
        float offx = xpos[ch],
              offy = typ;
        Set_Char(ch, typ, offx, offy);
      }
    }
  }

  // for use in weapons and notifs
  auto logos = Load_Image("images\\hud\\logos_17x16.png");
  // weapons
  {
    auto sc_sel = Load_Image("images\\hud\\logos_17x19.png");
    int i;
    // load selected
    for ( int ws = 0; ws != (int)Weapon_Status::size; ++ ws )
    for ( int wt = 0; wt != (int)Weapon::size;        ++ wt ) {
      float offx, offy = 0, h = 16;
      offx = wt;
      SheetContainer* sc = nullptr;
      switch ( (Weapon_Status)ws ) {
        case Weapon_Status::Selected:
          h = 19;
          sc = &sc_sel;
        break;
        case Weapon_Status::Unselected:
          sc = &logos;
        break;
        case Weapon_Status::No_Ammo:
          offy = 1;
          sc = &logos;
        break;
      }
      if ( sc != nullptr ) {
        wep[ws][wt] = SheetRect(*sc, {offx*17,    offy*h},
                                      {offx*17+17, offy*h+h});
      }
    }
  }
  // notifs
  {
    for ( int i = 0; i != (int)HUD::Notif_Type::size; ++ i ) {
      int id = 18+i;
      float offx = int(id%9) * 17,
            offy = int(id/9) * 16;
      notif[i] = SheetRect(logos, {offx, offy}, {offx + 17, offy + 16});
    }
  }
  // icons
  {
    auto sc_l  = Load_Image("images\\hud\\icons_22x22.png"),
         sc_s  = Load_Image("images\\hud\\icons_11x11.png"),
         sc_s2 = Load_Image("images\\hud\\icons_17x16.png");
    int w, h;
    int s;
    SheetContainer* sc;
    auto Set_Icon = [&](Icon_Type x, float offx, float offy) {
      icon[s][(int)x] = SheetRect(*sc,
        {offx*w,     offy*h},
        {offx*w + w, offy*h+h});
    };

    // ----- 22 x 22    ----------------------
    h = w = 22;
    s = (int)HUD::Icon_Size::large;
    sc = &sc_l;

    Set_Icon(Icon_Type::pu_strength,    0, 0);
    Set_Icon(Icon_Type::pu_immort,      0, 1);
    Set_Icon(Icon_Type::pu_vel,         0, 2);
    Set_Icon(Icon_Type::pu_camo,        0, 3);
    Set_Icon(Icon_Type::pu_time,        0, 4);
    Set_Icon(Icon_Type::arm_gold,       1, 0);
    Set_Icon(Icon_Type::arm_silver,     1, 1);
    Set_Icon(Icon_Type::arm_bronze,     1, 2);
    Set_Icon(Icon_Type::arm_nil,        1, 3);
    Set_Icon(Icon_Type::hp_mega       , 2, 0);
    Set_Icon(Icon_Type::hp_nil        , 2, 1);
    Set_Icon(Icon_Type::flag_blu_base , 2, 2);
    Set_Icon(Icon_Type::flag_blu_taken, 2, 3);
    Set_Icon(Icon_Type::team_blue     , 2, 4);
    Set_Icon(Icon_Type::dir_right     , 3, 0);
    Set_Icon(Icon_Type::dir_up        , 3, 1);
    Set_Icon(Icon_Type::flag_red_base , 3, 2);
    Set_Icon(Icon_Type::flag_red_taken, 3, 3);
    Set_Icon(Icon_Type::team_red      , 3, 4);

    // ----- 11 x 11    ----------------------
    h = w = 11;
    s = (int)HUD::Icon_Size::small;
    sc = &sc_s;

    Set_Icon(Icon_Type::pu_strength,    0, 0);
    Set_Icon(Icon_Type::pu_immort,      0, 1);
    Set_Icon(Icon_Type::pu_vel,         0, 2);
    Set_Icon(Icon_Type::pu_camo,        0, 3);
    Set_Icon(Icon_Type::pu_time,        0, 4);
    Set_Icon(Icon_Type::arm_gold,       1, 1);
    Set_Icon(Icon_Type::arm_silver,     1, 2);
    Set_Icon(Icon_Type::arm_bronze,     1, 3);
    Set_Icon(Icon_Type::arm_nil,        1, 4);
    Set_Icon(Icon_Type::hp_mega       , 2, 0);
    Set_Icon(Icon_Type::hp_nil        , 2, 1);

    // ---- 17 x 16   ------------------------
    h = 17;
    w = 16;
    s = (int)HUD::Icon_Size::small;
    sc = &sc_s2;

    Set_Icon(Icon_Type::flag_blu_base , 3, 1);
    Set_Icon(Icon_Type::flag_blu_taken, 3, 0);
    Set_Icon(Icon_Type::team_blue     , 1, 0);
    Set_Icon(Icon_Type::dir_right     , 0, 0);
    Set_Icon(Icon_Type::dir_up        , 0, 1);
    Set_Icon(Icon_Type::flag_red_base , 2, 1);
    Set_Icon(Icon_Type::flag_red_taken, 2, 0);
    Set_Icon(Icon_Type::team_red      , 1, 1);
  }
  // layout
  {
    // --- small
    int p = (int)HUD::Panel_Type::small;
    std::string hs = "images\\hud\\hudsmall\\";
    using PIT = HUD::Panel_Img_Type;
    panel[(int)PIT::stats_bg][p] = Load_Image(hs+"panel1_bg.png");
    panel[(int)PIT::stats_fg][p] = Load_Image(hs+"panel1_fg.png");
    panel[(int)PIT::timer_bg][p] = Load_Image(hs+"panel2_bg.png");
    panel[(int)PIT::timer_fg][p] = Load_Image(hs+"panel2_fg.png");
    panel[(int)PIT::score_bg][p] = Load_Image(hs+"panel3_bg.png");
    panel[(int)PIT::score_fg][p] = Load_Image(hs+"panel3_fg.png");
    panel[(int)PIT::item_bg] [p] = Load_Image(hs+"panel_timer_bg.png");
    panel[(int)PIT::item_fg] [p] = Load_Image(hs+"panel_timer_fg.png");
    // --- large
    p = (int)HUD::Panel_Type::large;
    hs = "images\\hud\\hudlarge\\";
    panel[(int)PIT::stats_bg][p] = Load_Image(hs+"panel1_bg.png");
    panel[(int)PIT::stats_fg][p] = Load_Image(hs+"panel1_fg.png");
    panel[(int)PIT::timer_bg][p] = Load_Image(hs+"panel2_bg.png");
    panel[(int)PIT::timer_fg][p] = Load_Image(hs+"panel2_fg.png");
    panel[(int)PIT::score_bg][p] = Load_Image(hs+"panel3_bg.png");
    panel[(int)PIT::score_fg][p] = Load_Image(hs+"panel3_fg.png");
    panel[(int)PIT::item_bg] [p] = Load_Image(hs+"panel_timer_bg.png");
    panel[(int)PIT::item_fg] [p] = Load_Image(hs+"panel_timer_fg.png");
    // --- race
    p = (int)HUD::Panel_Type::race;
    hs = "images\\hud\\hudrace\\";
    panel[(int)PIT::stats_bg][p] = Load_Image(hs+"panel1_bg.png");
    panel[(int)PIT::stats_fg][p] = Load_Image(hs+"panel1_fg.png");
    panel[(int)PIT::timer_bg][p] = Load_Image(hs+"panel2_bg.png");
    panel[(int)PIT::timer_fg][p] = Load_Image(hs+"panel2_fg.png");
  }

  // misc
  {
    // acceleration
    auto sh = Load_Image("images\\hud\\hudrace\\accel_4x7.png");
    for ( int i = 0; i != (int)HUD::Accel_Type::size; ++ i ) {
      accel[i] = SheetRect(sh, {i*4.f, 0.f}, {i*4.f + 4.f, 0.f});
    }
  }
}

// ---- sounds ----------------------------------------------------------------

// general
const int Sounds::loading_screen_music_size = 5;
Mix_Music* Sounds::loading_screen_music[Sounds::loading_screen_music_size];

void Sounds::Load() {
  loading_screen_music[0] = AOD::Sounds::Load_Music("audio\\Loading01.ogg");
  loading_screen_music[1] = AOD::Sounds::Load_Music("audio\\Loading02.ogg");
  loading_screen_music[2] = AOD::Sounds::Load_Music("audio\\Loading03.ogg");
  loading_screen_music[3] = AOD::Sounds::Load_Music("audio\\Loading04.ogg");
  loading_screen_music[4] = AOD::Sounds::Load_Music("audio\\Loading05.ogg");
}

void Sounds::Unload() {

}

// ---- general ---------------------------------------------------------------

void Load_All_Resources() {
  Images_Misc::Load();
  Images_Player::Load();
  Images_HUD::Load();
  Sounds::Load();
  Load_Hints();
}


std::vector<std::vector<std::string>> loading_hints;
static const int maximum_hint_width = 80;

void Load_Hints() {
  std::ifstream fil("docs\\load_hints.txt");
  loading_hints.push_back(std::vector<std::string>());
  // grab all hints
  while ( fil.good() ) {
    std::string t = "";
    do {
      t += fil.get();
    } while ( fil.good() && fil.peek() != '\n' );

    // remove blank spaces at end
    while ( t[t.size()-1] == ' ' ) t.pop_back();

    // "--" means new hint segment
    if ( t == "--" )
      loading_hints.push_back(std::vector<std::string>());
    else // is a valid hint
      loading_hints[loading_hints.size()-1].push_back(t);


    if ( !fil.good() ) break;
    while ( fil.peek() == '\n' ) fil.get();
  }

  // format hints
  for ( auto& hint : loading_hints ) {
    for ( int i = 0; i != hint.size(); ++ i ) {
      if ( hint[i].size() > maximum_hint_width ) {
        int off = maximum_hint_width;
        // find whitespace
        for ( ; off != -1; -- off ) {
          if ( hint[i][off] == ' ' )
            break;
        }
        ++off;
        if ( off == -1 ) continue;
        hint.insert(hint.begin() + i + 1, hint[i].substr(off));
        hint[i].erase(hint[i].begin() + off, hint[i].end());
      }
    }
  }


  // remove blank hints
  while ( loading_hints[loading_hints.size()-1].size() == 0 )
    loading_hints.pop_back();
}
