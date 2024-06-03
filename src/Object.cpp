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

Rectangle::Rectangle(sf::Texture &tex) {
  setTexture(tex);
  sf::Vector2f object_size =
      sf::Vector2f(getLocalBounds().width, getLocalBounds().height);
  setOrigin(object_size.x / 2, object_size.y / 2);
  m_size = object_size;
}

bool Circle::check_collision(Object *obj) const {
  if (dynamic_cast<Circle *>(obj)) {
    return check_collision(static_cast<Circle *>(obj));
  } else if (dynamic_cast<Rectangle *>(obj)) {
    return check_collision(static_cast<Rectangle *>(obj));
  }
}

bool Rectangle::check_collision(Object *obj) const {
  if (dynamic_cast<Circle *>(obj)) {
    return check_collision(static_cast<Circle *>(obj));
  } else if (dynamic_cast<Rectangle *>(obj)) {
    return check_collision(static_cast<Rectangle *>(obj));
  }
}

bool Circle::check_collision(Circle *obj) const {
  return len(obj->getPosition() - getPosition()) < (m_radius + obj->m_radius);
}

bool Circle::check_collision(Rectangle *obj) const {
  return (
    getPosition().x + m_radius >= obj->getPosition().x - obj->getSize().x / 2 &&
    getPosition().x - m_radius <= obj->getPosition().x + obj->getSize().x / 2 &&
    getPosition().y + m_radius >= obj->getPosition().y - obj->getSize().y / 2 &&
    getPosition().y - m_radius <= obj->getPosition().y + obj->getSize().y / 2
  );
}

bool Rectangle::check_collision(Circle *obj) const {
  return (
    getPosition().x + m_size.x >= obj->getPosition().x - obj->getRadius() / 2 &&
    getPosition().x - m_size.x <= obj->getPosition().x + obj->getRadius() / 2 &&
    getPosition().y + m_size.y >= obj->getPosition().y - obj->getRadius() / 2 &&
    getPosition().y - m_size.y <= obj->getPosition().y + obj->getRadius() / 2
  );
}

bool Rectangle::check_collision(Rectangle *obj) const {
  return (
    getPosition().x + m_size.x / 2 >= obj->getPosition().x - obj->getSize().x / 2 &&
    getPosition().x - m_size.x / 2 <= obj->getPosition().x + obj->getSize().x / 2 &&
    getPosition().y + m_size.y / 2 >= obj->getPosition().y - obj->getSize().y / 2 &&
    getPosition().y - m_size.y / 2 <= obj->getPosition().y + obj->getSize().y / 2
  );
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
