/*
 * tween.c
 *
 *  Created on: Sep 13, 2013
 *      Author: karlmka
 */
#include "tween.h"
#include "stdlib.h"

LList tween_pool;
LList manager_pool;

struct manager {
	LList tweens;
	float time;
};

struct tween {
	int dims;
	int done;
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
	tween_pool = llist_create();
	manager_pool = llist_create();
}

manager * tween_manager()
{
	manager *m;
	if(llist_size(tween_pool)){
		m = llist_pop(manager_pool);
	}else{
		m = calloc(1, sizeof *m);
	}
}

void tween_update_manager(manager *m, float dt)
{
	llist_begin_loop(m->tweens);
	while(llist_hasnext(m->tweens)){
		tween * t = llist_next(m->tweens);
		tween_step(t,dt);
	}
	llist_end_loop(m->tweens);
}

tween * tween_create(float *start, float *end, int num, float duration, float (*easing)(float))
{
	tween *t;
	if(llist_size(tween_pool)){
		t = llist_pop(tween_pool);
	}else{
		t = calloc(1, sizeof *t);
	}
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
	t->time += dt;
	if(t->time > t->duration){
		if(t->repeat){
			t->time = 0;
		}else{
			t->time = t->duration;
			t->done = 1;
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
	llist_destroy(tween_pool);
}
