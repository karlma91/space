/*
 * tween.c
 *
 *  Created on: Sep 13, 2013
 *      Author: karlmka
 */
#include "tween.h"
#include "pool.h"
#include "stdlib.h"

pool * tween_pool;
pool * manager_pool;

struct manager {
	LList tweens;
	float time;
};

struct tween {
	int dims;
	int done;
	int started;
	int repeat;
	float start[TWEEN_DIMS];
	float end[TWEEN_DIMS];
	float (*easing)(float);
	float duration;
	float time;
	void (*callback)(void *data);
};

void tween_init()
{
	tween_pool = pool_create(sizeof(tween));
	manager_pool = pool_create(sizeof(manager));
}

manager * tween_manager()
{
	return (manager *)pool_instance(manager_pool);
}

void tween_update_manager(manager *m, float dt)
{
	llist_iterate_func(m->tweens, tween_step, (void*)&dt);
}

tween * tween_create(float *start, float *end, int num, float duration, float (*easing)(float))
{
	tween *t = (tween *)pool_instance(tween_pool);

	t->dims = num;
	memcpy(t->start, start, num);
	memcpy(t->end, start, num);
	t->duration = duration;
	if(easing == NULL){
		t->easing = LinearInterpolation;
	}else{
		t->easing = easing;
	}
	return t;
}

void tween_step(tween *t, float dt)
{
	if(t->start){
		t->time += dt;
		if( t->time > t->duration){
			if(t->repeat){
				t->time = 0;
			}else{
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
	v.x = tween_pos_i(t,1);
	return v;
}

void tween_pos(tween *t, float *pos)
{
	int i;
	for(i=0; i < t->dims; i++){
		pos[i] = tween_pos_i(t, i);
	}
}

float tween_pos_i(tween *t, int i)
{
	return tween_move_f(t->start[i], t->end[i], t->time, t->duration, t->easing);
}


float tween_move_f(float start, float end, float time, float duration, float (*easing)(float))
{
	return start + easing(time/duration) * (end - start);
}

void tween_destroy()
{
	pool_destroy(manager_pool);
	pool_destroy(tween_pool);
}
