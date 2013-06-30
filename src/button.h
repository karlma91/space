/*
 * button.h
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "texture.h"

typedef struct {
	int pressed;

	float pos_x;
	float pos_y;

	float width;
	float height;

	int tex_id;
} button;

int button_push(button *btn);
int button_release(button *btn);

#endif /* BUTTON_H_ */
