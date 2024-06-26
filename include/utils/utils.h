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

#endif
