//
// Created by Seamus on 29/06/2026.
//
module;
#include <Windows.h>
#include <Print>
#include "SafetyHook.hpp"
export module multiplayerHooks;

safetyhook::InlineHook hFs_check_zip_crc;
safetyhook::InlineHook hAreDataAuthenticated;
safetyhook::InlineHook hCalc_file_crc;

bool __fastcall hkFs_check_zip_crc(void* _this){
    std::println("check_zip_crc called");
    return true;
}

bool __fastcall hkAreDataAuthenticated(void* _this){
    std::println("AreDataAuthenticatedToPlayMultiplayer called");
    return true;
}

bool __cdecl hkCalc_file_crc(void* args){
    std::println("calc_file_crc called");
    return true;
}

export namespace multiplayer {
    bool Init()
    {
        HMODULE hFs = GetModuleHandleA("filesystem_x64_rwdi.dll");
        HMODULE hEngine = GetModuleHandleA("engine_x64_rwdi.dll");
        if (!hFs || !hEngine) {
            printf("Game DLLs not found!\n");
            return false;
        }

        FARPROC pCalcCrc = GetProcAddress(hFs, "?calc_file_crc@fs@@YA_NAEAUCrcCalcArgs@1@@Z");
        if (pCalcCrc) {
            hCalc_file_crc = safetyhook::create_inline(reinterpret_cast<void*>(pCalcCrc), reinterpret_cast<void*>(&hkCalc_file_crc));
            std::println("Hooked calc_file_crc");
        }

        FARPROC pAuthCheck = GetProcAddress(hEngine, "?AreDataAuthenticatedToPlayMultiplayer@IGame@@QEBA_NXZ");
        if (pAuthCheck) {
            hAreDataAuthenticated = safetyhook::create_inline(reinterpret_cast<void*>(pAuthCheck), reinterpret_cast<void*>(&hkAreDataAuthenticated));
            std::println("Hooked AreDataAuthenticatedToPlayMultiplayer");
        }

        FARPROC pCrcCheck = GetProcAddress(hFs, "?check_zip_crc@izipped_buffer_file@fs@@UEAA_NXZ");
        if (pCrcCheck) {
            hFs_check_zip_crc = safetyhook::create_inline(reinterpret_cast<void*>(pCrcCheck), reinterpret_cast<void*>(&hkFs_check_zip_crc));
            std::println("Hooked check_zip_crc");
        }

        return true;
    }
}


