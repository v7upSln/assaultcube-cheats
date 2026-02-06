#pragma once
#include <windows.h>

namespace hooks {
    void Initialize();
    void Unhook();

    extern bool g_ShuttingDown;
}

typedef BOOL(__stdcall* twglSwapBuffers)(HDC hDc);