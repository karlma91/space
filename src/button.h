/*
 * button.h
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "SDL.h"

typedef struct {
	SDL_FingerID finger_id;

	int pressed; /* whether if button is currently pressed down */

	float pos_x; /* center x-coordinate of button */
	float pos_y; /* center y-coordinate of button */

	int tex_id; /* texture id */
	float width; /* width of button and touch area */
	float height; /* height og button and touch area */

	float r1x;
	float r2x;
	float r1y;
	float r2y;
} button;

//TODO add timeout for move and up?
void button_init(button *btn, float pos_x, float pos_y, float width, float height, int tex_id);
void button_render(button *btn);
void button_clear(button *btn);

int button_finger_down(button *btn, SDL_TouchFingerEvent *finger);
int button_finger_move(button *btn, SDL_TouchFingerEvent *finger);
int button_finger_up(button *btn, SDL_TouchFingerEvent *finger);

#endif /* BUTTON_H_ */
