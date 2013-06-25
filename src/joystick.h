/*
 * joystick.h
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

typedef struct {
	/* normalized joystick orientation */
	float axis_x; /* [-1, 1] */
	float axis_y; /* [-1, 1] */
	float direction; /* [0, 2pi] */
	float amplitude; /* [0, 1] */

	int active;
	float size; /* normalized size */

	float pos_x;
	float pos_y;
} joystick;

#define JOYSTICK_DEFAULT {0, 0, 0, 0, 0, 0.1f, -1, -1}

void joystick_place(joystick *stick, float pos_x, float pos_y);
void joystick_axis(joystick *stick, float x, float y);

void joystick_touch(joystick *stick, float x, float y);
void joystick_release(joystick *stick);


#endif /* JOYSTICK_H_ */
