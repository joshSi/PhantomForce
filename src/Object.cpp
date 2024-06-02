#include "Object.h"

#include "utils.h"

bool Object::g_draw_collisions = false;

Object::Object() : m_mass(0) {
  sf::Vector2f object_size =
      sf::Vector2f(getLocalBounds().width, getLocalBounds().height);
  setOrigin(object_size.x / 2, object_size.y / 2);
};

Object::Object(sf::Texture &tex) : m_mass(0) {
  setTexture(tex);
  sf::Vector2f object_size =
      sf::Vector2f(getLocalBounds().width, getLocalBounds().height);
  setOrigin(object_size.x / 2, object_size.y / 2);
};

Circle::Circle(sf::Texture &tex) {
  setTexture(tex);
  sf::Vector2f object_size =
      sf::Vector2f(getLocalBounds().width, getLocalBounds().height);
  setOrigin(object_size.x / 2, object_size.y / 2);
  m_radius = object_size.x / 2;
}

void Circle::collide(Object *obj) {
  if (dynamic_cast<Circle *>(obj)) {
    collide(static_cast<Circle *>(obj));
  } else if (dynamic_cast<Rectangle *>(obj)) {
    collide(static_cast<Rectangle *>(obj));
  }
}

void Rectangle::collide(Object *obj) {
  if (dynamic_cast<Circle *>(obj)) {
    collide(static_cast<Circle *>(obj));
  } else if (dynamic_cast<Rectangle *>(obj)) {
    collide(static_cast<Rectangle *>(obj));
  }
}

void Circle::collide(Circle *obj) {
  float dist = len(obj->getPosition() - getPosition());
  if (len(obj->getPosition() - getPosition()) < obj->getRadius() + m_radius) {
    if (obj->m_mass == 0) {
      const float weight = (obj->getRadius() + m_radius) / len(m_spd_vec);
      // perpendicular point forms right triangle between tangent pos & obj->pos
      const sf::Vector2f perpendicular(
          obj->getPosition().x + weight * m_spd_vec.y,
          obj->getPosition().y - weight * m_spd_vec.x);
      // direction of m_spd_vec, tangent of circles
      const float length = sqrtf(pow(obj->getRadius() + m_radius, 2) -
                                 len(perpendicular - obj->getPosition()));

      setPosition(perpendicular.x, perpendicular.y);
      // setPosition(perpendicular.x + length * m_spd_vec.x/len(m_spd_vec),
      // perpendicular.y - length * m_spd_vec.y/len(m_spd_vec));
      m_spd_vec = sf::Vector2f(0, 0);
    }
  }
}

void Circle::collide(Rectangle *obj) { float dist = len(obj->getPosition()); }

void Rectangle::collide(Circle *obj) {
  // TODO: Collision detection
}

void Rectangle::collide(Rectangle *obj) {
  // TODO: Collision detection
}

void Circle::drawCollision(sf::RenderTarget *target) const {
  sf::CircleShape circle(m_radius);
  circle.setOrigin(circle.getRadius(), circle.getRadius());
  circle.setPosition(getPosition());
  target->draw(circle);
}

void Rectangle::drawCollision(sf::RenderTarget *target) const {
  sf::RectangleShape rect(sf::Vector2f(m_size.x, m_size.y));
  rect.setOrigin(m_size.x / 2, m_size.y / 2);
  rect.setPosition(getPosition());
  target->draw(rect);
}
