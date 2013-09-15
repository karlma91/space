#include <stdlib.h>
#include <string.h>
#include "tween.h"
#include "pool.h"
#include "stdlib.h"

pool * tween_pool;

void tween_init()
{
	tween_pool = pool_create((int)(sizeof(tween)));
}

tween * tween_create(float *start, float *end, int num, float duration, float (*easing)(float))
{
	tween *t = (tween *)pool_instance(tween_pool);
	t->dims = num;
	memcpy(t->start, start, num*sizeof(float));
	memcpy(t->end, end, num*sizeof(float));
	t->duration = duration;
	if(easing == NULL) {
		t->easing = LinearInterpolation;
	}else{
		t->easing = easing;
	}
	t->dir = 1;
	t->running = 1;
	t->repeat = 0;
	t->done = 0;
	t->time = 0;
	return t;
}

tween * tween_cpv_create(cpVect start, cpVect end, float duration, float (*easing)(float))
{
	float s[2];
	s[0] = start.x;
	s[1] = start.y;
	float e[2];
	e[0] = end.x;
	e[1] = end.y;
	tween *t = tween_create(s, e, 2, duration, easing);
	return t;
}

void tween_start(tween *t)
{
	t->running = 1;
}

/**
 * time = 0 infinite repeats
 */
void tween_repeat(tween *t, int times, int yoyo)
{
	t->repeat = 1;
	t->yoyo = yoyo;
}

tween * tween_cpv_is_done_remove(tween *t, cpVect *a )
{
	if(t != NULL){
		if(t->done) {
			tween_release(t);
			return NULL;
		}else {
			*a = tween_cpv_pos(t);
		}
	}
	return t;
}

tween * tween_float_is_done_remove(tween *t, float *a )
{
	if(t != NULL){
		if(t->done) {
			tween_release(t);
			return NULL;
		}else {
			tween_pos(t, a);
		}
	}
	return t;
}


/**
 * useage:
 * t = tween_release(t);
 * to avoid changing unused tween
 */
tween * tween_release(tween *t)
{
	pool_release(tween_pool, t);
	return NULL;
}

void tween_update(float dt)
{
	llist_begin_loop(tween_pool->in_use);
	while(llist_hasnext(tween_pool->in_use)) {
		tween *t = llist_next(tween_pool->in_use);
		if(t->running) {
			if(!t->done) {
				tween_step(t, dt);
			}
		}
	}
	llist_end_loop(tween_pool->in_use);
}

static void tween_iter_step(void *t, void *d)
{
	float dt = *((float*)d);
	tween_step((tween*)t, dt);
}

void tween_step(tween *t, float dt)
{
	if(t->running) {
		t->time += dt * (t->dir);
		if( t->time > t->duration || t->time < 0) {
			if(t->repeat) {
				if(t->yoyo){
					t->dir *= -1;
				}else{
					t->time = 0;
				}
			}else {
				t->time = t->duration;
				t->done = 1;
			}
		}
	}
}

cpVect tween_cpv_pos(tween *t)
{
	cpVect v;
	v.x = tween_pos_i(t,0);
	v.y = tween_pos_i(t,1);
	return v;
}

void tween_pos(tween *t, float *pos)
{
	int i;
	for(i=0; i < t->dims; i++) {
		pos[i] = tween_pos_i(t, i);
	}
}

float tween_pos_i(tween *t, int i)
{
	return tween_tweenfunc(t->start[i], t->end[i], t->time, t->duration, t->easing);
}


float tween_tweenfunc(float start, float end, float time, float duration, float (*easing)(float))
{
	return start + easing(time/duration) * (end - start);
}

void tween_destroy()
{
	pool_destroy(tween_pool);
}
