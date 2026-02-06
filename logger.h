#pragma once
#include <string>

namespace Logger {
    void Initialize();
    void Uninitialize();

    void Log(const char* fmt, ...);

    void Error(const char* fmt, ...);

    void Trace(const char* fmt, ...);
}