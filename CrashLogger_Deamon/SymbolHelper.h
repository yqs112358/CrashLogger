#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <string>

using namespace std;

PSYMBOL_INFO GetSymbolInfo(HANDLE hProcess, void* address);
void CleanSymbolInfo(PSYMBOL_INFO pSymbol);

bool CreateModuleMap(HANDLE hProcess);
wstring MapModuleFromAddr(HANDLE hProcess, void* address);