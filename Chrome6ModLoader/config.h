#pragma once
#include <string>

namespace Config
{
    extern bool EnableConsole;
    extern bool VerboseLogging;

    void Load();
}