/*
 * accessors.c
 *
 *  Created on: Mar 7, 2014
 *      Author: karlmka
 */

#include "tween.h"
#include "chipmunk.h"
#include "we_utils.h"
#include "../graphics/sprite.h"
static int cpv_accessor(int action, void *data, double set[TWEEN_MAX_DIMENSION])
{
	cpVect *vs = (cpVect*) data;
	if(action == TWEEN_SET) {
		vs->x = (cpFloat)set[0];
		vs->y = (cpFloat)set[1];
	} else if (action == TWEEN_GET) {
		set[0] = (double)vs->x;
		set[1] = (double)vs->y;
	}
	return 2;
}

static int f_accessor(int action, void *data, double set[TWEEN_MAX_DIMENSION])
{
	float *f = (float*) data;
	if(action == TWEEN_SET) {
		*f = (float)set[0];
	} else if (action == TWEEN_GET) {
		set[0] = (double)*f;
	}
	return 1;
}
static int col_accessor(int action, void *data, double v[TWEEN_MAX_DIMENSION])
{
	Color *c = (Color*) data;
	if(action == TWEEN_SET) {
		c->r = (unsigned char)v[0];
		c->g = (unsigned char)v[1];
		c->b = (unsigned char)v[2];
		c->a = (unsigned char)v[3];
	} else if (action == TWEEN_GET) {
		v[0] = (double)c->r;
		v[1] = (double)c->g;
		v[2] = (double)c->b;
		v[3] = (double)c->a;
	}
	return 4;
}

static int spr_accessor(int action, void *data, double v[TWEEN_MAX_DIMENSION])
{
	sprite_ext *s = (sprite_ext*) data;
	if(action == TWEEN_SET) {
		s->pos.x = v[0];
		s->pos.y = v[1];
		s->angle =v[2];
	} else if (action == TWEEN_GET) {
		v[0] = s->pos.x;
		v[1] = s->pos.y;
		v[2] = s->angle;
	}
	return 3;
}

tween_accessor cpvect_accessor = cpv_accessor;
tween_accessor float_accessor = f_accessor;
tween_accessor color_accessor = col_accessor;
tween_accessor sprite_accessor = spr_accessor;


