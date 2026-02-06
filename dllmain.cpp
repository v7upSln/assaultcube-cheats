#include "pch.h"
#include "hooks.h"
#include "logger.h"
#include <MinHook.h>

bool g_Running = true;

DWORD WINAPI HackThread(HMODULE hModule) {
    Logger::Initialize();
    Logger::Log("[DLL] Injection Successful.");

    // Initialize MinHook globally first
    if (MH_Initialize() != MH_OK) {
        Logger::Log("[DLL] Failed to initialize MinHook!");
        FreeConsole();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    hooks::Initialize();

    while (g_Running) {
        if (GetAsyncKeyState(VK_DELETE) & 1) {
            Logger::Log("[DLL] Unload key (DELETE) pressed.");
            break;
        }
        Sleep(10);
    }

    // safe exit
    Logger::Log("[DLL] Starting safe unload...");

    Sleep(200);

    hooks::Unhook();

    Sleep(100);

    Logger::Log("[DLL] Cleanup complete. Exiting thread.");

    // Close console and exit
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule); // Performance/Stability optimization
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    }
    return TRUE;
}