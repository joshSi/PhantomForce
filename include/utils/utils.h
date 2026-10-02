#ifndef GLOBAL_H
#define GLOBAL_H
#define _USE_MATH_DEFINES
#include <math.h>

#include <SFML/Graphics.hpp>

// Weighted avg between x and y
template <typename T>
T converge(T x, T y, T weight) {
  return (weight * x) + (1 - weight) * y;
}

// Angle of Vector2<T>
template <typename T>
float get_angle(sf::Vector2<T> v) {
  return (atan2f(v.y, v.x) * 180 / M_PI);
}

// Length (magnitude) of Vector2<T>
template <typename T>
float len(const sf::Vector2<T> v) {
  return sqrtf(powf(v.x, 2) + powf(v.y, 2));
}

// Helper function keeps x between -180 & 180 degrees
template <typename T>
void normalize(T& x) {
  if (x > 180) {
    x -= 360;
  }
  if (x <= -180) {
    x += 360;
  }
}

template <typename T>
sf::Vector2<T> max(sf::Vector2<T> a, sf::Vector2<T> b) {
  return sf::Vector2<T>(std::max(a.x, b.x), std::max(a.y, b.y));
}

// Smallest rectangle containing every non-transparent pixel of an image
// (the whole image if it is fully transparent).
inline sf::IntRect opaqueBounds(const sf::Image& image) {
  const sf::Vector2u size = image.getSize();
  unsigned int min_x = size.x, min_y = size.y, max_x = 0, max_y = 0;
  bool any = false;
  for (unsigned int y = 0; y < size.y; ++y)
    for (unsigned int x = 0; x < size.x; ++x)
      if (image.getPixel({x, y}).a > 0) {
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
        any = true;
      }
  if (!any) return sf::IntRect({0, 0}, sf::Vector2i(size));
  return sf::IntRect(
      sf::Vector2i(static_cast<int>(min_x), static_cast<int>(min_y)),
      sf::Vector2i(static_cast<int>(max_x - min_x + 1),
                   static_cast<int>(max_y - min_y + 1)));
}

#endif
