#include "physics/PhysicsWorld.h"

#include <SFML/Graphics/VertexArray.hpp>
#include <algorithm>
#include <cmath>

namespace physics {

namespace {

// Overlap (in pixels) tolerated before positional correction kicks in. Leaving
// a hair of penetration keeps resting contacts alive between steps.
constexpr float kPenetrationSlop = 0.01f;
// Fraction of the remaining penetration removed per step. Less than 1 avoids
// visible popping when several bodies overlap at once.
constexpr float kCorrectionPercent = 0.8f;
// Approach speeds below this (plus two steps' worth of gravity) are treated as
// resting contacts and do not bounce.
constexpr float kRestingSpeed = 0.01f;
// Speeds below this are snapped to zero so damped bodies come to a full stop.
constexpr float kSleepSpeed = 0.01f;
constexpr float kEpsilon = 1e-6f;

}  // namespace

void PhysicsWorld::addBody(Body *body) {
  if (body == nullptr) return;
  if (std::find(m_bodies.begin(), m_bodies.end(), body) != m_bodies.end())
    return;
  m_bodies.push_back(body);
}

void PhysicsWorld::removeBody(Body *body) {
  m_bodies.erase(std::remove(m_bodies.begin(), m_bodies.end(), body),
                 m_bodies.end());
  m_contacts.erase(std::remove_if(m_contacts.begin(), m_contacts.end(),
                                  [body](const Contact &c) {
                                    return c.a == body || c.b == body;
                                  }),
                   m_contacts.end());
}

void PhysicsWorld::clear() {
  m_bodies.clear();
  m_contacts.clear();
}

void PhysicsWorld::step(float dt) {
  if (dt <= 0.f) return;

  findContacts();
  prepareContacts(dt);
  integrateForces(dt);
  for (unsigned i = 0; i < m_iterations; ++i)
    for (const Contact &contact : m_contacts) resolveContact(contact);
  integrateVelocities(dt);
  correctPositions();

  if (m_on_contact)
    for (const Contact &contact : m_contacts)
      m_on_contact(*contact.a, *contact.b, contact.manifold);
}

void PhysicsWorld::findContacts() {
  m_contacts.clear();

  // Broad phase: sort bounding boxes along x and only test pairs whose x
  // ranges overlap (sort-and-sweep).
  m_proxies.clear();
  m_proxies.reserve(m_bodies.size());
  for (Body *body : m_bodies) m_proxies.push_back({body, body->getAABB()});
  std::sort(m_proxies.begin(), m_proxies.end(),
            [](const Proxy &lhs, const Proxy &rhs) {
              return lhs.aabb.position.x < rhs.aabb.position.x;
            });

  for (std::size_t i = 0; i < m_proxies.size(); ++i) {
    const Proxy &pa = m_proxies[i];
    const float right = pa.aabb.position.x + pa.aabb.size.x;
    for (std::size_t j = i + 1; j < m_proxies.size(); ++j) {
      const Proxy &pb = m_proxies[j];
      if (pb.aabb.position.x >= right) break;  // nothing further can overlap
      if (pa.body->isStatic() && pb.body->isStatic()) continue;
      if (!overlaps(pa.aabb, pb.aabb)) continue;

      // Narrow phase
      Manifold manifold;
      if (pa.body->collide(*pb.body, manifold))
        m_contacts.push_back({pa.body, pb.body, manifold});
    }
  }
}

void PhysicsWorld::prepareContacts(float dt) {
  // The bounce speed is taken from the velocities before this step's gravity
  // is added. Measuring it afterwards would make every resting body bounce
  // off the ground a little each step and slowly gain energy.
  const float threshold = 2.f * (m_gravity * dt).length() + kRestingSpeed;
  for (Contact &contact : m_contacts) {
    const float approach =
        -(contact.b->getVelocity() - contact.a->getVelocity())
             .dot(contact.manifold.normal);
    if (approach > threshold) {
      const float restitution =
          std::min(contact.a->getRestitution(), contact.b->getRestitution());
      contact.restitution_bias = restitution * approach;
    } else {
      contact.restitution_bias = 0.f;
    }
  }
}

void PhysicsWorld::integrateForces(float dt) {
  for (Body *body : m_bodies) {
    sf::Vector2f velocity = body->getVelocity();
    if (!body->isStatic())
      velocity += (body->getForce() * body->getInverseMass() + m_gravity) * dt;
    if (body->getLinearDamping() > 0.f)
      velocity *= std::exp(-body->getLinearDamping() * dt);
    if (velocity.lengthSquared() < kSleepSpeed * kSleepSpeed)
      velocity = {0.f, 0.f};
    body->setVelocity(velocity);
  }
}

void PhysicsWorld::resolveContact(const Contact &contact) const {
  Body &a = *contact.a;
  Body &b = *contact.b;
  const float inv_mass_a = a.getInverseMass();
  const float inv_mass_b = b.getInverseMass();
  const float inv_mass_sum = inv_mass_a + inv_mass_b;
  if (inv_mass_sum == 0.f) return;  // two static bodies

  // Normal impulse: bring the separating speed up to the bounce target
  // (0 for a resting contact, e * approach speed for a bounce).
  const sf::Vector2f normal = contact.manifold.normal;
  sf::Vector2f relative = b.getVelocity() - a.getVelocity();
  const float along_normal = relative.dot(normal);
  const float j = (contact.restitution_bias - along_normal) / inv_mass_sum;
  if (j <= 0.f) return;  // already separating fast enough
  const sf::Vector2f impulse = normal * j;
  a.setVelocity(a.getVelocity() - impulse * inv_mass_a);
  b.setVelocity(b.getVelocity() + impulse * inv_mass_b);

  // Coulomb friction along the contact tangent, clamped by the normal impulse.
  relative = b.getVelocity() - a.getVelocity();
  sf::Vector2f tangent = relative - normal * relative.dot(normal);
  const float tangent_len_sq = tangent.lengthSquared();
  if (tangent_len_sq < kEpsilon) return;
  tangent /= std::sqrt(tangent_len_sq);

  const float mu = std::sqrt(a.getFriction() * b.getFriction());
  const float max_friction = j * mu;
  const float jt = std::clamp(-relative.dot(tangent) / inv_mass_sum,
                              -max_friction, max_friction);
  const sf::Vector2f friction = tangent * jt;
  a.setVelocity(a.getVelocity() - friction * inv_mass_a);
  b.setVelocity(b.getVelocity() + friction * inv_mass_b);
}

void PhysicsWorld::integrateVelocities(float dt) {
  for (Body *body : m_bodies) {
    // Static bodies with a velocity act as kinematic movers.
    const sf::Vector2f velocity = body->getVelocity();
    if (velocity != sf::Vector2f(0.f, 0.f)) body->translate(velocity * dt);
    body->clearForces();
  }
}

void PhysicsWorld::correctPositions() const {
  for (const Contact &contact : m_contacts) {
    Body &a = *contact.a;
    Body &b = *contact.b;
    const float inv_mass_sum = a.getInverseMass() + b.getInverseMass();
    if (inv_mass_sum == 0.f) continue;

    const float depth =
        std::max(contact.manifold.penetration - kPenetrationSlop, 0.f);
    if (depth == 0.f) continue;
    const sf::Vector2f correction =
        contact.manifold.normal * (depth * kCorrectionPercent / inv_mass_sum);
    a.translate(-correction * a.getInverseMass());
    b.translate(correction * b.getInverseMass());
  }
}

void PhysicsWorld::drawDebug(sf::RenderTarget &target) const {
  for (const Body *body : m_bodies) drawShape(target, *body);

  // Contact normals: a short line starting on body a's surface pointing at b.
  constexpr float kNormalLength = 6.f;
  const sf::Color color(255, 80, 80);
  sf::VertexArray lines(sf::PrimitiveType::Lines);
  for (const Contact &contact : m_contacts) {
    const sf::Vector2f n = contact.manifold.normal;
    const Shape &shape = contact.a->getShape();
    // Distance from a's centre to its surface along the normal.
    const float extent = shape.type == Shape::Type::Circle
                             ? shape.radius
                             : std::abs(n.x) * shape.half_size.x +
                                   std::abs(n.y) * shape.half_size.y;
    const sf::Vector2f start =
        contact.a->getPosition() +
        n * (extent - contact.manifold.penetration / 2.f);
    lines.append(sf::Vertex{start, color});
    lines.append(sf::Vertex{start + n * kNormalLength, color});
  }
  if (lines.getVertexCount() > 0) target.draw(lines);
}

}  // namespace physics
