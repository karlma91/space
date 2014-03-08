/*
 * tween2.c
 *
 *  Created on: 6. mars 2014
 *      Author: Karl
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "tween.h"
#include "pool.h"
pool * tween_pool;

void tween_init()
{
	tween_pool = pool_create(sizeof(tween_instance));
}

tween_system * tween_new_system()
{
	tween_system *s = calloc(1,sizeof(tween_system));
	s->tweens = llist_create();
	return s;
}

/*
 * creats a tween and adds it to a system
 */
tween * tween_system_new_tween(tween_system *s, tween_accessor accessor, void * data, float duration)
{
	tween * t = tween_new_tween(accessor,data,duration);
	tween_add(s, t);
	return t;
}

void tween_add(tween_system *s, tween_instance *t)
{
	llist_add(s->tweens, t);
}

void tween_update(tween_system *s, float delta)
{
	llist_begin_loop(s->tweens);
	while(llist_hasnext(s->tweens)){
		tween_instance * t = llist_next(s->tweens);
		if(t->type == TWEEN_TWEEN){
			tween_tween_update(t, delta);
		}else if(t->type == TWEEN_TIMELINE){
			tween_timeline_update(t, delta);
		}
		if(t->done && t->autoremove) {
			llist_remove(s->tweens, t);
			pool_release(tween_pool, t);
		}
	}
	llist_end_loop(s->tweens);
}
void tween_system_clear(tween_system *s)
{
	llist_begin_loop(s->tweens);
	while(llist_hasnext(s->tweens)){
		tween_instance * t = llist_next(s->tweens);
		llist_remove(s->tweens, t);
		pool_release(tween_pool, t);
	}
	llist_end_loop(s->tweens);
}
void tween_system_destoy(tween_system *s)
{
	llist_destroy(s->tweens);
	free(s);
}


void tween_delay(tween_instance *t, float delay)
{
	t->startdelay = delay;
}

void tween_set_callback(tween_instance *t, tween_callback callback, void *userdatadata)
{
	t->callback = callback;
	t->userdata = userdatadata;
}

void tween_call_callback(tween_instance *t){
	if(t->callback){
		t->callback(t, t->userdata);
	}
}

void tween_repeat(tween_instance *t, int yoyo, int times, float delay)
{
	t->yoyo = yoyo>0;
	t->repeats = times;
	t->repeats_left = times;
	t->repeatdelay = delay;
}

tween * tween_new_tween(tween_accessor accessor, void * data, float duration)
{
	tween *t = pool_instance(tween_pool);
	memset(t,0,sizeof(tween));
	t->type = TWEEN_TWEEN;
	t->autoremove = 1;
	t->running = 1;
	t->dir = 1;
	t->duration = duration;
	t->tw.easing = (easing_func)LinearInterpolation;
	t->tw.accessor = accessor;
	t->tw.accessor_data = data;
	t->dims = t->tw.accessor(TWEEN_GET, t->tw.accessor_data, t->tw.start);
	int i;
	for(i=0;i<t->dims;i++)
		t->tw.end[i] = t->tw.start[i];
	return t;
}
void tween_tween_reset(tween *t){
	t->time = 0;
	t->dir = 1;
	t->repeats_left = t->repeats;
	t->running = 1;
	t->done = 0;
}
void tween_tween_set_start(tween *t)
{
	t->dims = t->tw.accessor(TWEEN_GET, t->tw.accessor_data, t->tw.start);
}
void tween_dimtochange(tween *t, int dim);
void tween_easing(tween *t, easing_func e)
{
	t->tw.easing = e;
}

void tween_target(tween*t, int relative, double x, ...)
{
	  va_list ap;
	  va_start(ap, x);
	  t->tw.end[0] = x;
	  int i;
	  for(i = 1; i < t->dims; i++) {
		  double val = va_arg(ap,double);
		  if(relative == TWEEN_RELATIVE) {
			  t->tw.end[i] = t->tw.start[i] + val;
		  } else {
			  t->tw.end[i] = val;
		  }
	  }
	  va_end(ap);
}


float tween_interpolate(double start, double end, double time, double duration, easing_func easing)
{
	return start + easing(time/duration) * (end - start);
}

void tween_interpolateall(tween *t)
{
	double values[TWEEN_MAX_DIMENSION];
	int i;
	for(i=0; i < t->dims; i++) {
		values[i] =  tween_interpolate(t->tw.start[i], t->tw.end[i], t->time - t->startdelay, t->duration - t->startdelay, t->tw.easing);
	}
	t->tw.accessor(TWEEN_SET,t->tw.accessor_data, values);
}

void tween_tween_update(tween *t, float dt)
{
	if (t != NULL) {
		if (t->running) {
			t->time += dt * (t->dir);
			if (t->time > t->duration || t->time < 0) {
				if (t->repeats_left != 0) {
					if (t->repeats_left > 0) {
						t->repeats_left -= 1;
					}
					if (t->yoyo) {
						t->dir *= -1;
					} else {
						t->time = 0;
					}
				} else {
					t->time = t->duration;
					t->running = 0;
					t->done = 1;
					if (t->callback) {
						t->callback(t, t->userdata);
					}
					return;
				}
			}
			if(t->running && t->time >= t->startdelay ){
				tween_interpolateall(t);
			}
		}
	}
}

timeline * tween_new_timeline();
void tween_timeline_reset(timeline *t);
void tween_push(timeline *t, tween *tween);
void tween_timeline_update(timeline *t, float delta)
{

}

void tween_destroy()
{
	pool_destroy(tween_pool);
}

float catmullRomSpline(float a, float b, float c, float d, float t) {
	float t1 = (c - a) * 0.5f;
	float t2 = (d - b) * 0.5f;

	float h1 = 2 * t * t * t - 3 * t * t + 1;
	float h2 = -2 * t * t * t + 3 * t * t;
	float h3 = t * t * t - 2 * t * t + t;
	float h4 = t * t * t - t * t;

	return b * h1 + c * h2 + t1 * h3 + t2 * h4;
}

