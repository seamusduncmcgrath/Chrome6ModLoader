//
// Created by Seamus on 29/06/2026.
//
module;
#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <filesystem>
#include <iostream>
export module utils;

export namespace utils {
    void InitConsole()
    {
        AllocConsole();

        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "r", stdin);
    }

    uintptr_t FindPattern(HMODULE hModule, const char* signature)
    {
        static auto patternToByte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?') ++current;
                    bytes.push_back(-1);
                }
                else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)hModule;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)hModule + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = patternToByte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(hModule);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (auto j = 0ul; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) return reinterpret_cast<uintptr_t>(&scanBytes[i]);
        }
        return 0;
    }

    void PatchMemory(uintptr_t address, const std::vector<uint8_t>& bytes)
    {
        DWORD oldProtect;
        VirtualProtect((LPVOID)address, bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);

        memcpy((void*)address, bytes.data(), bytes.size());

        VirtualProtect((LPVOID)address, bytes.size(), oldProtect, &oldProtect);
    }

    std::string GetGameDir()
    {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return std::filesystem::path(buffer).parent_path().string();
    }
}
