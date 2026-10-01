#ifndef PLAYER_H
#define PLAYER_H
#include <SFML/Graphics.hpp>
#include <cstdint>

#include "Object.h"

struct MoveStats {
  float max_spd;  // Top speed in pixels per second
  float accel;    // Acceleration while a direction is held, pixels / s^2
  float fric;  // Linear damping (1/s): how quickly the player coasts to a stop
  float reload_rate;
};

// The player is a dynamic circular body. update() turns input into
// acceleration; the PhysicsWorld then moves it and resolves collisions.
class Player : public Circle {
 public:
  // Bits of the input mask passed to update()
  static constexpr uint8_t kRight = 1 << 0;
  static constexpr uint8_t kLeft = 1 << 1;
  static constexpr uint8_t kDown = 1 << 2;
  static constexpr uint8_t kUp = 1 << 3;

  Player(sf::Texture& tex, MoveStats* s, float r);

  // Applies movement input for a physics step of dt seconds.
  void update(uint8_t input, float dt, bool sprint = false);

 private:
  MoveStats* m_stat;
};

#endif
