//
// Created by Seamus on 29/06/2026.
//
module;
#include <Windows.h>
#include <Print>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <unordered_set>
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

    void LoadPaks(safetyhook::InlineHook& hAddSource)
    {
        constexpr unsigned int folderFlags = 7;
        constexpr unsigned int pakFlags = 1;

        std::filesystem::path modFolder = modFolderHandler();
        std::filesystem::path loadOrder = modFolder / "load_order.txt";

        std::vector<std::string> orderedPaks;
        std::unordered_set<std::string> loadedPaksSet;

        //read existing load order if it exists
        if (std::filesystem::exists(loadOrder)) {
            std::ifstream readOrder(loadOrder);

            std::string line;
            while (std::getline(readOrder, line))
            {
                if (!line.empty())
                    orderedPaks.push_back(line);
            }
            readOrder.close();
        }

        for (const auto& pakName : orderedPaks) {
            std::filesystem::path pakPath = modFolder / pakName;

            if (std::filesystem::exists(pakPath) and is_regular_file(pakPath)) {
                hAddSource.call<bool>(pakPath.string().c_str(), pakFlags);
                loadedPaksSet.insert(pakName);
                std::println("Loading {} from load order", pakName);
            }
            else {
                std::println("{} Is in load order but missing from folder!", pakName);
            }
        }

        //find paks not in load order, and append them to load order
        std::ofstream writeToLoadOrder(loadOrder, std::ios::app);

        for (const auto& entry: std::filesystem::directory_iterator(modFolder)) {
            const auto ext = entry.path().extension().string();

            if (ext == ".pak" || ext == ".mpak")
            {
                std::string fileName = entry.path().filename().string();

                //if it hasn't been loaded yet its a new mod
                if (!loadedPaksSet.contains(fileName)) {
                    hAddSource.call<bool>(entry.path().string().c_str(), pakFlags);
                    loadedPaksSet.insert(fileName);
                    std::println("Loading new mod {}", fileName);

                    writeToLoadOrder << fileName << "\n";
                }
            }
        }
        writeToLoadOrder.close();

        hAddSource.call<bool>(modFolder.string().c_str(), folderFlags);
    }

    /*
    void LoadPaks(safetyhook::InlineHook& hAddSource){
        constexpr unsigned int folderFlags = 7;
        constexpr unsigned int pakFlags = 1;

        std::filesystem::path modFolder = modFolderHandler();
        std::filesystem::path loadOrder = modFolder / "loadorder.txt";

        std::vector<std::string> loadedPaks;

        if (std::filesystem::exists(loadOrder)) {
            for (const auto& entry: std::filesystem::directory_iterator(modFolder)) {
                if (!entry.is_regular_file())
                    continue;

                const auto ext = entry.path().extension().string();

                if (ext == ".pak" || ext == ".mpak") {
                    hAddSource.call<bool>(entry.path().string().c_str(), pakFlags);
                    std::ofstream writeToLoadOrder(loadOrder);
                    writeToLoadOrder << entry.path().filename().string();
                    writeToLoadOrder.close();
                    std::println("Loading {}", entry.path().filename().string());
                }
            }
        }
        /*
        else {
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
*/

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
