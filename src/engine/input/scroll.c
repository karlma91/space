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

	float zoom, z_min, z_max;

	SDL_FingerID finger_1, finger_2;

	cpVect pf1, pf2;

	cpVect speed;
	cpVect offset;

	SDL_Scancode key_left, key_up, key_right, key_down, key_in, key_out;

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

	if(keys[scr->key_in]){
		scr->zoom *= 1 + 1 * dt;
	} else if(keys[scr->key_out]) {
		scr->zoom *= 1/(1 + 1 * dt);
	}

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
	scroll_priv * scr = (scroll_priv *) scr_id;
	draw_color4b(50,80,50,20);
	draw_box(0, cpv(scr->touch_data.get.t1x,scr->touch_data.get.t1y),cpv(scr->touch_data.get.width, scr->touch_data.get.height),0,0);
}

static int touch_down(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	float tx = finger->x, ty = finger->y;
	//normalized2game(&tx, &ty);

	//TODO support dual-touch
	if (INSIDE(scr, tx, ty)) {
		if (llist_contains(active_fingers, scr->finger_1)) {
			if (scr->finger_1 != finger->fingerId) {
				scr->finger_2 = finger->fingerId;
				scr->pf2 = cpv(finger->x,finger->y);
				scr->speed = cpvzero;
				llist_add(active_fingers, (void *)finger->fingerId);
				return 1;
			}
		} else {
			scr->finger_1 = finger->fingerId;
			scr->pf1 = cpv(finger->x,finger->y);
			scr->speed = cpvzero;
			llist_add(active_fingers, (void *)finger->fingerId);
			scr->scrolling = 1;
			//return 1; //TMP FIXME
		}
	}

	return 0;
}

static int touch_motion(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	/* add finger to this scroll if not active*/
	/*if (!llist_contains(active_fingers, finger->fingerId)) {
		scr->finger_1 = finger->fingerId;
		scr->speed = cpvzero;

		llist_add(active_fingers, (void *)finger->fingerId);
	} else*/

	if (llist_contains(active_fingers, scr->finger_1)) {
        float last_dist = cpvdist(scr->pf1, scr->pf2);
        float new_dist = last_dist;

		if (scr->finger_1 == finger->fingerId) {
			scr->pf1 = cpv(finger->x,finger->y);
		} else if (scr->finger_2 == finger->fingerId) {
			scr->pf2 = cpv(finger->x,finger->y);
		}

		if (llist_contains(active_fingers, scr->finger_2)) {
			new_dist = cpvdist(scr->pf1, scr->pf2);
		}

		if (llist_contains(active_fingers, finger->fingerId)) {
			scr->zoom = scr->zoom * new_dist / last_dist;
			if (scr->zoom < scr->z_min) scr->zoom = scr->z_min;
			if (scr->zoom > scr->z_max) scr->zoom = scr->z_max;
			//return 1;
			scr->scrolling = 1;
			//TODO get actual width and height (TODO ta hensyn til kameraviews)
			float zoom = current_view->zoom;
#warning current_view skal ikke være tilgjengelig i scroll motion!
			cpVect delta = cpv(-finger->dx*scr_id->get.width / zoom, finger->dy*scr_id->get.height / zoom);

			scr->offset = cpvadd(scr->offset, delta);
			scr->speed = cpvadd(cpvmult(scr->speed, 0.5), cpvmult(delta, 0.5));

			return 1;
		}
	}
	return 0;
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

void scroll_set_hotkeys(touchable * scr_id, SDL_Scancode key_left, SDL_Scancode key_up, SDL_Scancode key_right, SDL_Scancode key_down, SDL_Scancode key_zoomin, SDL_Scancode key_zoomout)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->key_left = key_left;
	scr->key_up = key_up;
	scr->key_right = key_right;
	scr->key_down = key_down;
	scr->key_in = key_zoomin;
	scr->key_out = key_zoomout;
}

scroll_p scroll_create(float pos_x, float pos_y, float width, float height, float friction, float max_speed)
{
	scroll_priv *scr = calloc(1, sizeof *scr);

	REGISTER_CALLS(scr);

	scroll_p scr_id = (scroll_p) scr;

	/* default keybindings */
	scroll_set_hotkeys(scr_id, SDL_SCANCODE_LEFT,SDL_SCANCODE_UP,SDL_SCANCODE_RIGHT,SDL_SCANCODE_DOWN,SDL_SCANCODE_PAGEUP,SDL_SCANCODE_PAGEDOWN);

	scr_id->type = CTRL_SCROLL;

	touch_margin(scr_id, 0);
	touch_place(scr_id, pos_x, pos_y);
	touch_area(scr_id, width, height);

	scr->friction = friction;
	scr->max_speed = max_speed;

	scr->offset = cpvzero;
	scr->speed = cpvzero;
	scr->scrolling = 0;

	scr->zoom = 1;
	scr->z_min= 0.01;
	scr->z_max= 1000;

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

void scroll_set_zoomlimit(scroll_p scr_id, float min, float max)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->z_min = min;
	scr->z_max = max;
}

float scroll_get_zoom(scroll_p scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	return scr->zoom;
}

void scroll_set_zoom(scroll_p scr_id, float zoom)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->zoom = zoom;
}
