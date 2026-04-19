#include <dpmi.h>
#include <math.h>

#include "mouse.hpp"

int mouse_state::display_counter = 0;

i16 mouse_state::x = 0, mouse_state::y = 0;
bool mouse_state::left = false, mouse_state::right = false, mouse_state::middle = false;

mouse_state::mouse_state() {
	__dpmi_regs regs;

	regs.x.ax = 0;
	__dpmi_int(0x33, &regs);
}

void mouse_state::enable_cursor() {
	__dpmi_regs regs;

	if (display_counter == 0) {
		regs.x.ax = 1;
		__dpmi_int(0x33, &regs);
		display_counter++;
	}
}
void mouse_state::disable_cursor() {
	__dpmi_regs regs;

	if (display_counter >= 1) {
		regs.x.ax = 2;
		__dpmi_int(0x33, &regs);
		display_counter--;
	}
}

void mouse_state::get_state() {
	__dpmi_regs regs;
	regs.x.ax = 0x0003;
	__dpmi_int(0x33, &regs);

	x = (u16) roundf((float) regs.x.cx / MOUSE_RES_W * 320);
	y = regs.x.dx;

	left = regs.x.bx & 1;
	right = regs.x.bx & 2;
	middle = regs.x.bx & 4;
}

void mouse_state::move(vec2i pos) {

	auto _pos = pos;
	_pos.x = (i16) roundf(_pos.x / 320.0f * 640);

	__dpmi_regs regs;
	regs.x.ax = 4;
	regs.x.cx = _pos.x;
	regs.x.dx = _pos.y;
	mouse_state::x = _pos.x;
	mouse_state::y = _pos.y;

	__dpmi_int(0x33, &regs);
}

vec2i mouse_state::get_delta() {
	get_state();

	i16 _x = x, _y = y;

	move(vec2i{160, 100});

	get_state();

	return vec2i{(i16) (_x - x), (i16) (_y - y)};
}