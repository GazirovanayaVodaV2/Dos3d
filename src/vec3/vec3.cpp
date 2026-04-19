#include <math.h>

#include "vec3.hpp"

vec3 vec3::operator+(vec3 a) {
	return {
			this->x + a.x,
			this->y + a.y,
			this->z + a.z,
	};
}

vec3 vec3::operator-(vec3 a) {
	return {
			this->x - a.x,
			this->y - a.y,
			this->z - a.z,
	};
}

vec3 vec3::operator*(vec3 a) {
	return cross(*this, a);
}

vec3 vec3::normalize() {
	return ::normalize(*this);
}

vec2f vec3::project() {
	return ::project(*this);
}

float vec3::dot(vec3 a) {
	return ::dot(*this, a);
}

float dot(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 cross(vec3 a, vec3 b) {
	return (vec3) {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x};
}

vec3 normalize(vec3 a) {
	float len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	if (len == 0) return (vec3) {0, 0, 0};
	return (vec3) {a.x / len, a.y / len, a.z / len};
}

vec2f project(vec3 p) {
	return (vec2f) {
			p.x / p.z,
			p.y / p.z};
}