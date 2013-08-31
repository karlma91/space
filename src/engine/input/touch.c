/*
 * touch.c
 *
 *  Created on: 16. juli 2013
 *      Author: Mathias
 */


/* private struct */

#include "touch.h"


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
	return (x >= t->get.t1x) && (x <= t->get.t2x) && (y >= t->get.t1y) && (y <= t->get.t2y);
}
