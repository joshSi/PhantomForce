#ifndef PLAYER_H
#define PLAYER_H
#include <SFML/Graphics.hpp>

#include "Object.h"

struct MoveStats {
  float max_spd;
  float accel;
  float fric;
  float reload_rate;
};

class Player : public Circle {
 public:
  Player(sf::Texture& tex, MoveStats* s);
  void move(sf::Vector2f velocity, float fr, bool sprint, uint8_t input);

 private:
  void checkCollision();
  MoveStats* m_stat;
};

#endif
