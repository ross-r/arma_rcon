#include <windows.h>
#include <cstdio>

// NOTE: Build under same mode as dll project!
//       Build: x64 Release

//
// Function definitions for our exported functions.
// CDECL calling convention so we don't need to worry about assembly registers like rbx/ecx etc
//

typedef void( __cdecl* connect_t )( const char*, int );
typedef void( __cdecl* disconnect_t )( );
typedef void( __cdecl* login_t )( const char* password );
typedef bool( __cdecl* isLoggedIn_t )();

template< typename T >
T Import( HMODULE handle, const char* symbol ) {
  return ( T ) GetProcAddress( handle, symbol );
}

int main() {

  // Attempt to load the dll library.
  HMODULE handle = LoadLibraryA( "arma_rcon_dll.dll" );
  if( handle == NULL ) {
    printf( "> LoadLibrary failed, is the dll in the same folder as the executable?\n" );
    return -1;
  }

  printf( "> arma_rcon_dll: %p\n", handle );

  // Now we want to obtain each export and call it.
  connect_t connect = Import< connect_t >( handle, "rcon_connect" );
  login_t login = Import< login_t >( handle, "rcon_login" );
  isLoggedIn_t isLoggedIn = Import< isLoggedIn_t >( handle, "rcon_isLoggedIn" );
  disconnect_t disconnect = Import< disconnect_t >( handle, "rcon_disconnect" );

  connect( "45.121.211.227", 2307 );
  login( "gBs@mB68s" );

  if( isLoggedIn() )
    printf( "Login successful.\n" );
  else
    printf( "Login failed\n" );

  disconnect();

  return 0;
}