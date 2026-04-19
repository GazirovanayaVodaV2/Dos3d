#ifndef VEC3_H
#define VEC3_H

#include "../utils/utils.hpp"

typedef struct vec2i {
	i16 x, y;
} vec2i;

typedef struct uvec2i {
	u16 x, y;
} uvec2i;

typedef struct vec2f {
	float x, y;
} vec2f;

struct vec3 {
	number x, y, z;

	vec3 operator+(vec3 a);
	vec3 operator-(vec3 a);
	vec3 operator*(vec3 a);

	vec3 normalize();
	vec2f project();

	float dot(vec3 a);
};

float dot(vec3 a, vec3 b);
vec3 cross(vec3 a, vec3 b);
vec3 normalize(vec3 a);
vec2f project(vec3 p);

#endif