# Commander Keen 1 - native Windows x64 port

**This is not a recompile of KEEN1.EXE.** The original is a 16-bit DOS program (Turbo C++, real mode, EGA hardware),
and the `keen1_re` decompilation does not compile as-is (its own README says so). So this is a new native engine
written from what `keen1_re` verified: file formats, tile tables, physics, collision, player states, enemy rules,
sound data. It reads the **original data files**; none are included.

## Build (Visual Studio 2019+ and CMake)
1. Copy `LEVEL*.CK1 EGAHEAD.CK1 EGALATCH.CK1 EGASPRIT.CK1 SOUNDS.CK1` into `data\` (not KEEN1.EXE).
2. Double-click `build_win64.bat` (or `build_win64.bat mingw`). Output: `build-win64\Release\CommanderKeen1.exe`.
   The exe is named so it can never collide with `KEEN1.EXE` (case-insensitive file names).
It also finds data in its own folder, `CKeen1\`, `..\CKeen1`, or a folder given on the command line.
Add `-DKEEN_CONSOLE=ON` to keep a console for error messages.

## Controls
Arrows (+ Home/PgUp/End/PgDn or numpad for diagonals) move - **Ctrl or Z** jump (hold = higher) - **Alt or X** pogo -
**Space** fire (= jump+pogo, as in the original) - P pause - ESC leave level / quit - F11 or Alt+Enter fullscreen.
Cheats from the original: C+T+Space (items), G+O+D (god mode).

## What is implemented
Title, world map (entrances, ship popup, teleporter pads, invisible walls, done-markers), all 16 levels' mechanics:
walking/ice, wind-up jump, pogo, shooting, doors/keys, items and score, extra lives every 20000, ship parts, hint poles,
exit door, death, Yorp / Garg / Vorticon / CAN / TANK / chain / ice cannon, camera, 24.3 fps timing (tick_delta 6),
PC-speaker sounds rendered from SOUNDS.CK1 with the original priority rule.

## Not implemented / guessed (be aware)
- No save/load, high scores, story/help screens, demo, finale animation (a text ending is shown), level-16 platform drop.
- Guessed because keen1_re has no answer: Yorp stun length, Garg/Vorticon/TANK timings and TANK speed, ice-shot speed,
  pogo-exit button, world-map "done" tile layout, text box look. Marked `GUESS` in `src/game/play.c`.
- The status line is my own (the original has no always-on HUD).

## Testing status
Tested here only with gcc on Linux against a **synthetic** data set (`tools/make_test_data.py`) using a headless
platform (`tests/plat_null.c`): loads, runs 2500 scripted/random frames under AddressSanitizer/UBSan without errors.
**Not yet compiled with MSVC, not run on Windows, not run with the real game files.**

## Port incomplete work in progress