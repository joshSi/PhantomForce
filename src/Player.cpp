#include "Player.h"

namespace {
constexpr float kSprintMultiplier = 1.5f;
}

Player::Player(sf::Texture& tex, MoveStats* s, float r)
    : sf::Sprite(tex), Circle(tex, r), m_stat(s) {
  setMass(1.f);
  setFixedRotation(
      true);            // the player faces the mouse, collisions never spin it
  setRestitution(0.f);  // the player does not bounce off walls
  setFriction(0.f);     // ...and slides along them freely
  setLinearDamping(m_stat->fric);
}

void Player::update(uint8_t input, float dt, bool sprint) {
  const float boost = sprint ? kSprintMultiplier : 1.f;
  setLinearDamping(m_stat->fric);

  const sf::Vector2f dir(static_cast<float>((input & kRight) != 0) -
                             static_cast<float>((input & kLeft) != 0),
                         static_cast<float>((input & kDown) != 0) -
                             static_cast<float>((input & kUp) != 0));
  if (dir != sf::Vector2f(0.f, 0.f))
    m_velocity += dir.normalized() * (m_stat->accel * boost * dt);

  const float max_spd = m_stat->max_spd * boost;
  if (m_velocity.lengthSquared() > max_spd * max_spd)
    m_velocity = m_velocity.normalized() * max_spd;
}
