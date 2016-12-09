#include "PlayerSheet.h"

#include <unordered_map>

// ugly
#define LEGANIM(X)   &leg_anims  [(int)Leg_Anim::X  ]
#define DEATHANIM(X) &death_anims[(int)Death_Anim::X]
#define HANDANIM(X)  &hand_anims [(int)Hand_Anim::X ]
#define BODYANIM(X)  &body_anims [(int)Body_Anim::X ]
#define HEADANIM(X)  &head_anims [(int)Head_Anim::X ]

PlayerSheet::PlayerSheet() : str_to_pja ({
  {"legs_stand_idle",        LEGANIM(stand_idle           )},
  {"legs_landing",           LEGANIM(landing              )},
  {"legs_punch_front",       LEGANIM(punch_front          )},
  {"legs_punch_back",        LEGANIM(punch_back           )},
  {"legs_to_crouch",         LEGANIM(to_crouch            )},
  {"legs_crouch_idle",       LEGANIM(crouch_idle          )},
  {"legs_walk_turn",         LEGANIM(walk_turn            )},
  {"legs_crouch_turn",       LEGANIM(crouch_turn          )},
  {"legs_turn",              LEGANIM(turn                 )},
  {"legs_run_forward",       LEGANIM(run_forward          )},
  {"legs_run_turn",          LEGANIM(run_turn             )},
  {"legs_run_back",          LEGANIM(run_back             )},
  {"legs_walk",              LEGANIM(walk                 )},
  {"legs_crouch_walk",       LEGANIM(crouch_walk          )},
  {"legs_air_to_crouch",     LEGANIM(air_to_crouch        )},
  {"legs_air_crouch_hold",   LEGANIM(air_crouch_hold      )},
  {"legs_air_to_crouch_rev", LEGANIM(air_to_crouch_rev    )},
  {"legs_jump_hi",           LEGANIM(jump_hi              )},
  {"legs_jump_hi_back",      LEGANIM(jump_hi_back         )},
  {"legs_jump_strafe1",      LEGANIM(jump_strafe1         )},
  {"legs_jump_strafe2",      LEGANIM(jump_strafe2         )},
  {"legs_slide1",            LEGANIM(slide1               )},
  {"legs_slide2",            LEGANIM(slide2               )},
  {"legs_air_idle",          LEGANIM(air_idle             )},
  {"legs_swim",              LEGANIM(swim                 )},
  {"legs_dash_vertical",     LEGANIM(dash_vertical        )},
  {"legs_dash_horiz1",       LEGANIM(dash_horiz1          )},
  {"legs_dash_horiz2",       LEGANIM(dash_horiz2          )},
  {"legs_walljump1",         LEGANIM(walljump1            )},
  {"legs_walljump2",         LEGANIM(walljump2            )},
  {"death_f",                DEATHANIM(death_f            )},
  {"death_b",                DEATHANIM(death_b            )},
  {"h_alarm_right_b",         HANDANIM(alarm_right_b        )},
  {"h_alarm_right_f",         HANDANIM(alarm_right_f        )},
  {"h_alarm_left_b",          HANDANIM(alarm_left_b         )},
  {"h_alarm_left_f",          HANDANIM(alarm_left_f         )},
  {"h_twowep_left_b",         HANDANIM(twowep_left_b        )},
  {"h_twowep_left_f",         HANDANIM(twowep_left_f        )},
  {"h_twowep_right_b",        HANDANIM(twowep_right_b       )},
  {"h_twowep_right_f",        HANDANIM(twowep_right_f       )},
  {"h_onewep_b",              HANDANIM(onewep_b             )},
  {"h_onewep_f",              HANDANIM(onewep_f             )},
  {"h_guard_left_b",          HANDANIM(guard_left_b         )},
  {"h_guard_left_f",          HANDANIM(guard_left_f         )},
  {"h_guard_right_b",         HANDANIM(guard_right_b        )},
  {"h_guard_right_f",         HANDANIM(guard_right_f        )},
  {"h_slide_1_b",             HANDANIM(slide_1_b            )},
  {"h_slide_1_f",             HANDANIM(slide_1_f            )},
  {"h_slide_2_b",             HANDANIM(slide_2_b            )},
  {"h_slide_2_f",             HANDANIM(slide_2_f            )},
  {"h_run_b",                 HANDANIM(run_b                )},
  {"h_run_f",                 HANDANIM(run_f                )},
  {"h_walk_b",                HANDANIM(walk_b               )},
  {"h_walk_f",                HANDANIM(walk_f               )},
  {"h_punch1_b",              HANDANIM(punch1_b             )},
  {"h_punch1_f",              HANDANIM(punch1_f             )},
  {"h_punch2_b",              HANDANIM(punch2_b             )},
  {"h_punch2_f",              HANDANIM(punch2_f             )},
  {"h_punch3_b",              HANDANIM(punch3_b             )},
  {"h_punch3_f",              HANDANIM(punch3_f             )},
  {"h_jump1_b",               HANDANIM(jump1_b              )},
  {"h_jump1_f",               HANDANIM(jump1_f              )},
  {"h_jump2_b",               HANDANIM(jump2_b              )},
  {"h_jump2_f",               HANDANIM(jump2_f              )},
  {"h_strafe1_b",             HANDANIM(strafe1_b            )},
  {"h_strafe1_f",             HANDANIM(strafe1_f            )},
  {"h_strafe2_b",             HANDANIM(strafe2_b            )},
  {"h_strafe2_f",             HANDANIM(strafe2_f            )},
  {"h_backflip_b",            HANDANIM(backflip_b           )},
  {"h_backflip_f",            HANDANIM(backflip_f           )},
  {"h_frontflip_b",           HANDANIM(frontflip_b          )},
  {"h_frontflip_f",           HANDANIM(frontflip_f          )},
  {"h_one_hand_spare_b",      HANDANIM(one_hand_spare_b     )},
  {"h_one_hand_spare_f",      HANDANIM(one_hand_spare_f     )},
  {"h_doppler_bad_support_b", HANDANIM(doppler_bad_support_b)},
  {"h_doppler_bad_support_f", HANDANIM(doppler_bad_support_f)},
  {"h_doppler_bad_b",         HANDANIM(doppler_bad_b        )},
  {"h_doppler_bad_f1",        HANDANIM(doppler_bad_f1       )},
  {"h_nil1",                  HANDANIM(nil1                 )},
  {"h_doppler_bad_f2",        HANDANIM(doppler_bad_f2       )},
  {"h_pericaliya_b",          HANDANIM(pericaliya_b         )},
  {"h_pericaliya_f",          HANDANIM(pericaliya_f         )},
  {"body_left2",            BODYANIM (body_left2          )},
  {"body_left",             BODYANIM (body_left           )},
  {"body_center",           BODYANIM (body_center         )},
  {"body_right",            BODYANIM (body_right          )},
  {"body_right2",           BODYANIM (body_right2         )},
  {"body_walk",             BODYANIM (body_walk           )},
  {"head_up",               HEADANIM (up                  )},
  {"head_mid",              HEADANIM (center              )},
  {"head_down",             HEADANIM (down                )}
})
{ /* empty constructor*/ }

#undef LEGANIM(X)
#undef DEATHANIM(X)
#undef HANDANIM(X)
#undef BODYANIM(X)

using PJA = PlayerJointAnimation;

PJA& PlayerSheet::R_Anim(Leg_Anim id) { return leg_anims[(int)id];    }

PJA& PlayerSheet::R_Anim(Death_Anim id) { return death_anims[(int)id]; }

using Hand_Pair = std::pair<bool, PJA&>;
Hand_Pair PlayerSheet::R_Anim(Hand_Anim id) {
  int typ = int(id) % 2;
  return Hand_Pair(!typ, hand_anims[(int)id]);
}

PJA& PlayerSheet::R_Anim(Head_Anim id) { return head_anims[(int)id]; }

PJA& PlayerSheet::R_Anim(Body_Anim id) { return body_anims[(int)id]; }

PlayerSheet::Leg_Anim PlayerSheet::Str_To_Leg_Anim(const std::string& str) {
  // TODO: make this static if there's use for it outside of debugging
  // same with str_to_hand_anim
  std::unordered_map<std::string, PlayerSheet::Leg_Anim> strtotl = {
    {"stand_idle",        Leg_Anim::stand_idle       },
    {"landing",           Leg_Anim::landing          },
    {"punch_front",       Leg_Anim::punch_front      },
    {"punch_back",        Leg_Anim::punch_back       },
    {"to_crouch",         Leg_Anim::to_crouch        },
    {"crouch_idle",       Leg_Anim::crouch_idle      },
    {"turn",              Leg_Anim::turn             },
    {"walk_turn",         Leg_Anim::walk_turn        },
    {"crouch_turn",       Leg_Anim::crouch_turn      },
    {"run_forward",       Leg_Anim::run_forward      },
    {"run_turn",          Leg_Anim::run_turn         },
    {"run_back",          Leg_Anim::run_back         },
    {"walk",              Leg_Anim::walk             },
    {"crouch_walk",       Leg_Anim::crouch_walk      },
    {"air_to_crouch",     Leg_Anim::air_to_crouch    },
    {"air_crouch_hold",   Leg_Anim::air_crouch_hold  },
    {"air_to_crouch_rev", Leg_Anim::air_to_crouch_rev},
    {"jump_hi",           Leg_Anim::jump_hi          },
    {"jump_hi_back",      Leg_Anim::jump_hi_back     },
    {"jump_strafe1",      Leg_Anim::jump_strafe1     },
    {"jump_strafe2",      Leg_Anim::jump_strafe2     },
    {"slide1",            Leg_Anim::slide1           },
    {"slide2",            Leg_Anim::slide2           },
    {"air_idle",          Leg_Anim::air_idle         },
    {"swim",              Leg_Anim::swim             },
    {"dash_vertical",     Leg_Anim::dash_vertical    },
    {"dash_horiz1",       Leg_Anim::dash_horiz1      },
    {"dash_horiz2",       Leg_Anim::dash_horiz2      },
    {"walljump1",         Leg_Anim::walljump1        },
    {"walljump2",         Leg_Anim::walljump2        }
  };
  auto k = strtotl.find(str);
  return (k == strtotl.end() ? Leg_Anim::size : k->second);
}

PlayerSheet::Hand_Anim PlayerSheet::Str_To_Hand_Anim(const std::string& str) {
  std::unordered_map<std::string, PlayerSheet::Hand_Anim> strtoh = {
    {"alarm_right_b",          Hand_Anim::alarm_right_b        },
    {"alarm_right_f",          Hand_Anim::alarm_right_f        },
    {"alarm_left_b",           Hand_Anim::alarm_left_b         },
    {"alarm_left_f",           Hand_Anim::alarm_left_f         },
    {"twowep_left_b",          Hand_Anim::twowep_left_b        },
    {"twowep_left_f",          Hand_Anim::twowep_left_f        },
    {"twowep_right_b",         Hand_Anim::twowep_right_b       },
    {"twowep_right_f",         Hand_Anim::twowep_right_f       },
    {"onewep_b",               Hand_Anim::onewep_b             },
    {"onewep_f",               Hand_Anim::onewep_f             },
    {"guard_left_b",           Hand_Anim::guard_left_b         },
    {"guard_left_f",           Hand_Anim::guard_left_f         },
    {"guard_right_b",          Hand_Anim::guard_right_b        },
    {"guard_right_f",          Hand_Anim::guard_right_f        },
    {"slide_1_b",              Hand_Anim::slide_1_b            },
    {"slide_1_f",              Hand_Anim::slide_1_f            },
    {"slide_2_b",              Hand_Anim::slide_2_b            },
    {"run_b",                  Hand_Anim::run_b                },
    {"run_f",                  Hand_Anim::run_f                },
    {"slide_2_f",              Hand_Anim::slide_2_f            },
    {"walk_b",                 Hand_Anim::walk_b               },
    {"walk_f",                 Hand_Anim::walk_f               },
    {"punch1_b",               Hand_Anim::punch1_b             },
    {"punch1_f",               Hand_Anim::punch1_f             },
    {"punch2_b",               Hand_Anim::punch2_b             },
    {"punch2_f",               Hand_Anim::punch2_f             },
    {"punch3_b",               Hand_Anim::punch3_b             },
    {"punch3_f",               Hand_Anim::punch3_f             },
    {"jump1_b",                Hand_Anim::jump1_b              },
    {"jump1_f",                Hand_Anim::jump1_f              },
    {"jump2_b",                Hand_Anim::jump2_b              },
    {"jump2_f",                Hand_Anim::jump2_f              },
    {"strafe1_b",              Hand_Anim::strafe1_b            },
    {"strafe1_f",              Hand_Anim::strafe1_f            },
    {"strafe2_b",              Hand_Anim::strafe2_b            },
    {"strafe2_f",              Hand_Anim::strafe2_f            },
    {"backflip_b",             Hand_Anim::backflip_b           },
    {"backflip_f",             Hand_Anim::backflip_f           },
    {"frontflip_b",            Hand_Anim::frontflip_b          },
    {"frontflip_f",            Hand_Anim::frontflip_f          },
    {"one_hand_spare_b",       Hand_Anim::one_hand_spare_b     },
    {"one_hand_spare_f",       Hand_Anim::one_hand_spare_f     },
    {"doppler_bad_support_b",  Hand_Anim::doppler_bad_support_b},
    {"doppler_bad_support_f",  Hand_Anim::doppler_bad_support_f},
    {"doppler_bad_b",          Hand_Anim::doppler_bad_b        },
    {"doppler_bad_f1",         Hand_Anim::doppler_bad_f1       },
    {"nil1",                   Hand_Anim::nil1                 },
    {"doppler_bad_f2",         Hand_Anim::doppler_bad_f2       },
    {"pericaliya_b",           Hand_Anim::pericaliya_b         },
    {"pericaliya_f",           Hand_Anim::pericaliya_f         },
  };
  auto k = strtoh.find(str);
  return (k == strtoh.end() ? Hand_Anim::size : k->second);
}

PlayerJointAnimation& PlayerSheet::Str_To_PlayerJointAnimation(
        std::string str) {
  auto k = str_to_pja.find(str);
  if ( k == str_to_pja.end() )
    throw "Could not find string in map for Str_To_PlayerJointAnimation";
  return *k->second;
}