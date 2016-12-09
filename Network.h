/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef NETWORK_H_
#define NETWORK_H_
#pragma once

#ifndef ENET_INCLUDE_H_
#define ENET_INCLUDE_H_
#include <enet\enet.h>
#endif
#include <vector>
#include <map>
#include <string>
#include <utility>

#include "Event.h"

using PacketVec = std::vector<uint8_t>;
namespace PulNet {
  extern ENetAddress client_address;
  extern uint8_t server_timestamp,
                 client_timestamp;
  /*enum class packet_event {
    listmap, join, spec, vimap, ch_name, say, votekick, kick,
    ch_map, promote, demote, kill, votemap, whisper, gtfo,
    keystate, spawn, setID, spawntime, setvar, refresh_player,
    newplayer, netObj_create, netObj_refresh, netObj_remove
  };*/
  enum class PacketEvent {
    nil,
    netObj_create, netObj_refresh, netObj_remove, fire_event,

    client_refresh, 
    connection, 
    client_connection, hard_client_refresh,

    query,
    size
  };

  enum class PacketQueries { // specific to "query" packet event
    // questions
    specific_player_info, team_info,
    // "demands"
    join_game
  };

  void Init();
  // listen for connections
  void Connect(std::string);
  void Disconnect();
  // Handle networking until nothing to be done, then send keystate
  void Handle_Network();
  
  // ---- utils ----
  enum class PacketFlag {
    UDP = 10,
    TCP = 1
  };
  // Sends packet to server with data using specified flag
  void Send_Packet(const void* data, int len, uint8_t sizeof_data,
                   PacketFlag flag);
  // Sends packet to destination with data using specified flag
  void Send_Packet(const std::string&, PacketFlag flag = PacketFlag::TCP);
  // Sends packet to destination with data and will stamp the packet
  // with a preamble (ei: netObj_Refresh would be "NR (DATA)")
  void Send_Packet(PacketEvent preamble, const std::string& data,
            PacketFlag = PacketFlag::TCP);
  // Sends packet to destination with data and will stamp the packet with
  // a preamble
  void Send_Packet(PacketEvent preamble, std::vector<uint8_t>& data,
            PacketFlag = PacketFlag::TCP);
  void Send_Packet(std::vector<uint8_t>& data, PacketFlag = PacketFlag::TCP);
  // Sends event to destination with
  void Send_Packet(const PulEvent::Event&, PacketFlag = PacketFlag::TCP);
  void Send_Hard_Client_Refresh();
}

#endif

