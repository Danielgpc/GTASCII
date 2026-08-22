#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"
#include <ncurses.h>

class Player {
public:
  Player(float x = 0, float y = 0);
  ~Player() = default;

  void handleInput(int key);
  void update(float dt, const ChunkMap& world);
  void render(WINDOW* win, int viewX, int viewY, int camX, int camY) const;
  void clampToWorld(int worldW, int worldH);

  // Try to open/close a nearby door
  bool interactDoor(ChunkMap& world);

  float x, y;
  float vx, vy;

  float accel;
  float friction = 6.0f;
  float currentMaxSpeed;

  float maxHp = 100.0f;
  float hp;
  float maxStamina = 100.0f;
  float stamina;

private:
  bool running = false;
};

#endif // !PLAYER_H
