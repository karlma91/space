/*
 * button.h
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "SDL.h"

typedef void *button;

//TODO add timeout for move and up?
button button_create(float pos_x, float pos_y, float width, float height, int tex_id);
void button_free(button btn);

void button_render(button btn);
void button_clear(button btn);

int button_finger_down(button btn, SDL_TouchFingerEvent *finger);
int button_finger_move(button btn, SDL_TouchFingerEvent *finger);
int button_finger_up(button btn, SDL_TouchFingerEvent *finger);

#endif /* BUTTON_H_ */
