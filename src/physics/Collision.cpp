#include "physics/Collision.h"

#include <algorithm>
#include <cmath>

namespace physics {

sf::FloatRect BoxCollider::toRect() const {
  return sf::FloatRect(center - half_size, half_size * 2.f);
}

BoxCollider BoxCollider::fromRect(const sf::FloatRect& rect) {
  return {rect.position + rect.size / 2.f, rect.size / 2.f};
}

bool overlaps(const sf::FloatRect& a, const sf::FloatRect& b) {
  return a.position.x < b.position.x + b.size.x &&
         b.position.x < a.position.x + a.size.x &&
         a.position.y < b.position.y + b.size.y &&
         b.position.y < a.position.y + a.size.y;
}

bool collide(const CircleCollider& a, const CircleCollider& b, Manifold& out) {
  const sf::Vector2f delta = b.center - a.center;
  const float radii = a.radius + b.radius;
  const float dist_sq = delta.lengthSquared();
  if (dist_sq >= radii * radii) return false;

  const float dist = std::sqrt(dist_sq);
  if (dist > 0.f) {
    out.normal = delta / dist;
    out.penetration = radii - dist;
  } else {
    // Concentric circles: any direction separates them, pick a fixed one so
    // the result is deterministic.
    out.normal = {0.f, -1.f};
    out.penetration = radii;
  }
  return true;
}

bool collide(const CircleCollider& a, const BoxCollider& b, Manifold& out) {
  // Work relative to the box centre: `delta` is the circle centre and
  // `closest` is the point of the box nearest to it.
  const sf::Vector2f delta = a.center - b.center;
  const sf::Vector2f closest(
      std::clamp(delta.x, -b.half_size.x, b.half_size.x),
      std::clamp(delta.y, -b.half_size.y, b.half_size.y));

  if (closest != delta) {
    // Circle centre is outside the box: collide against the closest point.
    const sf::Vector2f diff = delta - closest;  // box surface -> circle centre
    const float dist_sq = diff.lengthSquared();
    if (dist_sq >= a.radius * a.radius) return false;

    const float dist = std::sqrt(dist_sq);
    out.normal = -diff / dist;  // circle -> box
    out.penetration = a.radius - dist;
    return true;
  }

  // Circle centre is inside the box: push it out through the nearest face.
  const float to_face_x = b.half_size.x - std::abs(delta.x);
  const float to_face_y = b.half_size.y - std::abs(delta.y);
  if (to_face_x < to_face_y) {
    out.normal = {delta.x < 0.f ? 1.f : -1.f, 0.f};
    out.penetration = to_face_x + a.radius;
  } else {
    out.normal = {0.f, delta.y < 0.f ? 1.f : -1.f};
    out.penetration = to_face_y + a.radius;
  }
  return true;
}

bool collide(const BoxCollider& a, const CircleCollider& b, Manifold& out) {
  if (!collide(b, a, out)) return false;
  out.normal = -out.normal;
  return true;
}

bool collide(const BoxCollider& a, const BoxCollider& b, Manifold& out) {
  const sf::Vector2f delta = b.center - a.center;
  const float overlap_x = a.half_size.x + b.half_size.x - std::abs(delta.x);
  if (overlap_x <= 0.f) return false;
  const float overlap_y = a.half_size.y + b.half_size.y - std::abs(delta.y);
  if (overlap_y <= 0.f) return false;

  // Separate along the axis of least penetration.
  if (overlap_x < overlap_y) {
    out.normal = {delta.x < 0.f ? -1.f : 1.f, 0.f};
    out.penetration = overlap_x;
  } else {
    out.normal = {0.f, delta.y < 0.f ? -1.f : 1.f};
    out.penetration = overlap_y;
  }
  return true;
}

}  // namespace physics
