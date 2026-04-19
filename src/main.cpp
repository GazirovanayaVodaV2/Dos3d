#include "matrix/matrix.hpp"
#include "vec3/vec3.hpp"
#include <bios.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define GDB_IMPLEMENTATION

extern "C" {
#include "gdbstub.h"
}

#include "graphics/graphics.hpp"

vertex_t triangle_vertex_test(vertex_t point, const void *uniforms) {
	return point;
}

struct cube_vuniform {
	matrix model;
	camera *cam;
};

struct cube_funiform {
	texture *txt;
};

vertex_t cube_test(vertex_t point, const void *uniforms) {
	const struct cube_vuniform *cuniform = (const struct cube_vuniform *) uniforms;
	const camera *cam = cuniform->cam;

	matrix res;
	res = cam->lens.projection * cam->view * cuniform->model;
	vec3 v = res.project(point.point);
	return (vertex_t) {v, point.uv};
}

byte gradient_fragment_shader(data_for_fragment_shader *data, u32 pixel, const void *uniforms) {
	const struct cube_funiform *funiform = (const struct cube_funiform *) uniforms;

	return sample_texture(data->uv[0], data->uv[1], data->uv[2],
						  data->w1, data->w2, data->w3, funiform->txt);
}

byte triangle_fragment_test(u32 pixel, const void *uniforms) {
	return 15;
}

int main(void) {
	printf("Press 1 to start debug");
	if (getch() == '1') {
		putchar('\n');
		gdb_start();
	}

	init_graphics();

	camera cam = {
			.pos = {0.5f, 0, -3},
			.target = {0, 0, 0},
			.upvector = {0, 2, 0},
			.lens = {
					45,
					(float) VRAM_W / VRAM_H,
					0.01f,
					100.0f}};

	create_lens(&cam.lens);
	camera_lookat(&cam);

	/*vertex_buffer triangle;
	triangle.len = 6;
	triangle.stride = 3;
	triangle.points = (vec3[]) {
			{-1, 0, 1},
			{-0.5, 0.25, 1},
			{-0.25, 0, 1},
			{1, 0, 1},
			{0.25, -0.25, 1},
			{0.5, 0, 1},
	};*/

	texture bricks = load_texture("assets/textures/bricks.vga");

	vertex_t cube_vertices[] = {
			// Front
			{{-1, -1, 1}, {0, 0}},
			{{1, -1, 1}, {1, 0}},
			{{1, 1, 1}, {1, 1}},
			{{1, 1, 1}, {1, 1}},
			{{-1, 1, 1}, {0, 1}},
			{{-1, -1, 1}, {0, 0}},

			// Back
			{{-1, -1, -1}, {1, 0}},
			{{-1, 1, -1}, {1, 1}},
			{{1, 1, -1}, {0, 1}},
			{{1, 1, -1}, {0, 1}},
			{{1, -1, -1}, {0, 0}},
			{{-1, -1, -1}, {1, 0}},

			// Left
			{{-1, 1, 1}, {1, 1}},
			{{-1, 1, -1}, {0, 1}},
			{{-1, -1, -1}, {0, 0}},
			{{-1, -1, -1}, {0, 0}},
			{{-1, -1, 1}, {1, 0}},
			{{-1, 1, 1}, {1, 1}},

			// Right
			{{1, 1, 1}, {0, 1}},
			{{1, -1, 1}, {0, 0}},
			{{1, -1, -1}, {1, 0}},
			{{1, -1, -1}, {1, 0}},
			{{1, 1, -1}, {1, 1}},
			{{1, 1, 1}, {0, 1}},

			// Top
			{{-1, 1, -1}, {0, 0}},
			{{-1, 1, 1}, {0, 1}},
			{{1, 1, 1}, {1, 1}},
			{{1, 1, 1}, {1, 1}},
			{{1, 1, -1}, {1, 0}},
			{{-1, 1, -1}, {0, 0}},

			// Bottom
			{{-1, -1, -1}, {1, 1}},
			{{1, -1, -1}, {0, 1}},
			{{1, -1, 1}, {0, 0}},
			{{1, -1, 1}, {0, 0}},
			{{-1, -1, 1}, {1, 0}},
			{{-1, -1, -1}, {1, 1}},
	};

	vertex_buffer cube;
	cube.len = sizeof(cube_vertices) / sizeof(vertex_t);
	cube.stride = sizeof(vertex_t);
	cube.points = cube_vertices;

	//shader_program triangle_program = gc_create_shader_program(triangle_vertex_test, NULL,
	//														   triangle_fragment_test, NULL);

	struct cube_vuniform cube_vuniform = {
			.model = matrix(),
			.cam = &cam};
	struct cube_funiform cube_funiform = {
			&bricks};
	shader_program cube_program = gc_create_shader_program(cube_test, &cube_vuniform,
														   gradient_fragment_shader, &cube_funiform);

	//gc_bind_buffer(&triangle);
	//gc_bind_shader_program(&triangle_program);
	gc_bind_buffer(&cube);
	gc_bind_shader_program(&cube_program);

	while (!kbhit()) {
		gc_render_buffer();

		cam.pos.z -= 0.1;
		cam.pos.x += 0.1;
		cam.pos.y += 0.1;
		cam.target = cam.pos + vec3{-0.5, 0, 1};
		camera_lookat(&cam);

		gc_swap_buffer();


		gdb_checkpoint();
	}

	destroy_texture(&bricks);

	quit_graphics();
	return 0;
}