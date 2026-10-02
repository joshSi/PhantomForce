#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>

#include "physics/Collision.h"
#include "physics/Surface.h"

namespace physics {

// Collision shape of a body, centred on the body's position and rotated by
// the body's angle.
struct Shape {
  enum class Type { Circle, Box };

  Type type = Type::Circle;
  float radius = 1.f;                // used when type == Circle
  sf::Vector2f half_size{1.f, 1.f};  // used when type == Box

  static Shape circle(float radius);
  static Shape box(sf::Vector2f size);  // full width and height

  // World-space bounding box of the shape placed at `center`, rotated by
  // `angle` radians.
  sf::FloatRect aabb(sf::Vector2f center, float angle = 0.f) const;
  // Moment of inertia about the centre for a body of the given mass.
  float inertia(float mass) const;
  // Typical distance from the centre to the ground contact area, used to
  // turn floor friction into a torque on spinning bodies.
  float groundRadius() const;
};

// Narrow-phase test between two placed shapes. On success `out.normal` points
// from a towards b.
bool collide(const Shape& a, sf::Vector2f pos_a, float angle_a, const Shape& b,
             sf::Vector2f pos_b, float angle_b, Manifold& out);

// A rigid body simulated by the PhysicsWorld.
//
// A body with a mass of 0 (the default) is static: it has infinite mass and is
// never moved by collisions. A static body with a non-zero velocity behaves
// kinematically: it keeps moving and pushes everything out of its way.
//
// Bodies also rotate: collisions away from the centre of mass spin them, and
// a spinning body spins or drags whatever it touches. Angles are in radians
// and increase clockwise on screen (y points down). setFixedRotation(true)
// stops collisions from turning a body, e.g. for a player that always faces
// the mouse.
//
// The position and angle accessors are virtual so a body can live inside
// another object that already stores them (e.g. an sf::Transformable sprite).
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
  // Rotation in radians
  virtual float getAngle() const { return m_angle; }
  virtual void setAngle(float radians) { m_angle = radians; }

  const Shape& getShape() const { return m_shape; }
  void setShape(const Shape& shape);
  sf::FloatRect getAABB() const {
    return m_shape.aabb(getPosition(), getAngle());
  }

  // Narrow-phase test against another body. When they overlap, `out` is filled
  // with a normal pointing from this body towards `other`.
  bool collide(const Body& other, Manifold& out) const;
  bool checkCollision(const Body& other) const;
  // Moves this body the shortest distance that gets it out of `other`.
  void snapCollision(const Body& other);

  // --- Dynamics ------------------------------------------------------------

  // A mass of 0 makes the body static. The moment of inertia follows from the
  // mass and the shape.
  void setMass(float mass);
  float getMass() const { return m_mass; }
  float getInverseMass() const { return m_inv_mass; }
  bool isStatic() const { return m_inv_mass == 0.f; }
  float getInertia() const { return m_inertia; }
  float getInverseInertia() const { return m_inv_inertia; }

  // When fixed, collisions never change the angular velocity (the body can
  // still be rotated directly or given an angular velocity by hand).
  void setFixedRotation(bool fixed);
  bool isFixedRotation() const { return m_fixed_rotation; }

  // Velocity in pixels per second.
  void setVelocity(const sf::Vector2f& velocity) { m_velocity = velocity; }
  const sf::Vector2f& getVelocity() const { return m_velocity; }
  // Angular velocity in radians per second (positive = clockwise on screen).
  void setAngularVelocity(float omega) { m_angular_velocity = omega; }
  float getAngularVelocity() const { return m_angular_velocity; }
  // Velocity of the point `world_point` as it is carried by this body.
  sf::Vector2f getVelocityAtPoint(const sf::Vector2f& world_point) const {
    return m_velocity + cross(m_angular_velocity, world_point - getPosition());
  }

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
  // Same for the angular velocity.
  void setAngularDamping(float damping) { m_angular_damping = damping; }
  float getAngularDamping() const { return m_angular_damping; }

  // Whether the body rests on the floor and feels its surface (friction,
  // drag, grip). Turn off for projectiles and anything airborne.
  void setOnGround(bool on_ground) { m_on_ground = on_ground; }
  bool isOnGround() const { return m_on_ground; }
  // The floor under the body, refreshed by the world every step.
  const Surface& getSurface() const { return m_surface; }
  void setSurface(const Surface& surface) { m_surface = surface; }

  // Accumulates a force (mass * pixels / s^2) through the centre of mass to
  // apply during the next step.
  void applyForce(const sf::Vector2f& force) { m_force += force; }
  // Accumulates a force acting at `world_point`, which also produces torque.
  void applyForceAtPoint(const sf::Vector2f& force,
                         const sf::Vector2f& world_point);
  // Accumulates a torque (mass * pixels^2 / s^2) for the next step.
  void applyTorque(float torque) { m_torque += torque; }
  const sf::Vector2f& getForce() const { return m_force; }
  float getTorque() const { return m_torque; }
  void clearForces() {
    m_force = {0.f, 0.f};
    m_torque = 0.f;
  }
  // Instantly changes the velocity by impulse / mass.
  void applyImpulse(const sf::Vector2f& impulse) {
    m_velocity += impulse * m_inv_mass;
  }
  // Instantly applies an impulse at `world_point`, which also spins the body.
  void applyImpulseAtPoint(const sf::Vector2f& impulse,
                           const sf::Vector2f& world_point);

 protected:
  sf::Vector2f m_velocity{0.f, 0.f};
  sf::Vector2f m_force{0.f, 0.f};
  float m_angular_velocity = 0.f;
  float m_torque = 0.f;
  float m_mass = 0.f;
  float m_inv_mass = 0.f;
  float m_inertia = 0.f;
  float m_inv_inertia = 0.f;
  bool m_fixed_rotation = false;
  float m_restitution = 0.2f;
  float m_friction = 0.3f;
  float m_damping = 0.f;
  float m_angular_damping = 0.f;
  bool m_on_ground = true;
  Surface m_surface;

 private:
  void updateInertia();

  Shape m_shape;
  sf::Vector2f m_position{0.f, 0.f};  // unused when getPosition is overridden
  float m_angle = 0.f;                // unused when getAngle is overridden
};

// Draws the body's collision shape for debugging.
void drawShape(sf::RenderTarget& target, const Body& body);

}  // namespace physics

#endif
