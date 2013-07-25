/*
 * joystick.h
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include "SDL.h"
#include "touch.h"

typedef struct {
	touchable touch_data;

	SDL_FingerID finger_id;

	int pressed;

	/* normalized joystick orientation */
	float axis_x; /* [-1, 1] */
	float axis_y; /* [-1, 1] */
	float direction; /* [0, 2pi] */
	float amplitude; /* [0, 1] */

	float pos_x;
	float pos_y;

	float radius;

	float min_range; /* minimum distance from center of joystick before responding */

	int persistent;
} joystick;

joystick *joystick_create(int persistent, float radius, float min_radius, float region_x, float region_y, float region_width, float region_height);
void joystick_free(joystick *stick);

void joystick_axis(joystick *stick, float x, float y);

void joystick_touch(joystick *stick, float x, float y);
void joystick_release(joystick *stick);

#endif /* JOYSTICK_H_ */
