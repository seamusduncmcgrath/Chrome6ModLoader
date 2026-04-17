#pragma once

constexpr uintptr_t RPACK_OFFSET = 0x401A80;
constexpr const char* ENGINE_DLL = "engine_x64_rwdi.dll";

bool __fastcall fs_check_zip_crc_detour(void* _this);
bool __fastcall fs_add_source_detour(const char* path, unsigned int flags);
bool InitModLoader();
void LoadPaks();