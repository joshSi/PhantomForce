#ifndef OBJECT_H
#define OBJECT_H
#include <SFML/Graphics.hpp>

class Object : virtual public sf::Sprite {
 public:
  Object();
  Object(sf::Texture &tex);
  virtual bool checkCollision(Object *obj) const = 0;
  virtual void snapCollision(Object *obj) = 0;
  virtual void drawCollision(sf::RenderTarget *target) const = 0;

  sf::Vector2f m_spd_vec;
  float m_mass;  // 0 if static
  static bool g_draw_collisions;
};

class Circle;
class Rectangle;

class Circle : public Object {
 public:
  Circle();
  Circle(sf::Texture &tex);
  Circle(sf::Texture &tex, float r);

  bool checkCollision(Object *obj) const override;
  bool checkCollision(Circle *obj) const;
  bool checkCollision(Rectangle *obj) const;

  void snapCollision(Object *obj) override;
  void snapCollision(Circle *obj);
  void snapCollision(Rectangle *obj);

  void drawCollision(sf::RenderTarget *target) const override;
  float getRadius() const { return m_radius; }

 private:
  float m_radius;
};

class Rectangle : public Object {
 public:
  Rectangle();
  Rectangle(sf::Texture &tex);
  Rectangle(sf::Texture &tex, sf::Vector2f size);

  bool checkCollision(Object *obj) const override;
  bool checkCollision(Rectangle *obj) const;
  bool checkCollision(Circle *obj) const;

  void snapCollision(Object *obj) override;
  void snapCollision(Circle *obj);
  void snapCollision(Rectangle *obj);

  void drawCollision(sf::RenderTarget *target) const override;
  const sf::Vector2f getSize() const { return m_size; }

 private:
  sf::Vector2f m_size;
};

#endif
