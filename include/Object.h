#ifndef OBJECT_H
#define OBJECT_H
#include <SFML/Graphics.hpp>

class Object : virtual public sf::Sprite {
 public:
  Object() : m_mass(1){};
  virtual void collide(Object *obj) = 0;

  sf::Vector2f spd_vec;
  float m_mass;
};

class Circle;
class Rectangle;

class Circle : public Object {
 public:
  Circle() : Object(), m_radius(1) {}
  void collide(Object *obj) override;
  void collide(Circle *obj);
  void collide(Rectangle *obj);
  float m_radius;
};

class Rectangle : public Object {
 public:
  Rectangle() : Object(), m_size(2, 2) {}
  void collide(Object *obj) override;
  void collide(Rectangle *obj);
  void collide(Circle *obj);
  sf::Vector2f m_size;
};

#endif
