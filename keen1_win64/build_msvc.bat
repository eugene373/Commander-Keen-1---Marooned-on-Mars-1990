@echo off
rem Visual Studio x64 Native Tools prompt:   build_msvc.bat C:\path\to\KEEN1.EXE
cl /nologo /O2 /W0 /TC /D_CRT_SECURE_NO_WARNINGS /Isrc /Fe:keen1_selftest.exe src\selftest.c src\unlzexe.c src\lifted.c || exit /b 1
if not "%~1"=="" keen1_selftest.exe "%~1" tests\vectors.txt
