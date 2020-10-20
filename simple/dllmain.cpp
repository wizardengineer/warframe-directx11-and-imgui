// dllmain.cpp : Defines the entry point for the DLL application...
#include <thread>
#include "directx11-hook.hpp"

#pragma warning(disable : 4005)
#pragma warning(disable : 6386)


auto hacking() -> bool __stdcall
{
  MessageBox(NULL, L"injected", NULL, MB_ICONEXCLAMATION);
  wdi::create_console();

  wdi::console_log("testing simple");
 
  wdi::run_hook();

  FreeConsole();
  FreeLibraryAndExitThread(0, 0);
  return TRUE;
}

auto DllMain(
  HMODULE hModule, DWORD  reason, LPVOID lpReserved
) -> bool __stdcall
{

  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);

    std::thread j(hacking);

    if (j.joinable()) { j.join(); }

  }
  return TRUE;
}
