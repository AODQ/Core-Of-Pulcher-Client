/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#include "AOD.h"
#include "Client_Vars.h"
#include "Game_Manager.h"
#include "Network.h"
#include "Player.h"
#include "PulNet_Handler.h"

#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <WS2tcpip.h>

#ifndef ENET_INCLUDE_H_
#define ENET_INCLUDE_H_
#include <enet\enet.h>
#endif

namespace PulNet {
  ENetAddress client_address;
  uint8_t server_timestamp,
          client_timestamp;
  ENetHost* client_host;
  ENetPeer* server_peer = nullptr;
  static std::vector<std::string> tcp_packets_to_send;
  using pe = PacketEvent;
}


static std::vector<std::tuple<char*,int,PulNet::PacketFlag>> packets_to_send;

std::mutex send_packet_mutex;
std::atomic<bool> packets_to_send_lock; // don't Send_Packet until this is true

void PulNet::Send_Packet(const void* data, int len, uint8_t sizeof_data,
                         PacketFlag flag) {
  std::lock_guard<std::mutex> send_packet_lock ( send_packet_mutex );
  if ( PulNet::server_peer ) {
    // wait until we can send packets
    while ( packets_to_send_lock.load() != 0 );
    packets_to_send_lock.store(1);
    ENetPacket* packet = enet_packet_create(data, len*sizeof_data,
                            (_ENetPacketFlag)flag);
    enet_peer_send(PulNet::server_peer, 0, packet);
    enet_host_flush(PulNet::client_host);
    packets_to_send_lock.store(0);
  }
}


void PulNet::Send_Packet(const std::string& data, PacketFlag flag) {
  if ( PulNet::server_peer != nullptr )
    Send_Packet((void*)data.c_str(), data.size(), sizeof(char), flag);
}

void PulNet::Send_Packet(PacketEvent preamble, const std::string& data,
                         PacketFlag flag) {
  Send_Packet(std::to_string((int)preamble) + data, flag);
}

void PulNet::Send_Packet(std::vector<uint8_t>& data, PacketFlag pflag) {
  Send_Packet((void*)&data[0], data.size(), 1, pflag);
}

void PulNet::Send_Packet(PacketEvent preamble, std::vector<uint8_t>& data,
                         PacketFlag flag) {
  data.insert(data.begin(), (uint8_t)preamble);
  Send_Packet((const void*)&data[0], data.size(), 1, flag);
}

// ---- utility functions -----------------------------------------------------

/*,
  if ( arg1 == "DMAP" ) { // download map
      AOD::Output("Downloading map " + arg2 + " " + str);
      std::ofstream d_map(arg2, std::ios::binary);
      std::stringstream conv;
      conv << arg3;,
      unsigned int fil_len;
      conv >> fil_len;
      AOD::Output("Bytes left: " + std::to_string(fil_len));
      char* content;
      while ( fil_len > 0 ) {
        int len = Recv_TCP_Packet(Network::server_tcp_sock, content);
        if ( len <= 0 ) break;
        fil_len -= len;
        AOD::Output("Bytes left: " + std::to_string(fil_len));
        d_map.write(content,len);
      }
      AOD::Output("Finished downloading map");
      continue;
    }

  } while ( str != "" );
*/

// returns next parameter from a string received from TCP packet
static std::string R_Next_Parameter(std::string& str) {
  std::string arg = "";
  if ( str.size() == 0 ) return "";
  while ( str[0] == ' ' ) str.erase(str.begin(), str.begin() + 1);
  if ( str[0] == '"' ) { // look until end of \"
    str.erase(str.begin(), str.begin()+1);
    while ( str.size() != 0 && str[0] != '"' ) {
      arg += str[0];
      str.erase(str.begin(), str.begin()+1);
    }
    if ( str.size() != 0 ) // remove \"
      str.erase(str.begin(), str.begin()+1);
  } else { // look until end of space
    while ( str.size() != 0 && str[0] != ' ' ) {
      arg += str[0];
      str.erase(str.begin(), str.begin()+1);
    }
  }
  if ( str.size() != 0 ) // remove space if one exists
    str.erase(str.begin(), str.begin()+1);
  return arg;
}


// ---- declared functions ----------------------------------------------------

void PulNet::Init() {
  ex_assert(enet_initialize() == 0, "Couldn't init ENet");

  client_host = enet_host_create(nullptr, 1, 2,
    57600 / 8, // 56 K modem, 56 Kbps downstream
    14400 / 8  // 56 K modem, 14 Kbps upstream
  );

  ex_assert(client_host != nullptr, "Could not create client host");
  packets_to_send_lock.store(0);
  packets_to_send.clear();
}

void PulNet::Connect(std::string ip) {
  game_manager->State_Update(Loading_State::connecting_to_server);
  ENetEvent client_event;

  // string IP to integer IP
  sockaddr_in adr; 
  inet_pton(AF_INET, ip.c_str(), &adr.sin_addr);

  client_address.host = adr.sin_addr.S_un.S_addr;
  if ( ip == "" )
    client_address.host = ENET_HOST_BROADCAST;
  client_address.port = 9113;
  server_peer = enet_host_connect(client_host, &client_address, 2, 0);

  if ( server_peer == nullptr ) {
    AOD::Output("No available peers for connection");
    game_manager->State_Update(Loading_State::error);
    return;
  }

  // wait for connection to succeed
  if ( enet_host_service(client_host, &client_event, 15000) > 0 &&
       client_event.type == ENET_EVENT_TYPE_CONNECT ) {
    AOD::Output("Connection to " + Util::R_IP(client_event.peer->address.host)
        + ":" + std::to_string(client_event.peer->address.port) + " success");
    //Send_Packet(PulNet::packet_event::ch_name, CV::username);
    client_event.peer->timeoutMinimum = 9000000;
    client_event.peer->timeoutMaximum = 9000000;
  } else {
    AOD::Output("Could not connect to server");
    game_manager->State_Update(Loading_State::error);
  }

  // etc
  server_timestamp = client_timestamp = 0;
}

void PulNet::Disconnect() {
  enet_peer_disconnect(server_peer, 0);
}

// ---- PulNet handlers -------------------------------------------------------

void PulNet::Handle_Network() {
  static ENetEvent netevent;
  while ( enet_host_service(client_host, &netevent, 0) > 0 ) {
    switch ( netevent.type ) {
      case ENET_EVENT_TYPE_RECEIVE:
        // get packet type
        PulNet::Handle_Packet(netevent.packet->data,
                              netevent.packet->dataLength, netevent.peer);
        enet_packet_destroy( netevent.packet );
      break;
      case ENET_EVENT_TYPE_DISCONNECT:
        Handle_Disconnect(netevent.peer);
      break;
    }
  }

  // packets to send
  // wait until lock is OK then lock
  /*
  while ( packets_to_send_lock.load() != 0 );
  packets_to_send_lock.store(1);
  for ( auto& i : packets_to_send ) {
    //AOD::Output("Sending " + std::string(std::get<0>(i)));
    // send packet
    ENetPacket* packet = enet_packet_create(
      std::get<0>(i), std::get<1>(i), (int)std::get<2>(i));
    enet_peer_send(server_peer, 0, packet);
    enet_host_flush(PulNet::client_host);
    // dealloc char*
    free ( (void*)std::get<0>(i) );
  }
  packets_to_send.clear();
  packets_to_send_lock.store(0); // clear lock

  // standard player refresh
  PulNet::Handle_Player_Send();
  */
}



void PulNet::Send_Hard_Client_Refresh() {
  std::vector<uint8_t> hcr_vec;
  hcr_vec.push_back((uint8_t)PulNet::PacketEvent::hard_client_refresh);
  AOD::Output("HCR: " + std::to_string(hcr_vec[0]));
  Util::Append_Pack_String(hcr_vec, CV::username);
  hcr_vec.push_back(0);
  Util::Append_Pack_String(hcr_vec, "NIL");
  hcr_vec.push_back(0);
  auto pl = game_manager->c_player;
  Util::Append_Pack_String(hcr_vec, R_Player_Status_Str(pl));
  hcr_vec.push_back(0);
  Util::Append_Pack_Num(hcr_vec, 0);
  Util::Append_Pack_Num(hcr_vec, 0);
  PulNet::Send_Packet(hcr_vec);
}
