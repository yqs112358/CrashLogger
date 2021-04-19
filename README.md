# CrashLogger
A dll injected into process to dump stack when crashing.

Support me at https://www.minebbs.com/resources/crashlogger.2287/


### At Crash
- Result of stack walk will output both at console and file  .\logs\TrackBack.log
- Dumpbin database will generate at file .\logs\CrashDump.dmp
![image](https://user-images.githubusercontent.com/37969157/115204373-fd948700-a12a-11eb-9352-7ffb7182fea1.png)
Stack walk can give you a general view of possible problems.

Using CrashDump, WinDbg and .pdb files, you can get more infomation about the crash.
