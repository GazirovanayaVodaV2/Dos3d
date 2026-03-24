#include "matrix/matrix.h"
#include <bios.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define GDB_IMPLEMENTATION
#include "gdbstub.h"

#include "graphics/graphics.h"

vec3 triangle_vertex_test(vec3 point, const void *uniforms) {
	return point;
}

struct cube_uniform {
	matrix scale;
};

vec3 cube_test(vec3 point, const void *uniform) {
	const struct cube_uniform *cuniform = (const struct cube_uniform *) uniform;
	return mmult_vec(cuniform->scale, point);
}

byte gradient_fragment_shader(u32 pixel, const void *uniforms) {

	int x = pixel % VRAM_W;
	int y = pixel / VRAM_W;
	float intensity = (float) y / VRAM_H;
	byte color_base = 16;
	return (byte) (color_base + (intensity * 15));
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

	vertex_buffer triangle;
	triangle.len = 6;
	triangle.stride = 3;
	triangle.points = (vec3[]) {
			{-1, 0, 1},
			{-0.5, 0.25, 1},
			{-0.25, 0, 1},
			{1, 0, 1},
			{0.25, -0.25, 1},
			{0.5, 0, 1},
	};

	vertex_buffer cube;
	cube.len = 36;
	cube.stride = 3;
	cube.points = (vec3[]) {
			{-1, -1, 1},
			{1, -1, 1},
			{1, 1, 1},
			{1, 1, 1},
			{-1, 1, 1},
			{-1, -1, 1},
			{-1, -1, -1},
			{-1, 1, -1},
			{1, 1, -1},
			{1, 1, -1},
			{1, -1, -1},
			{-1, -1, -1},
			{-1, 1, 1},
			{-1, 1, -1},
			{-1, -1, -1},
			{-1, -1, -1},
			{-1, -1, 1},
			{-1, 1, 1},
			{1, 1, 1},
			{1, -1, 1},
			{1, -1, -1},
			{1, -1, -1},
			{1, 1, -1},
			{1, 1, 1},
			{-1, 1, -1},
			{-1, 1, 1},
			{1, 1, 1},
			{1, 1, 1},
			{1, 1, -1},
			{-1, 1, -1},
			{-1, -1, -1},
			{1, -1, -1},
			{1, -1, 1},
			{1, -1, 1},
			{-1, -1, 1},
			{-1, -1, -1}};

	shader_program triangle_program = gc_create_shader_program(triangle_vertex_test, NULL, triangle_fragment_test, NULL);

	struct cube_uniform cube_vuniform = {
			{0.5f, 0.0f, 0.0f, 0.0f,
			 0.0f, 0.3535f, 0.3535f, 0.0f,
			 0.0f, -0.3535f, 0.3535f, 0.0f,
			 0.0f, 0.0f, 1.5f, 1.0f}};
	shader_program cube_program = gc_create_shader_program(cube_test, &cube_vuniform, gradient_fragment_shader, NULL);

	//gc_bind_buffer(&triangle);
	//gc_bind_shader_program(&triangle_program);
	gc_bind_buffer(&cube);
	gc_bind_shader_program(&cube_program);

	while (!kbhit()) {
		gc_render_buffer();
		gc_swap_buffer();

		gdb_checkpoint();
	}

	quit_graphics();
	return 0;
}