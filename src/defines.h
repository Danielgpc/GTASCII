#ifndef DEFINES_H
#define DEFINES_H

#define TARGET_FPS 1.0 / 30.0 // 30 FPS

// Fixed game viewport — does not stretch with the terminal
constexpr int GAME_W = 120;
constexpr int GAME_H = 40;

// Info panel: sits to the right of the game, same height
constexpr int INFO_MIN_W = 20; // minimum useful info width

// Chat panel: full terminal width, sits under game+info
constexpr int CHAT_MIN_H = 6;  // minimum useful chat height
constexpr int INPUT_H = 3;     // input strip inside chat panel

// Inner game viewport (inside the border)
constexpr int VIEW_X = 1;
constexpr int VIEW_Y = 1;
constexpr int VIEW_W = GAME_W - 2; // 118
constexpr int VIEW_H = GAME_H - 2; // 38

#endif // !DEFINES_H
