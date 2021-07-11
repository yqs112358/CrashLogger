#include <windows.h>
#include <cstdio>
#include <string>
#include "../include/LoggerShareData.h"

#include <iostream>

using namespace std;

extern void DebuggerMain();
extern HANDLE hProcess;

int main(int argc,char **argv)
{
	if (argc <= 1)
	{
		printf("[CrashLogger] Don't execute this process independently.\n"
			"[CrashLogger] You must load CrashLogger plugin to be deamoned.\n");
		MessageBox(NULL, L"Don't execute this process independently.\r\nYou must load CrashLogger plugin to be deamoned.",L"Error",MB_OK);
		return 1;
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, stoul(argv[1]));
	if (hProcess == NULL)
	{
		printf("[CrashLogger][ERROR] Fail to Open the process deamoned! Error Code: %d\n", GetLastError());
		return 0;
	}

    printf("[CrashLogger] CrashLogger Deamon Process attached.\n");
	DebuggerMain();
	return 0;
}