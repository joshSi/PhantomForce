// Micro-benchmark for the physics engine. Prints the average and worst step
// time of a few scenarios; it never fails, so it can run in CI for visibility.
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

#include "physics/Body.h"
#include "physics/PhysicsWorld.h"
#include "physics/Surface.h"

using physics::Body;
using physics::PhysicsWorld;
using physics::Shape;
using physics::Surface;

namespace {

struct Result {
  double avg_us;
  double worst_us;
  std::size_t contacts;
  std::size_t narrow_tests;
};

Result run(PhysicsWorld &world, int steps, float dt) {
  double total = 0.0, worst = 0.0;
  std::size_t contacts = 0, tests = 0;
  for (int i = 0; i < steps; ++i) {
    world.step(dt);
    const double us = world.getStats().step_time_us;
    total += us;
    if (us > worst) worst = us;
    contacts += world.getStats().contacts;
    tests += world.getStats().narrowphase_tests;
  }
  return {total / steps, worst, contacts / steps, tests / steps};
}

void report(const char *name, std::size_t bodies, const Result &r) {
  std::printf(
      "%-34s %5zu bodies  avg %8.1f us  worst %8.1f us  %5zu contacts  %6zu "
      "tests/step\n",
      name, bodies, r.avg_us, r.worst_us, r.contacts, r.narrow_tests);
}

// Four static walls around a square arena
void addArena(std::vector<Body> &bodies, float size) {
  const float t = 50.f;
  bodies.emplace_back(Shape::box({size + 2 * t, t}));
  bodies.back().setPosition({size / 2, -t / 2});
  bodies.emplace_back(Shape::box({size + 2 * t, t}));
  bodies.back().setPosition({size / 2, size + t / 2});
  bodies.emplace_back(Shape::box({t, size}));
  bodies.back().setPosition({-t / 2, size / 2});
  bodies.emplace_back(Shape::box({t, size}));
  bodies.back().setPosition({size + t / 2, size / 2});
}

// N bodies bouncing around an arena in zero gravity on a metal floor, the
// shape of a busy top-down scene.
void benchArena(std::size_t n, bool mixed_shapes) {
  std::mt19937 rng(42);
  const float size = 2000.f;
  std::uniform_real_distribution<float> pos(30.f, size - 30.f);
  std::uniform_real_distribution<float> vel(-200.f, 200.f);
  std::uniform_real_distribution<float> radius(4.f, 14.f);

  std::vector<Body> bodies;
  bodies.reserve(n + 4);
  addArena(bodies, size);
  for (std::size_t i = 0; i < n; ++i) {
    const float r = radius(rng);
    if (mixed_shapes && i % 2)
      bodies.emplace_back(Shape::box({2 * r, 2 * r}));
    else
      bodies.emplace_back(Shape::circle(r));
    bodies.back().setPosition({pos(rng), pos(rng)});
    bodies.back().setVelocity({vel(rng), vel(rng)});
    bodies.back().setDensity(0.01f);
    bodies.back().setRestitution(0.5f);
  }
  PhysicsWorld world;
  world.setDefaultSurface(Surface::none());
  for (Body &b : bodies) world.addBody(&b);
  run(world, 30, 1.f / 120.f);  // warm up
  report(mixed_shapes ? "arena, circles and boxes" : "arena, circles",
         bodies.size(), run(world, 300, 1.f / 120.f));
}

// A pyramid-ish pile of crates settling on a floor under gravity: many
// persistent contacts, the worst case for the solver.
void benchPile(std::size_t rows) {
  std::vector<Body> bodies;
  bodies.reserve(rows * rows + 1);
  bodies.emplace_back(Shape::box({4000.f, 100.f}));
  bodies.back().setPosition({0.f, 50.f});
  const float s = 20.f;
  for (std::size_t row = 0; row < rows; ++row)
    for (std::size_t col = 0; col + row < rows; ++col) {
      bodies.emplace_back(Shape::box({s, s}));
      bodies.back().setPosition(
          {(col + row * 0.5f) * (s + 1.f), -(row + 0.5f) * (s + 0.5f)});
      bodies.back().setMass(1.f);
      bodies.back().setFriction(0.5f);
    }
  PhysicsWorld world;
  world.setGravity({0.f, 500.f});
  for (Body &b : bodies) world.addBody(&b);
  report("crate pyramid under gravity", bodies.size(),
         run(world, 300, 1.f / 120.f));
}

// Many bodies that are simply sitting still: should cost almost nothing.
void benchResting(std::size_t n) {
  std::vector<Body> bodies;
  bodies.reserve(n);
  const std::size_t per_row = 40;
  for (std::size_t i = 0; i < n; ++i) {
    bodies.emplace_back(Shape::box({14.f, 14.f}));
    bodies.back().setPosition({(i % per_row) * 15.f, (i / per_row) * 15.f});
    bodies.back().setMass(1.f);
  }
  PhysicsWorld world;
  world.setDefaultSurface(Surface::metal());
  for (Body &b : bodies) world.addBody(&b);
  report("resting crates, touching", bodies.size(),
         run(world, 300, 1.f / 120.f));
}

}  // namespace

int main() {
  std::printf("PhantomForce physics benchmark (times per 1/120 s step)\n");
  benchArena(200, false);
  benchArena(1000, false);
  benchArena(1000, true);
  benchPile(20);
  benchResting(1000);
  return 0;
}
