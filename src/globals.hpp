//
// Created by Seamus on 29/06/2026.
//
#pragma once
#include <cstdint>

class FFSAddSourceFlags {
public:
    enum ENUM {
        SUBDIRS          = 0x1,
        APPEND           = 0x2,
        STRIP_LAST_DIR   = 0x4,
        BROWSABLE        = 0x8,
        ALLOW_DUPLICATES = 0x10,
        PRELOAD          = 0x20,
    };
};

using fs_add_source_t = bool(__fastcall*)(const char* path, unsigned int flags);
typedef void (*InitializeGameScript_t)(LPCSTR scriptName, uint8_t flags);

enum class EIsGlobalPack : int { No = 0, Yes = 1 };
enum class EIsContentPack : int { No = 0, Yes = 1 };
enum class EUseCachePartition : int { No = 0, Yes = 1 };
enum class EIsCrossLevelPack : int { No = 0, Yes = 1 };
enum class EPackKind : int {};

using packloader_load_t = bool(__fastcall*)(void* pRuntime, const char* path, void** outPack, EIsGlobalPack, EIsContentPack, EUseCachePartition, EIsCrossLevelPack, EPackKind);

using AreDataAuthenticated_t = bool(__fastcall*)(void* _this);
using calc_file_crc_t = bool(__cdecl*)(void* args);
using fs_check_zip_crc_t = bool(__fastcall*)(void* _this);