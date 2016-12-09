#ifndef HUD_H_
#define HUD_H_
#pragma once

#include <string>

#include "Data.h" 
#include "Weapons.h"
#include "Vector.h"

#undef small

// variables for HUD
namespace HUD {
  // ---- ENUMS ---------------------------------------------------------------
  // ---- fonts ----
  enum class Font_Size { small, large, size };
  enum class Font_Type { purple, blue, gray, gold, silver, bronze, red,
                         green, size };
  enum class Font_Char { d0, d1, d2, d3, d4, d5, d6, d7, d8, d9,
                                   space, dash, dot, colon, size };
  // ---- notif ----
  enum class Notif_Type {
    red_flag_cap, blue_flag_cap, race_comp, race_comp_1st, race_comp_2nd,
    race_comp_3rd,
    
    frag_fetus_sec, frag_fetus_combo, frag_zeus_sec,frag_zeus_combo,
    frag_zeus_combo_2, frag_zeus_combo_3, frag_fists, frag_tele, frag_suicide,
    frag_hazard, frag_tk_red, frag_tk_blue, frag_chat, frag_afk,

    moved_red, moved_blue, moved_spec,

    kicked, banned, disconnect, size
  };
  // ---- icon ----
  enum class Icon_Size { small, large, size };
  enum class Icon_Type {
    pu_strength, pu_immort, pu_vel, pu_camo, pu_time,
    arm_gold, arm_silver, arm_bronze, arm_nil, nil,
    hp_mega, hp_nil,
    flag_blu_base, flag_blu_taken, team_blue,
    dir_right, dir_up,
    flag_red_base, flag_red_taken, team_red,
    size
  };
  // ---- layout ----
  enum class Panel_Img_Type {
    stats_bg, stats_fg, timer_bg, timer_fg,
    score_bg, score_fg, item_bg, item_fg, size
  };

  enum class Panel_Type {
    small, large, race, size
  };
  // ---- etc ----
  enum class Accel_Type {
    pos, nil, neg, size
  };

  // ---- HUD text/font -------------------------------------------------------
  class Text {
    using HUDChar = std::pair<char, AOD::Object*>;
    std::string s_text;
    std::vector<HUDChar> text;
    Font_Size font_size;
    Font_Type font_type;
    // call after set text/pos (repositions objects)
    void Update_Position();
    AOD::Vector pos;
    static Font_Char Char_To_FChar(char t);
    const int layer;
  public:
    Text(int layer);
    ~Text();
    void Set_Format(Font_Size, Font_Type);
    // doesn't change font size/font type
    void Set_Text(const std::string&);
    void Set_Position(const AOD::Vector& vec);

    inline Font_Size R_Ft_Size() const { return font_size; }
    inline Font_Type R_Ft_Type() const { return font_type; }
  };

  // ---- HUD pieces ----------------------------------------------------------
  namespace Pieces {
    class HUD_Piece_Base {
    protected:
      const Panel_Type Type;
      const Icon_Size  Icon_size;
      const Font_Size  Font_size;
    public:
      HUD_Piece_Base(Panel_Type);
    };
    class Item_Timer : HUD_Piece_Base {
      AOD::Object* item_icon, *panel_bg, *panel_fg;
      Text text;
      float timer;
    public:
      Item_Timer(Panel_Type, Icon_Type, int pos);
      ~Item_Timer();
      void Update();
      // returns whether item timer is done
      bool R_Done();
    };

    class Match_Timer : HUD_Piece_Base {
      AOD::Object *panel_bg, *panel_fg;
      Text timer;
      std::string Timer_To_String(const Util::Time&);
    public:
      Match_Timer(Panel_Type);
      ~Match_Timer();
      void Update();
    };

    class Match_Score : HUD_Piece_Base {
      AOD::Object *red_panel_bg, *red_panel_fg, *red_status,
                  *blu_panel_bg, *blu_panel_fg, *blu_status;
      Text red_score, blu_score;
    public:
      Match_Score(Panel_Type);
      ~Match_Score();
      void Update();
      void CTF_Update(bool red, bool in_base);
    };

    namespace Stats {
      class Race : HUD_Piece_Base {
        AOD::Object* panel_bg, *panel_fg, *horiz_vel, *vertic_vel,           
                   * horiz_accel, *vertic_accel;
        AOD::Object* weapons[(int)Weapon::size];
        AOD::Object* vertic_knobs[5],
                   * horiz_knobs[5];
        AOD::Vector prev_vel;
        bool horiz_accel_flip, vertic_accel_flip;
        Text horiz_text, vertic_text;
        static float horiz_damp, vertic_damp;
      public:
        Race();
        ~Race();
        // will only update velocity, need update_weapon for wep
        void Update();
        // since weapons don't update nearly as often as velocity,
        // just trigger this whenever a weapon change occurs
        void Update_Weapon();
        static void Set_Horiz_Damp(float );
        static void Set_Vertic_Damp(float );
        static float R_Horiz_Damp();
        static float R_Vertic_Damp();
      };

      class Regular : HUD_Piece_Base {
      public:
        Regular(Panel_Type);
        ~Regular();
        AOD::Object *panel_bg, *panel_fg, *health_icon, *armour_icon;
        AOD::Object *weapons[(int)Weapon::size];
        Text health_text, ammo_text, armour_text;
        void Update();
        // call after player switches, drops or gets weapon
        void Update_Weapon();
        // call after player's mega status is changed
        void Update_Health();
        // call after armour status is changed
        void Update_Armour();
      };

      union Stats {
        Race* race;
        Regular* regular;
      };
    }
  }

  // ---- HUD notifications ---------------------------------------------------
  class Notif {
    AOD::Text* left_text, *right_text;
    AOD::Object* icon;
  public:
    Notif(std::string left_text, std::string right_text, Notif_Type, int pos);
    ~Notif();
    // pushes position up
    void Push();
  };

  class Notifications {
    Util::Linked_List<Notif> notifs;
    float timer;
    void Push_Notif_Pos();
  public:
    Notifications();
    ~Notifications();
    
    void Clear();
    void Push_Back(const Notif& notif);
    void Update();
  };

  // ---- HUD -----------------------------------------------------------------
  class HUDMain {
    Panel_Type type;
    std::vector<Pieces::Item_Timer*> item_timer;
    Pieces::Match_Score match_score;
    Pieces::Match_Timer match_timer;
    Pieces::Stats::Stats stats;
    Notifications notifs;
    void Reset();
  public:
    HUDMain(Panel_Type);
    ~HUDMain();

    void Change_HUD(Panel_Type);
    void Update();
  };


  namespace Layout {
    extern AOD::Vector notif_icon, notif_text_left, notif_text_right,
                                                           notif_gap,

    race_time_text, race_time_panel, race_stats_panel, race_vel_text_x,
    race_vel_text_y, race_vel_dir_x, race_vel_dir_y, race_acc_dir_x,
    race_acc_dir_y, race_acc_ico_horiz, race_acc_ico_vertic, race_acc_knob_gap,
    race_wep_start, race_wep_gap,
    
    large_time_text, large_time_panel, large_score_panel_blu,
    large_score_panel_red, large_score_text_blu, large_score_text_red,
    large_score_icon_red, large_score_icon_blu, large_item_panel,
    large_item_icon, large_item_text, large_item_gap, large_stats_panel,
    large_ammo_text, large_wep_start, large_wep_gap, large_health_icon,
    large_health_text, large_armour_text, large_armour_icon,
    
    small_time_text, small_time_panel, small_score_panel_blu,
    small_score_panel_red, small_score_text_blu, small_score_text_red,
    small_score_icon_red, small_score_icon_blu, small_item_panel,
    small_item_icon, small_item_text, small_item_gap, small_stats_panel,
    small_ammo_text, small_wep_start, small_wep_gap, small_health_icon,
    small_health_text, small_armour_text, small_armour_icon;
  }
}
#endif