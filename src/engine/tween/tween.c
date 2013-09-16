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
	t->d.running =   TRUE;
	t->d.done =     FALSE;
	t->d.yoyo =     FALSE;
	t->repeat = 0;
	t->time = 0;
	return t;
}

void tween_start(tween *t)
{
	t->d.running = TRUE;
}

/**
 * times = negative infinite repeats
 */
void tween_repeat(tween *t, int times, int yoyo)
{
	t->repeat = times;
	if(yoyo != 0){
		t->d.yoyo = TRUE;
	}
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

static void tween_iter_step(void *t, void *d)
{
	float dt = *((float*)d);
	tween_step((tween*)t, dt);
}

void tween_step(tween *t, float dt)
{
	if(t->d.running) {
		t->time += dt * (t->dir);
		if( t->time > t->duration || t->time < 0) {
			if(t->repeat != 0) {
			    if(t->repeat > 0){
			        t->repeat -= 1;
			    }
				if(t->d.yoyo){
					t->dir *= -1;
				}else{
					t->time = 0;
				}
			}else {
				t->time = t->duration;
				t->d.done = TRUE;
			}
		}
	}
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
