#pragma once

enum class FFSAddSourceFlags : unsigned int
{
    SUBDIRS = 1,
    APPEND = 2,
    STRIP_LAST_DIR = 4,
    BROWSABLE = 8,
    ALLOW_DUPLICATES = 16,
    PRELOAD = 32,
    DISABLE_CACHE_INIT = 64
};

enum class EIsGlobalPack : int { No = 0, Yes = 1 };
enum class EIsContentPack : int { No = 0, Yes = 1 };
enum class EUseCachePartition : int { No = 0, Yes = 1 };
enum class EIsCrossLevelPack : int { No = 0, Yes = 1 };
enum class EPackKind : int {};

using fs_add_source_t = bool(__fastcall*)(const char* path, unsigned int flags); // do flags for enum
using fs_check_zip_crc_t = bool(__fastcall*)(void* _this); // use to remove CRC check
using PakLoader_Load = bool(__fastcall*)(void* pRuntime, const char* path, void** outPack, EIsGlobalPack, EIsContentPack, EUseCachePartition, EIsCrossLevelPack, EPackKind); // rpack loading
using AreDataAuthenticated_t = bool(__fastcall*)(void* _this); // other crc checks
using calc_file_crc_t = bool(__cdecl*)(void* args);
using LogPrintCallback = void(__cdecl*)(int, const char*, const char*);
using LogSetPrintCallback_t = LogPrintCallback(__cdecl*)(LogPrintCallback);