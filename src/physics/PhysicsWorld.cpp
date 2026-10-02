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
// Speeds below these are snapped to zero so damped bodies come to a full stop.
constexpr float kSleepSpeed = 0.01f;
constexpr float kSleepAngularSpeed = 0.001f;
constexpr float kEpsilon = 1e-6f;

// Applies `impulse` to `body` at the contact offset `r` (contact point minus
// body position) without the virtual position lookup of applyImpulseAtPoint.
void applyImpulse(Body &body, const sf::Vector2f &impulse,
                  const sf::Vector2f &r) {
  body.setVelocity(body.getVelocity() + impulse * body.getInverseMass());
  body.setAngularVelocity(body.getAngularVelocity() +
                          body.getInverseInertia() * cross(r, impulse));
}

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
      Contact contact{pa.body, pb.body, Manifold{}};
      if (pa.body->collide(*pb.body, contact.manifold))
        m_contacts.push_back(contact);
    }
  }
}

void PhysicsWorld::prepareContacts(float dt) {
  // The bounce speed is taken from the velocities before this step's gravity
  // is added. Measuring it afterwards would make every resting body bounce
  // off the ground a little each step and slowly gain energy.
  const float threshold = 2.f * (m_gravity * dt).length() + kRestingSpeed;
  for (Contact &contact : m_contacts) {
    const Body &a = *contact.a;
    const Body &b = *contact.b;
    const float restitution = std::min(a.getRestitution(), b.getRestitution());
    for (int i = 0; i < contact.manifold.contact_count; ++i) {
      const sf::Vector2f p = contact.manifold.contacts[i];
      const float approach =
          -(b.getVelocityAtPoint(p) - a.getVelocityAtPoint(p))
               .dot(contact.manifold.normal);
      contact.restitution_bias[i] =
          approach > threshold ? restitution * approach : 0.f;
    }
  }
}

void PhysicsWorld::integrateForces(float dt) {
  for (Body *body : m_bodies) {
    sf::Vector2f velocity = body->getVelocity();
    float omega = body->getAngularVelocity();
    if (!body->isStatic()) {
      velocity += (body->getForce() * body->getInverseMass() + m_gravity) * dt;
      omega += body->getTorque() * body->getInverseInertia() * dt;
    }
    if (body->getLinearDamping() > 0.f)
      velocity *= std::exp(-body->getLinearDamping() * dt);
    if (body->getAngularDamping() > 0.f)
      omega *= std::exp(-body->getAngularDamping() * dt);
    if (velocity.lengthSquared() < kSleepSpeed * kSleepSpeed)
      velocity = {0.f, 0.f};
    if (std::abs(omega) < kSleepAngularSpeed) omega = 0.f;
    body->setVelocity(velocity);
    body->setAngularVelocity(omega);
  }
}

void PhysicsWorld::resolveContact(const Contact &contact) const {
  Body &a = *contact.a;
  Body &b = *contact.b;
  const float inv_mass_a = a.getInverseMass();
  const float inv_mass_b = b.getInverseMass();
  if (inv_mass_a + inv_mass_b == 0.f) return;  // two static bodies
  const float inv_inertia_a = a.getInverseInertia();
  const float inv_inertia_b = b.getInverseInertia();
  const sf::Vector2f normal = contact.manifold.normal;
  const sf::Vector2f pos_a = a.getPosition();
  const sf::Vector2f pos_b = b.getPosition();
  const float count = static_cast<float>(contact.manifold.contact_count);
  const float mu = std::sqrt(a.getFriction() * b.getFriction());

  for (int i = 0; i < contact.manifold.contact_count; ++i) {
    const sf::Vector2f p = contact.manifold.contacts[i];
    const sf::Vector2f ra = p - pos_a;
    const sf::Vector2f rb = p - pos_b;

    // Normal impulse: bring the separating speed of this point up to the
    // bounce target (0 for a resting contact, e * approach speed otherwise).
    sf::Vector2f relative = b.getVelocity() +
                            cross(b.getAngularVelocity(), rb) -
                            a.getVelocity() - cross(a.getAngularVelocity(), ra);
    const float along_normal = relative.dot(normal);
    const float ra_cross_n = cross(ra, normal);
    const float rb_cross_n = cross(rb, normal);
    const float inv_mass_n = inv_mass_a + inv_mass_b +
                             ra_cross_n * ra_cross_n * inv_inertia_a +
                             rb_cross_n * rb_cross_n * inv_inertia_b;
    const float j =
        (contact.restitution_bias[i] - along_normal) / inv_mass_n / count;
    if (j <= 0.f) continue;  // already separating fast enough
    const sf::Vector2f impulse = normal * j;
    applyImpulse(a, -impulse, ra);
    applyImpulse(b, impulse, rb);

    // Coulomb friction along the contact tangent, clamped by the normal
    // impulse. Off-centre friction is what makes bodies roll and spin.
    relative = b.getVelocity() + cross(b.getAngularVelocity(), rb) -
               a.getVelocity() - cross(a.getAngularVelocity(), ra);
    sf::Vector2f tangent = relative - normal * relative.dot(normal);
    const float tangent_len_sq = tangent.lengthSquared();
    if (tangent_len_sq < kEpsilon) continue;
    tangent /= std::sqrt(tangent_len_sq);

    const float ra_cross_t = cross(ra, tangent);
    const float rb_cross_t = cross(rb, tangent);
    const float inv_mass_t = inv_mass_a + inv_mass_b +
                             ra_cross_t * ra_cross_t * inv_inertia_a +
                             rb_cross_t * rb_cross_t * inv_inertia_b;
    const float max_friction = j * mu;
    const float jt = std::clamp(-relative.dot(tangent) / inv_mass_t / count,
                                -max_friction, max_friction);
    const sf::Vector2f friction = tangent * jt;
    applyImpulse(a, -friction, ra);
    applyImpulse(b, friction, rb);
  }
}

void PhysicsWorld::integrateVelocities(float dt) {
  for (Body *body : m_bodies) {
    // Static bodies with a velocity act as kinematic movers.
    const sf::Vector2f velocity = body->getVelocity();
    if (velocity != sf::Vector2f(0.f, 0.f)) body->translate(velocity * dt);
    const float omega = body->getAngularVelocity();
    if (omega != 0.f) body->setAngle(body->getAngle() + omega * dt);
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

  // Contact points as small crosses with the contact normal pointing at b.
  constexpr float kCrossSize = 1.5f;
  constexpr float kNormalLength = 6.f;
  const sf::Color color(255, 80, 80);
  sf::VertexArray lines(sf::PrimitiveType::Lines);
  for (const Contact &contact : m_contacts) {
    const sf::Vector2f n = contact.manifold.normal;
    for (int i = 0; i < contact.manifold.contact_count; ++i) {
      const sf::Vector2f p = contact.manifold.contacts[i];
      lines.append(sf::Vertex{p - sf::Vector2f(kCrossSize, 0.f), color});
      lines.append(sf::Vertex{p + sf::Vector2f(kCrossSize, 0.f), color});
      lines.append(sf::Vertex{p - sf::Vector2f(0.f, kCrossSize), color});
      lines.append(sf::Vertex{p + sf::Vector2f(0.f, kCrossSize), color});
      lines.append(sf::Vertex{p, color});
      lines.append(sf::Vertex{p + n * kNormalLength, color});
    }
  }
  if (lines.getVertexCount() > 0) target.draw(lines);
}

}  // namespace physics
