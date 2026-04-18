#pragma once

bool __fastcall fs_check_zip_crc_detour(void* _this);
bool __fastcall fs_add_source_detour(const char* path, unsigned int flags);
bool InitModLoader();
void LoadPaks();