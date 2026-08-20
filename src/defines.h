#ifndef DEFINES_H
#define DEFINES_H

#define TARGET_FPS 1.0 / 30.0 // 30 FPS

// Fixed game viewport — does not stretch with the terminal
constexpr int GAME_W = 120;
constexpr int GAME_H = 40;

// Side panel keeps this vertical size; its width fills the rest of the terminal
constexpr int SIDE_H = 40;
constexpr int SIDE_MIN_W = 24; // minimum useful chat width

// Inner game viewport (inside the border)
constexpr int VIEW_X = 1;
constexpr int VIEW_Y = 1;
constexpr int VIEW_W = GAME_W - 2; // 118
constexpr int VIEW_H = GAME_H - 2; // 38

// Side panel section heights (vertical layout stays fixed)
constexpr int INFO_H = 8;    // top info bar
constexpr int INPUT_H = 5;   // bottom chat input (multiline)
constexpr int LOG_H  = SIDE_H - INFO_H - INPUT_H; // middle log

#endif // !DEFINES_H
