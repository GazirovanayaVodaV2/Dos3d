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

static inline u32 get_index(const vec2i pos) {
	return pos.y * VRAM_W + pos.x;
}

static inline void set_pixel(const vec2i pos, const byte color) {
	if (pos.y < VRAM_H && pos.x < VRAM_W) {
		const u32 screen_point = get_index(pos);
		global_graphic_context.double_buffer[screen_point] = color;
	}
}

static inline void set_pixeli(const u32 pos, const byte color) {
	if (pos < VRAM_SIZE) {
		global_graphic_context.double_buffer[pos] = color;
	}
}

line_params get_line_params(const vec2i begin, const vec2i end) {
	const int xdist = end.x - begin.x;
	const int ydist = end.y - begin.y;

	const int absolutexdist = abs(xdist);
	const int absoluteydist = abs(ydist);

	const bool isvertical = (xdist == 0);
	const bool ishorizontal = (ydist == 0);

	const float deltaX = (!ishorizontal) ? (float) xdist / absoluteydist : 0.0f;
	const float deltaY = (!isvertical) ? (float) ydist / absolutexdist : 0.0f;

	const i8 xdir = (xdist > 0) ? 1 : (xdist < 0 ? -1 : 0);
	const i8 ydir = (ydist > 0) ? 1 : (ydist < 0 ? -1 : 0);

	return (line_params) {
			xdist,
			absolutexdist,
			ydist,
			absoluteydist,
			isvertical,
			ishorizontal,
			deltaX,
			deltaY,
			xdir,
			ydir};
}

void draw_line(const vec2i begin, const vec2i end, const byte color) {
	const line_params params = get_line_params(begin, end);

	if (params.isvertical) {
		for (int i = 0; i <= params.absoluteydist; i++) {
			set_pixel((vec2i) {begin.x, begin.y + i * params.ydir}, color);
		}
		return;
	} else if (params.ishorizontal) {
		for (int i = 0; i <= params.absolutexdist; i++) {
			set_pixel((vec2i) {begin.x + i * params.xdir, begin.y}, color);
		}
		return;
	} else {
		float weight = 0.0f;
		for (int i = 0; i <= params.absolutexdist; i++) {
			weight += params.deltaY;
			set_pixel((vec2i) {begin.x + i * params.xdir, (int) roundf(begin.y + weight)}, color);
		}
	}
}

static void draw_triangle(const vec2i v1, const vec2i v2, const vec2i v3, const byte color) {
	vec2i verts[3] = {v1, v2, v3};
	//sorting by y
	if (verts[0].y > verts[1].y) {
		SWAP(verts, 0, 1);
	}
	if (verts[1].y > verts[2].y) {
		SWAP(verts, 1, 2);
	}
	if (verts[0].y > verts[1].y) {
		SWAP(verts, 0, 1);
	}

	const vec2i top = verts[0], middle = verts[1], bottom = verts[2];

	const line_params longest = get_line_params(top, bottom),
					  from_top_to_middle = get_line_params(top, middle),
					  from_middle_to_bottom = get_line_params(middle, bottom);
	float long_w = 0.0f, middle_w = 0.0f, bottom_w = 0.0f;
	bool is_reached_middle = false;

	for (int iter = 0, sec_iter = 0; iter < longest.absoluteydist; iter++) {
		long_w += longest.deltaX;
		vec2i longest_line_pixel, middle_line_pixel, bottom_line_pixel;
		longest_line_pixel = (vec2i) {(int) roundf(top.x + long_w), top.y + iter * longest.ydir};


		if (is_reached_middle) {
			bottom_w += from_middle_to_bottom.deltaX;
			bottom_line_pixel = (vec2i) {(int) roundf(middle.x + bottom_w), middle.y + sec_iter * from_middle_to_bottom.ydir};

			draw_line(longest_line_pixel, bottom_line_pixel, color);
			sec_iter++;
		} else {
			is_reached_middle = iter == from_top_to_middle.absoluteydist;

			middle_w += from_top_to_middle.deltaX;
			middle_line_pixel = (vec2i) {(int) roundf(top.x + middle_w), top.y + iter * from_top_to_middle.ydir};
			draw_line(longest_line_pixel, middle_line_pixel, color);
		}
	}
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

		/*for (int i = 0; i < point_count; i++) {
			u32 screen_point = get_index(points[i]);

			byte fcolor = global_graphic_context.current_program->m_fshader(screen_point, global_graphic_context.current_program->m_funiforms);
		}*/
		//temp solution
		byte fcolor = global_graphic_context.current_program->m_fshader(get_index(points[0]), global_graphic_context.current_program->m_funiforms);
		draw_triangle(points[0], points[1], points[2], fcolor);
	}
}
void gc_swap_buffer() {
	memcpy(global_graphic_context.vram, global_graphic_context.double_buffer, VRAM_SIZE);
	memset(global_graphic_context.double_buffer, 0, VRAM_SIZE);
}