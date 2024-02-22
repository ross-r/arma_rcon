#pragma once

//
// This is where all exports will be defined and code will be reampped to use the internal functions.
// This file should follow C99 standard as we don't want to complicate anything.
// All of the internal code can be in what ever standard and use modern stl, but for simplicity in implementations
// we want these calling conventions to be as straight forward as possible.
//

#include "rcon.hpp"
using namespace rcon;

// NOTE; We prefix with rcon_ as we cannot override default linked functions.
//       E.G; connect() is already linked to a different object and we cannot compile if we export something called "connect"

extern "C" {
  __declspec( dllexport ) void __cdecl rcon_connect( const char* ip, int port ) {
    g_rconClient.connect( ip, port );
  }

  __declspec( dllexport ) void __cdecl rcon_login( const char* password ) {
    g_rconClient.login( password );
  }

  __declspec( dllexport ) bool rcon_isLoggedIn() {
    return g_rconClient.isLoggedIn();
  }

  __declspec( dllexport ) void __cdecl rcon_disconnect() {
    g_rconClient.disconnect();
  }
}