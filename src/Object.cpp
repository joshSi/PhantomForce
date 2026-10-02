#include "Object.h"

bool Object::g_draw_collisions = false;

namespace {

// sf::Sprite has no default constructor, so objects built without a texture
// reference this empty one. It is intentionally never destroyed.
const sf::Texture &get_null_texture() {
  static const sf::Texture *tex = new sf::Texture();
  return *tex;
}

}  // namespace

Object::Object() : sf::Sprite(get_null_texture()) { centerOrigin(); }

Object::Object(sf::Texture &tex) : sf::Sprite(tex) { centerOrigin(); }

Object::Object(sf::Texture &tex, const physics::Shape &shape)
    : sf::Sprite(tex), physics::Body(shape) {
  centerOrigin();
}

void Object::centerOrigin() { setOrigin(getLocalBounds().size / 2.f); }

// --- Circle -----------------------------------------------------------------

Circle::Circle() : sf::Sprite(get_null_texture()) {
  setShape(physics::Shape::circle(1.f));
}

Circle::Circle(sf::Texture &tex) : sf::Sprite(tex), Object(tex) {
  setShape(physics::Shape::circle(getLocalBounds().size.x / 2.f));
}

Circle::Circle(sf::Texture &tex, float r)
    : sf::Sprite(tex), Object(tex, physics::Shape::circle(r)) {}

// --- Rectangle --------------------------------------------------------------

Rectangle::Rectangle() : sf::Sprite(get_null_texture()) {
  setShape(physics::Shape::box({2.f, 2.f}));
}

Rectangle::Rectangle(sf::Texture &tex) : sf::Sprite(tex), Object(tex) {
  setShape(physics::Shape::box(getLocalBounds().size));
}

Rectangle::Rectangle(sf::Texture &tex, sf::Vector2f size)
    : sf::Sprite(tex), Object(tex, physics::Shape::box(size)) {}
