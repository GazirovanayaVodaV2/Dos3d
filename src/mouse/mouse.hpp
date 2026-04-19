#pragma once

#include "../utils/utils.hpp"
#include "../vec3/vec3.hpp"

#define MOUSE_RES_W 640
#define MOUSE_RES_H 200

struct mouse_state {
private:
	static int display_counter;

public:
	static i16 x, y;
	static bool left, right, middle;

	mouse_state();

	static void enable_cursor();
	static void disable_cursor();
	static void get_state();

	static void move(vec2i pos);
	static vec2i get_delta();
};
