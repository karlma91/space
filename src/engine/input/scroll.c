/*
 * scroll_p.c
 *
 *  Created on: 14. juli 2013
 *      Author: Mathias
 */
#include "scroll.h"
#include "../engine.h"
#include "../data/llist.h"

#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"


typedef struct {
	touchable touch_data;

	float friction;

	int scrolling;

	float max_speed;

	SDL_FingerID finger_1, finger_2;

	float hs;
	float vs;

	float x_offset;
	float y_offset;

	SDL_Scancode key_left, key_up, key_right, key_down;

} scroll_priv;


static int keypress_down(touchable *scr_id, SDL_Scancode key)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	float f = 0.9*dt;

	if (key == scr->key_left) {
		scr->x_offset += scr->max_speed * f;
		return 1;
	} else if (key == scr->key_up) {
		scr->y_offset -= scr->max_speed * f;
		return 1;
	} else if (key == scr->key_right) {
		scr->x_offset -= scr->max_speed * f;
		return 1;
	} else if (key == scr->key_down) {
		scr->y_offset += scr->max_speed * f;
		return 1;
	}

	return 0;
}

static void update(touchable * scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	if (!scr->scrolling) {
		scr->x_offset += scr->hs;
		scr->y_offset += scr->vs;
	} else {
		scr->hs *= scr->friction;
		scr->vs *= scr->friction;
	}
	scr->hs *= scr->friction;
	scr->vs *= scr->friction;
}

static void render(touchable * scr_id)
{
}

static int touch_down(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	//TODO support dual-touch
	if (INSIDE(scr, tx, ty)) {
		scr->finger_1 = finger->fingerId;

		scr->hs = 0;
		scr->vs = 0;

		llist_add(active_fingers, (void *)finger->fingerId);

		scr->scrolling = 1;
		return 1;
	}

	return 0;
}

static int touch_motion(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	/* add finger to this scroll if not active*/
	if (!llist_contains(active_fingers, finger->fingerId)) {
		scr->finger_1 = finger->fingerId;

		scr->hs = 0;
		scr->vs = 0;

		llist_add(active_fingers, (void *)finger->fingerId);
	} else if (scr->finger_1 != finger->fingerId) {
		return 0;
	}

	scr->scrolling = 1;

	//TODO get actual width and height (TODO ta hensyn til kameraviews)
	float dx = finger->dx * scr_id->get.width;
	float dy = -finger->dy * scr_id->get.height;

	scr->x_offset += dx;
	scr->y_offset += dy;

	scr->hs = scr->hs * 0.5 + 0.5 * dx;
	scr->vs = scr->vs * 0.5 + 0.5 * dy;

	/* limit speed */
	float speed = hypotf(scr->hs, scr->vs);
	if (speed > scr->max_speed * dt) {
		float k = (scr->max_speed * dt) / speed;
		scr->hs = scr->hs*(k * 0.7 + 0.3);
		scr->vs = scr->vs*(k * 0.7 + 0.3);
	}

	return 1;
}

static int touch_up(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	if (scr->finger_1 == finger->fingerId) {
		scroll_priv * scr = (scroll_priv *) scr_id;
		scr->scrolling = 0;
		scr->finger_1 = NULL;
	}
	return 0;
}

void scroll_set_hotkeys(touchable * scr_id, SDL_Scancode key_left, SDL_Scancode key_up, SDL_Scancode key_right, SDL_Scancode key_down)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->key_left = key_left;
	scr->key_up = key_up;
	scr->key_right = key_right;
	scr->key_down = key_down;
}

scroll_p scroll_create(float pos_x, float pos_y, float width, float height, float friction, float max_speed)
{
	scroll_priv *scr = calloc(1, sizeof *scr);

	REGISTER_CALLS(scr);

	scroll_p scr_id = (scroll_p) scr;

	/* default keybindings */
	scroll_set_hotkeys(scr_id, SDL_SCANCODE_LEFT,SDL_SCANCODE_UP,SDL_SCANCODE_RIGHT,SDL_SCANCODE_DOWN);

	scr_id->type = CTRL_SCROLL;

	touch_margin(scr_id, 0);
	touch_place(scr_id, pos_x, pos_y);
	touch_area(scr_id, width, height);

	scr->friction = friction;
	scr->max_speed = max_speed;

	scr->x_offset = 0;
	scr->y_offset = 0;
	scr->hs = 0;
	scr->vs = 0;
	scr->scrolling = 0;

	return scr_id;
}

float scroll_get_xoffset(scroll_p scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	return scr->x_offset;
}

float scroll_get_yoffset(scroll_p scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	return scr->y_offset;
}
