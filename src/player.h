#ifndef PLAYER_H
class Player {
public:
  Player(float x = 0, float y = 0);
  Player(Player &&) = default;
  Player(const Player &) = default;
  Player &operator=(Player &&) = default;
  Player &operator=(const Player &) = default;
  ~Player();

private:
};

Player::Player() {}

Player::~Player() {}
#endif // !PLAYER_H
