#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>

#include "physics/Collision.h"

namespace physics {

// Collision shape of a body, centred on the body's position.
struct Shape {
  enum class Type { Circle, Box };

  Type type = Type::Circle;
  float radius = 1.f;                // used when type == Circle
  sf::Vector2f half_size{1.f, 1.f};  // used when type == Box

  static Shape circle(float radius);
  static Shape box(sf::Vector2f size);  // full width and height

  // World-space bounding box of the shape placed at `center`.
  sf::FloatRect aabb(sf::Vector2f center) const;
};

// Narrow-phase test between two placed shapes. On success `out.normal` points
// from a towards b.
bool collide(const Shape& a, sf::Vector2f pos_a, const Shape& b,
             sf::Vector2f pos_b, Manifold& out);

// A rigid body simulated by the PhysicsWorld.
//
// A body with a mass of 0 (the default) is static: it has infinite mass and is
// never moved by collisions. A static body with a non-zero velocity behaves
// kinematically: it keeps moving and pushes everything out of its way.
//
// The position accessors are virtual so a body can live inside another object
// that already stores a position (e.g. an sf::Transformable sprite).
class Body {
 public:
  Body() = default;
  explicit Body(const Shape& shape);
  virtual ~Body() = default;

  virtual sf::Vector2f getPosition() const { return m_position; }
  virtual void setPosition(sf::Vector2f position) { m_position = position; }
  void translate(const sf::Vector2f& delta) {
    setPosition(getPosition() + delta);
  }

  const Shape& getShape() const { return m_shape; }
  void setShape(const Shape& shape) { m_shape = shape; }
  sf::FloatRect getAABB() const { return m_shape.aabb(getPosition()); }

  // Narrow-phase test against another body. When they overlap, `out` is filled
  // with a normal pointing from this body towards `other`.
  bool collide(const Body& other, Manifold& out) const;
  bool checkCollision(const Body& other) const;
  // Moves this body the shortest distance that gets it out of `other`.
  void snapCollision(const Body& other);

  // --- Dynamics ------------------------------------------------------------

  // A mass of 0 makes the body static.
  void setMass(float mass);
  float getMass() const { return m_mass; }
  float getInverseMass() const { return m_inv_mass; }
  bool isStatic() const { return m_inv_mass == 0.f; }

  // Velocity in pixels per second.
  void setVelocity(const sf::Vector2f& velocity) { m_velocity = velocity; }
  const sf::Vector2f& getVelocity() const { return m_velocity; }

  // Bounciness, clamped to [0, 1]: 0 is perfectly inelastic, 1 perfectly
  // elastic. The lower of the two values is used when bodies collide.
  void setRestitution(float restitution) {
    m_restitution = std::clamp(restitution, 0.f, 1.f);
  }
  float getRestitution() const { return m_restitution; }

  // Coulomb friction coefficient (>= 0) applied where bodies touch. The
  // geometric mean of the two values is used when bodies collide.
  void setFriction(float friction) { m_friction = friction; }
  float getFriction() const { return m_friction; }

  // Exponential velocity decay rate in 1/s: after t seconds a coasting body
  // keeps exp(-damping * t) of its speed. 0 means it never slows down.
  void setLinearDamping(float damping) { m_damping = damping; }
  float getLinearDamping() const { return m_damping; }

  // Accumulates a force (mass * pixels / s^2) to apply during the next step.
  void applyForce(const sf::Vector2f& force) { m_force += force; }
  const sf::Vector2f& getForce() const { return m_force; }
  void clearForces() { m_force = {0.f, 0.f}; }
  // Instantly changes the velocity by impulse / mass.
  void applyImpulse(const sf::Vector2f& impulse) {
    m_velocity += impulse * m_inv_mass;
  }

 protected:
  sf::Vector2f m_velocity{0.f, 0.f};
  sf::Vector2f m_force{0.f, 0.f};
  float m_mass = 0.f;
  float m_inv_mass = 0.f;
  float m_restitution = 0.2f;
  float m_friction = 0.3f;
  float m_damping = 0.f;

 private:
  Shape m_shape;
  sf::Vector2f m_position{0.f, 0.f};  // unused when getPosition is overridden
};

// Draws the body's collision shape for debugging.
void drawShape(sf::RenderTarget& target, const Body& body);

}  // namespace physics

#endif
