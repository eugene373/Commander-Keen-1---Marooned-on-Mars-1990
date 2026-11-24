@echo off
rem This port has no DOS/Watcom build; build.bat is just a shortcut for the native 64-bit build.
rem Usage: build.bat          (Visual Studio)      build.bat mingw      (MinGW-w64)
call "%~dp0build_win64.bat" %1
