#include <Windows.h>   
#include <cstdio>     
#include <string>     
#include <iostream>    
#include <filesystem>
#include "MinHook.h"
#include "utils.h"
#include "ModLoader.h"
#include "Config.h"
#include "GameTypes.h"

DWORD WINAPI InitThread(LPVOID)
{
    Config::Load();

    if (Config::EnableConsole)
    {
        InitConsole();
    }
    if (!InitModLoader())
    {
        printf("InitModLoader failed!");
    }
    return 0;
}

//prob better way to do this
std::string DecodeAddSourceFlags(unsigned int flags)
{
    if (flags == 0) return "0 (NONE/GENERIC)";

    std::string result = "";

    if (flags & (unsigned int)FFSAddSourceFlags::SUBDIRS) result += "SUBDIRS | ";
    if (flags & (unsigned int)FFSAddSourceFlags::APPEND) result += "APPEND | ";
    if (flags & (unsigned int)FFSAddSourceFlags::STRIP_LAST_DIR) result += "STRIP_LAST_DIR | ";
    if (flags & (unsigned int)FFSAddSourceFlags::BROWSABLE) result += "BROWSABLE | ";
    if (flags & (unsigned int)FFSAddSourceFlags::ALLOW_DUPLICATES) result += "ALLOW_DUPLICATES | ";
    if (flags & (unsigned int)FFSAddSourceFlags::PRELOAD) result += "PRELOAD | ";
    if (flags & (unsigned int)FFSAddSourceFlags::DISABLE_CACHE_INIT) result += "DISABLE_CACHE_INIT | ";

    if (result.length() > 3) {
        result = result.substr(0, result.length() - 3);
    }

    return result;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);CreateThread(nullptr,0,InitThread,nullptr,0,nullptr);
    }
    return TRUE;
}