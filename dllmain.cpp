#include "pch.h"
#include <dbghelp.h>
#include <cstdio>
#include <cstdlib>
#include <crtdbg.h>
#include <eh.h>
#include <cstddef>
#include <cstdarg>
#include <cstring>

#define MAX_STACK_FRAMES 50
#define LOG_OUTPUT_PATH ".\\logs\\TrackBack.log"
#define DUMP_OUTPUT_PATH L".\\logs\\CrashDump.dmp"
#define CRT_ERR_CODE 0xE0000001
#define SLEEP_BEFORE_ABORT 3000

FILE* fLog;
HANDLE hDumpFile;
char moduleName[MAX_PATH] = { 0 };
bool inSEH = true;

#define log(format,...)                         \
    printf(format, __VA_ARGS__);                \
    fflush(stdout);                             \
    if (fLog > 0)                               \
    {                                           \
        fprintf(fLog, format, __VA_ARGS__);     \
        fflush(fLog);                           \
    }                   

void ReadModuleName(void *address)
{
    HMODULE hModule;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)(address), &hModule);
    GetModuleFileNameA(hModule, moduleName, MAX_PATH);
}

// CRT exception
void CrtInvalidParameterHandler(const wchar_t* expression,const wchar_t* function,
    const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
    log("\n[CRT Exception] CRT Invalid Exception detected! Expression: %ls", expression);
    RaiseException(CRT_ERR_CODE, EXCEPTION_NONCONTINUABLE, 0, NULL);
}
void CrtPurecallHandler()
{
    log("\n[CRT Exception] CRT Purecall Exception detected!");
    RaiseException(CRT_ERR_CODE, EXCEPTION_NONCONTINUABLE, 0, NULL);
}
void CrtTerminationHandler()
{
    log("\n[CRT Exception] CRT Termination detected!");
    RaiseException(CRT_ERR_CODE, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

LONG WINAPI CrashLogger(PEXCEPTION_POINTERS pe)
{
    HANDLE hProcess = GetCurrentProcess();
    ReadModuleName(pe->ExceptionRecord->ExceptionAddress);
    log("\n[Crashed!] From module <%s>\n", moduleName);


    ////////// CrashDump //////////
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pe;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;
        MiniDumpWriteDump(hProcess, GetCurrentProcessId(), hDumpFile, MiniDumpNormal,
            &dumpInfo, NULL, NULL);
        log("[CrashLogger] Minidump generated at %ls\n", DUMP_OUTPUT_PATH);
    }

    ////////// StackWalk //////////
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
        {
            if (strcmp(pSymbol->Name, "KiUserExceptionDispatcher") == 0)
            {
                inSEH = false;
                continue;
            }
            if (!inSEH)
            {
                ReadModuleName((void*)(uintptr_t)(pSymbol->Address));
                log("[TrackBack] Function %s at (0x%llX) [%s]\n", pSymbol->Name, pSymbol->Address, moduleName);
            }
        }
        else
            log("[TrackBack] Function ???????? at (0x????????)\n");
        //Line
        if (!inSEH && SymGetLineFromAddr64(hProcess, address, &displacementLine, &line))
            log("[TrackBack] At File %s : Line %d \n", line.FileName, line.LineNumber);
    }
    SymCleanup(hProcess);


    log("\n");
    if (fLog != NULL && fLog != INVALID_HANDLE_VALUE)
        fclose(fLog);

    Sleep(SLEEP_BEFORE_ABORT);
    return EXCEPTION_CONTINUE_SEARCH;
}

void InitCrashLogger()
{
    //SEH
    SetUnhandledExceptionFilter(CrashLogger);
    //CRT
    _set_invalid_parameter_handler(CrtInvalidParameterHandler);
    _set_purecall_handler(CrtPurecallHandler);
    set_terminate(CrtTerminationHandler);

    //Logger
    CreateDirectory(L"logs", NULL);

    HANDLE hDumpFile = CreateFile(DUMP_OUTPUT_PATH, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile == INVALID_HANDLE_VALUE)
    {
        log("[CrashLogger][ERROR] Fail to open CoreDump file! Error Code:%d\n", GetLastError());
    }

    errno_t res = fopen_s(&fLog, LOG_OUTPUT_PATH, "a");
    if (res != 0)
    {
        fLog = NULL;
        log("[CrashLogger][ERROR] Fail to open Log file! Error Code:%d\n", res);
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        InitCrashLogger();
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

