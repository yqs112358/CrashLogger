@echo off
mkdir RELEASE
mkdir RELEASE\CrashLogger
copy /Y x64\Release\CrashLogger.dll RELEASE\
copy /Y x64\Release\CrashLogger_Deamon.exe RELEASE\CrashLogger\