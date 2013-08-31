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
#include "../graphics/sprite.h"

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

	float draw_axx;
	float draw_axy;
	float draw_dir;
	float draw_amp;
	float draw_x;
	float draw_y;

	float radius;

	float min_range; /* minimum distance from center of joystick before responding */

	sprite spr_back;
	sprite spr_front;

	SDL_Scancode key_left, key_up, key_right, key_down;

	int persistent;
} joystick;

joystick *joystick_create(int persistent, float radius, float min_radius, float center_x, float center_y, float width, float height, SPRITE_ID spr_back, SPRITE_ID spr_front);
void joystick_free(joystick *stick);

void joystick_set_hotkeys(joystick *stick, SDL_Scancode key_left, SDL_Scancode key_up, SDL_Scancode key_right, SDL_Scancode key_down);
void joystick_axis(joystick *stick, float x, float y);

void joystick_touch(joystick *stick, float x, float y);
void joystick_release(joystick *stick);

void joystick_place(joystick *stick, float x, float y);

#endif /* JOYSTICK_H_ */
