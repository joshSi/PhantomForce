#include "physics/Body.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>

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

sf::FloatRect Shape::aabb(sf::Vector2f center, float angle) const {
  if (type == Type::Circle)
    return sf::FloatRect(center - sf::Vector2f(radius, radius),
                         sf::Vector2f(2.f * radius, 2.f * radius));
  return BoxCollider{center, half_size, angle}.bounds();
}

float Shape::inertia(float mass) const {
  if (type == Type::Circle) return 0.5f * mass * radius * radius;
  // Solid rectangle: m (w^2 + h^2) / 12
  const sf::Vector2f size = half_size * 2.f;
  return mass * (size.x * size.x + size.y * size.y) / 12.f;
}

float Shape::groundRadius() const {
  if (type == Type::Circle) return radius;
  return (half_size.x + half_size.y) / 2.f;
}

bool collide(const Shape& a, sf::Vector2f pos_a, float angle_a, const Shape& b,
             sf::Vector2f pos_b, float angle_b, Manifold& out) {
  if (a.type == Shape::Type::Circle) {
    const CircleCollider ca{pos_a, a.radius};
    if (b.type == Shape::Type::Circle)
      return collide(ca, CircleCollider{pos_b, b.radius}, out);
    return collide(ca, BoxCollider{pos_b, b.half_size, angle_b}, out);
  }
  const BoxCollider ba{pos_a, a.half_size, angle_a};
  if (b.type == Shape::Type::Circle)
    return collide(ba, CircleCollider{pos_b, b.radius}, out);
  return collide(ba, BoxCollider{pos_b, b.half_size, angle_b}, out);
}

Body::Body(const Shape& shape) : m_shape(shape) {}

void Body::setShape(const Shape& shape) {
  m_shape = shape;
  updateInertia();
}

void Body::setMass(float mass) {
  m_mass = mass > 0.f ? mass : 0.f;
  m_inv_mass = m_mass > 0.f ? 1.f / m_mass : 0.f;
  updateInertia();
}

void Body::setFixedRotation(bool fixed) {
  m_fixed_rotation = fixed;
  updateInertia();
}

void Body::updateInertia() {
  m_inertia = m_shape.inertia(m_mass);
  m_inv_inertia =
      (m_inertia > 0.f && !m_fixed_rotation) ? 1.f / m_inertia : 0.f;
}

void Body::applyForceAtPoint(const sf::Vector2f& force,
                             const sf::Vector2f& world_point) {
  m_force += force;
  m_torque += cross(world_point - getPosition(), force);
}

void Body::applyImpulseAtPoint(const sf::Vector2f& impulse,
                               const sf::Vector2f& world_point) {
  m_velocity += impulse * m_inv_mass;
  m_angular_velocity +=
      m_inv_inertia * cross(world_point - getPosition(), impulse);
}

bool Body::collide(const Body& other, Manifold& out) const {
  return physics::collide(m_shape, getPosition(), getAngle(), other.m_shape,
                          other.getPosition(), other.getAngle(), out);
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
    // Spoke so the spin is visible
    const sf::Vector2f spoke = rotate({shape.radius, 0.f}, body.getAngle());
    sf::VertexArray line(sf::PrimitiveType::Lines, 2);
    line[0] = sf::Vertex{body.getPosition(), kDebugOutline};
    line[1] = sf::Vertex{body.getPosition() + spoke, kDebugOutline};
    target.draw(line);
  } else {
    sf::RectangleShape rect(shape.half_size * 2.f);
    rect.setOrigin(shape.half_size);
    rect.setPosition(body.getPosition());
    rect.setRotation(sf::radians(body.getAngle()));
    rect.setFillColor(kDebugFill);
    rect.setOutlineColor(kDebugOutline);
    rect.setOutlineThickness(-0.5f);
    target.draw(rect);
  }
}

}  // namespace physics
