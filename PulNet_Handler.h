#ifndef PULNET_HANDLER_H_
#define PULNET_HANDLER_H_
#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include "Network.h"
#include "NetEvent_Handler.h"
namespace PulNet {
  void Handle_Packet(enet_uint8* data, size_t len, ENetPeer* peer);
  void Handle_Packet(PulNet::PacketEvent, std::vector<uint8_t>&, ENetPeer* peer);
  void Handle_Player_Send();
  void Handle_Disconnect(ENetPeer* peer);
};

#endif
