#include "pch.h"
#include <dbghelp.h>
#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <cstring>

#define MAX_STACK_FRAMES 50
#define LOG_OUTPUT_PATH ".\\logs\\TrackBack.log"
#define DUMP_OUTPUT_PATH L".\\logs\\CrashDump.dmp"

PTOP_LEVEL_EXCEPTION_FILTER SystemHandler;
FILE* fLog;

void log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    fflush(stdout);
    if (fLog != NULL)
    {
        vfprintf(fLog, format, args);
        fflush(fLog);
    }
    va_end(args);
}

LONG WINAPI CrashLogger(PEXCEPTION_POINTERS pe)
{
    log("[Crashed!]\n");
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    /// <summary>
    /// StackWalk
    /// </summary>
    /// <param name="pe"></param>
    /// <returns></returns>
    SymInitialize(hProcess, NULL, TRUE);
    void* pStack[MAX_STACK_FRAMES];
    WORD frames = CaptureStackBackTrace(0, MAX_STACK_FRAMES, pStack, NULL);

    for (WORD i = 0; i < frames; ++i) {
        DWORD64 address = (DWORD64)(pStack[i]);

        DWORD64 displacementSym = 0;
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        DWORD displacementLine = 0;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        //Function
        if (SymFromAddr(hProcess, address, &displacementSym, pSymbol))
            if(strcmp(pSymbol->Name,"UnhandledExceptionFilter") != 0)
                log("[TrackBack] Function %s at (0x%llX)\n", pSymbol->Name, pSymbol->Address);
        //Line
        if (SymGetLineFromAddr64(hProcess, address, &displacementLine, &line)) 
            log("[TrackBack] At File %s : Line %d \n", line.FileName, line.LineNumber);
    }
    SymCleanup(hProcess);

    /// <summary>
    /// CrashDump
    /// </summary>
    /// <param name="pe"></param>
    /// <returns></returns>
    HANDLE hDumpFile = CreateFile(DUMP_OUTPUT_PATH, GENERIC_WRITE, 0, NULL, 
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pe;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;
        MiniDumpWriteDump(hProcess, GetCurrentProcessId(), hDumpFile, MiniDumpNormal,
            &dumpInfo, NULL, NULL);
    }

    return SystemHandler(pe);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        SystemHandler = SetUnhandledExceptionFilter(CrashLogger);
        fopen_s(&fLog, LOG_OUTPUT_PATH, "w");
        printf("[CrashLogger] CrashLogger loaded.\n");
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        fclose(fLog);
        break;
    }
    return TRUE;
}

