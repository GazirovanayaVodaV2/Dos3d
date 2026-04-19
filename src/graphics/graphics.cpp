#include <stddef.h>
#include <stdio.h>
#include <sys/nearptr.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

#include "graphics.hpp"
#include "../vec3/vec3.hpp"
#include "float.h"
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

	/*for (int i = 0; i < VRAM_SIZE; i++) {
		global_graphic_context.Zbuffer[i] = FLT_MAX;
	}*/
	memset(global_graphic_context.Zbuffer, 0.0f, VRAM_SIZE * sizeof(number));
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
	if (pos.x >= 0 && pos.y >= 0 && pos.x < VRAM_W && pos.y < VRAM_H) {
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
			set_pixel((vec2i) {begin.x, (i16) (begin.y + i * params.ydir)}, color);
		}
		return;
	} else if (params.ishorizontal) {
		for (int i = 0; i <= params.absolutexdist; i++) {
			set_pixel((vec2i) {(i16) (begin.x + i * params.xdir), begin.y}, color);
		}
		return;
	} else {
		float weight = 0.0f;
		for (int i = 0; i <= params.absolutexdist; i++) {
			weight += params.deltaY;
			set_pixel((vec2i) {(i16) (begin.x + i * params.xdir), (i16) roundf(begin.y + weight)}, color);
		}
	}
}

static void draw_line_shaded(const vec2i begin, const vec2i end, const shader_program *sprogram,
							 const projected_vectex_t v1, const projected_vectex_t v2, const projected_vectex_t v3) {
	const line_params params = get_line_params(begin, end);
	data_for_fragment_shader data;
	data.uv[0] = v1.uv;
	data.uv[1] = v2.uv;
	data.uv[2] = v3.uv;
	if (params.isvertical) {
		for (int i = 0; i <= params.absoluteydist; i++) {
			vec2i p = (vec2i) {begin.x, (i16) (begin.y + i * params.ydir)};
			u32 pixel_i = get_index(p);

			get_barycentric(v1.point, v2.point, v3.point, p, &data.w1, &data.w2, &data.w3);
			number current_z = v1.z * data.w1 + v2.z * data.w2 + v3.z * data.w3;
			if (current_z > 0.01f) {
				current_z = 1.0f / current_z;
				if (current_z > global_graphic_context.Zbuffer[pixel_i]) {
					global_graphic_context.Zbuffer[pixel_i] = current_z;
					byte color = sprogram->m_fshader(&data, pixel_i, sprogram->m_funiforms);
					set_pixeli(pixel_i, color);
				}
			}
		}
		return;
	} else if (params.ishorizontal) {
		for (int i = 0; i <= params.absolutexdist; i++) {
			vec2i p = (vec2i) {(i16) (begin.x + i * params.xdir), begin.y};
			u32 pixel_i = get_index(p);

			get_barycentric(v1.point, v2.point, v3.point, p, &data.w1, &data.w2, &data.w3);
			number current_z = v1.z * data.w1 + v2.z * data.w2 + v3.z * data.w3;
			if (current_z > 0.01f) {
				current_z = 1.0f / current_z;
				if (current_z > global_graphic_context.Zbuffer[pixel_i]) {
					global_graphic_context.Zbuffer[pixel_i] = current_z;
					byte color = sprogram->m_fshader(&data, pixel_i, sprogram->m_funiforms);
					set_pixeli(pixel_i, color);
				}
			}
		}
		return;
	} else {
		float weight = 0.0f;
		for (int i = 0; i <= params.absolutexdist; i++) {
			weight += params.deltaY;
			vec2i p = (vec2i) {(i16) (begin.x + i * params.xdir), (i16) roundf(begin.y + weight)};
			u32 pixel_i = get_index(p);

			get_barycentric(v1.point, v2.point, v3.point, p, &data.w1, &data.w2, &data.w3);
			number current_z = v1.z * data.w1 + v2.z * data.w2 + v3.z * data.w3;
			if (current_z > 0.01f) {
				current_z = 1.0f / current_z;
				if (current_z > global_graphic_context.Zbuffer[pixel_i]) {
					global_graphic_context.Zbuffer[pixel_i] = current_z;
					byte color = sprogram->m_fshader(&data, pixel_i, sprogram->m_funiforms);
					set_pixeli(pixel_i, color);
				}
			}
		}
	}
}

static void draw_triangle(const projected_vectex_t v1, const projected_vectex_t v2, const projected_vectex_t v3,
						  const shader_program *sprogram) {
	projected_vectex_t proj_verts[3] = {v1, v2, v3};
	vec2i verts[3] = {v1.point, v2.point, v3.point};
	vec2f uvs[3] = {v1.uv, v2.uv, v3.uv};
	//sorting by y

	if (proj_verts[0].point.y > proj_verts[1].point.y) {
		SWAP(verts, 0, 1);
		SWAP(proj_verts, 0, 1);
		SWAP(uvs, 0, 1);
	}
	if (proj_verts[1].point.y > proj_verts[2].point.y) {
		SWAP(verts, 1, 2);
		SWAP(proj_verts, 1, 2);
		SWAP(uvs, 1, 2);
	}
	if (verts[0].y > verts[1].y) {
		SWAP(verts, 0, 1);
		SWAP(proj_verts, 0, 1);
		SWAP(uvs, 0, 1);
	}

	const vec2i top = verts[0], middle = verts[1], bottom = verts[2];
	const vec2f topuv = uvs[0], middleuv = uvs[1], bottomuv = uvs[2];
	const projected_vectex_t top_proj = {top, proj_verts[0].z, topuv},
							 middle_proj = {middle, proj_verts[1].z, middleuv},
							 bottom_proj = {bottom, proj_verts[2].z, bottomuv};


	const line_params longest = get_line_params(top, bottom),
					  from_top_to_middle = get_line_params(top, middle),
					  from_middle_to_bottom = get_line_params(middle, bottom);
	float long_w = 0.0f, middle_w = 0.0f, bottom_w = 0.0f;
	bool is_reached_middle = false;

	for (int iter = 0, sec_iter = 0; iter < longest.absoluteydist; iter++) {
		long_w += longest.deltaX;
		vec2i longest_line_pixel, middle_line_pixel, bottom_line_pixel;
		longest_line_pixel = (vec2i) {(i16) roundf(top.x + long_w), (i16) (top.y + iter * longest.ydir)};


		if (is_reached_middle) {
			bottom_w += from_middle_to_bottom.deltaX;
			bottom_line_pixel = (vec2i) {(i16) roundf(middle.x + bottom_w),
										 (i16) (middle.y + sec_iter * from_middle_to_bottom.ydir)};

			draw_line_shaded(longest_line_pixel, bottom_line_pixel, sprogram, top_proj, middle_proj, bottom_proj);
			sec_iter++;
		} else {
			is_reached_middle = iter == from_top_to_middle.absoluteydist;

			middle_w += from_top_to_middle.deltaX;
			middle_line_pixel = (vec2i) {(i16) roundf(top.x + middle_w),
										 (i16) (top.y + iter * from_top_to_middle.ydir)};
			draw_line_shaded(longest_line_pixel, middle_line_pixel, sprogram, top_proj, middle_proj, bottom_proj);
		}
	}//m_fshader(u32 coord, global_graphic_context.current_program->m_funiforms);
}

void gc_render_buffer() {
	if (global_graphic_context.current_vbuffer && global_graphic_context.current_program->m_vshader && global_graphic_context.current_program->m_fshader) {
		size_t point_count = global_graphic_context.current_vbuffer->len;
		u16 stride = global_graphic_context.current_vbuffer->stride;

		projected_vectex_t points[point_count];
		for (int i = 0; i < point_count; i++) {

			vertex_t vertex = global_graphic_context.current_vbuffer->points[i];

			vertex_t vpoint = global_graphic_context.current_program->m_vshader(vertex, global_graphic_context.current_program->m_vuniforms);

			number z_val = vpoint.point.z;
			if (fabs(z_val) < 0.01f) z_val = 0.01f;

			vec2f projected_point = {vpoint.point.x, vpoint.point.y};

			projected_point.x = (projected_point.x + 1) / 2.0f;
			projected_point.y = (projected_point.y + 1) / 2.0f;

			projected_point.y = 1.0f - projected_point.y;
			projected_point.x *= VRAM_W;
			projected_point.y *= VRAM_H;

			if (projected_point.x < 0) projected_point.x = 0;
			if (projected_point.x >= VRAM_W) projected_point.x = VRAM_W - 1;
			if (projected_point.y < 0) projected_point.y = 0;
			if (projected_point.y >= VRAM_H) projected_point.y = VRAM_H - 1;


			points[i].point.x = (u16) roundf(projected_point.x);
			points[i].point.y = (u16) roundf(projected_point.y);
			points[i].uv = vpoint.uv;
			points[i].z = z_val;
		}

		for (int i = 0; i < point_count; i += 3) {
			if (points[i].z < 0.1f || points[i + 1].z < 0.1f || points[i + 2].z < 0.1f) {
				continue;
			}
			draw_triangle(points[i], points[i + 1], points[i + 2], global_graphic_context.current_program);
		}
	}
}
void gc_swap_buffer() {
	memcpy(global_graphic_context.vram, global_graphic_context.double_buffer, VRAM_SIZE);
	memset(global_graphic_context.double_buffer, 0, VRAM_SIZE);
	memset(global_graphic_context.Zbuffer, 0.0f, VRAM_SIZE * sizeof(number));
}

void get_barycentric(const vec2i v1, const vec2i v2, const vec2i v3, const vec2i p,
					 number *w1, number *w2, number *w3) {
	number det = (v2.y - v3.y) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.y - v3.y);

	*w1 = ((v2.y - v3.y) * (p.x - v3.x) + (v3.x - v2.x) * (p.y - v3.y)) / det;
	*w2 = ((v3.y - v1.y) * (p.x - v3.x) + (v1.x - v3.x) * (p.y - v3.y)) / det;
	*w3 = 1.0f - *w1 - *w2;
}

byte sample_texture(const vec2f uv1, const vec2f uv2, const vec2f uv3,
					float w1, float w2, float w3,
					const texture *txt) {
	float u = w1 * uv1.x + w2 * uv2.x + w3 * uv3.x;
	float v = w1 * uv1.y + w2 * uv2.y + w3 * uv3.y;
	u = 1.0f - u;
	v = 1.0f - v;


	int tx = (int) (u * (txt->w - 1)) % txt->w;
	int ty = (int) (v * (txt->h - 1)) % txt->h;
	return txt->pixels[ty * txt->w + tx];
}

texture load_texture(const char *path) {
	texture res = {0};

	FILE *file = fopen(path, "rb");
	if (file) {
		fread(&res.w, sizeof(byte), 1, file);
		fread(&res.h, sizeof(byte), 1, file);
		size_t data_size = res.w * res.h * sizeof(byte);
		res.pixels = (byte *) malloc(data_size);
		fread(res.pixels, 1, data_size, file);
	} else {
		printf("Failed to load texture: %s\n", path);
		exit(-1);
	}

	return res;
}

void destroy_texture(texture *self) {
	free(self->pixels);
}

void create_lens(camera_lens *lens) {
	lens->projection.identity();
	float fov_rad = lens->fov * (3.14159f / 180.0f);
	float scale = 1.0f / tanf(fov_rad / 2);
	float range_inv = 1.0f / (lens->near_cliping - lens->far_cliping);

	lens->projection.m_mat[0][0] = scale;
	lens->projection.m_mat[1][1] = scale;
	lens->projection.m_mat[2][2] = (lens->far_cliping + lens->near_cliping) * range_inv;
	lens->projection.m_mat[2][3] = -1.0f;
	lens->projection.m_mat[3][2] = (2.0f * lens->far_cliping * lens->near_cliping) * range_inv;
	lens->projection.m_mat[3][3] = 0.0f;
}

void camera_lookat(camera *self) {
	//basis
	vec3 f = normalize({self->pos.x - self->target.x,
						self->pos.y - self->target.y,
						self->pos.z - self->target.z});


	vec3 r = normalize(cross(self->upvector, f));


	vec3 u = cross(f, r);


	self->view.m_mat[0][0] = r.x;
	self->view.m_mat[0][1] = u.x;
	self->view.m_mat[0][2] = f.x;
	self->view.m_mat[0][3] = 0.0f;

	self->view.m_mat[1][0] = r.y;
	self->view.m_mat[1][1] = u.y;
	self->view.m_mat[1][2] = f.y;
	self->view.m_mat[1][3] = 0.0f;

	self->view.m_mat[2][0] = r.z;
	self->view.m_mat[2][1] = u.z;
	self->view.m_mat[2][2] = f.z;
	self->view.m_mat[2][3] = 0.0f;

	self->view.m_mat[3][0] = -dot(r, self->pos);
	self->view.m_mat[3][1] = -dot(u, self->pos);
	self->view.m_mat[3][2] = -dot(f, self->pos);
	self->view.m_mat[3][3] = 1.0f;
}