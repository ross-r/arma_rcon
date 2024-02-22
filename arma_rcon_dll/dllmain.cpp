#include <windows.h>

// Expose our exports.
#include "exports.hpp"

// We don't care about anything in this main function as the purpose of this DLL is to export functions.
// We won't execute anything upon DllMain being called in a process.
BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {
  switch( ul_reason_for_call ) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }

  return TRUE;
}

