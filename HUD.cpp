#include "AOD.h"
#include "Game_Manager.h"
#include "HUD.h"
#include "Data.h"
#include "Images.h"
#include "Item.h"
#include "Item_Armour.h"
#include "Item_Health.h"
#include "Item_Powerup.h"
#include "Player.h"

// ---- TEXT ------------------------------------------------------------------

using HText = HUD::Text;

HText::Text(int l) : layer(l) {
  s_text = "";
  text.clear();
  pos = {-250, -250};
}
HText::~Text() {
  for ( auto i : text )
    if ( i.second ) AOD::Remove(i.second);
}

void HText::Set_Position(const AOD::Vector& vec) {
  pos = vec;
}
void HText::Set_Text(const std::string& str) {
  if ( str.size() < text.size() ) {
    for ( auto &i : text )
      AOD::Remove(i.second);
    text.clear();
  }
  for ( int i = text.size(); i < str.size(); ++ i ) { // pad space
    HUDChar t;
    t.first = ' ';
    t.second = new AOD::Object();
    AOD::Add(t.second, layer);
    t.second->Set_Is_Static_Pos(1);
    text.push_back(t);
  }
  for ( int i = 0; i != str.size(); ++ i ) { // set chars
    if ( text[i].first != str[i] ) { // no change in char?
      text[i].first = str[i];
      Font_Char ch = Char_To_FChar(str[i]);
      text[i].second->Set_Sprite(
        Images_HUD::hchar[(int)font_size][(int)font_type][(int)ch]);
    }
  }
  Update_Position();
}
void HText::Set_Format(HUD::Font_Size fs, HUD::Font_Type ft) {
  font_size = fs; font_type = ft;
  for ( auto& i : text ) {
    Font_Char fc = Char_To_FChar(i.first);
    i.second->Set_Sprite(Images_HUD::hchar[(int)fs][(int)ft][(int)fc]);
  }
}
void HText::Update_Position() {
  for ( int i = 0; i != text.size(); ++ i ) {
    // different width based on font size, ignore y as it's centered anyways
    text[i].second->Set_Position(
      {pos.x + i * (font_size == Font_Size::large?18:10), pos.y} );
  }
}
HUD::Font_Char HText::Char_To_FChar(char t) {
  Font_Char ch = Font_Char::space;
  switch ( t ) {
    case '.': ch = Font_Char::dot;   break;
    case '-': ch = Font_Char::dash;  break;
    case ':': ch = Font_Char::colon; break;
    default:
      if ( t >= '0' && t <= '9' ) {
        ch = (Font_Char)((int)Font_Char::d0 + (t - '0'));
      }
  }
  return ch;
}

// ---- Utility ---------------------------------------------------------------

AOD::Vector Center_Text(const AOD::Vector& position, const std::string& str,
                              bool large, int csiz = 3) {
  return (position + AOD::Vector((large?9:5)*(csiz-str.size()),0));
}

// ---- HUD pieces ------------------------------------------------------------

// -- base --
using HPBase = HUD::Pieces::HUD_Piece_Base;

HPBase::HUD_Piece_Base(HUD::Panel_Type t) :
  Type(t),
  Icon_size( (t == Panel_Type::large || t == Panel_Type::race ?
              Icon_Size::large : Icon_Size::small)),
  Font_size( (t == Panel_Type::large || t == Panel_Type::race ?
              Font_Size::large : Font_Size::small) )
{}

// -- item timer --
using ITimer = HUD::Pieces::Item_Timer;

unsigned const int IT_layer_item_icon = 102,
                   IT_layer_panel_bg  = 100,
                   IT_layer_panel_fg  = 101,
                   IT_layer_text      = 102;

ITimer::Item_Timer(HUD::Panel_Type pt, HUD::Icon_Type it, int pos) :
                     HUD_Piece_Base(pt), text(102) {
  item_icon = new AOD::Object();
  panel_bg  = new AOD::Object();
  panel_fg  = new AOD::Object();
  AOD::Add(item_icon, IT_layer_item_icon);
  AOD::Add(panel_bg , IT_layer_panel_bg);
  AOD::Add(panel_fg , IT_layer_panel_fg);

  item_icon->Set_Sprite(Images_HUD::icon[(int)Type][(int)it]);
  panel_bg->Set_Sprite (
    Images_HUD::panel[(int)Panel_Img_Type::item_bg][(int)pt]);
  panel_fg->Set_Sprite (
    Images_HUD::panel[(int)Panel_Img_Type::item_fg][(int)pt]);
  item_icon->Set_Is_Static_Pos(1);
  panel_bg->Set_Is_Static_Pos(1);
  panel_fg->Set_Is_Static_Pos(1);
  switch ( Type ) {
    case Panel_Type::large:
      item_icon->Set_Position(Layout::large_item_icon  +
                              Layout::large_item_gap*pos);
      panel_bg->Set_Position (Layout::large_item_panel +
                              Layout::large_item_gap*pos);
      panel_fg->Set_Position (panel_bg->R_Position());
      text.Set_Position      (Layout::large_item_text  +
                              Layout::large_item_gap*pos);
    break;
    case Panel_Type::small:
      item_icon->Set_Position(Layout::small_item_icon  +
                              Layout::small_item_gap*pos);
      panel_bg->Set_Position (Layout::small_item_panel +
                              Layout::small_item_gap*pos);
      panel_fg->Set_Position (panel_bg->R_Position());
      text.Set_Position      (Layout::small_item_text  +
                              Layout::small_item_gap*pos);
    break;
    // -- no item timer on race --
  }
  switch ( it ) {
    case Icon_Type::arm_bronze:
      timer = Arm_Item_Bronze::Resp_time;
      text.Set_Format(Font_size, Arm_Item_Bronze::Font_type);
    break;
    case Icon_Type::arm_gold:
      timer = Arm_Item_Gold::Resp_time;
      text.Set_Format(Font_size, Arm_Item_Gold::Font_type);
    break;
    case Icon_Type::arm_silver:
      timer = Arm_Item_Silver::Resp_time;
      text.Set_Format(Font_size, Arm_Item_Silver::Font_type);
    break;
    case Icon_Type::hp_mega:
      timer = HP_Item_Mega::Resp_time;
      text.Set_Format(Font_size, Arm_Item_Gold::Font_type);
    break;
    case Icon_Type::pu_camo:
      timer = Powerup_Item_Camouflage::Resp_time;
      text.Set_Format(Font_size, Powerup_Item_Camouflage::Font_type);
    break;
    case Icon_Type::pu_immort:
      timer = Powerup_Item_Immortality::Resp_time;
      text.Set_Format(Font_size, Powerup_Item_Immortality::Font_type);
    break;
    case Icon_Type::pu_strength:
      timer = Powerup_Item_Strength::Resp_time;
      text.Set_Format(Font_size, Powerup_Item_Strength::Font_type);
    break;
    case Icon_Type::pu_time:
      timer = Powerup_Item_Time::Resp_time;
      text.Set_Format(Font_size, Powerup_Item_Time::Font_type);
    break;
    case Icon_Type::pu_vel:
      timer = Powerup_Item_Velocity::Resp_time;
      text.Set_Format(Font_size, Powerup_Item_Velocity::Font_type);
    break;
  }
  timer += AOD::R_MS();
  Update(); // set actual text
}
ITimer::~Item_Timer() {
  AOD::Remove(item_icon);
  AOD::Remove(panel_bg);
  AOD::Remove(panel_fg);
}

void ITimer::Update() {
  timer -= AOD::R_MS();
  if ( timer > 0 )
    text.Set_Text(std::to_string(timer/1000));
  else
    text.Set_Text("0");
}
bool ITimer::R_Done() { return timer <= 0; }

// -- match timer --
using MTimer = HUD::Pieces::Match_Timer;

unsigned const int MT_layer_panel_bg = 102,
                   MT_layer_panel_fg = 103,
                   MT_layer_text     = 104;

MTimer::Match_Timer(HUD::Panel_Type pt) :
                      HUD_Piece_Base(pt), timer(MT_layer_text) {
  panel_bg = new AOD::Object();
  panel_fg = new AOD::Object();
  AOD::Add(panel_bg, MT_layer_panel_bg);
  AOD::Add(panel_fg, MT_layer_panel_fg);
  panel_bg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::timer_bg][(int)pt]);
  panel_fg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::timer_fg][(int)pt]);
  panel_bg->Set_Is_Static_Pos(1);
  panel_fg->Set_Is_Static_Pos(1);
  // -- sprites/position --
  switch ( pt ) {
    case Panel_Type::large:
      timer.Set_Position(Layout::large_time_text);
      panel_bg->Set_Position(Layout::large_time_panel);
      panel_fg->Set_Position(panel_bg->R_Position());
    break;
    case Panel_Type::small:
      timer.Set_Position(Layout::small_time_text);
      panel_bg->Set_Position(Layout::small_time_panel);
      panel_fg->Set_Position(panel_bg->R_Position());      
    break;
    case Panel_Type::race:
      timer.Set_Position(Layout::race_time_text);
      panel_bg->Set_Position(Layout::race_time_panel);
      panel_fg->Set_Position(panel_bg->R_Position());
    break;
  }
  // -- text --
  timer.Set_Format(Font_size, Font_Type::gray);
  Update();
}
MTimer::~Match_Timer() {
  AOD::Remove(panel_bg);
  AOD::Remove(panel_fg);
}

void MTimer::Update() {
  const auto& time = game_manager->R_Match_Time();
  /* AOD::Output("3333: " + std::to_string(time.R_Seconds())); */
  timer.Set_Text(Timer_To_String(game_manager->R_Match_Time()));
}
std::string MTimer::Timer_To_String(const Util::Time& t) {
  int hour = t.R_Hours(),
      min  = t.R_Minutes(),
      sec  = t.R_Seconds(),
      mill = t.R_Milliseconds();
  std::string tim;
  // get formatting type & format
  switch ( Type ) {
    case Panel_Type::race:
      // no hours
      min += hour*60;
      // minutes
      if ( min > 99 ) { min = 99; sec = 99; mill = 999; }
      if ( min < 10 ) tim += '0';
      tim += std::to_string(min) + ':';
      // seconds
      if ( sec < 10 ) tim += '0';
      tim += std::to_string(sec) + '.';
      // milliseconds
      if ( mill < 100 ) {
        tim += '0';
        if ( mill < 10 ) tim += '0';
      }
      tim += std::to_string(mill);
    break;
    case Panel_Type::large: case Panel_Type::small:
      // no hours
      min += hour*60;
      // minutes
      if ( min > 99 ) { min = 99; sec = 99; }
      if ( min < 10 ) tim += '0';
      tim += std::to_string(min) + ':';
      // secs
      if ( sec < 10 ) tim += '0';
      tim += std::to_string(sec);
    break;
  }
  return tim;
}

// -- match score --
using MScore = HUD::Pieces::Match_Score;

unsigned const int MS_layer_red_panel_bg = 100,
                   MS_layer_red_panel_fg = 101,
                   MS_layer_red_status   = 102,
                   MS_layer_blu_panel_fg = 100,
                   MS_layer_blu_panel_bg = 101,
                   MS_layer_blu_status   = 102,
                   MS_layer_text         = 102;

MScore::Match_Score(HUD::Panel_Type pt) :
              HUD_Piece_Base(pt),
              blu_score(MS_layer_text), red_score(MS_layer_text) {
  if ( pt == Panel_Type::race ) { // nothing
    blu_score.Set_Text("");
    red_score.Set_Text("");
    return;
  }
  red_panel_bg = new AOD::Object();
  red_panel_fg = new AOD::Object();
  red_status   = new AOD::Object();
  blu_panel_bg = new AOD::Object();
  blu_panel_fg = new AOD::Object();
  blu_status   = new AOD::Object();

  AOD::Add(red_panel_bg, MS_layer_red_panel_bg);
  AOD::Add(red_panel_fg, MS_layer_red_panel_fg);
  AOD::Add(red_status  , MS_layer_red_status  );
  AOD::Add(blu_panel_fg, MS_layer_blu_panel_bg);
  AOD::Add(blu_panel_bg, MS_layer_blu_panel_fg);
  AOD::Add(blu_status  , MS_layer_blu_status  );

  red_panel_bg->Set_Is_Static_Pos(1);
  red_panel_fg->Set_Is_Static_Pos(1);
  red_status  ->Set_Is_Static_Pos(1);
  blu_panel_bg->Set_Is_Static_Pos(1);
  blu_panel_fg->Set_Is_Static_Pos(1);
  blu_status->Set_Is_Static_Pos(1);

  // --- set images
  red_panel_bg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::score_bg][(int)pt]);
  red_panel_fg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::score_fg][(int)pt]);
  red_status  ->Set_Sprite(
    Images_HUD::icon[(int)Icon_size][(int)Icon_Type::team_red]);
  blu_panel_bg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::score_bg][(int)pt]);
  blu_panel_bg->Flip_X();
  blu_panel_fg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::score_fg][(int)pt]);
  blu_panel_fg->Flip_X();
  blu_status  ->Set_Sprite(
    Images_HUD::icon[(int)Icon_size][(int)Icon_Type::team_blue]);

  // -- set positions
  switch ( pt ) {
    case Panel_Type::large:
      red_panel_bg->Set_Position(Layout::large_score_panel_red);
      red_panel_fg->Set_Position(Layout::large_score_panel_red);
      blu_panel_bg->Set_Position(Layout::large_score_panel_blu);
      blu_panel_fg->Set_Position(Layout::large_score_panel_blu);
      red_status->Set_Position(  Layout::large_score_icon_red);
      blu_status->Set_Position(  Layout::large_score_icon_blu);
      red_score.Set_Position(    Layout::large_score_text_red);
      blu_score.Set_Position(    Layout::large_score_text_blu);
    break;
    case Panel_Type::small:
      red_panel_bg->Set_Position(Layout::small_score_panel_red);
      red_panel_fg->Set_Position(Layout::small_score_panel_red);
      blu_panel_bg->Set_Position(Layout::small_score_panel_blu);
      blu_panel_fg->Set_Position(Layout::small_score_panel_blu);
      red_status->Set_Position(  Layout::small_score_icon_red);
      blu_status->Set_Position(  Layout::small_score_icon_blu);
      red_score.Set_Position(    Layout::small_score_text_red);
      blu_score.Set_Position(    Layout::small_score_text_blu);       
    break;
    // -- no race --
  };

  // -- set text
  red_score.Set_Format(Font_size, Font_Type::red);
  blu_score.Set_Format(Font_size, Font_Type::blue);
  
  Update();
}
MScore::~Match_Score() {
  AOD::Remove(red_panel_bg);
  AOD::Remove(red_panel_fg);
  AOD::Remove(red_status  );
  AOD::Remove(blu_panel_bg);
  AOD::Remove(blu_panel_fg);
  AOD::Remove(blu_status  );
}

void MScore::Update() {
  if ( Type == Panel_Type::race ) return;
  const Match* M = game_manager->R_Match();
  bool large = Font_size == Font_Size::large;
  if ( M ) {
    // reset position
    std::string str = std::to_string(M->R_Red_Score());
    red_score.Set_Position(
      Center_Text((large?Layout::large_score_text_red :
                         Layout::small_score_text_red), str, large, 2));
    str = std::to_string(M->R_Blu_Score());
    blu_score.Set_Position(
      Center_Text((large?Layout::large_score_text_blu :
                         Layout::small_score_text_blu), str, large, 2));
    // reset text
    red_score.Set_Text(std::to_string(M->R_Red_Score()));
    blu_score.Set_Text(std::to_string(M->R_Blu_Score()));
  } else {
    red_score.Set_Text("0");
    blu_score.Set_Text("0");
  }
}
void MScore::CTF_Update(bool red, bool in_base) {
  red_status->Set_Sprite( Images_HUD::icon[(int)(Icon_size)]
    [red ?
      (int)(in_base?Icon_Type::flag_red_base : Icon_Type::flag_red_taken) :
      (int)(in_base?Icon_Type::flag_blu_base : Icon_Type::flag_blu_taken)]);
}

// -- stats: race --
using SRace = HUD::Pieces::Stats::Race;

unsigned const int SRace_layer_panel_bg = 100,
                   SRace_layer_panel_fg = 101,
                   SRace_layer_icon     = 102,
                   SRace_layer_text     = 102,
                   SRace_layer_knob     = 102;

float SRace::horiz_damp = 1.0, SRace::vertic_damp = 1.0;

SRace::Race() : HUD_Piece_Base(HUD::Panel_Type::race),
                horiz_text(SRace_layer_text), vertic_text(SRace_layer_text) {
  panel_bg     = new AOD::Object();
  panel_fg     = new AOD::Object();
  horiz_vel    = new AOD::Object();
  vertic_vel   = new AOD::Object();
  horiz_accel  = new AOD::Object();
  vertic_accel = new AOD::Object();

  AOD::Add(panel_bg    , SRace_layer_panel_bg);
  AOD::Add(panel_fg    , SRace_layer_panel_fg);
  AOD::Add(horiz_vel   , SRace_layer_icon);
  AOD::Add(vertic_vel  , SRace_layer_icon);
  AOD::Add(horiz_accel , SRace_layer_icon);
  AOD::Add(vertic_accel, SRace_layer_icon);

  panel_bg    ->Set_Is_Static_Pos(1);
  panel_fg    ->Set_Is_Static_Pos(1);
  horiz_vel   ->Set_Is_Static_Pos(1);
  vertic_vel  ->Set_Is_Static_Pos(1);
  horiz_accel ->Set_Is_Static_Pos(1);
  vertic_accel->Set_Is_Static_Pos(1);

  panel_bg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::stats_bg][(int)Panel_Type::race]);
  panel_fg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::stats_fg][(int)Panel_Type::race]);
  panel_bg->Set_Position(Layout::race_stats_panel);
  panel_fg->Set_Position(Layout::race_stats_panel);

  horiz_text.Set_Position(Layout::race_vel_text_x);
  vertic_text.Set_Position(Layout::race_vel_text_y);
  horiz_text.Set_Format(Font_Size::large, Font_Type::green);
  vertic_text.Set_Format(Font_Size::large, Font_Type::purple);

  // ---- knobs / icons -------
  horiz_accel_flip = vertic_accel_flip = 0;
  for ( int i = 0; i != (int)Weapon::size; ++ i )
    weapons[i] = nullptr;

  prev_vel = {0,0};
  for ( int i = 0; i != 5; ++ i ) {
    horiz_knobs [i] = nullptr;
    vertic_knobs[i] = nullptr;
  }
};
SRace::~Race() {
  AOD::Remove(panel_bg    );
  AOD::Remove(panel_fg    );
  AOD::Remove(horiz_vel   );
  AOD::Remove(vertic_vel  );
  AOD::Remove(horiz_accel );
  AOD::Remove(vertic_accel);

  for ( int i = 0; i != (int)Weapon::size; ++ i ) {
    if ( weapons[i] )
      AOD::Remove(weapons[i]);
  }

  for ( int i = 0; i != 5; ++ i ) {
    if ( horiz_knobs[i] != nullptr )
      AOD::Remove(horiz_knobs[i]);
    if ( vertic_knobs[i] != nullptr )
      AOD::Remove(vertic_knobs[i]);
  }
}

// using this method, if velocity hasn't changed, very little
// computation should even occur.
void SRace::Update() {
  if ( !game_manager->c_player->pl_handle ) return;
  auto& vel = game_manager->c_player->pl_handle->R_Velocity();
  vel *= 1000/AOD::R_MS();

  if ( !game_manager->c_player->pl_handle->R_In_Air() )
    vel.y = 0;
  // ---- bad, temporary. --- FIX ME ----------
  if ( vel.y < 30 && vel.y > 0 ) vel.y = 0;

  static std::vector<float> velx_avgvec;
  velx_avgvec.push_back(vel.x);
  if ( velx_avgvec.size() >= 15 )
    velx_avgvec.erase(velx_avgvec.begin());
  
  float velx_avg = 0;
  for ( const auto& i : velx_avgvec )
    velx_avg += i;
  velx_avg /= velx_avgvec.size();

  // --- set text --------
  // -- horiz
  std::string str = std::to_string((int)abs(vel.x));
  while ( str.size() < 4 ) str.insert(str.begin(), ' ');
  horiz_text.Set_Text(str);
  //horiz_text.Set_Position(Center_Text(Layout::race_vel_text_x, str, 1, 4));
  /*if ( vel.x < 0 && horiz_text.R_Ft_Type() != Font_Type::purple )
    horiz_text.Set_Format(Font_Size::large, Font_Type::purple);
  if ( vel.x > 0 && horiz_text.R_Ft_Type() != Font_Type::green )
    horiz_text.Set_Format(Font_Size::large, Font_Type::green);
  horiz_text.Set_Text (std::to_string((int)abs(vel.x)));*/
  // -- vertic
  str = std::to_string((int)abs(vel.y));
  while ( str.size() < 4 ) str.insert(str.begin(), ' ');
  //vertic_text.Set_Position(Center_Text(Layout::race_vel_text_y, str, 1, 4));
  vertic_text.Set_Text(str);
  
  /*
  AOD::Vector paccel = vel - prev_vel;
  prev_vel = vel;
  // -- set knobs --
  // apply damp
  paccel.x *= horiz_damp;
  paccel.y *= vertic_damp;
  int x = paccel.x, y = paccel.y;
  // set limits
  if ( x > 5 ) x =  5; if ( x < -5 ) x = -5;
  if ( y > 5 ) y =  5; if ( y < -5 ) y = -5;
  AOD::Output(std::to_string(x) + ", " + std::to_string(y));
  // pad space & set position
  for ( int i = 0; i != abs(x); ++ i ) {
    if ( horiz_knobs[i] == nullptr ) {
      horiz_knobs[i] = new AOD::Object();
      AOD::Add(horiz_knobs[i]);
      horiz_knobs[i]->Set_Sprite(Images_HUD::accel[(int)
        (x>0? Accel_Type::pos : Accel_Type::neg)]);
    }
    horiz_knobs[i]->Set_Position(Layout::race_acc_ico_horiz +
      (Layout::race_acc_knob_gap*i));
  }
  for ( int i = 0; i != abs(y); ++ i ) {
    if ( vertic_knobs[i] == nullptr ) {
      vertic_knobs[i] = new AOD::Object();
      AOD::Add(vertic_knobs[i]);
      vertic_knobs[i]->Set_Sprite(Images_HUD::accel[(int)
        (y>0? Accel_Type::pos : Accel_Type::neg)]);
    }
    vertic_knobs[i]->Set_Position({0.f, 0.f + 0.f*y});
    vertic_knobs[i]->Set_Position(Layout::race_acc_ico_vertic +
      (Layout::race_acc_knob_gap*i));
  }
  // rem space
  for ( int i = abs(x); i != 5; ++ i ) {
    if ( horiz_knobs[i] != nullptr ) {
      AOD::Remove(horiz_knobs[i]);
      horiz_knobs[i] = nullptr;
    }
  }
  for ( int i = abs(y); i != 5; ++ i ) {
    if ( vertic_knobs[i] != nullptr ) {
      AOD::Remove(vertic_knobs[i]);
      vertic_knobs[i] = nullptr;
    }
  }
  // -- set text & horiz/vertic vel dirs --
  if ( vel.x > 0 && prev_vel.x < 0 ) { // going positive
    horiz_text.Set_Format(Font_Size::large, Font_Type::green);
    horiz_vel->Flip_X();
  }
  if ( vel.x < 0 && prev_vel.x > 0 ) { // going negative
    horiz_text.Set_Format(Font_Size::large, Font_Type::purple);
    horiz_vel->Flip_X();
  }
  if ( vel.y > 0 && prev_vel.y < 0 ) { // going positive
    vertic_text.Set_Format(Font_Size::large, Font_Type::green);
    vertic_vel->Flip_X();
  }
  if ( vel.y < 0 && prev_vel.y > 0 ) { // going negative
    vertic_text.Set_Format(Font_Size::large, Font_Type::purple);
    vertic_vel->Flip_X();
  }
  // -- set horiz/vertic accel dirs (TODO) --
  if (( horiz_accel_flip && paccel.x > 0) ||
      (!horiz_accel_flip && paccel.x < 0)) {
    horiz_accel->Flip_X();
    horiz_accel_flip ^= 1;
  }
  if (( vertic_accel_flip && paccel.y > 0) ||
      (!vertic_accel_flip && paccel.y < 0)) {
    vertic_accel->Flip_Y();
    vertic_accel_flip ^= 1;
  }  */  
}
void SRace::Update_Weapon() {
  Player* pl = game_manager->c_player->pl_handle;
  if ( pl == nullptr ) return;
  //Layout::small_wep_gap
  auto New_Wep = [&](int i) {
    weapons[i] = new AOD::Object();
    AOD::Add(weapons[i]);
    weapons[i]->Set_Position(Layout::race_wep_start);
    AOD::Vector wrap = {(float)((int)i % 4), (float)((int)i / 4)};
    weapons[i]->Add_Position(Layout::race_wep_gap * wrap);
  };

  for ( int i = 0; i != (int)Weapon::size; ++ i ) {
    Weapon_Status ws = pl->R_Weapon_Status((Weapon)i);
    switch ( ws ) {
      case Weapon_Status::No_Weapon: // only one that removes image
        if ( weapons[i] != nullptr ) {
          AOD::Remove(weapons[i]);
          weapons[i] = nullptr;
        }
      continue;
      default: // adds/sets image no matter status
        if ( weapons[i] == nullptr ) New_Wep(i);
        weapons[i]->Set_Sprite(Images_HUD::wep[(int)ws][i]);
      break;
    }
  }
}

// -- stats: regular --
using SReg = HUD::Pieces::Stats::Regular;

unsigned const int SReg_layer_panel_bg    = 100,
                   SReg_layer_panel_fg    = 101,
                   SReg_layer_icon        = 102,
                   SReg_layer_text        = 102;

SReg::Regular(HUD::Panel_Type pt) :
      HUD_Piece_Base(pt),
      health_text(SReg_layer_text), ammo_text(SReg_layer_text),
      armour_text(SReg_layer_text) {
  panel_bg    = new AOD::Object();
  panel_fg    = new AOD::Object();
  health_icon = new AOD::Object();
  armour_icon = new AOD::Object();

  AOD::Add(panel_bg   , SReg_layer_panel_bg   );
  AOD::Add(panel_fg   , SReg_layer_panel_fg   );
  AOD::Add(health_icon, SReg_layer_icon);
  AOD::Add(armour_icon, SReg_layer_icon);

  for ( int i = 0; i != (int)Weapon::size; ++ i ) {
    weapons[i] = new AOD::Object();
    weapons[i]->Set_Sprite(
      Images_HUD::wep[(int)Weapon_Status::No_Weapon][i]);
    weapons[i]->Set_Is_Static_Pos(1);
    if ( Type == Panel_Type::small )
      weapons[i]->Set_Position(
        Layout::small_wep_start + Layout::small_wep_gap*i);
    else
      weapons[i]->Set_Position(
        Layout::large_wep_start + Layout::large_wep_gap*i);
    AOD::Add(weapons[i], SReg_layer_icon);
  }

  panel_bg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::stats_bg][(int)Type]);
  panel_fg->Set_Sprite(
    Images_HUD::panel[(int)Panel_Img_Type::stats_fg][(int)Type]);
  health_icon->Set_Sprite(
    Images_HUD::icon[(int)Icon_Size::large][(int)Icon_Type::hp_nil]);
  armour_icon->Set_Sprite(
    Images_HUD::icon[(int)Icon_Size::large][(int)Icon_Type::arm_nil]);

  panel_bg->Set_Is_Static_Pos(1);
  panel_fg->Set_Is_Static_Pos(1);
  health_icon->Set_Is_Static_Pos(1);
  armour_icon->Set_Is_Static_Pos(1);

  if ( Type == Panel_Type::small ) {
    panel_bg->Set_Position   (Layout::small_stats_panel);
    panel_fg->Set_Position   (Layout::small_stats_panel);
    health_icon->Set_Position(Layout::small_health_icon);
    armour_icon->Set_Position(Layout::small_armour_icon);
  } else {
    panel_bg->Set_Position(   Layout::large_stats_panel);
    panel_fg->Set_Position(   Layout::large_stats_panel);
    health_icon->Set_Position(Layout::large_health_icon);
    armour_icon->Set_Position(Layout::large_armour_icon);
  }
  
  health_text.Set_Format(Font_Size::large, Font_Type::blue);
  armour_text.Set_Format(Font_Size::large, Font_Type::blue);
  ammo_text  .Set_Format(Font_Size::large, Font_Type::gray);

  Update();
}
SReg::~Regular() {
  AOD::Remove(panel_bg   );
  AOD::Remove(panel_fg   );
  AOD::Remove(health_icon);
  AOD::Remove(armour_icon);

  for ( int i = 0; i != (int)Weapon::size; ++ i ) {
    if ( weapons[i] )
      AOD::Remove(weapons[i]);
  }
}

static void SReg_Text_Update(HUD::Text& text, bool large, int t,
                   const AOD::Vector& lpos, const AOD::Vector& spos) {
  using namespace HUD;
  std::string str = std::to_string(t);
  text.Set_Position(Center_Text( (large?lpos : spos),
                                  std::to_string(t), 1));
  text.Set_Text(str);
}

void SReg::Update() {
  // update health, armour and ammo
  if ( game_manager->c_player->pl_handle == nullptr ) return;
  bool large = (Font_size == Font_Size::large);
  const auto& pl_handle = game_manager->c_player->pl_handle;
  // --- health
  SReg_Text_Update(health_text, large, pl_handle->R_Health(),
          Layout::large_health_text, Layout::small_health_text);
  // --- armour
  SReg_Text_Update(armour_text, large, pl_handle->R_Armour(),
          Layout::large_armour_text, Layout::small_armour_text);
  // --- ammo
  SReg_Text_Update(ammo_text, large, pl_handle->R_Armour(),
          Layout::large_ammo_text, Layout::small_ammo_text);
  
  // --- update icons ---
  Update_Weapon();
  Update_Health();
  Update_Armour();
}
void SReg::Update_Weapon() {
  Player* pl = game_manager->c_player->pl_handle;
  if ( pl == nullptr ) return;

  for ( int i = 0; i != (int)Weapon::size; ++ i ) {
    Weapon_Status ws = pl->R_Weapon_Status((Weapon)i);
    weapons[i]->Set_Sprite(Images_HUD::wep[(int)ws][i]);
  }
}
void SReg::Update_Health() {
  Player* pl = game_manager->c_player->pl_handle;
  if ( pl == nullptr ) return;
  health_icon->Set_Sprite(Images_HUD::icon[(int)Icon_Size::large]
    [(int)(pl->R_Mega_Stack() > 0 ? Icon_Type::hp_mega : Icon_Type::hp_nil)]);
}
void SReg::Update_Armour() {
  Player* pl = game_manager->c_player->pl_handle;
  if ( pl == nullptr ) {
    armour_icon->Set_Sprite(Images_HUD::icon[(int)Icon_Size::large]
                                            [(int)Icon_Type::arm_bronze]);
  }
  Icon_Type typ;
  switch ( pl->R_Armour_Type() ) {
    case Item_Type::arm_bronze: typ = Icon_Type::arm_bronze; break;
    case Item_Type::arm_silver: typ = Icon_Type::arm_silver; break;
    case Item_Type::arm_gold  : typ = Icon_Type::arm_gold  ; break;
    default: typ = Icon_Type::arm_nil; break;
  }
  armour_icon->Set_Sprite(Images_HUD::icon[(int)Icon_Size::large][(int)typ]);
}

// ---- HUD notifications -----------------------------------------------------

// hud notifs
using HNotif = HUD::Notif;

unsigned const int HNotif_icon_layer = 100;

HNotif::Notif(std::string lstr, std::string rstr,
                  HUD::Notif_Type type, int pos) {
  left_text = new AOD::Text (Layout::notif_text_left, lstr);
  right_text = new AOD::Text(Layout::notif_text_right, rstr);
  
  AOD::Add(left_text );
  AOD::Add(right_text);

  icon = new AOD::Object();
  AOD::Add(icon, HNotif_icon_layer);
  icon->Set_Sprite(Images_HUD::notif[(int)type]);
  icon->Set_Position(Layout::notif_icon + Layout::notif_gap*pos);
};
HNotif::~Notif() {
  AOD::Remove(left_text);
  AOD::Remove(right_text);
  AOD::Remove(icon);
}
void HNotif::Push() {
  // push position up
  left_text->Set_Position(left_text->R_Position() + Layout::notif_gap);
  right_text->Set_Position(right_text->R_Position() + Layout::notif_gap);
  icon->Add_Position(Layout::notif_gap);
}

// hud notifications
using HNotific = HUD::Notifications;

HNotific::Notifications() {
  timer = -1;
}
HNotific::~Notifications() {
  Clear();
}
void HNotific::Clear() {
  while ( true ) {
    Notif* t = notifs.Pop();
    if ( t == nullptr ) break;
    delete t;
  }
}
void HNotific::Push_Back(const HUD::Notif& notif) {
  Push_Notif_Pos();
  
  // push notif on ll
  notifs.Push(new Notif( notif ));
  if ( notifs.R_Size() > 5 )
    delete notifs.Pop();
  timer = 5500;
}
void HNotific::Update() {
  if ( timer <= 0 && notifs.R_Size() > 0 ) {
    timer = 5500;
    delete notifs.Pop();
    Push_Notif_Pos();
  }
}
void HNotific::Push_Notif_Pos() {
  auto node = notifs.R_Head();
  // loop through nodes and apply push (to pos)
  while ( node != nullptr ) {
    node->data->Push();
    node = node->next;
  }
}

// ---- HUD -------------------------------------------------------------------

using _HUD = HUD::HUDMain;

_HUD::HUDMain(HUD::Panel_Type pt) :
    match_score(pt), match_timer(pt) {
  type = pt;
  switch ( type ) {
    case Panel_Type::large: case Panel_Type::small:
      stats.regular = new Pieces::Stats::Regular(pt);
    break;
    case Panel_Type::race:
      stats.race = new Pieces::Stats::Race;
    break;
  }
};
_HUD::~HUDMain() {
  Reset();
}
void _HUD::Reset() {
  switch ( type ) {
    case Panel_Type::large: case Panel_Type::small:
      delete stats.regular;
    break;
    case Panel_Type::race:
      delete stats.race;
    break;
  }
  for ( auto i : item_timer )
    delete i;
}
void _HUD::Change_HUD(HUD::Panel_Type pt) {
  Reset();
  Util::Reconstruct(match_score, pt);
  Util::Reconstruct(match_timer, pt);
  switch ( type ) {
    case Panel_Type::large: case Panel_Type::small:
      stats.regular = new Pieces::Stats::Regular(pt);
    break;
    case Panel_Type::race:
      stats.race = new Pieces::Stats::Race;
    break;
  } 
}
void _HUD::Update() {
  match_score.Update();
  match_timer.Update();
  switch ( type ) {
    case Panel_Type::large: case Panel_Type::small:
      stats.regular->Update();
    break;
    case Panel_Type::race:
      stats.race->Update();
    break;
  }
  notifs.Update();
  for ( auto i : item_timer )
    i->Update();
}

// ---- LAYOUT ----------------------------------------------------------------

namespace HLay = HUD::Layout;

AOD::Vector // -- NOTIFS ----------------------------
            HLay::notif_icon            = {705,  12},
            HLay::notif_text_left       = {693,  12},
            HLay::notif_text_right      = {717,  12},
            HLay::notif_gap             = {  0,  20},
            // -- RACE ------------------------------
            HLay::race_time_text        = {327,   8},
            HLay::race_time_panel       = {400,  15},
            HLay::race_stats_panel      = {400, 568},
            HLay::race_vel_text_x       = {295, 572},
            HLay::race_vel_text_y       = {295, 589},
            HLay::race_vel_dir_x        = {273, 572},
            HLay::race_vel_dir_y        = {273, 589},
            HLay::race_acc_dir_x        = {526, 572},
            HLay::race_acc_dir_y        = {526, 589},
            HLay::race_acc_ico_horiz    = {480, 572},
            HLay::race_acc_ico_vertic   = {480, 589},
            HLay::race_acc_knob_gap     = {  7,   0},
            HLay::race_wep_start        = {372, 572},
            HLay::race_wep_gap          = { 20,  20},
            // -- LARGE -----------------------------
            HLay::large_time_text       = {365,  11},
            HLay::large_time_panel      = {400,  16},
            HLay::large_score_panel_blu = {487,  19},
            HLay::large_score_panel_red = {313,  19},
            HLay::large_score_text_blu  = {510,  13},
            HLay::large_score_text_red  = {250,  13},
            HLay::large_score_icon_blu  = {482,  11},
            HLay::large_score_icon_red  = {318,  11},
            HLay::large_item_panel      = {761, 587},
            HLay::large_item_icon       = {787, 587},
            HLay::large_item_text       = {748, 587},
            HLay::large_item_gap        = {  0, -20},
            HLay::large_stats_panel     = {400, 567},
            HLay::large_ammo_text       = {380, 570},
            HLay::large_wep_start       = {324, 590},
            HLay::large_wep_gap         = { 19,   0},
            HLay::large_health_icon     = {151, 587},
            HLay::large_health_text     = {172, 583},
            HLay::large_armour_text     = {590, 583},
            HLay::large_armour_icon     = {651, 587},
            // -- SMALL -----------------------------
            HLay::small_time_text       = {381,  10},
            HLay::small_time_panel      = {401,  12},
            HLay::small_score_panel_blu = {410,  10},
            HLay::small_score_panel_red = {390,  10},
            HLay::small_score_text_blu  = {458,  12},
            HLay::small_score_text_red  = {335,  12},
            HLay::small_score_icon_red  = {357,  10},
            HLay::small_score_icon_blu  = {443,  10},
            HLay::small_item_panel      = {778, 593},
            HLay::small_item_icon       = {793, 593},
            HLay::small_item_text       = {773, 593},
            HLay::small_item_gap        = { 0,  -20},
            HLay::small_stats_panel     = {400, 577},
            HLay::small_ammo_text       = {383, 573},
            HLay::small_wep_start       = {324, 590},
            HLay::small_wep_gap         = { 19,   0},
            HLay::small_health_icon     = {280, 588},
            HLay::small_health_text     = {218, 592},
            HLay::small_armour_text     = {543, 592},
            HLay::small_armour_icon     = {519, 588};
