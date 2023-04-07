#ifndef GLOBAL_H
#define GLOBAL_H
#include <math.h>
#include <SFML/Graphics.hpp>

const float PI = 3.14159265358979323846264338327950288;

// Weighted avg between x and y
template<typename T>
T converge(T x, T y, T weight) {
	return (weight * x) + (1 - weight) * y;
}

// Angle between two Vector2<T>s
template<typename T>
float get_angle(sf::Vector2<T> v) {
	return (atan2f(v.y, v.x) * 180 / PI);
}

// Length (magnitude) of Vector2<T>
template<typename T>
float len(const sf::Vector2<T> v) {
	return sqrtf(powf(v.x, 2) + powf(v.y, 2));
}

template<typename T>
void normalize(T& x) {
	if (x > 180) {
		x -= 360;
	}
	if (x <= -180) {
		x += 360;
	}
}

#endif
