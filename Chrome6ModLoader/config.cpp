#include <Windows.h>
#include "Config.h"
#include "utils.h"

namespace Config
{
    bool EnableConsole = true;
    bool VerboseLogging = false;

    void Load()
    {
        std::string iniPath = GetGameDir() + "\\mods\\modloader.ini";

        EnableConsole = GetPrivateProfileIntA("Settings", "EnableConsole", 1, iniPath.c_str()) == 1;
        VerboseLogging = GetPrivateProfileIntA("Settings", "VerboseLogging", 0, iniPath.c_str()) == 1;

        if (GetFileAttributesA(iniPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            WritePrivateProfileStringA("Settings", "EnableConsole", "1", iniPath.c_str());
            WritePrivateProfileStringA("Settings", "VerboseLogging", "0", iniPath.c_str());
        }
    }
}