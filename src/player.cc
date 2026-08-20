#include "player.h"
#include <ncurses.h>
#include <algorithm>

Player::Player(float x, float y) : x(x), y(y), vx(0.0f), vy(0.0f) {
  accel = 15.0f;   // Reduced impulse force for responsive tap controls
  friction = 5.0f; // Damping rate
}

void Player::handleInput(int key) {
  switch (key) {
    case KEY_UP:    case 'w': vy -= accel; break;
    case KEY_DOWN:  case 's': vy += accel; break;
    case KEY_LEFT:  case 'a': vx -= accel; break;
    case KEY_RIGHT: case 'd': vx += accel; break;
    default: break;
  }
}

void Player::update(float dt) {
  // Prevent dt spikes from breaking physics
  if (dt > 0.1f) dt = 0.1f;

  // Move player
  x += vx * dt;
  y += vy * dt;

  // Apply stable friction damping
  float damping = std::max(0.0f, 1.0f - friction * dt);
  vx *= damping;
  vy *= damping;
}

void Player::render() const {
  mvaddch(static_cast<int>(y), static_cast<int>(x), '@');
}