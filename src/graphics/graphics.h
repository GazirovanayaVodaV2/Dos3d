#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stddef.h>
#include <stdlib.h>
#include <bios.h>
#include <dpmi.h>
#include <stdbool.h>
#include "../utils/utils.h"
#include "../vec3/vec3.h"

#define VRAM_SIZE 64000
#define VRAM_W 320
#define VRAM_H 200

#define TEXT_MODE 0x3
#define VIDEO_MODE 0x13

void set_video_mode(int mode);

typedef struct vertex_t {
	vec3 point;
	vec2f uv;
} vertex_t;

typedef struct projected_vectex_t {
	vec2i point;
	number z;
	vec2f uv;
} projected_vectex_t;

typedef struct vertex_buffer {
	vertex_t *points;
	size_t len;
	u16 stride;
} vertex_buffer;

typedef struct texture {
	byte *pixels;
	byte w, h;
} texture;

typedef struct data_for_fragment_shader {
	vec2f uv[3];
	number w1, w2, w3;
} data_for_fragment_shader;

typedef vertex_t (*vertex_shader)(vertex_t v, const void *uniforms);
typedef byte (*fragment_shader)(data_for_fragment_shader *data,
								u32 pixel, const void *uniforms);

typedef struct shader_program {
	vertex_shader m_vshader;
	fragment_shader m_fshader;

	void *m_vuniforms, *m_funiforms;
} shader_program;

typedef struct graphic_context {
	byte *vram;
	byte double_buffer[VRAM_SIZE];
	number Zbuffer[VRAM_SIZE];

	vertex_buffer *current_vbuffer;
	const shader_program *current_program;
} graphic_context;

extern graphic_context global_graphic_context;

typedef struct line_params {
	const int xdist;
	const int absolutexdist;
	const int ydist;
	const int absoluteydist;
	const bool isvertical;
	const bool ishorizontal;
	const float deltaX;
	const float deltaY;
	const i8 xdir;
	const i8 ydir;
} line_params;

line_params get_line_params(const vec2i begin, const vec2i end);

void draw_line(const vec2i begin, const vec2i end, const byte color);

void init_graphics();
void quit_graphics();

void gc_bind_buffer(vertex_buffer *buffer);
shader_program gc_create_shader_program(vertex_shader vshader, void *vuniforms,
										fragment_shader fshader, void *funiforms);
void gc_bind_shader_program(const shader_program *program);

void gc_render_buffer();

void gc_swap_buffer();

void get_barycentric(const vec2i v1, const vec2i v2, const vec2i v3, const vec2i p,
					 number *w1, number *w2, number *w3);

byte sample_texture(const vec2f uv1, const vec2f uv2, const vec2f uv3,
					float w1, float w2, float w3,
					const texture *txt);

texture load_texture(const char *path);
void destroy_texture(texture *self);

#endif