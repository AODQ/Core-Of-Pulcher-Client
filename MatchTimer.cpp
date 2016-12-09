#include "Data.h"
#include "Game_Manager.h"
#include "MatchTimer.h"
#include "NetObject_Types.h"
#include <fstream>


MatchTimer::MatchTimer() : AODP::NetObj(0, NetObj_Type::match_timer,
                                   static_cast<AOD::Object*>(this)) {
  timer.Set_Time();
}

int MatchTimer::R_Milliseconds() const { return timer.R_Milliseconds(); }
int MatchTimer::R_Seconds()      const { return timer.R_Seconds();      }
int MatchTimer::R_Hours()        const { return timer.R_Hours();        }
const Util::Time& MatchTimer::R_Timer() const { return timer; }

void MatchTimer::Update() {
  timer.Set_Time((uint32_t)rotation);
}
