#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdlib.h>
#include <bios.h>
#include <dpmi.h>
#include "../utils/utils.h"
#include "../vec3/vec3.h"

#define VRAM_SIZE 64000
#define VRAM_W 320
#define VRAM_H 200

typedef struct vertex_buffer {
	vec3 *points;
	size_t len;
	byte primary_color;
} vertex_buffer;

typedef struct vshader_result {
	vec3 point;
	const void *uniforms
} vshader_result;

typedef struct fshader_result {
	byte color;
	const void *uniforms;
} fshader_result;

typedef struct render_result {
	vshader_result m_vres;
	fshader_result m_fres;
} render_result;

typedef vshader_result (*vertex_shader)(vec3 point, const void *uniforms);
typedef fshader_result (*fragment_shader)(u32 pixel, const void *uniforms);

typedef struct shader_program {
	vertex_shader m_vshader;
	fragment_shader m_fshader;

	void *m_vuniforms, *m_funiforms;
} shader_program;

typedef struct graphic_context {
	byte *vram;
	byte double_buffer[VRAM_SIZE];

	vertex_buffer *current_vbuffer;
	shader_program program;
} graphic_context;

graphic_context init_graphics();

void gc_bind_buffer(graphic_context *self, vertex_buffer *buffer);
void gc_create_shader_program(graphic_context *self, vertex_shader vshader, fragment_shader fshader);

void gc_render_buffer(graphic_context *self);

void gc_swap_buffer(graphic_context *self);

#endif