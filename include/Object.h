#ifndef OBJECT_H
#define OBJECT_H
#include <SFML/Graphics.hpp>

#include "physics/Body.h"

// A drawable sprite that is also a rigid body in the physics engine. The
// body's position is the sprite's position, so moving one moves the other.
// See physics::Body for the simulation properties (mass, velocity, ...).
class Object : virtual public sf::Sprite, public physics::Body {
 public:
  Object();
  explicit Object(sf::Texture &tex);
  Object(sf::Texture &tex, const physics::Shape &shape);
  virtual ~Object() = default;

  // physics::Body position is backed by the sprite transform
  sf::Vector2f getPosition() const override {
    return sf::Transformable::getPosition();
  }
  void setPosition(sf::Vector2f position) override {
    sf::Transformable::setPosition(position);
  }

  // Draws the collision shape (not the sprite) for debugging.
  void drawCollision(sf::RenderTarget *target) const {
    physics::drawShape(*target, *this);
  }

  static bool g_draw_collisions;

 private:
  void centerOrigin();
};

class Circle : public Object {
 public:
  Circle();
  // Radius defaults to half the texture width.
  explicit Circle(sf::Texture &tex);
  Circle(sf::Texture &tex, float r);

  float getRadius() const { return getShape().radius; }
  void setRadius(float r) { setShape(physics::Shape::circle(r)); }
};

class Rectangle : public Object {
 public:
  Rectangle();
  // Size defaults to the texture size.
  explicit Rectangle(sf::Texture &tex);
  Rectangle(sf::Texture &tex, sf::Vector2f size);

  sf::Vector2f getSize() const { return getShape().half_size * 2.f; }
  void setSize(const sf::Vector2f &size) {
    setShape(physics::Shape::box(size));
  }
};

#endif
