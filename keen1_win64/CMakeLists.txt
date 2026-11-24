# Commander Keen 1 -- native Windows x64 port.
#
#   cmake -S . -B build-win64 -A x64                    (Visual Studio 2019+)
#   cmake --build build-win64 --config Release
#
#   cmake -S . -B build-win64 -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release     (MinGW-w64, x86_64)
#   cmake --build build-win64
cmake_minimum_required(VERSION 3.16)
project(keen1 C)

if(NOT WIN32)
  message(FATAL_ERROR "This CMake build targets Windows (Win32 window + GDI + winmm).")
endif()

option(KEEN_CONSOLE "Keep a console window next to the game (shows error messages)" OFF)

file(GLOB KEEN_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_SOURCE_DIR}/src/core/*.c
     ${CMAKE_SOURCE_DIR}/src/game/*.c)
list(APPEND KEEN_SOURCES ${CMAKE_SOURCE_DIR}/src/platform/plat_win32.c ${CMAKE_SOURCE_DIR}/src/platform/main.c)

add_executable(keen1 ${KEEN_SOURCES})
target_include_directories(keen1 PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/game)
# The original DOS program is called KEEN1.EXE. Windows file names are case-insensitive, so this build must not
# be called keen1.exe in case someone drops the original next to it.
set_target_properties(keen1 PROPERTIES OUTPUT_NAME "CommanderKeen1")
target_link_libraries(keen1 PRIVATE winmm gdi32 user32)
target_compile_definitions(keen1 PRIVATE _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)

if(MSVC)
  target_compile_options(keen1 PRIVATE /W3 /wd4244 /wd4267 /wd4996)
  if(NOT KEEN_CONSOLE)
    target_link_options(keen1 PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
  endif()
else()
  target_compile_options(keen1 PRIVATE -std=gnu99 -Wall -Wno-unused)
  target_link_options(keen1 PRIVATE -static)          # no libgcc/winpthread DLLs to ship
  if(NOT KEEN_CONSOLE)
    target_link_options(keen1 PRIVATE -mwindows)
  endif()
endif()

# Put the game data next to the exe (everything in data/ except documentation).
file(GLOB KEEN_DATA CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/data/*)
list(FILTER KEEN_DATA EXCLUDE REGEX "(KEEN1|DOS4GW)\\.EXE$|\\.BAT$|\\.txt$")
if(KEEN_DATA)
  add_custom_command(TARGET keen1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KEEN_DATA} $<TARGET_FILE_DIR:keen1>
    COMMENT "Copying game data")
endif()

# ---- headless smoke test (synthetic data, no window): scripted input, virtual clock -------------------------------
add_executable(keen1_smoke EXCLUDE_FROM_ALL ${CMAKE_SOURCE_DIR}/src/core/codecs.c ${CMAKE_SOURCE_DIR}/src/core/data.c
               ${CMAKE_SOURCE_DIR}/src/core/tiletab.c ${CMAKE_SOURCE_DIR}/src/game/game.c ${CMAKE_SOURCE_DIR}/src/game/play.c
               ${CMAKE_SOURCE_DIR}/src/game/map.c ${CMAKE_SOURCE_DIR}/src/game/render.c
               ${CMAKE_SOURCE_DIR}/src/platform/main.c ${CMAKE_SOURCE_DIR}/tests/plat_null.c)
target_include_directories(keen1_smoke PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/game)
target_compile_definitions(keen1_smoke PRIVATE _CRT_SECURE_NO_WARNINGS)
