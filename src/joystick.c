/*
 * joystick.c
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#include "joystick.h"
#include "math.h"

void joystick_touch(joystick *stick, float pos_x, float pos_y)
{
	if (!stick->active) {
		stick->active = 1;
		joystick_place(stick, pos_x, pos_y);
		joystick_axis(stick, 0, 0);
	} else {
		float dx = (pos_x - stick->pos_x) / stick->size;
		float dy = -(pos_y - stick->pos_y) / stick->size; //dy inverted //todo ta hensyn til ratio

		float length = hypotf(dx, dy);
		dx /= length;
		dy /= length;

		if (length > stick->min_range) {
			if (length > 1) {
				stick->pos_x = pos_x - stick->size * dx;
				stick->pos_y = pos_y + stick->size * dy;
				stick->amplitude = 1;
			} else {
				stick->amplitude = length;
			}
			float dir = atan2f(dy,dx);
			stick->direction = dir < 0 ? dir + 2*M_PI : dir;

			stick->axis_x = dx;
			stick->axis_y = dy;
		}
	}
}

void joystick_release(joystick *stick)
{
	stick->active = 0;
	joystick_axis(stick, 0, 0);

}

void joystick_place(joystick *stick, float pos_x, float pos_y)
{
	stick->pos_x = pos_x;
	stick->pos_y = pos_y;
}

void joystick_axis(joystick *stick, float x, float y)
{
	stick->axis_x = x < -1 ? -1 : (x > 1 ? 1 : x);
	stick->axis_y = y < -1 ? -1 : (y > 1 ? 1 : y);

	float dir = (x || y) ? atan2f(y,x) : 0;
	stick->direction = dir < 0 ? dir + 2*M_PI : dir;
	stick->amplitude = hypotf(y,x);
}
