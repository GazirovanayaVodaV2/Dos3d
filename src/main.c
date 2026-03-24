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
	triangle.len = 3;
	triangle.points = (vec3[]) {
			{-0.5, 0, 1},
			{0, 0.25, 1},
			{0.5, 0, 1},
	};

	shader_program triangle_program = gc_create_shader_program(triangle_vertex_test, NULL, triangle_fragment_test, NULL);

	gc_bind_buffer(&triangle);
	gc_bind_shader_program(&triangle_program);

	while (!kbhit()) {
		gc_render_buffer();
		gc_swap_buffer();

		gdb_checkpoint();
	}

	quit_graphics();
	return 0;
}