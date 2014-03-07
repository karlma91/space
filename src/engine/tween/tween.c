/*
 * tween2.c
 *
 *  Created on: 6. mars 2014
 *      Author: Karl
 */
#include <stdio.h>
#include <stdarg.h>
#include "tween.h"
#include "pool.h"
pool * tween_pool;

tween_init()
{
	tween_pool = pool_create(sizeof(tween_instance));
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
	t->type = TWEEN_TWEEN;
	t->running = 1;
	t->dir = 1;
	t->repeats = 0;
	t->repeats_left = 0;
	t->startdelay = 0;
	t->duration = duration;
	t->tw.easing = (easing_func)LinearInterpolation;
	t->tw.accessor = accessor;
	t->tw.accessor_data = data;
	t->dims = t->tw.accessor(TWEEN_GET, t->tw.accessor_data, t->tw.start);
	return t;
}
void tween_tween_reset(tween *t){
	t->time = 0;
	t->dir = 1;
	t->repeats_left = t->repeats;
	t->running = 1;
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
	  int i;
	  for(i=0; i<t->dims; i++) {
		  double val;
		  if(i==0){
			  val = x;
		  }else{
			  val = va_arg(ap,double);
		  }
		  if(relative){
			  t->tw.end[i] = t->tw.start[i] + val;
		  }else{
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
		values[i] =  tween_interpolate(t->tw.start[i], t->tw.end[i], t->time, t->duration, t->tw.easing);
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
					if (t->callback) {
						t->callback(t->tw.accessor_data, t->userdata);
					}
					return;
				}
			}
			if(t->running){
				tween_interpolateall(t);
			}
		}
	}
}

timeline * tween_new_timeline();
void timeline_reset(timeline *t);
void tween_push(timeline *t, tween *tween);
void timeline_update(timeline *t, float delta);

tween_system * tween_new_system();
void tween_add(tween_system *s, tween_instance *t);
void tween_update(tween_system *s, float delta);

void tween_destroy()
{
	pool_destroy(tween_pool);
}

