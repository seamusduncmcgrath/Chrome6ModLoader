//
// Created by Seamus on 29/06/2026.
//
module;
#include <Windows.h>
#include <Print>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include "src/globals.hpp"
#include "SafetyHook.hpp"
export module loader;

import utils;

std::filesystem::path modFolderHandler()
{
    std::string modDir = utils::GetGameDir() + "\\mods";
    if (!std::filesystem::exists(modDir)) {
        std::filesystem::create_directory(modDir);
        MessageBoxW(nullptr, L"Created mods directory!", L"CE6ML", MB_OK);
    }

    return modDir;
}

export namespace loader {
    bool packsLoaded = false;

    void LoadPaks(safetyhook::InlineHook& hAddSource){
        constexpr unsigned int folderFlags = 7;
        constexpr unsigned int pakFlags = 1;

        auto modFolder = modFolderHandler();

        hAddSource.call<bool>(modFolder.string().c_str(), folderFlags);

        for (const auto& entry: std::filesystem::directory_iterator(modFolder)) {
            if (!entry.is_regular_file())
                continue;

            const auto ext = entry.path().extension().string();

            if (ext == ".pak" || ext == ".mpak") {
                hAddSource.call<bool>(entry.path().string().c_str(), pakFlags);
                std::println("Loading {}", entry.path().filename().string());
            }
        }
    }

    void loadRpacks(void* pRuntime, safetyhook::InlineHook& hPackLoaderLoad){

        auto modFolder = modFolderHandler();

        for (const auto& entry : std::filesystem::directory_iterator(modFolder))
        {
            if (entry.path().extension() == ".rpack")
            {
                std::string filename = entry.path().filename().string();
                std::string cleanName = entry.path().stem().string();

                if (cleanName.ends_with("_pc")) {
                    cleanName = cleanName.substr(0, cleanName.length() - 3);
                }
                std::println("Loading rpack {}", filename);

                auto result = hPackLoaderLoad.call<bool>(pRuntime, cleanName.c_str(), nullptr, EIsGlobalPack::Yes, EIsContentPack::No, EUseCachePartition::Yes, EIsCrossLevelPack::Yes, (EPackKind)0);
                if (!result) {
                    std::println("Failed to load {}", filename);
                }
            }
        }
    }
}
