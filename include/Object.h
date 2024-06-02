#ifndef OBJECT_H
#define OBJECT_H
#include <SFML/Graphics.hpp>

class Object : virtual public sf::Sprite {
 public:
  Object();
  Object(sf::Texture &tex);
  virtual void collide(Object *obj) = 0;
  virtual void drawCollision(sf::RenderTarget *target) const = 0;

  sf::Vector2f m_spd_vec;
  float m_mass;  // 0 if static
  static bool g_draw_collisions;
};

class Circle;
class Rectangle;

class Circle : public Object {
 public:
  Circle() : Object(), m_radius(1) {}
  Circle(sf::Texture &tex);
  Circle(sf::Texture &tex, float r) : Object(tex), m_radius(r) {}

  void collide(Object *obj) override;
  void collide(Circle *obj);
  void collide(Rectangle *obj);
  void drawCollision(sf::RenderTarget *target) const override;
  float getRadius() const { return m_radius; }

 private:
  float m_radius;
};

class Rectangle : public Object {
 public:
  Rectangle() : Object(), m_size(2, 2) {}
  void collide(Object *obj) override;
  void collide(Rectangle *obj);
  void collide(Circle *obj);
  void drawCollision(sf::RenderTarget *target) const override;
  sf::Vector2f getSize() const { return m_size; }

 private:
  sf::Vector2f m_size;
};

#endif
