@echo off
rem Always build from this script's own directory, no matter how it's launched.
cd /d "%~dp0"
rem Native 64-bit Windows build (x86-64 / PE32+).
rem
rem When double-clicked this uses the Visual Studio CMake generator, which
rem auto-locates the installed MSVC toolchain -- no Developer Command Prompt
rem (vcvars) is required beforehand.
rem
rem To build with MinGW-w64 (gcc must be on PATH) instead:  build_win64.bat mingw
rem Put the original game data files in the data\ folder first (see data\PUT_GAME_FILES_HERE.txt).
if /i "%1"=="mingw" goto mingw
cmake -S . -B build-win64 -A x64 || goto fail
cmake --build build-win64 --config Release || goto fail
echo.
echo Built build-win64\Release\CommanderKeen1.exe
goto end
:mingw
cmake -S . -B build-win64 -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release || goto fail
cmake --build build-win64 || goto fail
echo.
echo Built build-win64\CommanderKeen1.exe
goto end
:fail
echo Build failed.
exit /b 1
:end
