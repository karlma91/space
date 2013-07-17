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

	float scroll_hs;
	float scroll_vs;

	float x_offset;
	float y_offset;

} scroll_p_priv;

static void update(touchable * scr_id)
{
	scroll_p_priv * scr = (scroll_p_priv *) scr_id;

	if (!scr->scrolling) {
		scr->x_offset += scr->scroll_hs;
		scr->y_offset += scr->scroll_vs;
	} else {
		scr->scroll_hs *= scr->friction;
		scr->scroll_vs *= scr->friction;
	}
	scr->scroll_hs *= scr->friction;
	scr->scroll_vs *= scr->friction;
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
		float dx = finger->dx*scr_id->get.width;
		float dy = -finger->dy*scr_id->get.height;

		scr->x_offset += dx;
		scr->y_offset += dy;

		/* limit speed */

		scr->scroll_hs = scr->scroll_hs*0.5 + 0.5*dx;
		scr->scroll_vs = scr->scroll_vs*0.5 + 0.5*dy;

		float speed = hypotf(scr->scroll_hs, scr->scroll_vs);
		if (speed > scr->max_speed * dt) {
			float k = scr->max_speed / speed;
			scr->scroll_hs *= k;
			scr->scroll_vs *= k;
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
	REGISTER_CALLS(scr);

	touch_margin(scr, 0);
	touch_place(scr, pos_x, pos_y);
	touch_area(scr, width, height);

	scr->friction = friction;
	scr->max_speed = max_speed;

	scr->x_offset = 0;
	scr->y_offset = 0;
	scr->scroll_hs = 0;
	scr->scroll_vs = 0;
	scr->scrolling = 0;

	return scr;
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
