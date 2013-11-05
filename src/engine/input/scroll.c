/*
 * scroll_p.c
 *
 *  Created on: 14. juli 2013
 *      Author: Mathias
 */
#include "scroll.h"
#include "../state/statesystem.h"
#include "../engine.h"
#include "../data/llist.h"

#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"


typedef struct {
	touchable touch_data;

	int have_bounds;
	cpBB bounds;

	float friction;
	int scrolling;
	float max_speed;

	SDL_FingerID finger_1, finger_2;

	cpVect speed;
	cpVect offset;

	SDL_Scancode key_left, key_up, key_right, key_down;

} scroll_priv;


static int keypress_down(touchable *scr_id, SDL_Scancode key)
{
	return 0;
}

static void update(touchable * scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	float spd = scr->max_speed * 0.8 * dt;

	scr->offset.x +=  (keys[scr->key_left] - keys[scr->key_right]) * spd;
	scr->offset.y +=  (keys[scr->key_down] - keys[scr->key_up]) * spd;

	if (!scr->scrolling) {
		scr->offset = cpvadd(scr->offset, scr->speed);
	} else {
		scr->speed = cpvmult(scr->speed, scr->friction);
	}
	scr->speed = cpvmult(scr->speed, scr->friction);
	if (scr->have_bounds) {
		scr->offset = cpBBClampVect(scr->bounds, scr->offset);
	}

}

static void render(touchable * scr_id)
{
}

static int touch_down(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	float tx = finger->x, ty = finger->y;
	//normalized2game(&tx, &ty);

	//TODO support dual-touch
	if (INSIDE(scr, tx, ty)) {
		scr->finger_1 = finger->fingerId;

		scr->speed = cpvzero;

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

		scr->speed = cpvzero;

		llist_add(active_fingers, (void *)finger->fingerId);
	} else if (scr->finger_1 != finger->fingerId) {
		return 0;
	}

	scr->scrolling = 1;
	//TODO get actual width and height (TODO ta hensyn til kameraviews)
	float zoom = current_view->zoom;
	cpVect delta = cpv(-finger->dx*scr_id->get.width / zoom, finger->dy*scr_id->get.height / zoom);

	scr->offset = cpvadd(scr->offset, delta);
	scr->speed = cpvadd(cpvmult(scr->speed, 0.5), cpvmult(delta, 0.5));

	return 1;
}

static int touch_up(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	if (scr->finger_1 == finger->fingerId) {
		scroll_priv * scr = (scroll_priv *) scr_id;
		scr->scrolling = 0;
		scr->finger_1 = -1;
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

	scr->offset = cpvzero;
	scr->speed = cpvzero;
	scr->scrolling = 0;

	return scr_id;
}

float scroll_get_xoffset(scroll_p scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	return scr->offset.x;
}

float scroll_get_yoffset(scroll_p scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	return scr->offset.y;
}

cpVect scroll_get_offset(scroll_p scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	return scr->offset;
}

void scroll_set_offset(scroll_p scr_id, cpVect offset)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->offset = offset;
}

void scroll_set_bounds(scroll_p scr_id, cpBB bounds)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->have_bounds = 1;
	scr->bounds = bounds;
}
