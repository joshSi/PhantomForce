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
  m_last_pos = getPosition();
}

void Player::checkCollision() {
  if (m_objects_ref != nullptr)
    for (int i = 0; i < m_objects_ref->size(); i++) {
      if (Circle::checkCollision((*m_objects_ref)[i])) {
        if (dynamic_cast<Circle*>((*m_objects_ref)[i])) {
          const sf::Vector2f delta =
              (*m_objects_ref)[i]->getPosition() - getPosition();
          const sf::Vector2f tangent = {delta.y, -delta.x};
          const float cos =
              tangent.x * m_spd_vec.x + tangent.y * m_spd_vec.y / len(delta);

          m_spd_vec = cos * tangent / len(tangent) * 0.01f;
          snapCollision((*m_objects_ref)[i]);
        } else if (dynamic_cast<Rectangle*>((*m_objects_ref)[i])) {
          const sf::Vector2f delta =
              (*m_objects_ref)[i]->getPosition() - getPosition();
          if (abs(delta.x) /
                  static_cast<Rectangle*>((*m_objects_ref)[i])->getSize().x <
              abs(delta.y) /
                  static_cast<Rectangle*>((*m_objects_ref)[i])->getSize().y) {
            m_spd_vec = {0, m_spd_vec.y};
          } else {
            m_spd_vec = {m_spd_vec.x, 0};
          }
          snapCollision((*m_objects_ref)[i]);
        }
      }
    }
}

void Player::setObjects(std::vector<Object*>* objs) { m_objects_ref = objs; }
