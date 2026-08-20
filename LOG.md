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