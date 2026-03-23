#include <sys/nearptr.h>
#include "graphics.h"
#include "stdbool.h"
#include "stdlib.h"

#include "../vec3/vec3.h"
#include "string.h"

graphic_context init_graphics() {
	graphic_context res;
	res.vram = (unsigned char *) (__djgpp_conventional_base + 0xa0000);

	res.current_vbuffer = NULL;
	res.program.m_vshader = NULL;
	res.program.m_fshader = NULL;
	return res;
}

void gc_bind_buffer(graphic_context *self, vertex_buffer *buffer) {
	self->current_vbuffer = buffer;
}

void gc_create_shader_program(graphic_context *self, vertex_shader vshader, fragment_shader fshader) {
	self->program.m_vshader = vshader;
	self->program.m_fshader = fshader;
}


void gc_render_buffer(graphic_context *self) {
	if (self->current_vbuffer && self->program.m_vshader && self->program.m_fshader) {
		for (int i = 0; i < self->current_vbuffer->len; i++) {
			vec3 point = self->current_vbuffer->points[i];
			vshader_result vres = self->program.m_vshader(point, self->program.m_vuniforms);

			vec3 screen_point_v = project(vres.point);
			u32 screen_point = screen_point_v.y * VRAM_W + screen_point_v.x;
			fshader_result fres = self->program.m_fshader(screen_point, self->program.m_funiforms);

			self->double_buffer[screen_point] = fres.color;
		}
	}
}
void gc_swap_buffer(graphic_context *self) {
	memcpy(self->vram, self->double_buffer, VRAM_SIZE);
	memset(self->double_buffer, 0, VRAM_SIZE);
}