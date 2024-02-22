#include "rcon.hpp"

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment( lib, "Ws2_32.lib" )

#include <cstdint>
#include <cstring>
#include <stdlib.h>
#include <vector>

#include "crc.hpp"

// TODO;
//  * Reliable UDP packet receiving code, make a good way to match packet checksums to verify a response for a packet has been received.
//    - The issue with this is we may receive a response that doesn't align with what we sent due to the size of the response buffer.
//    - We may also get mixed and matched responses into one single buffer.
//
//  * Export all related functions and make this a bindable DLL for node applications, things such as electron, etc..
//  * Add code to build the 3 basic BE packets, login, command and keep alive.
//    - Login packet done.

// Macro to convert checksum array from packet_header_t to raw uint32
#define RAW_CHECKSUM( checksum_array ) *( uint32_t* ) ( &checksum_array[ 0 ] )

// Macro to expand out a checksum array for printf
#define EXPAND_CHECKSUM( checksum_array ) checksum_array[ 0 ], checksum_array[ 1 ], checksum_array[ 2 ], checksum_array[ 3 ]

enum BE_RCON_TYPES : int8_t {
  LOGIN,
  COMMAND,
  MESSAGE,
  KEEP_ALIVE
};

namespace rcon {
  namespace net {
    bool build( packet_t* p, const uint8_t* payload, const size_t payload_length ) {
      // No packet_t struct passed.
      if( p == NULL )
        return false;

      // Construct the length of the buffer.
      // NOTE: We remove 1 to take into account the null terminator.
      p->length = sizeof( uint8_t ) * ( payload_length + sizeof( packet_header_t ) - 1 );

      // Allocate enough space for the packet.
      p->buffer = ( uint8_t* ) malloc( p->length );
      if( p->buffer == NULL )
        return false;

      // Zero out the buffer.
      memset( p->buffer, 0, p->length );

      // Construct BE Rcon packet header.
      packet_header_t header;
      header.signature[ 0 ] = 'B';
      header.signature[ 1 ] = 'E';

      // Perform CRC32 operation on the entire payload.
      // NOTE: The CRC32 operation needs the 0xFF seperator to be included in it.
      p->buffer[ 0 ] = 0xFF;
      memcpy( &p->buffer[ 1 ], &payload[ 0 ], payload_length );

      // Calculate the checksum.
      uint32_t checksum = crypto::crc32( &p->buffer[ 0 ], payload_length );

      // Extract 2 byte sequences from 8 byte checksum and assign the values in the packet header.
      header.checksum[ 0 ] = ( checksum >> 0 );
      header.checksum[ 1 ] = ( checksum >> 8 );
      header.checksum[ 2 ] = ( checksum >> 16 );
      header.checksum[ 3 ] = ( checksum >> 24 );

      // Finish of the header packet, this is used to split the header and differentiate between header and payload.
      header.end = 0xff;

      // Copy in the packet header.
      memcpy( &p->buffer[ 0 ], &header, sizeof( header ) );

      // Also copy a copy of the header into the packet for ease of use.
      memcpy( &p->header, &header, sizeof( header ) );

      // Copy in the payload.
      memcpy( &p->buffer[ sizeof( packet_header_t ) ], &payload[ 0 ], sizeof( uint8_t ) * payload_length );

      // We're done.
      return true;
    }

    void release( packet_t* p ) {
      if( p == NULL )
        return;

      if( p->buffer == NULL )
        return;

      // Free the buffer and null out values.
      free( p->buffer );
      p->length = 0;
      p->buffer = NULL;
    }
  }

  RConClient g_rconClient{};

  RConClient::RConClient() {
    m_pSocket = NULL;
  }

  bool RConClient::connect( const std::string ip, const int port ) {
    WSAData wsaData;
    if( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
      return false;

    m_pSocket = socket( AF_INET, SOCK_DGRAM, 0 );
    if( m_pSocket == INVALID_SOCKET )
      return false;

    if( WSAGetLastError() != 0 )
      return false;

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons( port );
    inet_pton( AF_INET, ip.c_str(), &hint.sin_addr );

    if( ::connect( m_pSocket, ( sockaddr* ) &hint, sizeof( hint ) ) == SOCKET_ERROR ) {
      WSACleanup();
      return false;
    }

    return true;
  }

  void RConClient::disconnect() {
    if( m_pSocket == INVALID_SOCKET )
      return;

    // If no error occurs, shutdown returns zero. Otherwise, a value of SOCKET_ERROR is returned, and a specific error code can be retrieved by calling WSAGetLastError.
    // TODO; Error handler.
    shutdown( m_pSocket, SD_BOTH );
    closesocket( m_pSocket );
  }

  void RConClient::setupRecv() {

  }

  void RConClient::read( net::packet_t* packet ) {

    std::vector< uint8_t > vAccumulatedBlock;

    // Wait for a response and read into 128 byte buffers.
    uint8_t buffer[ 128 ];
    int iReceivedBytes = recv( m_pSocket, ( char* ) buffer, 128, 0 );

    // TODO; Error handler.
    if( iReceivedBytes <= 0 )
      return;

    // Make sure we received a BE packet.
    // TODO; Error handler.
    if( iReceivedBytes < sizeof( net::packet_header_t ) )
      return;

    // Resize our accumulative response buffer.
    vAccumulatedBlock.reserve( vAccumulatedBlock.size() + iReceivedBytes );

    // Copy buffer into our accumulative response buffer.
    vAccumulatedBlock.insert( vAccumulatedBlock.begin(), &buffer[ 0 ], &buffer[ min( iReceivedBytes, 128 ) ] );

    // Response was bigger than our buffer.
    if( iReceivedBytes >= 128 ) {
      // TODO; Complete.
    }

    // Parse the response buffer.
    net::packet_header_t header;
    memcpy( &header, vAccumulatedBlock.data(), sizeof( net::packet_header_t ) );

    // Make sure the checksum matches.
    uint32_t checksum = crypto::crc32( &vAccumulatedBlock[ sizeof( net::packet_header_t ) - 1 ], vAccumulatedBlock.size() - ( sizeof( net::packet_header_t ) - 1 ) );
    packet->header.checksum[ 0 ] = ( checksum >> 0 );
    packet->header.checksum[ 1 ] = ( checksum >> 8 );
    packet->header.checksum[ 2 ] = ( checksum >> 16 );
    packet->header.checksum[ 3 ] = ( checksum >> 24 );

    printf( "> Packet Received Checksum: ( %08x ) %02x %02x %02x %02x\n", RAW_CHECKSUM( header.checksum ), EXPAND_CHECKSUM( header.checksum ) );
    printf( "> Packet Sent Checksum: ( %08x ) %02x %02x %02x %02x\n", RAW_CHECKSUM( packet->header.checksum ), EXPAND_CHECKSUM( packet->header.checksum ) );

    // Remove the packet header out of the buffer so we can read the payload.
    vAccumulatedBlock.erase( vAccumulatedBlock.begin(), vAccumulatedBlock.begin() + sizeof( net::packet_header_t ) );

    printf( "> Payload:\n\t" );
    for( size_t i{}; i < vAccumulatedBlock.size(); i++ )
      printf( "%02x ", vAccumulatedBlock[ i ] );
    printf( "\n" );

    // Obtain the packet type.
    const int iReceivedType = vAccumulatedBlock[ 0 ];
    const int iSentType = packet->buffer[ sizeof( net::packet_header_t ) ];

    // Check to make sure the response matches our sent packet.
    printf( "> iReceivedType: %d\n", iReceivedType );
    printf( "> iSentType: %d\n", iSentType );
    printf( "> Payload Length: %d\n", vAccumulatedBlock.size() );
    if( iReceivedType != iSentType )
      printf( "> Packet mismatch!\n" );

    // Obtain the response.
    if( iReceivedType == BE_RCON_TYPES::LOGIN ) {

      // Login packet has a 1 byte response.
      const bool bSuccess = ( bool ) vAccumulatedBlock[ 1 ];

      // Set logged in status.
      m_bLoggedIn = bSuccess;
    }
    else if( iReceivedType == BE_RCON_TYPES::COMMAND ) {

      // TODO;

    }
  }

  void RConClient::login( const std::string password ) {

    //
    // Construct login packet and send it to the server.
    //

    // Alloc enough space for our password and the payload type.
    const size_t iPayloadLength = password.length() + 1;
    uint8_t* payload = ( uint8_t* ) malloc( iPayloadLength );
    if( payload == NULL ) // TODO; Error handler.
      return;

    // Assign BE packet type identifier.
    payload[ 0 ] = BE_RCON_TYPES::LOGIN;

    // Copy in the password to the payload.
    memcpy( &payload[ 1 ], &password[ 0 ], sizeof( char ) * password.length() );

    // Build the BE packet.
    // TODO; Add error handling here.
    net::packet_t packet;
    if( !net::build( &packet, payload, iPayloadLength ) )
      return;

    // Send the packet to the server.
    send( m_pSocket, ( char* ) packet.buffer, packet.length, 0 );

    // We no longer need the payload, all the information is now stored in our packet.
    free( payload );

    // Read the response and validate that the received data matches our packet.
    read( &packet );

    // Cleanup.
    net::release( &packet );
  }

  bool RConClient::isLoggedIn() {
    return m_bLoggedIn;
  }
}