#include "player.h"
#include <algorithm>
#include <cmath>

Player::Player(float x, float y) : x(x), y(y), vx(0.0f), vy(0.0f) {
  accel = 15.0f;
  friction = 5.0f;
  currentMaxSpeed = 3.5f;
  stamina = maxStamina;
  hp = maxHp;
}

void Player::handleInput(int key) {
  running = (key == 'W' || key == 'A' || key == 'S' || key == 'D');

  if (running) {
    accel = 10.0f;
    currentMaxSpeed = 7.0f;
  } else {
    accel = 6.0f;
    currentMaxSpeed = 3.5f;
  }

  switch (key) {
  case KEY_UP:
  case 'w':
  case 'W':
    vy -= accel;
    break;
  case KEY_DOWN:
  case 's':
  case 'S':
    vy += accel;
    break;
  case KEY_LEFT:
  case 'a':
  case 'A':
    vx -= accel;
    break;
  case KEY_RIGHT:
  case 'd':
  case 'D':
    vx += accel;
    break;
  default:
    break;
  }
}

void Player::update(float dt, const ChunkMap& world) {
  if (dt > 0.1f) dt = 0.1f;

  // Water slows both walk and run
  bool inWater = isWater(static_cast<int>(x), static_cast<int>(y), world);
  float speedMul = inWater ? 0.35f : 1.0f;
  float maxSpd = currentMaxSpeed * speedMul;

  float newX = x + vx * dt;
  float newY = y + vy * dt;

  // Axis-separated collision so you can slide along walls
  int tryX = static_cast<int>(newX);
  int tryY = static_cast<int>(y);
  if (!isSolid(tryX, tryY, world)) {
    x = newX;
  } else {
    vx = 0.0f;
  }

  tryX = static_cast<int>(x);
  tryY = static_cast<int>(newY);
  if (!isSolid(tryX, tryY, world)) {
    y = newY;
  } else {
    vy = 0.0f;
  }

  float damping = std::max(0.0f, 1.0f - friction * dt);
  vx *= damping;
  vy *= damping;

  float speed = std::sqrt(vx * vx + vy * vy);
  if (speed > maxSpd && speed > 0.0001f) {
    float scale = maxSpd / speed;
    vx *= scale;
    vy *= scale;
  }
}

bool Player::interactDoor(ChunkMap& world) {
  return tryToggleDoor(static_cast<int>(x), static_cast<int>(y), world);
}

void Player::clampToWorld(int worldW, int worldH) {
  if (x < 0.0f) { x = 0.0f; vx = 0.0f; }
  if (y < 0.0f) { y = 0.0f; vy = 0.0f; }
  if (x >= static_cast<float>(worldW)) {
    x = static_cast<float>(worldW - 1);
    vx = 0.0f;
  }
  if (y >= static_cast<float>(worldH)) {
    y = static_cast<float>(worldH - 1);
    vy = 0.0f;
  }
}

void Player::render(WINDOW* win, int viewX, int viewY, int camX, int camY) const {
  int sx = static_cast<int>(x) - camX;
  int sy = static_cast<int>(y) - camY;
  if (sx >= 0 && sy >= 0) {
    mvwaddch(win, viewY + sy, viewX + sx, '@');
  }
}
