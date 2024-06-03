#include "Player.h"

#include "utils.h"

Player::Player(sf::Texture& tex, MoveStats* s, float r)
    : Circle(tex, r), m_stat(s) {}

void Player::move(sf::Vector2f velocity, float fr, bool sprint, uint8_t input) {
  sf::Vector2f v(
      int((input & 0b00000001) != 0) - int((input & 0b00000010) != 0),
      int((input & 0b00000100) != 0) - int((input & 0b00001000) != 0));

  if (len(v)) m_spd_vec += v * (m_stat->accel * fr / len(v));

  if (len(m_spd_vec)) {
    m_spd_vec *= powf(m_stat->fric, fr);
    if (len(m_spd_vec) < 0.05f) m_spd_vec *= 0.f;
  }

  if (len(m_spd_vec) > m_stat->max_spd)
    m_spd_vec *= (m_stat->max_spd / len(m_spd_vec));

  sf::Sprite::move(m_spd_vec * fr);
  checkCollision();
	// printf("Framerate: %f", fr);
	m_last_pos = getPosition();
}

void Player::checkCollision() {
  if (m_objects_ref != nullptr)
    for (int i = 0; i < m_objects_ref->size(); i++) {
      if (Circle::check_collision((*m_objects_ref)[i]))
			{
				sf::Sprite::setPosition(m_last_pos);
				m_spd_vec = sf::Vector2f(0.f, 0.f);
			}
    }
}

void Player::setObjects(std::vector<Object*>* objs) { m_objects_ref = objs; }
