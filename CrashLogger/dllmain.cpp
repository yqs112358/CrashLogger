#include "pch.h"
#include <cstdio>
#include <cstring>
#include "../include/LoggerShareData.h"

bool InitDeamonProcess()
{
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;

    SECURITY_ATTRIBUTES sa;
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);

    wchar_t deamonPath[MAX_PATH];
    
    wsprintf(deamonPath, L"%ls %u", DEAMON_PROCESS_PATH, GetCurrentProcessId());
    if (!CreateProcess(NULL, deamonPath, &sa, &sa, TRUE, 0, NULL, NULL, &si, &pi))
    {
        printf("[CrashLogger][ERROR] Could not Create Deamon Process! Error Code: %d\n", GetLastError());
        return false;
    }
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (IsDebuggerPresent())
        {
            printf("[CrashLogger][Warning] Existing debugger detected. CrashLogger will not work.\n");
            return TRUE;
        }
        if (!InitDeamonProcess())
            return FALSE;
        printf("[CrashLogger] CrashLogger loaded.\n");
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

