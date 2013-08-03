/*
 * joystick.c
 *
 *  Created on: 23. juni 2013
 *      Author: Mathias
 */

#include <math.h>

#include "joystick.h"
#include "../graphics/draw.h"
#include "../data/llist.h"
#include "../engine.h"

#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"

//FIXME one controller cancels the other

joystick *joystick_create(int persistent, float radius, float min_radius, float center_x, float center_y, float width, float height, SPRITE_ID spr_id)
{
	joystick *stick = calloc(1, sizeof *stick);

	REGISTER_CALLS(stick);

	stick->touch_data.type = CTRL_JOYSTICK;

	stick->touch_data.get.width = width;
	stick->touch_data.get.height = height;

	stick->persistent = persistent;
	stick->radius = radius;
	stick->min_range = min_radius;

	touch_place((touchable *) stick, center_x, center_y);

	joystick_axis(stick,0,0);
	joystick_release(stick);

	joystick_place(stick, center_x, center_y);

	sprite_create(&(stick->spr), spr_id, radius*2, radius*2, 0);

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
	float min_x = stick->touch_data.get.t1x + stick->radius;
	float max_x = stick->touch_data.get.t2x - stick->radius;
	float min_y = stick->touch_data.get.t1y + stick->radius;
	float max_y = stick->touch_data.get.t2y - stick->radius;

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

static void update(touchable * stick_id)
{
	joystick *stick = (joystick *) stick_id;

}

static void render(touchable * stick_id)
{
	joystick *stick = (joystick *) stick_id;

	cpVect btn_pos = {stick->pos_x,stick->pos_y};

	draw_color4f(1,1,1,1);

	sprite_set_index_normalized(&(stick->spr), stick->amplitude);
	sprite_render(&(stick->spr), &btn_pos, stick->direction * 180 / M_PI - 90);

	//draw_flush_simple();
}

static int touch_down(touchable * stick_id, SDL_TouchFingerEvent *finger)
{
	joystick *stick = (joystick *) stick_id;

	if (llist_contains(active_fingers, (void *)finger->fingerId)) {
		return 0; // fingerId already registered by another object
	}

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	//TODO decide whether to allow new finger to ++capture++ active finger or not
	if (touch_is_inside(stick_id, tx, ty)) {
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

static int touch_motion(touchable * stick_id, SDL_TouchFingerEvent *finger)
{
	joystick *stick = (joystick *) stick_id;

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (!stick->pressed) {
		if (llist_contains(active_fingers, (void *)finger->fingerId)) {
			return 0;
		} else { // fingerId available
			if (touch_is_inside(stick_id, tx, ty)) {
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
	if (!touch_is_inside(stick_id, tx,ty)) {
		//joystick_release(stick);
		//return 0;
	}

	joystick_touch(stick, tx, ty);

	return 1;
}

static int touch_up(touchable * stick_id, SDL_TouchFingerEvent *finger)
{
	joystick *stick = (joystick *) stick_id;

	if (!stick->pressed || stick->finger_id != finger->fingerId)
		return 0;

	if (!llist_contains(active_fingers, (void *)finger->fingerId)) {
		return 0; // finger not registered as active
	}

	joystick_release(stick);

	return 0;
}

