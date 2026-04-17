#include <Windows.h> 
#include <cstdio>  
#include <string>      
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>   
#include <format>
#include "MinHook.h"
#include "utils.h"
#include "GameTypes.h"
#include "ModLoader.h"
#include "Config.h"

namespace fs = std::filesystem;
bool g_RPacksLoaded = false;

fs_add_source_t fs_add_source_original = nullptr;
fs_check_zip_crc_t fs_check_zip_crc_original = nullptr;
LoadDataPack_t LoadDataPack_original = nullptr;
AreDataAuthenticated_t AreDataAuthenticated_original = nullptr;
calc_file_crc_t calc_file_crc_original = nullptr;
LogPrintCallback g_originalLogCallback = nullptr;

void LoadPaks()
{
    unsigned int folderFlags = (unsigned int)FFSAddSourceFlags::SUBDIRS | (unsigned int)FFSAddSourceFlags::APPEND | (unsigned int)FFSAddSourceFlags::STRIP_LAST_DIR;
    unsigned int pakFlags = (unsigned int)FFSAddSourceFlags::SUBDIRS;
    std::string decodedFolderFlags = DecodeAddSourceFlags(folderFlags);

    std::string baseDir = GetGameDir();
    std::string modDir = baseDir + "\\mods";
    std::string loadOrderFile = modDir + "\\load_order.txt";

    printf("[Mod Loader] AddSource: %s\n", modDir.c_str());
    printf("             Flags: %u -> [%s]\n", folderFlags, decodedFolderFlags.c_str());
    fs_add_source_original(modDir.c_str(), folderFlags);

    std::vector<std::string> loadedPaks;

    if (fs::exists(loadOrderFile))
    {
        unsigned int pakFlags = (unsigned int)FFSAddSourceFlags::SUBDIRS;
        std::string decodedPakFlags = DecodeAddSourceFlags(pakFlags);

        std::ifstream file(loadOrderFile);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#') continue; //skips empty lines or comments
            std::string targetPak = modDir + "\\" + line;

            if (fs::exists(targetPak))
            {
                printf("[Mod Loader] AddSource: %s\n", targetPak.c_str());
                printf("             Flags: %u -> [%s]\n", pakFlags, decodedPakFlags.c_str());
                auto result = fs_add_source_original(targetPak.c_str(), pakFlags);
                loadedPaks.push_back(line);
            }
        }
    }

    //loads paks that where missing from load order txt
    for (const auto& entry : fs::directory_iterator(modDir))
    {
        if (entry.path().extension() == ".pak")
        {
            std::string decodedPakFlags = DecodeAddSourceFlags(pakFlags); //maybe better way to do this
            std::string fileName = entry.path().filename().string();
            //checks if already loaded
            if (std::find(loadedPaks.begin(), loadedPaks.end(), fileName) == loadedPaks.end())
            {
                printf("[Mod Loader] AddSource: %s [Unlisted]\n", entry.path().string().c_str());
                printf("             Flags: %u -> [%s]\n", pakFlags, decodedPakFlags.c_str());
                fs_add_source_original(entry.path().string().c_str(), pakFlags);
            }
        }
    }
}

void LoadCustomRPacks(void* pRuntime)
{
    printf("[Mod Loader] Loading RPACKs...\n");
    std::string modDir = GetGameDir() + "\\mods";

    if (!fs::exists(modDir)) return;

    for (const auto& entry : fs::directory_iterator(modDir))
    {
        if (entry.path().extension() == ".rpack")
        {
            std::string filename = entry.path().filename().string();
            std::string cleanName = entry.path().stem().string(); //strips extension

            //Now we just check if the stem ends in "_pc"
            if (cleanName.ends_with("_pc")) {
                cleanName = cleanName.substr(0, cleanName.length() - 3);
            }

            printf("[Mod Loader] Loading RPACK: %s\n", filename.c_str());

            bool result = LoadDataPack_original(pRuntime, cleanName.c_str(), nullptr,
                EIsGlobalPack::Yes, EIsContentPack::No, EUseCachePartition::Yes, EIsCrossLevelPack::Yes, (EPackKind)0);

            if (!result)
            {
                std::cout << std::format("Failed to load {}\n", filename);
            }
        }
    }
}

bool __fastcall fs_add_source_detour(const char* path, unsigned int flags)
{
    auto result = fs_add_source_original(path, flags);
    std::string decodedFlags = DecodeAddSourceFlags(flags); //shows add source flags
    printf("[Mod Loader] AddSource: %s\n", path);
    printf("             Flags: %u -> [%s]\n", flags, decodedFlags.c_str()); 

    static bool loaded = false;
    if (!loaded && path && strstr(path, "Data3.pak")) //maybe dataen or something is better?
    {
        loaded = true;
        LoadPaks();
    }
    return result;
}

bool __fastcall LoadDataPack_Detour(
    void* pRuntime, const char* path, void** outPack,
    EIsGlobalPack isGlobal,
    EIsContentPack isContent,
    EUseCachePartition isCache,
    EIsCrossLevelPack isCross,
    EPackKind kind)
{
    if (!g_RPacksLoaded) {
        g_RPacksLoaded = true;
        LoadCustomRPacks(pRuntime);
    }
  return LoadDataPack_original(pRuntime, path, outPack, isGlobal, isContent, isCache, isCross, kind);
}


bool __fastcall fs_check_zip_crc_detour(void* _this) { return true; }
bool __fastcall AreDataAuthenticated_detour(void* _this) { return true; }
bool __cdecl calc_file_crc_detour(void* args) { return true; }

void __cdecl LogCallback(int level, const char* tag, const char* msg)
{
    if (Config::VerboseLogging)
    {
        printf("[%d][%s] %s", level, tag, msg);
    }

    if (g_originalLogCallback)
    {
        g_originalLogCallback(level, tag, msg);
    }
}

bool InitModLoader()
{
    HMODULE hFs = GetModuleHandleA("filesystem_x64_rwdi.dll");
    HMODULE hEngine = GetModuleHandleA("engine_x64_rwdi.dll");
    if (!hFs || !hEngine)
    {
        printf("FATAL: Game DLLs not found!");
        return false;
    }

    MH_Initialize();

    //swap to pattern scan or something soon
    uintptr_t rpackAddr = FindPattern(hEngine, "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 55 41 54 41 56 48 8D 6C 24 D9 48 81 EC 90 00 00 00");

    if (MH_CreateHook((LPVOID)rpackAddr, &LoadDataPack_Detour, (LPVOID*)&LoadDataPack_original) == MH_OK)
    {
        printf("[Mod Loader] Found PackLoader::Load at: 0x%p\n", (void*)rpackAddr);
        MH_EnableHook((LPVOID)rpackAddr);
    }
    else { printf("Failed to hook LoadDataPack!\n"); }

    void* addSourceAddr = GetProcAddress(hFs, "?add_source@fs@@YA_NPEBDW4ENUM@FFSAddSourceFlags@@@Z");
    void* crcCheckAddr = GetProcAddress(hFs, "?check_zip_crc@izipped_buffer_file@fs@@UEAA_NXZ");
    void* initGameScriptAddr = GetProcAddress(hEngine, "InitializeGameScript");
    void* authCheckAddr = GetProcAddress(hEngine, "?AreDataAuthenticatedToPlayMultiplayer@IGame@@QEBA_NXZ");
    void* calcCrcAddr = GetProcAddress(hFs, "?calc_file_crc@fs@@YA_NAEAUCrcCalcArgs@1@@Z");
    auto pLogSetPrintCallback = (LogSetPrintCallback_t)GetProcAddress(hFs, "?LogSetPrintCallback@@YAP6AXW4TYPE@ELevel@Log@@PEBD1@ZP6AX011@Z@Z");

    if (pLogSetPrintCallback)
    {
        g_originalLogCallback = pLogSetPrintCallback(LogCallback);
        printf("[Mod Loader] Native Engine Log Callback registered.\n");
    }
    else
    {
        printf("Failed to find LogSetPrintCallback export!\n");
    }

    if (authCheckAddr)
    {
        MH_CreateHook(authCheckAddr, &AreDataAuthenticated_detour, (LPVOID*)&AreDataAuthenticated_original);
        MH_EnableHook(authCheckAddr);
        printf("[Mod Loader] Hooked AreDataAuthenticatedToPlayMultiplayer.\n");
    }
    else { printf("Failed to find AreDataAuthenticatedToPlayMultiplayer!\n"); }

    if (addSourceAddr)
    {
        MH_CreateHook(addSourceAddr, &fs_add_source_detour, (LPVOID*)&fs_add_source_original);
        MH_EnableHook(addSourceAddr);
    }
    else { printf("Failed to find fs_add_source"); }
    if (crcCheckAddr)
    {
        MH_CreateHook(crcCheckAddr, &fs_check_zip_crc_detour, (LPVOID*)&fs_check_zip_crc_original);
        MH_EnableHook(crcCheckAddr);
    }
    else { printf("Failed to find fs_check_zip_crc"); }
    if (calcCrcAddr)
    {
        MH_CreateHook(calcCrcAddr, &calc_file_crc_detour, (LPVOID*)&calc_file_crc_original);
        MH_EnableHook(calcCrcAddr);
    }
    else { printf("Failed to find calc_file_crc export!\n"); }
    return true;
}