#include <stdlib.h>
#include <stdio.h>
#include "texture.hpp"

texture::texture(const char *path) {


	FILE *file = fopen(path, "rb");
	if (file) {
		fread(&this->w, sizeof(byte), 1, file);
		fread(&this->h, sizeof(byte), 1, file);
		size_t data_size = this->w * this->h * sizeof(byte);
		this->pixels = (byte *) malloc(data_size);
		fread(this->pixels, 1, data_size, file);
	} else {
		printf("Failed to load texture: %s\n", path);
		exit(-1);
	}
}

texture::~texture() {
	free(this->pixels);
}

byte texture::sample(const vec2f uv1, const vec2f uv2, const vec2f uv3,
					 float w1, float w2, float w3) {
	float u = w1 * uv1.x + w2 * uv2.x + w3 * uv3.x;
	float v = w1 * uv1.y + w2 * uv2.y + w3 * uv3.y;
	u = 1.0f - u;
	v = 1.0f - v;


	int tx = (int) (u * (this->w - 1)) % this->w;
	int ty = (int) (v * (this->h - 1)) % this->h;
	return this->pixels[ty * this->w + tx];
}