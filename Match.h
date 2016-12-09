#ifndef MATCH_H_
#define MATCH_H_
#pragma once
#include "Utility.h"

class Match {
public:
  enum class Match_Type {
    CTF, Duel, Race, FFA
  };
private:
  Util::Time match_timer;
public:
  Match();
  virtual void Update() = 0;
  virtual int R_Red_Score() const = 0;
  virtual int R_Blu_Score() const = 0;
  Match_Type R_Match_Type() const;
  const Util::Time& R_Match_Time() const;
};

class Match_CTF : Match {
public:
  int red_flags, blu_flags;
  bool red_flag_inbase,
       blu_flag_inbase;
public:
  Match_CTF();
  void Update();
  int R_Red_Score() const;
  int R_Blu_Score() const;
  bool R_Blu_Flag_Status() const;
  bool R_Red_Flag_Status() const;
};

#endif