#include <sys/nearptr.h>
#include "graphics.h"
#include "stdbool.h"
#include "stdlib.h"
#include <math.h>

#include "../vec3/vec3.h"
#include "string.h"

graphic_context global_graphic_context;

void set_video_mode(int mode) {
	__dpmi_regs regs = {0};
	regs.x.ax = mode;
	__dpmi_int(0x10, &regs);
}

void init_graphics() {
	__djgpp_nearptr_enable();

	set_video_mode(VIDEO_MODE);

	global_graphic_context.vram = (unsigned char *) (__djgpp_conventional_base + 0xa0000);

	global_graphic_context.current_vbuffer = NULL;
	global_graphic_context.current_program = NULL;
}

void quit_graphics() {
	__djgpp_nearptr_disable();
	set_video_mode(TEXT_MODE);
}

void gc_bind_buffer(vertex_buffer *buffer) {
	global_graphic_context.current_vbuffer = buffer;
}

shader_program gc_create_shader_program(vertex_shader vshader, void *vuniforms,
										fragment_shader fshader, void *funiforms) {
	shader_program res;
	res.m_vshader = vshader;
	res.m_vuniforms = vuniforms;
	res.m_fshader = fshader;
	res.m_funiforms = funiforms;
	return res;
}

void gc_bind_shader_program(const shader_program *program) {
	global_graphic_context.current_program = program;
}

void gc_render_buffer() {
	if (global_graphic_context.current_vbuffer && global_graphic_context.current_program->m_vshader && global_graphic_context.current_program->m_fshader) {
		size_t point_count = global_graphic_context.current_vbuffer->len;

		vec2i points[point_count];
		for (int i = 0; i < point_count; i++) {

			vec3 point = global_graphic_context.current_vbuffer->points[i];
			vec3 vpoint = global_graphic_context.current_program->m_vshader(point, global_graphic_context.current_program->m_vuniforms);
			vpoint.x = (vpoint.x + 1) / 2.0f;
			vpoint.y = (vpoint.y + 1) / 2.0f;

			vec2f projected_point = project(vpoint);
			projected_point.y = 1.0f - projected_point.y;
			projected_point.x *= VRAM_W;
			projected_point.y *= VRAM_H;
			points[i].x = (u16) roundf(projected_point.x);
			points[i].y = (u16) roundf(projected_point.y);
		}

		for (int i = 0; i < point_count; i++) {
			u16 screen_point = points[i].y * VRAM_W + points[i].x;

			byte fcolor = global_graphic_context.current_program->m_fshader(screen_point, global_graphic_context.current_program->m_funiforms);
			global_graphic_context.double_buffer[screen_point] = fcolor;
		}

		/*
		
			vec3 screen_point_v = project(vpoint);
			u32 screen_point = screen_point_v.y * VRAM_W + screen_point_v.x;
			byte fcolor = global_graphic_context.program.m_fshader(screen_point, global_graphic_context.program.m_funiforms);

			global_graphic_context.double_buffer[screen_point] = fcolor;
		
		*/
	}
}
void gc_swap_buffer() {
	memcpy(global_graphic_context.vram, global_graphic_context.double_buffer, VRAM_SIZE);
	memset(global_graphic_context.double_buffer, 0, VRAM_SIZE);
}