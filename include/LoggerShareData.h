#pragma once

#define MACHINE_TYPE IMAGE_FILE_MACHINE_AMD64
#define DAEMON_PROCESS_PATH L".\\CrashLogger_Daemon.exe"
#define TRACKBACK_OUTPUT_PATH "logs\\Crash\\TrackBack_"
#define DUMP_OUTPUT_PATH "logs\\Crash\\CrashDump_"

#define CRT_EXCEPTION_CODE 0xE06D7363
#define CRT_RAISE_CODE CRT_EXCEPTION_CODE
#define SLEEP_BEFORE_ABORT 3