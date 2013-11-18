/*
 * scroll_p.c
 *
 *  Created on: 14. juli 2013
 *      Author: Mathias
 */

#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"

#include "scroll.h"
#include "../state/statesystem.h"
#include "../engine.h"
#include "../data/llist.h"


typedef struct {
	touchable touch_data;

	int have_bounds;
	cpBB bounds;

	float friction;
	int scrolling;
	float max_speed;

	float zoom, z_min, z_max;
	float rot;
	float rotate_delta;
	float zoom_delta;

	/* external touch_down, touch_motion, and touch_up should return 1 if touch is consumed */
	pre_touch_callback pre_touch_down;
	pre_touch_callback pre_touch_motion;
	pre_touch_callback pre_touch_up;

	touch_unique_id touch_1, touch_2;

	cpVect pf1, pf2;

	cpVect speed;
	cpVect offset;

	SDL_Scancode key_left, key_up, key_right, key_down, key_in, key_out, key_rotcw, key_rotcc;

	int consume_events;

} scroll_priv;


static int keypress_down(touchable *scr_id, SDL_Scancode key)
{
	return 0;
}

static void update(touchable * scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	float zoom = 1;
	if (scr->touch_data.container) {
		zoom = scr->touch_data.container->zoom;
	}
	float spd = scr->max_speed * 0.8 * dt;
	//TODO check if mouse is inside touch area
	cpVect delta = {(keys[scr->key_left] - keys[scr->key_right]) * spd,
					(keys[scr->key_down] - keys[scr->key_up]) * spd};

	if(keys[scr->key_in]){
		scr->zoom *= 1 + 1 * dt;
	} else if(keys[scr->key_out]) {
		scr->zoom *= 1/(1 + 1 * dt);
	}

	if(keys[scr->key_rotcc]){
		scr->rot += WE_2PI * dt / 4;
	} else if(keys[scr->key_rotcw]) {
		scr->rot -= WE_2PI * dt / 4;
	}
	if (scr->zoom_delta) {
		delta = cpvmult(delta, 1  / zoom);
	}
	if (scr->rotate_delta) {
		delta = cpvrotate(delta, cpvforangle(-scr->rot));
	}
	scr->offset = cpvadd(scr->offset, delta);

	if (!scr->scrolling) {
		scr->offset = cpvadd(scr->offset, scr->speed);
	} else {
		scr->speed = cpvmult(scr->speed, scr->friction * 0.9);
	}
	scr->speed = cpvmult(scr->speed, scr->friction);
	if (scr->have_bounds) {
		//scr->offset = cpvadd(cpvmult(cpBBClampVect(scr->bounds, scr->offset),0.2),cpvmult(scr->offset,0.8));
		scr->offset = cpBBClampVect(scr->bounds, scr->offset);
	}

}

static void render(touchable * scr_id)
{
	extern int debug_draw;
	if (debug_draw) {
		scroll_priv * scr = (scroll_priv *) scr_id;
		draw_color4b(20,40,20,10);
		draw_box(0, cpv(scr->touch_data.get.t1x,scr->touch_data.get.t1y),cpv(scr->touch_data.get.width, scr->touch_data.get.height),0,0);
		draw_color4b(255,0,0,128);
		draw_box(0, scr->offset, cpv(10,10),0,1);
	}
}

#define FINGER_1 0x1  //(1<<0)
#define FINGER_2 0x2  //(1<<1)
#define FINGER_ID 0x4 //(1<<2)

static int touch_down(touchable * scr_id, SDL_TouchFingerEvent *finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	float tx = finger->x, ty = finger->y;
	//normalized2game(&tx, &ty);

	if (INSIDE(scr, tx, ty)) {
		int id = finger_status(scr->touch_1, finger->fingerId) | (finger_status(scr->touch_2, finger->fingerId) << 1);
		int active = finger_status(scr->touch_1, -1) | (finger_status(scr->touch_2, -1) << 1);
		active |= id ? FINGER_ID << 2 : (finger_active(finger->fingerId) << 2);

		if (active & FINGER_ID) {
			return 0;
		}

		if (scr->pre_touch_down && scr->pre_touch_down(finger)) return 1;

		if (active & FINGER_1) {
			if (!(id & FINGER_1) && !(active & FINGER_2)) {
				scr->touch_2 = finger_bind(finger->fingerId);
				if (scr->touch_2 != -1) {
					scr->pf2 = cpv(finger->x,finger->y);
					scr->speed = cpvzero;
					return scr->consume_events;
				}
			}
		} else {
			scr->touch_1 = finger_bind(finger->fingerId);
			if (scr->touch_1 != -1) {
				scr->pf1 = cpv(finger->x,finger->y);
				scr->speed = cpvzero;
				scr->scrolling = 1;
				return scr->consume_events;
			}
		}
	}

	return 0;
}

static int touch_motion(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv *scr = (scroll_priv *) scr_id;

	int id = finger_status(scr->touch_1, finger->fingerId) | (finger_status(scr->touch_2, finger->fingerId) << 1);
	int active = finger_status(scr->touch_1, -1) | (finger_status(scr->touch_2, -1) << 1);
	active |= id ? FINGER_ID : (finger_active(finger->fingerId) << 2);

	if (INSIDE(scr, finger->x, finger->y)) {
		if ((active & FINGER_ID) == 0) {
			if (scr->pre_touch_motion && scr->pre_touch_motion(finger)) return 1;
		}

		if (active & FINGER_1) {
			cpVect last_diff = cpvsub(scr->pf2, scr->pf1);
			cpVect last_zcenter = cpvmult(cpvadd(scr->pf1, scr->pf2), 0.5);
			cpVect dim = cpv(scr_id->get.width, scr_id->get.height);
			float last_dist, new_dist;
			float add_rot = 0;

			if (id & FINGER_1) {
				scr->pf1 = cpv(finger->x,finger->y);
			} else if (id & FINGER_2) {
				scr->pf2 = cpv(finger->x,finger->y);
			} else {
				if (active & FINGER_ID) {
					return 0;
				} else if (!(active & FINGER_2)) {
					scr->touch_2 = finger_bind(finger->fingerId);
					scr->pf2 = cpv(finger->x,finger->y);
					scr->speed = cpvzero;
				}
			}

			cpVect new_diff = cpvsub(scr->pf2, scr->pf1);
			if (active & FINGER_2) {
				last_dist = cpvlength(last_diff);
				new_dist = cpvlength(new_diff);
				float product = last_dist * new_dist;
				if (product > 0) {
					float cos_alpha = cpvdot(last_diff, new_diff) / (product);
					float sin_alpha = cpvdot(cpvperp(last_diff), new_diff) / (product);
					cos_alpha = cos_alpha > 1 ? 1 : cos_alpha < -1 ? -1 : cos_alpha;
					add_rot = acosf(cos_alpha) * (sin_alpha > 0 ? 1 : -1);
				}
			} else {
				last_dist = 1;
				new_dist = 1;
			}

			if (active & FINGER_ID) {
				//float last_zoom = scr->zoom;
				scr->zoom = scr->zoom * new_dist / last_dist;
				if (scr->zoom < scr->z_min) scr->zoom = scr->z_min;
				if (scr->zoom > scr->z_max) scr->zoom = scr->z_max;
				scr->rot += add_rot * scr->rotate_delta;
				//return 1;
				float zoom = 1;
				if (scr->touch_data.container) {
					zoom = scr->touch_data.container->zoom;
				}
				//TODO get actual width and height (TODO ta hensyn til kameraviews)
				//cpVect delta = cpv(finger->dx*scr_id->get.width, -finger->dy*scr_id->get.height);
				cpVect delta = cpv(finger->dx*dim.x, -finger->dy*dim.y);
				if (scr->zoom_delta) {
					//TODO translate delta for correct in-place zoom
					if (active & FINGER_2) {
						cpVect zcenter = cpvmult(cpvadd(scr->pf1, scr->pf2), 0.5);
						delta = cpvsub(zcenter, last_zcenter);
					}
					delta = cpvmult(delta, 1  / zoom);
				}
				if (scr->rotate_delta) {
					delta = cpvrotate(delta, cpvforangle(-scr->rot));
				}
				scr->offset = cpvadd(scr->offset, delta);
				scr->speed = cpvadd(cpvmult(scr->speed, 0.5), cpvmult(delta, 0.5));

				if (scr->have_bounds) {
					//scr->offset = cpvadd(cpvmult(cpBBClampVect(scr->bounds, scr->offset),0.2),cpvmult(scr->offset,0.8));
					scr->offset = cpBBClampVect(scr->bounds, scr->offset);
				}
				return scr->consume_events;
			}
		} else { /* finger_1 inactive */
			if (!(active & FINGER_ID)) {  /* finger available */
				scr->touch_1 = finger_bind(finger->fingerId);
				scr->pf1 = cpv(finger->x,finger->y);
				scr->speed = cpvzero;
				scr->scrolling = 1;
				return scr->consume_events;
			} else if (id & FINGER_2) { /* transfer finger_2 to finger_1 */
				scr->touch_1 = scr->touch_2;
				scr->touch_2 = -1;
				scr->pf1 = scr->pf2;
				return scr->consume_events;
			}
		}
	} else { /* remove finger if not inside scroll */
		if (id && (active & FINGER_ID)) {
			if (id & FINGER_1) {
				scr->touch_1 = scr->touch_2;
				scr->pf1 = scr->pf2;
				if ((id & FINGER_2) == 0) scr->scrolling = 0;
			}
			finger_unbind(scr->touch_2);
			scr->touch_2 = -1;
		}
	}
	return 0;
}

static int touch_up(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_priv * scr = (scroll_priv *) scr_id;

	int id = finger_status(scr->touch_1, finger->fingerId) | (finger_status(scr->touch_2, finger->fingerId) << 1);
	int active = finger_status(scr->touch_1, -1) | (finger_status(scr->touch_2, -1) << 1);
	active |= id ? FINGER_ID : (finger_active(finger->fingerId) << 2);

	if (id && (active & FINGER_ID)) {
		if (id & FINGER_1) {
			touch_unique_id  last_id = scr->touch_1;
			scr->touch_1 = scr->touch_2;
			scr->touch_2 = last_id != scr->touch_2 ? last_id : -1; // avoids use of the same touch_id
			scr->pf1 = scr->pf2;
		}
		if ((id & FINGER_2) == 0) scr->scrolling = 0;
		scr->touch_2 = -1;
		return scr->consume_events;
	} else {
		if (scr->pre_touch_up && scr->pre_touch_up(finger)) return 1;
	}

	return 0;
}

void scroll_set_hotkeys(touchable * scr_id, SDL_Scancode key_left, SDL_Scancode key_up, SDL_Scancode key_right, SDL_Scancode key_down, SDL_Scancode key_zoomin, SDL_Scancode key_zoomout, SDL_Scancode key_rot_cw, SDL_Scancode key_rot_cc)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->key_left = key_left;
	scr->key_up = key_up;
	scr->key_right = key_right;
	scr->key_down = key_down;
	scr->key_in = key_zoomin;
	scr->key_out = key_zoomout;
	scr->key_rotcw = key_rot_cw;
	scr->key_rotcc = key_rot_cc;
}

scroll_p scroll_create(float pos_x, float pos_y, float width, float height, float friction, float max_speed, int rotate_delta, int zoom_delta, int consume_events)
{
	scroll_priv *scr = calloc(1, sizeof *scr);

	REGISTER_CALLS(scr);

	scroll_p scr_id = (scroll_p) scr;

	/* default keybindings */
	scroll_set_hotkeys(scr_id, SDL_SCANCODE_LEFT, SDL_SCANCODE_UP,
			SDL_SCANCODE_RIGHT, SDL_SCANCODE_DOWN, SDL_SCANCODE_PAGEUP,
			SDL_SCANCODE_PAGEDOWN, SDL_SCANCODE_END, SDL_SCANCODE_HOME);

	scr_id->type = CTRL_SCROLL;

	touch_margin(scr_id, 0);
	touch_place(scr_id, pos_x, pos_y);
	touch_area(scr_id, width, height);

	scr->friction = friction;
	scr->max_speed = max_speed;

	scr->offset = cpvzero;
	scr->speed = cpvzero;
	scr->scrolling = 0;

	scr->rotate_delta = rotate_delta;
	scr->rot = 0;

	scr->zoom = 1;
	scr->z_min= 0.01;
	scr->z_max= 1000;
	scr->zoom_delta = zoom_delta;

	scr->touch_1 = -1;
	scr->touch_2 = -1;

	scr->consume_events = consume_events;
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

void scroll_set_callback(scroll_p scr_id, pre_touch_callback touch_down, pre_touch_callback touch_motion, pre_touch_callback touch_up)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	scr->pre_touch_down = touch_down;
	scr->pre_touch_motion = touch_motion;
	scr->pre_touch_up = touch_up;
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

float scroll_get_rotation(scroll_p scr_id)
{
	scroll_priv * scr = (scroll_priv *) scr_id;
	return scr->rot;
}
