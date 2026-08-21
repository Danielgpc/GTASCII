#ifndef PLAYER_H
#define PLAYER_H

#include <ncurses.h>

class Player {
public:
  Player(float x = 0, float y = 0);
  ~Player() = default;

  void handleInput(int key);
  void update(float dt);
  void render(WINDOW* win, int viewX, int viewY, int camX, int camY) const;
  void clampToWorld(int worldW, int worldH);

  float x, y;
  float vx, vy;

  float accel;
  float friction = 6.0f;
  float currentMaxSpeed; // Replaces the global MAX_SPEED constant
};

#endif // !PLAYER_H