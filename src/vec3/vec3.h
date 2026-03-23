#ifndef VEC3_H
#define VEC3_H

#include "../utils/utils.h"

typedef struct vec3 {
	number x, y, z;
} vec3;

float dot(vec3 a, vec3 b);
vec3 cross(vec3 a, vec3 b);
vec3 normalize(vec3 a);
vec3 project(vec3 p);

#endif