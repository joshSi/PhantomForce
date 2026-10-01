#include "physics/Body.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace physics {

namespace {
const sf::Color kDebugFill(255, 255, 255, 60);
const sf::Color kDebugOutline(255, 255, 255, 200);
}  // namespace

Shape Shape::circle(float radius) {
  Shape s;
  s.type = Type::Circle;
  s.radius = radius;
  return s;
}

Shape Shape::box(sf::Vector2f size) {
  Shape s;
  s.type = Type::Box;
  s.half_size = size / 2.f;
  return s;
}

sf::FloatRect Shape::aabb(sf::Vector2f center) const {
  if (type == Type::Circle)
    return sf::FloatRect(center - sf::Vector2f(radius, radius),
                         sf::Vector2f(2.f * radius, 2.f * radius));
  return sf::FloatRect(center - half_size, half_size * 2.f);
}

bool collide(const Shape& a, sf::Vector2f pos_a, const Shape& b,
             sf::Vector2f pos_b, Manifold& out) {
  if (a.type == Shape::Type::Circle) {
    const CircleCollider ca{pos_a, a.radius};
    if (b.type == Shape::Type::Circle)
      return collide(ca, CircleCollider{pos_b, b.radius}, out);
    return collide(ca, BoxCollider{pos_b, b.half_size}, out);
  }
  const BoxCollider ba{pos_a, a.half_size};
  if (b.type == Shape::Type::Circle)
    return collide(ba, CircleCollider{pos_b, b.radius}, out);
  return collide(ba, BoxCollider{pos_b, b.half_size}, out);
}

Body::Body(const Shape& shape) : m_shape(shape) {}

void Body::setMass(float mass) {
  m_mass = mass > 0.f ? mass : 0.f;
  m_inv_mass = m_mass > 0.f ? 1.f / m_mass : 0.f;
}

bool Body::collide(const Body& other, Manifold& out) const {
  return physics::collide(m_shape, getPosition(), other.m_shape,
                          other.getPosition(), out);
}

bool Body::checkCollision(const Body& other) const {
  Manifold unused;
  return collide(other, unused);
}

void Body::snapCollision(const Body& other) {
  // Small gap so the two bodies no longer register as overlapping.
  constexpr float kSkin = 0.01f;
  Manifold m;
  if (collide(other, m)) translate(-m.normal * (m.penetration + kSkin));
}

void drawShape(sf::RenderTarget& target, const Body& body) {
  const Shape& shape = body.getShape();
  if (shape.type == Shape::Type::Circle) {
    sf::CircleShape circle(shape.radius);
    circle.setOrigin(sf::Vector2f(shape.radius, shape.radius));
    circle.setPosition(body.getPosition());
    circle.setFillColor(kDebugFill);
    circle.setOutlineColor(kDebugOutline);
    circle.setOutlineThickness(-0.5f);
    target.draw(circle);
  } else {
    sf::RectangleShape rect(shape.half_size * 2.f);
    rect.setOrigin(shape.half_size);
    rect.setPosition(body.getPosition());
    rect.setFillColor(kDebugFill);
    rect.setOutlineColor(kDebugOutline);
    rect.setOutlineThickness(-0.5f);
    target.draw(rect);
  }
}

}  // namespace physics
