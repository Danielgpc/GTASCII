# GTASCII - LOG

## 2026-08-19
- Created basic projet structure
    - Makefile compilation (currently only works on MacOS)
        - Creates an ```bin/``` folder and puts the binaries in there
        - Has **clearn**, **all** and **run** targets
    - Source files in ```src/```
        - Main.cc &rarr; Project main entry point containing
        - Player.cc/h &rarr; Player class (Not compiling and implemented yet)

## 2026-08-20
- Finaly finished the first playable demo of the game
    - Ended player class implementation with:
        - Constructor, destructor
        - Handle input, update and render
    - Added player.cc to **SRC** in Makefile
    - Created ```defines.h``` for public definitions
    - Added dt and traget FPS for main loop so the game is real time not turn based like other terminal-based roguelikes
    - Expanded world to a full **4×4 chunk grid** (64×64 tiles)
    - Added chunk files `assets/map/0_0.txt` … `3_3.txt` with varied glyph/color layouts
    - `loadAllChunks()` loads the entire map at startup
    - Camera follows the player and clamps at world edges
    - Player is clamped to world bounds so free walking stays inside the map
    - HUD shows world position and current chunk coordinates
- Fixed 120x40 layout with borders
    - Left: bordered game viewport (78x38 inner)
    - Right: info bar (FPS, POS, CHK, VEL) + scrollable log + chat input
    - Log format: HH:MM:SS  TAG  message (INFO/WARN/DEBUG/SYSTEM/player)
    - Press t to type in chat, Enter to send, Esc to cancel
    - New ui.h / ui.cc module
- Fixed resize glitches and broken borders
    - Switched from ACS box chars to plain ASCII (+ - |) so macOS no longer shows "?"
    - UI is a fixed 120x40 frame (does not stretch with terminal)
    - KEY_RESIZE handled: full clear + window recreate, leftover cells wiped

## 2026-08-21
- Updated player movement mechanics
    - Implemented running (Shift/Uppercase keys) and walking (lowercase keys) with distinct acceleration values.
    - Replaced global `MAX_SPEED` with a dynamic `currentMaxSpeed` variable to correctly enforce velocity caps based on the current movement state.
- Improved Makefile build system
    - Modified targets to compile individual `.o` object files into the `bin/` directory before linking the final executable.

## 2026-08-22
- Restructured terminal UI layout to match sketch
    - Top-left: fixed 120×40 game window
    - Top-right: info panel (fills remaining width) — FPS/POS/CHK/VEL + HP/STM colored bars
    - Bottom: full-width chat panel (fills remaining height)
- Split logging out of UI into `logger.h` / `logger.cc` (UI delegates to Logger)
- Renamed chunk module → map (`map.h` / `map.cc`); removed `chunk.h` / `chunk.cc`
- Chunk size 16→64; world still 4×4 chunks → **256×256 tiles**
- Chunk files now have three sections: GLYPHS, COLORS, TYPES
    - TYPES: 0 normal, 1 solid, 2 water, 3 closed door (4 = open door at runtime)
- Collision: solid tiles + closed doors block movement (axis-separated slide)
- Water: movement speed ×0.35 while standing on water tiles
- Doors: press `e` to toggle open/closed on adjacent/current door tile
- New creative world (`tools/generate_world.py`):
    - Road grid spanning chunks, residential houses, warehouses, downtown, industrial
    - Parks with trees/bushes/flowers, lakes, south-east coastline
    - Buildings intentionally cross chunk borders
- Glyph palette: `. , ' = - # H + T Y * ~ o` (richer than old `#=.` only)
- Chat commands: `/hp`, `/stm`, `/heal`, `/tp`
- Updated `tools/map_to_chunks.py` for 64×64 + TYPES section
