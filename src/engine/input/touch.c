/*
 * touch.c
 *
 *  Created on: 16. juli 2013
 *      Author: Mathias
 */


/* private struct */

#include "touch.h"
#include "we_utils.h"


static void update_touch_region(touchable *t)
{
	float x, y, s_2;

	x = t->get.x;
	y = t->get.y;

	s_2 = t->get.width / 2 + t->get.margin;
	t->get.t1x = x - s_2;
	t->get.t2x = x + s_2;

	s_2 = t->get.height / 2 + t->get.margin;
	t->get.t1y = y - s_2;
	t->get.t2y = y + s_2;
}

void touch_place(touchable *t, float x, float y)
{
	t->get.x = x;
	t->get.y = y;

	update_touch_region(t);
}

void touch_area(touchable *t, float w, float h)
{
	t->get.width = w;
	t->get.height = h;

	update_touch_region(t);
}

void touch_margin(touchable *t, float margin)
{
	t->get.margin = margin;

	update_touch_region(t);
}

int touch_is_inside(touchable *t, float x, float y)
{
	return WE_INSIDE_RECT(x,y,t->get.t1x,t->get.t1y,t->get.t2x,t->get.t2y);
}


#define MAX_FINGERS 40

static int next_touch_id = 0;
typedef struct finger_data {
	SDL_FingerID finger_id;
	touch_unique_id touch_id;
	float timestamp_bind;
	float timestamp_status;
} finger_data;

static finger_data fingers[MAX_FINGERS]; //TODO use unique single-use id-numbers?


void finger_init(void)
{
	memset(fingers,0xFF, MAX_FINGERS * sizeof (finger_data));
}

/* binds finger_id to the returned unique touch_id, returns -1 if id is already bound */
touch_unique_id finger_bind(SDL_FingerID finger_id)
{
	return -1;
}

/* same as finger_bind but captures earlier binding if any */
touch_unique_id finger_bind_force(SDL_FingerID finger_id)
{
	return -1;
}

/* returns whether if touch_id is active or not*/
int finger_status(touch_unique_id touch_id)
{
	return 0;
}

/* unbinds the unique_touch_id associated with SDL_fingerID */
void finger_release(SDL_FingerID finger_id)
{

}

/* unbinds the given unique_touch_id */
void finger_unbind(touch_unique_id touch_id)
{

}


