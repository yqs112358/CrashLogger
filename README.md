# CrashLogger
A dll injected into process to dump stack when crashing.

Support me at https://www.minebbs.com/resources/crashlogger.2287/


### At Crash
- Result of stack walk will output both at console and file  .\logs\TrackBack.log
- Dumpbin database will generate at file .\logs\CrashDump.dmp

Stack walk can give you a general view of possible problems.
Using CrashDump, WinDbg and .pdb files, you can get more infomation about the crash.
