/*
 * joystick.c
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#include "joystick.h"
#include "main.h"
#include "math.h"
#include "draw.h"

//FIXME one controller cancels the other

#define inside(stick,px,py) ((px >= stick->region_x1) && (px <= stick->region_x2) && (py >= stick->region_y1) && (py <= stick->region_y2))

joystick *joystick_create(int persistent, float radius, float min_radius, float region_x, float region_y, float region_width, float region_height)
{
	joystick *stick = malloc(sizeof(*stick));

	stick->persistent = persistent;
	stick->radius = radius;
	stick->min_range = min_radius;

	stick->region_x1 = region_x;
	stick->region_x2 = region_x + region_width;
	stick->region_y1 = region_y;
	stick->region_y2 = region_y + region_height;

	joystick_axis(stick,0,0);
	joystick_release(stick);

	return stick;
}

void joystick_free(joystick *stick)
{
	free(stick);
}

void joystick_touch(joystick *stick, float pos_x, float pos_y)
{
	if (!stick->pressed) {
		stick->pressed = 1;
		joystick_place(stick, pos_x, pos_y);
		joystick_axis(stick, 0, 0);
	} else {
		float dx = (pos_x - stick->pos_x) / stick->radius;
		float dy = (pos_y - stick->pos_y) / stick->radius;

		float length = hypotf(dx, dy);

		if (length * stick->radius > stick->min_range) {
			if (length > 1) {
				dx /= length;
				dy /= length;

				joystick_place(stick, pos_x - stick->radius * dx, pos_y - stick->radius * dy);
			}
		}
		joystick_axis(stick, dx, dy);
	}
}

void joystick_release(joystick *stick)
{
	stick->pressed = 0;

	llist_remove(active_fingers, stick->finger_id);
	stick->finger_id = 0;

	if (!stick->persistent) {
		joystick_axis(stick, 0, 0);
	}
}

void joystick_place(joystick *stick, float pos_x, float pos_y)
{
	float min_x = stick->region_x1 + stick->radius;
	float max_x = stick->region_x2 - stick->radius;
	float min_y = stick->region_y1 + stick->radius;
	float max_y = stick->region_y2 - stick->radius;

	stick->pos_x = pos_x > max_x ? max_x : pos_x < min_x ? min_x : pos_x;
	stick->pos_y = pos_y > max_y ? max_y : pos_y < min_y ? min_y : pos_y;
}

void joystick_axis(joystick *stick, float x, float y)
{
	stick->axis_x = x < -1 ? -1 : (x > 1 ? 1 : x);
	stick->axis_y = y < -1 ? -1 : (y > 1 ? 1 : y);

	float dir = (x || y) ? atan2f(y,x) : 0;
	stick->direction = dir < 0 ? dir + 2*M_PI : dir;
	stick->amplitude = hypotf(y,x);
}

void joystick_render(joystick *stick)
{
	float x0 = stick->pos_x;
	float y0 = stick->pos_y;
	float x1 = x0 + stick->axis_x * stick->radius;
	float y1 = y0 + stick->axis_y * stick->radius;
	//if (x0 != x1 && y0 != y1)
		draw_quad_line(x0,y0,x1,y1, 20);
}

int joystick_finger_down(joystick *stick, SDL_TouchFingerEvent *finger)
{
	if (llist_contains(active_fingers, (void *)finger->fingerId)) {
		return 0; // fingerId already registered by another object
	}

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	//TODO decide whether to allow new finger to ++capture++ active finger or not
	if (inside(stick, tx, ty)) {
		stick->pressed = 1;
		llist_remove(active_fingers, (void *)stick->finger_id);
		stick->finger_id = finger->fingerId;
		joystick_place(stick, tx, ty);
		joystick_axis(stick, 0, 0);

		llist_add(active_fingers, (void *)finger->fingerId);

		return 1;
	}

	return 0;
}

int joystick_finger_move(joystick *stick, SDL_TouchFingerEvent *finger)
{
	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (!stick->pressed) {
		if (llist_contains(active_fingers, (void *)finger->fingerId)) {
			return 0;
		} else { // fingerId available
			if (inside(stick, tx, ty)) {
				stick->pressed = 1;
				llist_remove(active_fingers, (void *)stick->finger_id);
				stick->finger_id = finger->fingerId;
				llist_add(active_fingers, (void *)finger->fingerId);
			} else {
				return 0;
			}
		}
	} else if (stick->finger_id != finger->fingerId) {
		return 0;
	}

	//TODO decide whether if movement outside region should release joystick or ++not++
	if (!inside(stick, tx,ty)) {
		//joystick_release(stick);
		//return 0;
	}

	joystick_touch(stick, tx, ty);

	return 1;
}

int joystick_finger_up(joystick *stick, SDL_TouchFingerEvent *finger)
{
	if (!stick->pressed || stick->finger_id != finger->fingerId)
		return 0;

	if (!llist_contains(active_fingers, (void *)finger->fingerId)) {
		return 0; // finger not registered as active
	}

	joystick_release(stick);

	return 0;
}

