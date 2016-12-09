#include "Event.h"


std::vector<std::string> PulEvent::event_detail = {
  {"player joined match"}
};

using PEvent = PulEvent::Event;

PEvent::Event(uint16_t eID, uint16_t e_bitmask,
              const std::vector<uint8_t>& e_data) :
  ID ( eID ), bitmask ( e_bitmask ), data ( e_data )
{}