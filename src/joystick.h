/*
 * joystick.h
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include "SDL.h"

typedef struct {
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

	float region_x1;
	float region_y1;
	float region_x2;
	float region_y2;
} joystick;

joystick *joystick_create(int persistent, float radius, float min_radius, float region_x, float region_y, float region_width, float region_height);
void joystick_free(joystick *stick);

void joystick_place(joystick *stick, float pos_x, float pos_y);
void joystick_axis(joystick *stick, float x, float y);

void joystick_touch(joystick *stick, float x, float y);
void joystick_release(joystick *stick);

void joystick_render(joystick *stick);

int joystick_finger_down(joystick *stick, SDL_TouchFingerEvent *finger);
int joystick_finger_move(joystick *stick, SDL_TouchFingerEvent *finger);
int joystick_finger_up(joystick *stick, SDL_TouchFingerEvent *finger);

#endif /* JOYSTICK_H_ */
