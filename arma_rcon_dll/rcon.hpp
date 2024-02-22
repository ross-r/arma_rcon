#pragma once

#include <cstdint>
#include <string>

//
// https://www.battleye.com/downloads/BERConProtocol.txt
//
namespace rcon {
  namespace net {
    struct packet_header_t {
      uint8_t signature[ 2 ];
      uint8_t checksum[ 4 ];
      uint8_t end;
    };

    struct packet_t {
      packet_header_t header;
      uint8_t* buffer;
      size_t length;
    };

    bool build( packet_t* p, const uint8_t* payload, const size_t payload_length );
    void release( packet_t* p );
  }

  class RConClient {
  private:
    uintptr_t m_pSocket;

    bool m_bLoggedIn;

    void read( net::packet_t* packet );

  public:
    explicit RConClient();

    bool connect( const std::string ip, const int port );
    void disconnect();
    void setupRecv();

    void login( const std::string password );
    bool isLoggedIn();
  };

  extern RConClient g_rconClient;
}