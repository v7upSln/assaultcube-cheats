#include "pch.h"
#include "logger.h"
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <ctime>

namespace Logger {
    FILE* pFile = nullptr;

    void Initialize() {
        if (GetConsoleWindow()) return;

        if (AllocConsole()) {
            freopen_s(&pFile, "CONOUT$", "w", stdout);
            freopen_s(&pFile, "CONOUT$", "w", stderr);
            SetConsoleTitleA("Internal Trace Console");
        }

        Log("[INIT] Logger initialized. High-verbosity mode active.");
    }

    void Uninitialize() {
        if (pFile) {
            Log("[EXIT] Closing logger...");
            fclose(pFile);
        }
        FreeConsole();
    }

    void Log(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);

        printf("[INFO] ");
        vprintf(fmt, args);
        printf("\n");

        va_end(args);
    }

    void Error(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);

        printf("[ERROR] !!! ");
        vprintf(fmt, args);
        printf(" !!!\n");

        va_end(args);
    }

    void Trace(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);

        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);

        printf("[TRACE] %s\n", buf);
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");

        va_end(args);
    }
}