/*
 * joystick.h
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

//Todo keep this structure private or not?
typedef struct {
	/* normalized joystick orientation */
	float axis_x; /* [-1, 1] */
	float axis_y; /* [-1, 1] */
	float direction; /* [0, 2pi] */
	float amplitude; /* [0, 1] */

	float pos_x;
	float pos_y;
} joystick;


void joystick_place(joystick *stick, float pos_x, float pos_y);
void joystick_axis(joystick *stick, float x, float y);
void joystick_pos(joystick *stick, float x, float y);


#endif /* JOYSTICK_H_ */
