#ifndef PHYSICS_COLLISION_H
#define PHYSICS_COLLISION_H
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

// Pure geometry used by the narrow phase of the physics engine. Nothing in
// here knows about sprites or bodies; it only works on simple shapes so it can
// be unit tested in isolation.
namespace physics {

// Result of a narrow-phase test between two shapes A and B.
// `normal` is a unit vector pointing from A towards B and `penetration` is the
// overlap depth along that normal. Moving A by -normal * penetration (or B by
// +normal * penetration) separates the two shapes.
struct Manifold {
  sf::Vector2f normal{0.f, 0.f};
  float penetration = 0.f;
};

// Circle described by its centre and radius.
struct CircleCollider {
  sf::Vector2f center;
  float radius;
};

// Axis-aligned box described by its centre and half extents.
struct BoxCollider {
  sf::Vector2f center;
  sf::Vector2f half_size;

  sf::FloatRect toRect() const;
  static BoxCollider fromRect(const sf::FloatRect& rect);
};

// Broad-phase test: true when the two rectangles share any area. Rectangles
// that only touch along an edge do not overlap.
bool overlaps(const sf::FloatRect& a, const sf::FloatRect& b);

// Narrow-phase tests. Each returns true and fills `out` when the shapes
// overlap; shapes that merely touch are not considered colliding.
bool collide(const CircleCollider& a, const CircleCollider& b, Manifold& out);
bool collide(const CircleCollider& a, const BoxCollider& b, Manifold& out);
bool collide(const BoxCollider& a, const CircleCollider& b, Manifold& out);
bool collide(const BoxCollider& a, const BoxCollider& b, Manifold& out);

}  // namespace physics

#endif
