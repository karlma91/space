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

	int active; /* if currently in use */
	float size; /* normalized size, percentage of height or width*/

	float pos_x; /* normalized screen position */
	float pos_y;

	float min_range; /* minimum distance from center of joystick before responding */
} joystick;

#define JOYSTICK_DEFAULT {0, 0, 0, 0, 0, 0.07f, -1, -1, 0.3}

void joystick_place(joystick *stick, float pos_x, float pos_y);
void joystick_axis(joystick *stick, float x, float y);

void joystick_touch(joystick *stick, float x, float y);
void joystick_release(joystick *stick);


#endif /* JOYSTICK_H_ */
