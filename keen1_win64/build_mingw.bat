@echo off
rem MinGW-w64 (gcc), 64-bit:   build_mingw.bat C:\path\to\KEEN1.EXE
gcc -std=gnu99 -O2 -w -Isrc -o keen1_selftest.exe src\selftest.c src\unlzexe.c src\lifted.c || exit /b 1
if not "%~1"=="" keen1_selftest.exe "%~1" tests\vectors.txt
