#ifndef PHYSICS_SURFACE_H
#define PHYSICS_SURFACE_H

namespace physics {

// The floor a body is resting on. In a top-down game the ground is not a
// collision shape, so its effect on sliding and spinning bodies is modelled
// here instead. The PhysicsWorld samples the surface under every body each
// step (see PhysicsWorld::setSurfaceSampler) and applies:
//
//  - Coulomb friction: a constant deceleration of friction * ground gravity
//    until the body stops. Low values give long slides (ice), high values
//    stop bodies quickly (concrete, rough metal).
//  - Drag: an extra exponential decay of the velocity, in 1/s, for loose or
//    soft ground that absorbs motion (sand, mud, shallow water).
//  - Grip: how much traction a self-propelled body gets, in [0, 1]. The
//    player scales its driving acceleration and its own damping by this, so
//    it can neither accelerate nor stop well on ice.
//
// Spinning bodies feel the same friction and drag as a torque.
struct Surface {
  float friction = 0.f;
  float drag = 0.f;
  float grip = 1.f;

  // A frictionless, dragless floor: bodies coast forever (the default).
  static Surface none() { return {0.f, 0.f, 1.f}; }
  // Indoor metal floor: firm footing, objects stop within a short distance.
  static Surface metal() { return {0.4f, 0.f, 1.f}; }
  // Rough concrete or stone: stops everything quickly.
  static Surface concrete() { return {0.6f, 0.f, 1.f}; }
  // Ice: almost no friction, poor traction.
  static Surface ice() { return {0.03f, 0.f, 0.2f}; }
  // Desert sand: high friction plus drag from sinking in, mediocre traction.
  static Surface sand() { return {0.6f, 2.f, 0.7f}; }
};

}  // namespace physics

#endif
