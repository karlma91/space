/*
 * joystick.c
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#include "joystick.h"
#include "math.h"


void joystick_place(joystick *stick, float pos_x, float pos_y)
{
	stick->pos_x = pos_x;
	stick->pos_y = pos_y;
}

void joystick_axis(joystick *stick, float x, float y)
{
	stick->axis_x = x;
	stick->axis_y = y;

	float dir = atan2f(y,x);
	stick->direction = dir < 0 ? dir + 2*M_PI : dir;
	stick->amplitude = hypotf(y,x);
}

void joystick_pos(joystick *stick, float x, float y)
{
	stick->pos_x = x;
	stick->pos_y = y;
}
