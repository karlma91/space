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

	float hs;
	float vs;

	float x_offset;
	float y_offset;

} scroll_p_priv;

static void update(touchable * scr_id)
{
	scroll_p_priv * scr = (scroll_p_priv *) scr_id;

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
	scroll_p_priv * scr = (scroll_p_priv *) scr_id;

	return 0;
}

static int touch_motion(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_p_priv * scr = (scroll_p_priv *) scr_id;


	//TODO register finger to scroller
	if (!llist_contains(active_fingers, finger->fingerId)) {
		scr->scrolling = 1;

		//TODO get actual width and height (TODO ta hensyn til kameraviews)
		float dx = finger->dx * scr_id->get.width;
		float dy = -finger->dy * scr_id->get.height;

		scr->x_offset += dx;
		scr->y_offset += dy;

		/* limit speed */

		scr->hs = scr->hs*0.5 + 0.5*dx;
		scr->vs = scr->vs*0.5 + 0.5*dy;

		float speed = hypotf(scr->hs, scr->vs);
		if (speed > scr->max_speed*dt) {
			float k = (scr->max_speed*dt) / speed;
			scr->hs *= k;
			scr->vs *= k;
		}
	}
	return 0;
}

static int touch_up(touchable * scr_id, SDL_TouchFingerEvent * finger)
{
	scroll_p_priv * scr = (scroll_p_priv *) scr_id;
	scr->scrolling = 0;
	return 0;
}


scroll_p scroll_create(float pos_x, float pos_y, float width, float height, float friction, float max_speed)
{
	scroll_p_priv *scr = malloc(sizeof(*scr));
	scroll_p scr_id = (scroll_p) scr;
	REGISTER_CALLS(scr);

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
	scroll_p_priv * scr = (scroll_p_priv *) scr_id;
	return scr->x_offset;
}

float scroll_get_yoffset(scroll_p scr_id)
{
	scroll_p_priv * scr = (scroll_p_priv *) scr_id;
	return scr->y_offset;
}
