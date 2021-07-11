#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
#include <string>
#include <filesystem>
#include <ctime>
#include <map>
#include "../include/LoggerShareData.h"
#include "SymbolHelper.h"

using namespace std;

extern HANDLE hProcess;
extern HANDLE hThread;
FILE* fLog;
HANDLE hDumpFile;

#define log(format,...)                         \
    printf(format, __VA_ARGS__);                \
    fflush(stdout);                             \
    if (fLog > 0)                               \
    {                                           \
        fprintf(fLog, format, __VA_ARGS__);     \
        fflush(fLog);                           \
    }  

string GetDateTime()
{
	time_t t = time(NULL);
	tm ts;
	localtime_s(&ts, &t);
	char buf[24] = { 0 };
	strftime(buf, 24, "%Y%m%d_%H-%M-%S", &ts);
	return string(buf);
}

bool CreateLogFiles()
{
	filesystem::create_directories(filesystem::path(DUMP_OUTPUT_PATH).remove_filename());
	string dateTimeStr = GetDateTime();

	string dumpPath = DUMP_OUTPUT_PATH + dateTimeStr + ".dmp";
	HANDLE hDumpFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile == INVALID_HANDLE_VALUE || hDumpFile == NULL)
	{
		log("[CrashLogger][ERROR] Fail to open CoreDump file! Error Code:%d\n", GetLastError());
	}

	string trackPath = TRACKBACK_OUTPUT_PATH + dateTimeStr + ".log";
	errno_t res = fopen_s(&fLog, trackPath.c_str(), "w");
	if (res != 0)
	{
		fLog = NULL;
		log("[CrashLogger][ERROR] Fail to open Log file! Error Code: %d\n", res);
	}

	return true;
}

void CoreDump(PEXCEPTION_POINTERS e)
{
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = e;
		dumpInfo.ThreadId = GetThreadId(hThread);
		dumpInfo.ClientPointers = FALSE;
		if (!MiniDumpWriteDump(hProcess, GetProcessId(hProcess), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL))
		{
			log("[CrashLogger][ERROR] Fail to Generate Minidump! Error Code: %u\n", GetLastError());
		}
		else
			printf("-- Minidump generated in Directory ./logs/Crash\n");
	}
}

void TrackBack(PEXCEPTION_POINTERS e)
{
	STACKFRAME64 stackFrame = { 0 };
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrPC.Offset = e->ContextRecord->Rip;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = e->ContextRecord->Rsp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = e->ContextRecord->Rbp;
	PCONTEXT pContext = e->ContextRecord;

	while (StackWalk64(MACHINE_TYPE, hProcess, hThread, &stackFrame, pContext, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
	{
		DWORD64 address = stackFrame.AddrPC.Offset;

		//Function
		PSYMBOL_INFO info;
		if (info = GetSymbolInfo(hProcess, (void*)stackFrame.AddrPC.Offset))
		{
			log("[TrackBack] Function %s at 0x%llX  [%ls]\n", info->Name, info->Address, MapModuleFromAddr(hProcess, (void*)address).c_str());

			//Line
			DWORD displacement = 0;
			IMAGEHLP_LINE64 line;
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

			if (SymGetLineFromAddr64(hProcess, address, &displacement, &line))
				log("-- At File %s : Line %d \n", line.FileName, line.LineNumber);
		}
		else
			log("[TrackBack] Function ???????? at 0x????????\n");
	}
}

void LogCrash(PEXCEPTION_POINTERS e)
{
	if (!CreateLogFiles() || !CreateModuleMap(hProcess))
		return;

	printf("\n");
	log("[Crashed!]\n");
	log("-- Unhandled Exception in -> %ls\n", MapModuleFromAddr(hProcess, e->ExceptionRecord->ExceptionAddress).c_str());
	log("-- Exception Code: %u\n", e->ExceptionRecord->ExceptionCode);
	if(e->ExceptionRecord->ExceptionCode == CRT_EXCEPTION_CODE)
		log("-- C++ STL Exception detected! CrashLogger may not give correct information about this exception.\n")

	CoreDump(e);
	CloseHandle(hDumpFile);

	TrackBack(e);
	if (fLog != NULL && fLog != INVALID_HANDLE_VALUE)
		fclose(fLog);

	log("\n");
}