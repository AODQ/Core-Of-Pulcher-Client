#ifndef PULCHER_EVENT_H_
#define PULCHER_EVENT_H_
#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace PulEvent {
  struct Event {
    uint16_t ID;
    uint16_t bitmask;
    std::vector<uint8_t> data;
    Event(uint16_t ID, uint16_t bitmask, const std::vector<uint8_t>& data);
  };

  extern std::vector<std::string> event_detail;

  enum class EventType {
    change_name, player_join
  };
};

#endif