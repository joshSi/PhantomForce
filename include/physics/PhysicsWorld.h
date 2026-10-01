#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H
#include <SFML/Graphics/RenderTarget.hpp>
#include <functional>
#include <vector>

#include "physics/Body.h"
#include "physics/Collision.h"

namespace physics {

// A pair of overlapping bodies found during a step. The manifold normal points
// from `a` towards `b`.
struct Contact {
  Body *a;
  Body *b;
  Manifold manifold;
  // Separating speed the pair should have after bouncing (0 = no bounce).
  float restitution_bias = 0.f;
};

// 2D rigid-body simulation.
//
// Each call to step(dt):
//   1. finds overlapping pairs (sort-and-sweep broad phase on bounding boxes,
//      then exact circle / box tests),
//   2. decides which contacts bounce from the velocities before gravity is
//      applied, so resting bodies never pick up energy from gravity,
//   3. integrates forces, gravity and damping into velocities,
//   4. resolves every contact with restitution and friction impulses,
//   5. integrates velocities into positions,
//   6. nudges overlapping bodies apart so they do not sink into each other.
//
// Positions are in pixels, time in seconds. Call step() with a fixed dt for
// stable, frame-rate independent results. The world does not own its bodies.
class PhysicsWorld {
 public:
  using ContactCallback =
      std::function<void(Body &a, Body &b, const Manifold &manifold)>;

  void addBody(Body *body);
  void removeBody(Body *body);
  void clear();
  const std::vector<Body *> &getBodies() const { return m_bodies; }

  // Acceleration applied to every dynamic body, in pixels / s^2.
  void setGravity(const sf::Vector2f &gravity) { m_gravity = gravity; }
  const sf::Vector2f &getGravity() const { return m_gravity; }

  // Number of impulse-solver passes per step. More passes make stacks and
  // pile-ups settle faster at a small CPU cost.
  void setIterations(unsigned iterations) {
    m_iterations = iterations > 0 ? iterations : 1;
  }
  unsigned getIterations() const { return m_iterations; }

  // Called once per contact at the end of every step, e.g. for game logic.
  void setContactCallback(ContactCallback callback) {
    m_on_contact = std::move(callback);
  }

  // Advances the simulation by dt seconds.
  void step(float dt);

  // Contacts found during the most recent step.
  const std::vector<Contact> &getContacts() const { return m_contacts; }

  // Draws every body's collision shape plus the contact normals.
  void drawDebug(sf::RenderTarget &target) const;

 private:
  struct Proxy {
    Body *body;
    sf::FloatRect aabb;
  };

  void findContacts();
  void prepareContacts(float dt);
  void integrateForces(float dt);
  void resolveContact(const Contact &contact) const;
  void integrateVelocities(float dt);
  void correctPositions() const;

  std::vector<Body *> m_bodies;
  std::vector<Contact> m_contacts;
  std::vector<Proxy> m_proxies;  // scratch space for the broad phase
  sf::Vector2f m_gravity{0.f, 0.f};
  unsigned m_iterations = 8;
  ContactCallback m_on_contact;
};

}  // namespace physics

#endif
