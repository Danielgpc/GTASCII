#include "player.h"
#include <algorithm>

Player::Player(float x, float y) : x(x), y(y), vx(0.0f), vy(0.0f) {
  accel = 15.0f;
  friction = 5.0f;
  currentMaxSpeed = 3.5f;
}

void Player::handleInput(int key) {
  // Check for uppercase keys (Shift)
  bool isRunning = (key == 'W' || key == 'A' || key == 'S' || key == 'D');
  
  if (isRunning) {
    accel = 10.0f;          // A bit lower than the original 15.0f
    currentMaxSpeed = 7.0f; // A bit lower than the original 10.0f[cite: 6]
  } else {
    accel = 6.0f;           
    currentMaxSpeed = 3.5f; 
  }

  switch (key) {
    case KEY_UP:    case 'w': case 'W': vy -= accel; break;
    case KEY_DOWN:  case 's': case 'S': vy += accel; break;
    case KEY_LEFT:  case 'a': case 'A': vx -= accel; break;
    case KEY_RIGHT: case 'd': case 'D': vx += accel; break;
    default: break;
  }
}

void Player::update(float dt) {
  if (dt > 0.1f) dt = 0.1f;

  x += vx * dt;
  y += vy * dt;

  float damping = std::max(0.0f, 1.0f - friction * dt);
  vx *= damping;
  vy *= damping;

  // velocity cap
  float speed = std::sqrt(vx * vx + vy * vy);
  if (speed > currentMaxSpeed) {
    float scale = currentMaxSpeed / speed;
    vx *= scale;
    vy *= scale;
  }
}

void Player::clampToWorld(int worldW, int worldH) {
  if (x < 0.0f) { x = 0.0f; vx = 0.0f; }
  if (y < 0.0f) { y = 0.0f; vy = 0.0f; }
  if (x >= static_cast<float>(worldW)) { x = static_cast<float>(worldW - 1); vx = 0.0f; }
  if (y >= static_cast<float>(worldH)) { y = static_cast<float>(worldH - 1); vy = 0.0f; }
}

void Player::render(WINDOW* win, int viewX, int viewY, int camX, int camY) const {
  int sx = static_cast<int>(x) - camX;
  int sy = static_cast<int>(y) - camY;
  if (sx >= 0 && sy >= 0) {
    // Caller ensures viewport bounds; still guard lightly
    mvwaddch(win, viewY + sy, viewX + sx, '@');
  }
}
