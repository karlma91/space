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
#include "../graphics/sprite.h"
#include "we_utils.h"

#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"

//FIXME one controller cancels the other

static int keypress_down(touchable *touch_id, SDL_Scancode key)
{
	// doesn't work for joysticks
	return 0;
}

joystick *joystick_create(int persistent, float radius, float min_radius, float center_x, float center_y, float width, float height, SPRITE_ID spr_back, SPRITE_ID spr_front)
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

	joystick_place(stick, center_x, center_y + (height/2.1 + radius) * (center_y >= GAME_HEIGHT/4 ? 1 : -1));
	stick->draw_x = stick->pos_x;
	stick->draw_y = stick->pos_y;

	sprite_create(&(stick->spr_back), spr_back, radius*2, radius*2, 0);
	sprite_create(&(stick->spr_front), spr_front, radius, radius*2, 0);

	return stick;
}

void joystick_reposition(joystick *stick, float radius, float min_radius, float center_x, float center_y, float width, float height)
{
	stick->touch_data.type = CTRL_JOYSTICK;

	stick->touch_data.get.width = width;
	stick->touch_data.get.height = height;

	stick->radius = radius;
	stick->min_range = min_radius;

	touch_place((touchable *) stick, center_x, center_y);

	joystick_axis(stick,0,0);
	joystick_release(stick);

	joystick_place(stick, center_x, center_y + (height/2.1 + radius) * (center_y >= GAME_HEIGHT/4 ? 1 : -1));
	stick->draw_x = stick->pos_x;
	stick->draw_y = stick->pos_y;

	sprite_set_size(&(stick->spr_back), radius*2, radius*2);
	sprite_set_size(&(stick->spr_front), radius, radius*2);
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

void joystick_set_hotkeys(joystick *stick, SDL_Scancode key_left, SDL_Scancode key_up, SDL_Scancode key_right, SDL_Scancode key_down)
{
	stick->key_left = key_left;
	stick->key_up = key_up;
	stick->key_right = key_right;
	stick->key_down = key_down;
}

void joystick_axis(joystick *stick, float x, float y)
{
	// Rotate joystick to match viewport orientation
	if (stick->touch_data.container) {
		float x_ = x, y_ = y;
		switch(stick->touch_data.container->port_orientation) {
			case 0: x =  x_; y =  y_; break;
			case 1: x = -y_; y =  x_; break;
			case 2: x = -x_; y = -y_; break;
			case 3: x =  y_; y = -x_; break;
		}
	}

	stick->axis_x = x < -1 ? -1 : (x > 1 ? 1 : x);
	stick->axis_y = y < -1 ? -1 : (y > 1 ? 1 : y);

	float dir = (x || y) ? atan2f(y,x) : 0;
	stick->direction = dir < 0 ? dir + 2*M_PI : dir;
	stick->amplitude = hypotf(y,x);
}

#define STCK_FRCT 0.7
#define PULL_FRCT 0.9
static void update(touchable * stick_id)
{
	joystick *stick = (joystick *) stick_id;

	if (!stick->pressed) {
		int axis_x = (stick->key_right && keys[stick->key_right]) - (stick->key_left && keys[stick->key_left]);
		int axis_y = (stick->key_up && keys[stick->key_up]) - (stick->key_down && keys[stick->key_down]);
		joystick_axis(stick, axis_x, axis_y);
	}

	float x = stick->draw_axx * STCK_FRCT + stick->axis_x * (1-STCK_FRCT);
	stick->draw_axx = x;

	float y = stick->draw_axy * STCK_FRCT + stick->axis_y * (1-STCK_FRCT);
	stick->draw_axy = y;

	float dir = (x || y) ? atan2f(y,x) : 0;
	stick->draw_dir = dir < 0 ? dir + 2*M_PI : dir;
	stick->draw_amp = hypotf(y,x);

	stick->draw_x = stick->draw_x * PULL_FRCT + stick->pos_x * (1-PULL_FRCT);
	stick->draw_y = stick->draw_y * PULL_FRCT + stick->pos_y * (1-PULL_FRCT);
}

static void render(touchable * stick_id)
{
	joystick *stick = (joystick *) stick_id;

	cpVect btn_pos = {stick->draw_x,stick->draw_y};

	draw_color4f(1,1,1,1);
	sprite_render(&(stick->spr_back), btn_pos, 0);
	sprite_set_index_normalized(&(stick->spr_front), stick->draw_amp);

	// Counter viewport orientation
	float a = stick->draw_dir - WE_PI_2;
	if (stick->touch_data.container) {
		a -= stick->touch_data.container->priv_port_angle;
	}

	sprite_render(&(stick->spr_front), btn_pos, a);
	//draw_flush_simple();
}

static int touch_down(touchable * stick_id, SDL_TouchFingerEvent *finger)
{
	joystick *stick = (joystick *) stick_id;

	if (llist_contains(active_fingers, (void *)finger->fingerId)) {
		return 0; // fingerId already registered by another object
	}

	float tx = finger->x, ty = finger->y;
	//normalized2game(&tx, &ty);

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
	//normalized2game(&tx, &ty);

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

