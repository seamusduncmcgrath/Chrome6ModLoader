#pragma once
#include <Windows.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <filesystem>

std::string GetGameDir();
void InitConsole();
std::string DecodeAddSourceFlags(unsigned int flags);
uintptr_t FindPattern(HMODULE hModule, const char* signature);