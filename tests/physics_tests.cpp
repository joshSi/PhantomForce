// Unit tests for the physics engine. No framework: each CHECK records a failure
// and the process exit code reports the result so CTest can run it.
#include <cmath>
#include <cstdio>
#include <vector>

#include "physics/Body.h"
#include "physics/Collision.h"
#include "physics/PhysicsWorld.h"

namespace {

int g_checks = 0;
int g_failures = 0;

#define CHECK(cond)                                               \
  do {                                                            \
    ++g_checks;                                                   \
    if (!(cond)) {                                                \
      ++g_failures;                                               \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    }                                                             \
  } while (0)

bool near(float a, float b, float eps = 1e-3f) { return std::abs(a - b) < eps; }
bool near(sf::Vector2f a, sf::Vector2f b, float eps = 1e-3f) {
  return near(a.x, b.x, eps) && near(a.y, b.y, eps);
}

using physics::Body;
using physics::BoxCollider;
using physics::CircleCollider;
using physics::Manifold;
using physics::PhysicsWorld;
using physics::Shape;

Body circle(float radius) { return Body(Shape::circle(radius)); }
Body box(sf::Vector2f size) { return Body(Shape::box(size)); }

// Mirrors how the game's Object stores the position outside the body (in the
// sprite transform) by overriding the virtual position accessors.
class ExternalPositionBody : public Body {
 public:
  explicit ExternalPositionBody(const Shape &shape) : Body(shape) {}
  sf::Vector2f getPosition() const override { return external_position; }
  void setPosition(sf::Vector2f position) override {
    external_position = position;
  }
  sf::Vector2f external_position{0.f, 0.f};
};

// --- Collision geometry -----------------------------------------------------

void test_circle_vs_circle() {
  Manifold m;
  CHECK(physics::collide(CircleCollider{{0.f, 0.f}, 5.f},
                         CircleCollider{{9.f, 0.f}, 5.f}, m));
  CHECK(near(m.normal, {1.f, 0.f}));
  CHECK(near(m.penetration, 1.f));

  // Touching circles do not collide
  CHECK(!physics::collide(CircleCollider{{0.f, 0.f}, 5.f},
                          CircleCollider{{10.f, 0.f}, 5.f}, m));
  CHECK(!physics::collide(CircleCollider{{0.f, 0.f}, 1.f},
                          CircleCollider{{0.f, 3.f}, 1.f}, m));

  // Concentric circles still produce a usable manifold
  CHECK(physics::collide(CircleCollider{{2.f, 2.f}, 3.f},
                         CircleCollider{{2.f, 2.f}, 4.f}, m));
  CHECK(near(m.normal.length(), 1.f));
  CHECK(near(m.penetration, 7.f));
}

void test_circle_vs_box() {
  Manifold m;
  const BoxCollider box{{5.f, 0.f}, {3.f, 3.f}};  // spans x in [2, 8]

  // Circle just short of the left face
  CHECK(!physics::collide(CircleCollider{{0.f, 0.f}, 2.f}, box, m));
  // Circle overlapping the left face by 0.5
  CHECK(physics::collide(CircleCollider{{0.5f, 0.f}, 2.f}, box, m));
  CHECK(near(m.normal, {1.f, 0.f}));
  CHECK(near(m.penetration, 0.5f));

  // Circle hitting a corner: the normal points at the corner
  const BoxCollider corner_box{{4.f, 4.f}, {3.f, 3.f}};  // corner at (1, 1)
  CHECK(physics::collide(CircleCollider{{0.f, 0.f}, 2.f}, corner_box, m));
  const float inv_sqrt2 = 1.f / std::sqrt(2.f);
  CHECK(near(m.normal, {inv_sqrt2, inv_sqrt2}));
  CHECK(near(m.penetration, 2.f - std::sqrt(2.f)));

  // Circle centre inside the box: pushed out through the nearest face
  const BoxCollider wide{{0.f, 0.f}, {3.f, 2.f}};
  CHECK(physics::collide(CircleCollider{{1.5f, 0.f}, 1.f}, wide, m));
  CHECK(near(m.normal, {-1.f, 0.f}));
  CHECK(near(m.penetration, 2.5f));
  CHECK(physics::collide(CircleCollider{{0.f, -1.5f}, 1.f}, wide, m));
  CHECK(near(m.normal, {0.f, 1.f}));
  CHECK(near(m.penetration, 1.5f));

  // Box-vs-circle is the mirror image
  Manifold reversed;
  CHECK(physics::collide(box, CircleCollider{{0.5f, 0.f}, 2.f}, reversed));
  CHECK(near(reversed.normal, {-1.f, 0.f}));
  CHECK(near(reversed.penetration, 0.5f));
}

void test_box_vs_box() {
  Manifold m;
  const BoxCollider a{{0.f, 0.f}, {2.f, 2.f}};
  CHECK(physics::collide(a, BoxCollider{{3.f, 1.f}, {2.f, 2.f}}, m));
  CHECK(near(m.normal, {1.f, 0.f}));
  CHECK(near(m.penetration, 1.f));

  CHECK(physics::collide(a, BoxCollider{{-1.f, -3.5f}, {2.f, 2.f}}, m));
  CHECK(near(m.normal, {0.f, -1.f}));
  CHECK(near(m.penetration, 0.5f));

  CHECK(!physics::collide(a, BoxCollider{{5.f, 0.f}, {2.f, 2.f}}, m));
  CHECK(!physics::collide(a, BoxCollider{{4.f, 0.f}, {2.f, 2.f}}, m));  // touch
  CHECK(!physics::collide(a, BoxCollider{{3.f, 4.f}, {2.f, 2.f}}, m));
}

void test_aabb_overlap() {
  const sf::FloatRect a({0.f, 0.f}, {4.f, 4.f});
  CHECK(physics::overlaps(a, sf::FloatRect({3.f, 3.f}, {4.f, 4.f})));
  CHECK(!physics::overlaps(a, sf::FloatRect({4.f, 0.f}, {4.f, 4.f})));
  CHECK(!physics::overlaps(a, sf::FloatRect({0.f, 5.f}, {4.f, 4.f})));

  const BoxCollider box = BoxCollider::fromRect(a);
  CHECK(near(box.center, {2.f, 2.f}));
  CHECK(near(box.half_size, {2.f, 2.f}));
  CHECK(near(box.bounds().position, a.position));
  CHECK(near(box.bounds().size, a.size));
}

// --- Rotation
// -----------------------------------------------------------------

constexpr float kPi = 3.14159265f;

void test_rotated_box_bounds() {
  const BoxCollider upright{{0.f, 0.f}, {2.f, 1.f}, 0.f};
  CHECK(near(upright.bounds().size, {4.f, 2.f}));
  const BoxCollider turned{{5.f, 5.f}, {2.f, 1.f}, kPi / 2.f};
  CHECK(near(turned.bounds().position, {4.f, 3.f}));
  CHECK(near(turned.bounds().size, {2.f, 4.f}));
  const BoxCollider diagonal{{0.f, 0.f}, {2.f, 1.f}, kPi / 4.f};
  CHECK(near(diagonal.bounds().size.x, 3.f * std::sqrt(2.f)));
  CHECK(near(diagonal.bounds().size.y, 3.f * std::sqrt(2.f)));

  sf::Vector2f corners[4];
  turned.getVertices(corners);
  CHECK(near(corners[0], {6.f, 3.f}));  // (-2, -1) rotated 90 degrees clockwise
  CHECK(near(corners[2], {4.f, 7.f}));

  const Shape shape = Shape::box({4.f, 2.f});
  CHECK(near(shape.aabb({0.f, 0.f}, kPi / 2.f).size, {2.f, 4.f}));
}

void test_circle_vs_rotated_box() {
  // A 4x2 box turned on its side occupies x in [-1, 1] and y in [-2, 2]
  const BoxCollider box{{0.f, 0.f}, {2.f, 1.f}, kPi / 2.f};
  Manifold m;
  CHECK(!physics::collide(CircleCollider{{2.5f, 0.f}, 1.f}, box, m));
  CHECK(!physics::collide(CircleCollider{{0.f, -3.5f}, 1.f}, box, m));
  CHECK(physics::collide(CircleCollider{{1.5f, 0.f}, 1.f}, box, m));
  CHECK(near(m.normal, {-1.f, 0.f}));
  CHECK(near(m.penetration, 0.5f));
  CHECK(m.contact_count == 1);
  CHECK(near(m.contacts[0], {1.f, 0.f}));
  CHECK(physics::collide(CircleCollider{{0.f, -2.5f}, 1.f}, box, m));
  CHECK(near(m.normal, {0.f, 1.f}));
  CHECK(near(m.penetration, 0.5f));
}

void test_box_vs_box_contact_points() {
  Manifold m;
  const BoxCollider a{{0.f, 0.f}, {2.f, 2.f}};
  // Face contact: two contact points along a's right face
  CHECK(physics::collide(a, BoxCollider{{3.f, 1.f}, {2.f, 2.f}}, m));
  CHECK(m.contact_count == 2);
  CHECK(near(m.contacts[0].x, 2.f) && near(m.contacts[1].x, 2.f));
  CHECK(near(std::min(m.contacts[0].y, m.contacts[1].y), -1.f));
  CHECK(near(std::max(m.contacts[0].y, m.contacts[1].y), 2.f));

  // A diamond (45 degree box) poking its corner into a box: one contact point
  const BoxCollider diamond{{0.f, 0.f}, {1.f, 1.f}, kPi / 4.f};
  const float tip = std::sqrt(2.f);
  CHECK(!physics::collide(diamond, BoxCollider{{2.5f, 0.f}, {1.f, 1.f}}, m));
  CHECK(physics::collide(diamond, BoxCollider{{2.2f, 0.f}, {1.f, 1.f}}, m));
  CHECK(near(m.normal, {1.f, 0.f}));
  CHECK(near(m.penetration, tip - 1.2f));
  CHECK(m.contact_count == 1);
  CHECK(near(m.contacts[0], {tip, 0.f}));

  // Reversed order flips the normal but finds the same contact
  CHECK(physics::collide(BoxCollider{{2.2f, 0.f}, {1.f, 1.f}}, diamond, m));
  CHECK(near(m.normal, {-1.f, 0.f}));
  CHECK(near(m.contacts[0], {tip, 0.f}));
}

void test_inertia() {
  Body b = box({4.f, 2.f});
  b.setMass(3.f);
  CHECK(near(b.getInertia(), 5.f));  // 3 * (16 + 4) / 12
  CHECK(near(b.getInverseInertia(), 0.2f));
  Body c = circle(2.f);
  c.setMass(2.f);
  CHECK(near(c.getInertia(), 4.f));  // 0.5 * 2 * 2^2
  c.setShape(Shape::circle(1.f));
  CHECK(near(c.getInertia(), 1.f));  // follows the shape
  c.setFixedRotation(true);
  CHECK(near(c.getInverseInertia(), 0.f));
  c.applyImpulseAtPoint({0.f, 10.f}, c.getPosition() + sf::Vector2f(1.f, 0.f));
  CHECK(near(c.getAngularVelocity(), 0.f));
  CHECK(near(c.getVelocity(), {0.f, 5.f}));  // still pushed
  c.setFixedRotation(false);
  c.applyImpulseAtPoint({0.f, 10.f}, c.getPosition() + sf::Vector2f(1.f, 0.f));
  CHECK(near(c.getAngularVelocity(), 10.f));  // r x J / I = 10 / 1

  Body rock = box({2.f, 2.f});  // static: no inertia either
  CHECK(near(rock.getInverseInertia(), 0.f));
}

void test_velocity_at_point_and_forces() {
  Body b = circle(1.f);
  b.setMass(2.f);  // I = 1
  b.setVelocity({1.f, 0.f});
  b.setAngularVelocity(2.f);  // clockwise on screen
  // A point to the right of the centre moves down (y grows) when spinning
  CHECK(near(b.getVelocityAtPoint({3.f, 0.f}), {1.f, 6.f}));

  Body c = box({2.f, 2.f});
  c.setMass(3.f);                               // I = 3 * 8 / 12 = 2
  c.applyForceAtPoint({0.f, 6.f}, {1.f, 0.f});  // torque = 1 * 6 = 6
  CHECK(near(c.getForce(), {0.f, 6.f}));
  CHECK(near(c.getTorque(), 6.f));
  c.clearForces();
  CHECK(near(c.getTorque(), 0.f));
}

void test_torque_and_angular_damping() {
  Body b = box({4.f, 2.f});
  b.setMass(3.f);  // I = 5
  PhysicsWorld world;
  world.addBody(&b);
  b.applyTorque(10.f);
  world.step(0.5f);
  CHECK(near(b.getAngularVelocity(), 1.f));  // 10 / 5 * 0.5
  CHECK(near(b.getAngle(), 0.5f));
  world.step(0.5f);
  CHECK(near(b.getAngularVelocity(), 1.f));  // torque cleared, coasting
  CHECK(near(b.getAngle(), 1.f));

  b.setAngularDamping(1.f);
  world.step(1.f);
  CHECK(near(b.getAngularVelocity(), std::exp(-1.f), 0.01f));
  b.setAngularDamping(50.f);
  for (int i = 0; i < 20; ++i) world.step(0.1f);
  CHECK(b.getAngularVelocity() == 0.f);

  // Static bodies ignore torque but a hand-set spin still turns them
  Body rock = circle(1.f);
  rock.applyTorque(100.f);
  rock.setAngularVelocity(2.f);
  world.addBody(&rock);
  world.step(0.5f);
  CHECK(near(rock.getAngularVelocity(), 2.f));
  CHECK(near(rock.getAngle(), 1.f));
}

// Angular momentum of a body about the origin: r x m v + I w
float angularMomentum(const Body &b, sf::Vector2f position) {
  return b.getMass() * physics::cross(position, b.getVelocity()) +
         b.getInertia() * b.getAngularVelocity();
}

void test_off_centre_hit_spins_box() {
  Body crate = box({2.f, 2.f});
  crate.setMass(1.f);
  crate.setFriction(0.f);
  crate.setRestitution(0.f);
  Body ball = circle(0.5f);
  ball.setMass(1.f);
  ball.setFriction(0.f);
  ball.setRestitution(0.f);
  ball.setPosition({-1.4f, -0.8f});  // hits the left face near its top corner
  ball.setVelocity({10.f, 0.f});

  const sf::Vector2f crate_pos = crate.getPosition();
  const sf::Vector2f ball_pos = ball.getPosition();
  const sf::Vector2f momentum_before = ball.getVelocity() * ball.getMass() +
                                       crate.getVelocity() * crate.getMass();
  const float angular_before =
      angularMomentum(ball, ball_pos) + angularMomentum(crate, crate_pos);

  PhysicsWorld world;
  world.addBody(&crate);
  world.addBody(&ball);
  world.step(0.01f);

  CHECK(crate.getVelocity().x > 0.f);
  CHECK(crate.getAngularVelocity() >
        0.f);  // pushed on its top: spins clockwise
  CHECK(ball.getVelocity().x < 10.f);
  CHECK(
      near(ball.getAngularVelocity(), 0.f));  // no friction: ball keeps no spin
  // Linear and angular momentum are conserved by the collision impulse
  // (measured with the positions at the moment of impact).
  const sf::Vector2f momentum_after = ball.getVelocity() * ball.getMass() +
                                      crate.getVelocity() * crate.getMass();
  CHECK(near(momentum_after, momentum_before, 1e-3f));
  const float angular_after =
      angularMomentum(ball, ball_pos) + angularMomentum(crate, crate_pos);
  CHECK(near(angular_after, angular_before, 1e-3f));
  CHECK(crate.getAngle() > 0.f);  // and it actually turned
}

void test_fixed_rotation_body_does_not_spin() {
  Body crate = box({2.f, 2.f});
  crate.setMass(1.f);
  crate.setFixedRotation(true);
  Body ball = circle(0.5f);
  ball.setMass(1.f);
  ball.setPosition({-1.4f, -0.8f});
  ball.setVelocity({10.f, 0.f});

  PhysicsWorld world;
  world.addBody(&crate);
  world.addBody(&ball);
  world.step(0.01f);
  CHECK(crate.getVelocity().x > 0.f);
  CHECK(crate.getAngularVelocity() == 0.f);
  CHECK(crate.getAngle() == 0.f);
}

void test_ball_rolls_on_floor() {
  Body floor = box({200.f, 8.f});
  floor.setPosition({0.f, 5.f});  // top face at y = 1
  floor.setFriction(0.5f);
  Body ball = circle(1.f);
  ball.setMass(1.f);
  ball.setFriction(0.5f);
  ball.setRestitution(0.f);
  ball.setPosition({0.f, 0.05f});
  ball.setVelocity({10.f, 0.f});

  PhysicsWorld world;
  world.setGravity({0.f, 100.f});
  world.addBody(&floor);
  world.addBody(&ball);
  for (int i = 0; i < 60; ++i) world.step(0.01f);

  // Friction converts sliding into rolling: moving right on a floor below
  // means spinning clockwise, and the contact point comes (nearly) to rest.
  // The ball sits a hair inside the floor, so its lever arm is just under 1.
  CHECK(ball.getAngularVelocity() > 0.f);
  CHECK(ball.getVelocity().x < 10.f);
  CHECK(near(ball.getVelocity().x, 10.f * 2.f / 3.f, 0.1f));  // v = 2/3 v0
  const sf::Vector2f contact(ball.getPosition().x, 1.f);      // floor surface
  CHECK(near(ball.getVelocityAtPoint(contact).x, 0.f, 0.1f));
  CHECK(ball.getAngle() > 0.f);
}

class ExternalAngleBody : public Body {
 public:
  explicit ExternalAngleBody(const Shape &shape) : Body(shape) {}
  float getAngle() const override { return external_angle; }
  void setAngle(float radians) override { external_angle = radians; }
  float external_angle = 0.f;
};

void test_external_angle_is_used() {
  ExternalAngleBody b(Shape::box({2.f, 2.f}));
  b.setMass(1.f);
  b.setAngularVelocity(1.f);
  b.external_angle = kPi / 4.f;  // starts as a diamond
  CHECK(near(b.getAABB().size.x, 2.f * std::sqrt(2.f)));
  PhysicsWorld world;
  world.addBody(&b);
  world.step(0.5f);
  CHECK(near(b.external_angle, kPi / 4.f + 0.5f));
}

// --- Body -------------------------------------------------------------------

void test_body_properties() {
  Body c = circle(1.f);
  CHECK(c.isStatic());
  CHECK(near(c.getInverseMass(), 0.f));
  c.setMass(4.f);
  CHECK(!c.isStatic());
  CHECK(near(c.getInverseMass(), 0.25f));
  c.applyImpulse({8.f, 0.f});
  CHECK(near(c.getVelocity(), {2.f, 0.f}));
  c.setMass(-1.f);  // negative mass is clamped to static
  CHECK(c.isStatic());

  c.setShape(Shape::circle(3.f));
  c.setPosition({10.f, 20.f});
  const sf::FloatRect aabb = c.getAABB();
  CHECK(near(aabb.position, {7.f, 17.f}));
  CHECK(near(aabb.size, {6.f, 6.f}));

  Body r = box({4.f, 2.f});
  r.setPosition({1.f, 1.f});
  CHECK(near(r.getAABB().position, {-1.f, 0.f}));
  CHECK(near(r.getAABB().size, {4.f, 2.f}));
  CHECK(r.getShape().type == Shape::Type::Box);
  CHECK(near(r.getShape().half_size, {2.f, 1.f}));

  r.translate({1.f, -1.f});
  CHECK(near(r.getPosition(), {2.f, 0.f}));
}

void test_body_collide_and_snap() {
  Body c = circle(2.f);
  c.setPosition({0.5f, 0.f});
  Body r = box({6.f, 6.f});
  r.setPosition({5.f, 0.f});

  Manifold m;
  CHECK(c.collide(r, m));
  CHECK(near(m.normal, {1.f, 0.f}));
  CHECK(near(m.penetration, 0.5f));
  CHECK(r.collide(c, m));
  CHECK(near(m.normal, {-1.f, 0.f}));
  CHECK(c.checkCollision(r));

  c.snapCollision(r);
  CHECK(!c.checkCollision(r));
  CHECK(c.getPosition().x < 0.f);              // moved away from the box
  CHECK(near(c.getPosition().x, 0.f, 0.05f));  // ...but only just

  Body far = circle(1.f);
  far.setPosition({100.f, 100.f});
  CHECK(!c.checkCollision(far));
}

void test_external_position_is_used() {
  ExternalPositionBody ball(Shape::circle(1.f));
  ball.setMass(1.f);
  ball.setVelocity({10.f, 0.f});
  ExternalPositionBody wall(Shape::box({2.f, 10.f}));
  wall.external_position = {3.f, 0.f};  // left face at x = 2

  PhysicsWorld world;
  world.addBody(&ball);
  world.addBody(&wall);
  world.step(0.15f);  // ball reaches x = 1.5, overlapping the wall by 0.5
  CHECK(near(ball.external_position, {1.5f, 0.f}));
  world.step(0.01f);
  CHECK(ball.external_position.x < 1.5f);  // pushed back out
  CHECK(ball.getVelocity().x <= 0.f);      // and no longer moving into it
  CHECK(near(wall.external_position, {3.f, 0.f}));
}

// --- PhysicsWorld
// -------------------------------------------------------------

void test_gravity_and_integration() {
  Body ball = circle(1.f);
  ball.setMass(1.f);
  PhysicsWorld world;
  world.setGravity({0.f, 100.f});
  world.addBody(&ball);
  world.step(0.1f);
  CHECK(near(ball.getVelocity(), {0.f, 10.f}));
  CHECK(near(ball.getPosition(), {0.f, 1.f}));
  world.step(0.1f);
  CHECK(near(ball.getVelocity(), {0.f, 20.f}));
  CHECK(near(ball.getPosition(), {0.f, 3.f}));

  // Static bodies ignore gravity
  Body rock = circle(1.f);
  world.addBody(&rock);
  world.step(0.1f);
  CHECK(near(rock.getPosition(), {0.f, 0.f}));
  CHECK(near(rock.getVelocity(), {0.f, 0.f}));
}

void test_forces_and_damping() {
  Body crate = box({2.f, 2.f});
  crate.setMass(2.f);
  PhysicsWorld world;
  world.addBody(&crate);

  crate.applyForce({20.f, 0.f});
  world.step(0.5f);
  CHECK(near(crate.getVelocity(), {5.f, 0.f}));
  CHECK(near(crate.getForce(), {0.f, 0.f}));  // forces are cleared each step
  world.step(0.5f);
  CHECK(near(crate.getVelocity(), {5.f, 0.f}));  // coasting without damping

  crate.setLinearDamping(1.f);
  crate.setVelocity({100.f, 0.f});
  world.step(1.f);
  CHECK(near(crate.getVelocity().x, 100.f * std::exp(-1.f), 0.01f));

  // Heavy damping eventually stops the body completely
  crate.setLinearDamping(50.f);
  for (int i = 0; i < 20; ++i) world.step(0.1f);
  CHECK(crate.getVelocity() == sf::Vector2f(0.f, 0.f));
}

void test_kinematic_body() {
  Body mover = box({2.f, 2.f});  // static (mass 0) but moving
  mover.setVelocity({5.f, 0.f});
  PhysicsWorld world;
  world.setGravity({0.f, 100.f});
  world.addBody(&mover);
  world.step(1.f);
  CHECK(near(mover.getPosition(), {5.f, 0.f}));
  CHECK(near(mover.getVelocity(), {5.f, 0.f}));  // unaffected by gravity
}

void test_elastic_head_on_collision() {
  Body a = circle(1.f);
  Body b = circle(1.f);
  for (Body *c : {&a, &b}) {
    c->setMass(1.f);
    c->setRestitution(1.f);
    c->setFriction(0.f);
  }
  a.setPosition({0.f, 0.f});
  a.setVelocity({10.f, 0.f});
  b.setPosition({1.5f, 0.f});
  b.setVelocity({-10.f, 0.f});

  PhysicsWorld world;
  world.addBody(&a);
  world.addBody(&b);
  world.step(0.01f);

  CHECK(world.getContacts().size() == 1);
  // Equal masses swap velocities in a perfectly elastic collision
  CHECK(near(a.getVelocity(), {-10.f, 0.f}));
  CHECK(near(b.getVelocity(), {10.f, 0.f}));
  // Positional correction pushed them apart
  CHECK(b.getPosition().x - a.getPosition().x > 1.5f);
}

void test_unequal_masses_conserve_momentum() {
  Body light = circle(1.f);
  Body heavy = circle(1.f);
  light.setMass(1.f);
  heavy.setMass(3.f);
  light.setRestitution(0.5f);
  heavy.setRestitution(0.5f);
  light.setFriction(0.f);
  heavy.setFriction(0.f);
  light.setPosition({0.f, 0.f});
  light.setVelocity({8.f, 0.f});
  heavy.setPosition({1.9f, 0.f});

  PhysicsWorld world;
  world.addBody(&light);
  world.addBody(&heavy);
  world.step(0.01f);

  const float momentum = light.getMass() * light.getVelocity().x +
                         heavy.getMass() * heavy.getVelocity().x;
  CHECK(near(momentum, 8.f));
  // Coefficient of restitution: separation speed = e * approach speed
  CHECK(near(heavy.getVelocity().x - light.getVelocity().x, 0.5f * 8.f));
}

void test_static_body_reflects_dynamic() {
  Body wall = box({10.f, 10.f});
  wall.setRestitution(1.f);
  wall.setFriction(0.f);
  Body ball = circle(1.f);
  ball.setMass(1.f);
  ball.setRestitution(1.f);
  ball.setFriction(0.f);
  ball.setPosition({5.5f, 0.f});  // overlapping the right face at x = 5
  ball.setVelocity({-10.f, 3.f});

  PhysicsWorld world;
  world.addBody(&wall);
  world.addBody(&ball);
  world.step(0.01f);

  CHECK(near(ball.getVelocity(), {10.f, 3.f}));  // only the normal flips
  CHECK(near(wall.getPosition(), {0.f, 0.f}));
  CHECK(near(wall.getVelocity(), {0.f, 0.f}));
}

void test_positional_correction_separates_resting_bodies() {
  Body floor = box({100.f, 10.f});
  floor.setPosition({0.f, 5.f});  // top face at y = 0
  Body ball = circle(2.f);
  ball.setMass(1.f);
  ball.setPosition({0.f, -1.f});  // sunk 1 pixel into the floor

  PhysicsWorld world;
  world.addBody(&floor);
  world.addBody(&ball);
  for (int i = 0; i < 30; ++i) world.step(1.f / 60.f);

  CHECK(ball.getPosition().y <= -2.f + 0.02f);  // resting on the surface
  CHECK(near(ball.getPosition().x, 0.f));
  CHECK(near(ball.getVelocity(), {0.f, 0.f}, 0.05f));
}

void test_resting_under_gravity_does_not_bounce() {
  for (float restitution : {0.f, 0.2f, 0.5f, 0.9f}) {
    Body floor = box({100.f, 10.f});
    floor.setPosition({0.f, 5.f});  // top face at y = 0
    floor.setRestitution(restitution);
    Body ball = circle(1.f);
    ball.setMass(1.f);
    ball.setRestitution(restitution);
    ball.setPosition({0.f, -1.f});  // sitting exactly on the floor

    PhysicsWorld world;
    world.setGravity({0.f, 500.f});
    world.addBody(&floor);
    world.addBody(&ball);
    for (int i = 0; i < 120; ++i) world.step(1.f / 60.f);

    // Comes to rest on the surface instead of jittering or gaining height
    CHECK(near(ball.getPosition().y, -1.f, 0.02f));
    CHECK(ball.getVelocity() == sf::Vector2f(0.f, 0.f));
  }
}

void test_dropped_ball_bounces_then_settles() {
  Body floor = box({100.f, 10.f});
  floor.setPosition({0.f, 5.f});  // top face at y = 0
  Body ball = circle(1.f);
  ball.setMass(1.f);
  ball.setRestitution(0.5f);
  ball.setPosition({0.f, -21.f});  // 20 px above the floor

  PhysicsWorld world;
  world.setGravity({0.f, 500.f});
  world.addBody(&floor);
  world.addBody(&ball);

  bool bounced = false;
  float lowest = ball.getPosition().y;
  for (int i = 0; i < 300; ++i) {
    world.step(1.f / 60.f);
    lowest = std::max(lowest, ball.getPosition().y);
    if (ball.getVelocity().y < -1.f) bounced = true;  // moving back up
  }
  CHECK(bounced);
  // Penetration is bounded by one step of travel at the impact speed
  // (sqrt(2 * 500 * 20) px/s / 60 ~= 2.4 px) and is corrected straight away.
  CHECK(lowest < -1.f + 2.5f);
  CHECK(near(ball.getPosition().y, -1.f, 0.02f));
  CHECK(ball.getVelocity() == sf::Vector2f(0.f, 0.f));
}

void test_restitution_is_clamped() {
  Body b = circle(1.f);
  b.setRestitution(2.f);
  CHECK(near(b.getRestitution(), 1.f));
  b.setRestitution(-1.f);
  CHECK(near(b.getRestitution(), 0.f));
}

void test_friction_slows_sliding_body() {
  auto run = [](float mu) {
    Body floor = box({200.f, 8.f});
    floor.setPosition({0.f, 5.f});  // top face at y = 1
    floor.setFriction(mu);
    Body ball = circle(1.f);
    ball.setMass(1.f);
    ball.setFriction(mu);
    ball.setRestitution(0.f);
    ball.setPosition({0.f, 0.05f});
    ball.setVelocity({10.f, 0.f});

    PhysicsWorld world;
    world.setGravity({0.f, 100.f});
    world.addBody(&floor);
    world.addBody(&ball);
    world.step(0.01f);
    return ball.getVelocity();
  };

  const sf::Vector2f frictionless = run(0.f);
  CHECK(near(frictionless, {10.f, 0.f}));

  // Normal impulse cancels gravity's 1 px/s; friction removes at most mu * j
  const sf::Vector2f rough = run(0.5f);
  CHECK(near(rough, {9.5f, 0.f}));
}

void test_broad_phase_only_reports_overlapping_pairs() {
  Body wide = box({20.f, 4.f});  // x in [-10, 10]
  Body inside = circle(1.f);
  inside.setMass(1.f);
  inside.setPosition({5.f, 0.f});
  Body far = circle(1.f);  // overlaps on x with `wide` but not on y
  far.setMass(1.f);
  far.setPosition({-5.f, 20.f});
  Body beyond = circle(1.f);
  beyond.setMass(1.f);
  beyond.setPosition({30.f, 0.f});

  PhysicsWorld world;
  world.addBody(&beyond);
  world.addBody(&far);
  world.addBody(&inside);
  world.addBody(&wide);
  world.addBody(&wide);  // duplicates are ignored
  CHECK(world.getBodies().size() == 4);

  int callbacks = 0;
  world.setContactCallback([&](Body &a, Body &b, const Manifold &m) {
    ++callbacks;
    CHECK((&a == &wide && &b == &inside) || (&a == &inside && &b == &wide));
    CHECK(m.penetration > 0.f);
  });
  world.step(0.01f);
  CHECK(world.getContacts().size() == 1);
  CHECK(callbacks == 1);
  CHECK(near(far.getPosition(), {-5.f, 20.f}));
  CHECK(near(beyond.getPosition(), {30.f, 0.f}));

  world.removeBody(&inside);
  CHECK(world.getBodies().size() == 3);
  CHECK(world.getContacts().empty());
  world.step(0.01f);
  CHECK(world.getContacts().empty());

  world.clear();
  CHECK(world.getBodies().empty());
}

void test_static_pairs_are_skipped() {
  Body a = box({4.f, 4.f});
  Body b = box({4.f, 4.f});
  b.setPosition({1.f, 1.f});
  PhysicsWorld world;
  world.addBody(&a);
  world.addBody(&b);
  world.step(0.01f);
  CHECK(world.getContacts().empty());
  CHECK(near(a.getPosition(), {0.f, 0.f}));
  CHECK(near(b.getPosition(), {1.f, 1.f}));
}

void test_zero_or_negative_dt_is_ignored() {
  Body ball = circle(1.f);
  ball.setMass(1.f);
  ball.setVelocity({1.f, 0.f});
  PhysicsWorld world;
  world.addBody(&ball);
  world.step(0.f);
  world.step(-1.f);
  CHECK(near(ball.getPosition(), {0.f, 0.f}));
}

}  // namespace

int main() {
  test_circle_vs_circle();
  test_circle_vs_box();
  test_box_vs_box();
  test_aabb_overlap();
  test_rotated_box_bounds();
  test_circle_vs_rotated_box();
  test_box_vs_box_contact_points();
  test_inertia();
  test_velocity_at_point_and_forces();
  test_torque_and_angular_damping();
  test_off_centre_hit_spins_box();
  test_fixed_rotation_body_does_not_spin();
  test_ball_rolls_on_floor();
  test_external_angle_is_used();
  test_body_properties();
  test_body_collide_and_snap();
  test_external_position_is_used();
  test_gravity_and_integration();
  test_forces_and_damping();
  test_kinematic_body();
  test_elastic_head_on_collision();
  test_unequal_masses_conserve_momentum();
  test_static_body_reflects_dynamic();
  test_positional_correction_separates_resting_bodies();
  test_resting_under_gravity_does_not_bounce();
  test_dropped_ball_bounces_then_settles();
  test_restitution_is_clamped();
  test_friction_slows_sliding_body();
  test_broad_phase_only_reports_overlapping_pairs();
  test_static_pairs_are_skipped();
  test_zero_or_negative_dt_is_ignored();

  std::printf("%d checks, %d failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
