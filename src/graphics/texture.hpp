#pragma once

#include "../utils/utils.hpp"
#include "../vec3/vec3.hpp"

class texture {
public:
	byte *pixels = nullptr;
	byte w = 0, h = 0;
	texture() = default;
	texture(const char *path);
	~texture();

	byte sample(const vec2f uv1, const vec2f uv2, const vec2f uv3,
				float w1, float w2, float w3);
};