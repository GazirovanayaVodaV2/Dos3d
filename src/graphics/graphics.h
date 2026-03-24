#ifndef GRAPHICS_H
#define GRAPHICS_H

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

typedef struct vertex_buffer {
	vec3 *points;
	size_t len;
	u16 stride;
} vertex_buffer;

typedef vec3 (*vertex_shader)(vec3 point, const void *uniforms);
typedef byte (*fragment_shader)(u32 pixel, const void *uniforms);

typedef struct shader_program {
	vertex_shader m_vshader;
	fragment_shader m_fshader;

	void *m_vuniforms, *m_funiforms;
} shader_program;

typedef struct graphic_context {
	byte *vram;
	byte double_buffer[VRAM_SIZE];

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

#endif