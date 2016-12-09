#include "AOD.h"
#include "Match.h"


Match::Match() {}

const Util::Time& Match::R_Match_Time() const {
  return match_timer;
}

// ---- MATCH CTF -------------------------------------------------------------

Match_CTF::Match_CTF() {
  red_flags = 0;
  blu_flags = 0;
  red_flag_inbase = 1;
  blu_flag_inbase = 1;
}

void Match_CTF::Update() {
  if ( AOD::Input::keys[ SDL_SCANCODE_Q ] ) {
    --red_flags;
    --blu_flags;
  }
  if ( AOD::Input::keys[ SDL_SCANCODE_E ] ) {
    ++red_flags;
    ++blu_flags;
  }
}

int Match_CTF::R_Red_Score() const {
  return red_flags;
}

int Match_CTF::R_Blu_Score() const {
  return blu_flags;
}

bool Match_CTF::R_Blu_Flag_Status() const {
  return blu_flag_inbase;
}

bool Match_CTF::R_Red_Flag_Status() const {
  return red_flag_inbase;
}