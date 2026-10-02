#include "physics/Collision.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace physics {

sf::Vector2f rotate(sf::Vector2f v, float angle) {
  const float c = std::cos(angle);
  const float s = std::sin(angle);
  return {c * v.x - s * v.y, s * v.x + c * v.y};
}

sf::FloatRect BoxCollider::bounds() const {
  const float c = std::abs(std::cos(angle));
  const float s = std::abs(std::sin(angle));
  const sf::Vector2f extent(c * half_size.x + s * half_size.y,
                            s * half_size.x + c * half_size.y);
  return sf::FloatRect(center - extent, extent * 2.f);
}

void BoxCollider::getVertices(sf::Vector2f out[4]) const {
  const sf::Vector2f h = half_size;
  const sf::Vector2f local[4] = {
      {-h.x, -h.y}, {h.x, -h.y}, {h.x, h.y}, {-h.x, h.y}};
  for (int i = 0; i < 4; ++i) out[i] = center + rotate(local[i], angle);
}

BoxCollider BoxCollider::fromRect(const sf::FloatRect& rect) {
  return {rect.position + rect.size / 2.f, rect.size / 2.f, 0.f};
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
  out.contact_count = 1;
  out.contacts[0] = a.center + out.normal * (a.radius - out.penetration / 2.f);
  return true;
}

bool collide(const CircleCollider& a, const BoxCollider& b, Manifold& out) {
  // Work in the box's local frame, where it is axis aligned: `delta` is the
  // circle centre and `closest` the point of the box nearest to it.
  const sf::Vector2f delta = rotate(a.center - b.center, -b.angle);
  const sf::Vector2f closest(
      std::clamp(delta.x, -b.half_size.x, b.half_size.x),
      std::clamp(delta.y, -b.half_size.y, b.half_size.y));

  sf::Vector2f normal;   // local, circle -> box
  sf::Vector2f contact;  // local
  if (closest != delta) {
    // Circle centre is outside the box: collide against the closest point.
    const sf::Vector2f diff = delta - closest;  // box surface -> circle centre
    const float dist_sq = diff.lengthSquared();
    if (dist_sq >= a.radius * a.radius) return false;

    const float dist = std::sqrt(dist_sq);
    normal = -diff / dist;
    out.penetration = a.radius - dist;
    contact = closest;
  } else {
    // Circle centre is inside the box: push it out through the nearest face.
    const float to_face_x = b.half_size.x - std::abs(delta.x);
    const float to_face_y = b.half_size.y - std::abs(delta.y);
    if (to_face_x < to_face_y) {
      normal = {delta.x < 0.f ? 1.f : -1.f, 0.f};
      out.penetration = to_face_x + a.radius;
      contact = {delta.x < 0.f ? -b.half_size.x : b.half_size.x, delta.y};
    } else {
      normal = {0.f, delta.y < 0.f ? 1.f : -1.f};
      out.penetration = to_face_y + a.radius;
      contact = {delta.x, delta.y < 0.f ? -b.half_size.y : b.half_size.y};
    }
  }
  out.normal = rotate(normal, b.angle);
  out.contact_count = 1;
  out.contacts[0] = b.center + rotate(contact, b.angle);
  return true;
}

bool collide(const BoxCollider& a, const CircleCollider& b, Manifold& out) {
  if (!collide(b, a, out)) return false;
  out.normal = -out.normal;
  return true;
}

// --- Box vs box: separating axis test + reference/incident face clipping ----

namespace {

struct Polygon {
  sf::Vector2f vertices[4];  // corners, face i runs from vertex i to i + 1
  sf::Vector2f normals[4];   // outward normal of face i
};

Polygon toPolygon(const BoxCollider& box) {
  Polygon p;
  box.getVertices(p.vertices);
  const sf::Vector2f local_normals[4] = {
      {0.f, -1.f}, {1.f, 0.f}, {0.f, 1.f}, {-1.f, 0.f}};
  for (int i = 0; i < 4; ++i)
    p.normals[i] = rotate(local_normals[i], box.angle);
  return p;
}

// Vertex of `p` furthest along `dir`.
sf::Vector2f support(const Polygon& p, sf::Vector2f dir) {
  sf::Vector2f best = p.vertices[0];
  float best_dot = best.dot(dir);
  for (int i = 1; i < 4; ++i) {
    const float d = p.vertices[i].dot(dir);
    if (d > best_dot) {
      best_dot = d;
      best = p.vertices[i];
    }
  }
  return best;
}

// Separation of `b` along the face normals of `a`. The result is negative
// when the polygons overlap; `face` receives the face with the least overlap.
float axisOfLeastPenetration(int& face, const Polygon& a, const Polygon& b) {
  float best = -std::numeric_limits<float>::max();
  face = 0;
  for (int i = 0; i < 4; ++i) {
    const sf::Vector2f n = a.normals[i];
    const sf::Vector2f s = support(b, -n);
    const float d = n.dot(s - a.vertices[i]);
    if (d > best) {
      best = d;
      face = i;
    }
  }
  return best;
}

// The face of `inc` that points most against the reference face normal.
void incidentFace(sf::Vector2f out[2], const Polygon& ref, const Polygon& inc,
                  int ref_face) {
  const sf::Vector2f n = ref.normals[ref_face];
  int best = 0;
  float best_dot = std::numeric_limits<float>::max();
  for (int i = 0; i < 4; ++i) {
    const float d = n.dot(inc.normals[i]);
    if (d < best_dot) {
      best_dot = d;
      best = i;
    }
  }
  out[0] = inc.vertices[best];
  out[1] = inc.vertices[(best + 1) % 4];
}

// Clips the segment `face` against the half plane n . x <= c. Returns how many
// points survived (the segment is replaced in place).
int clip(sf::Vector2f n, float c, sf::Vector2f face[2]) {
  sf::Vector2f out[2];
  int count = 0;
  const float d1 = n.dot(face[0]) - c;
  const float d2 = n.dot(face[1]) - c;
  if (d1 <= 0.f) out[count++] = face[0];
  if (d2 <= 0.f) out[count++] = face[1];
  if (d1 * d2 < 0.f) {
    const float alpha = d1 / (d1 - d2);
    out[count++] = face[0] + (face[1] - face[0]) * alpha;
  }
  face[0] = out[0];
  face[1] = out[1];
  return count;
}

}  // namespace

bool collide(const BoxCollider& a, const BoxCollider& b, Manifold& out) {
  const Polygon pa = toPolygon(a);
  const Polygon pb = toPolygon(b);

  int face_a = 0;
  const float sep_a = axisOfLeastPenetration(face_a, pa, pb);
  if (sep_a >= 0.f) return false;
  int face_b = 0;
  const float sep_b = axisOfLeastPenetration(face_b, pb, pa);
  if (sep_b >= 0.f) return false;

  // The polygon whose face gives the least overlap is the reference. Prefer
  // `a` slightly so the choice does not flicker when both are nearly equal.
  const bool flip = !(sep_a >= sep_b * 0.95f + sep_a * 0.01f);
  const Polygon& ref = flip ? pb : pa;
  const Polygon& inc = flip ? pa : pb;
  const int ref_face = flip ? face_b : face_a;

  sf::Vector2f inc_face[2];
  incidentFace(inc_face, ref, inc, ref_face);

  // Clip the incident face to the sides of the reference face, keeping only
  // the part that lies behind the reference face.
  const sf::Vector2f v1 = ref.vertices[ref_face];
  const sf::Vector2f v2 = ref.vertices[(ref_face + 1) % 4];
  const sf::Vector2f side = (v2 - v1).normalized();
  const sf::Vector2f ref_normal(side.y, -side.x);
  const float ref_c = ref_normal.dot(v1);
  if (clip(-side, -side.dot(v1), inc_face) < 2) return false;
  if (clip(side, side.dot(v2), inc_face) < 2) return false;

  out.normal = flip ? -ref_normal : ref_normal;
  out.contact_count = 0;
  float penetration = 0.f;
  for (int i = 0; i < 2; ++i) {
    const float separation = ref_normal.dot(inc_face[i]) - ref_c;
    if (separation <= 0.f) {
      out.contacts[out.contact_count++] = inc_face[i];
      penetration -= separation;
    }
  }
  if (out.contact_count == 0) return false;
  out.penetration = penetration / static_cast<float>(out.contact_count);
  return true;
}

}  // namespace physics
