/*
 * tween.h
 *
 *  Created on: Sep 13, 2013
 *      Author: karlmka
 */

#ifndef TWEEN_H_
#define TWEEN_H_

#include "we_data.h"
#include "../engine.h"
#include "chipmunk.h"
#include "easing.h"

#define TWEEN_DIMS 4
#define TWEEN_INFINITE -1
#define TRUE 1
#define FALSE 0

typedef struct tween tween;

typedef struct {
	unsigned int running    : 1;
	unsigned int done       : 1;
	unsigned int yoyo       : 1;
} tween_data;

struct tween {
    tween_data d;
    signed int repeat;
    char usertype;
	int dims;			// elements in start and end
	signed char dir;
	float start[TWEEN_DIMS];
	float end[TWEEN_DIMS];
	float (*easing)(float);
	float duration;
	float time;
	void (*callback)(void *data);
};

/**
 * Array tween
 */
void tween_init();
tween * tween_create(float *start, float *end, int num, float duration, float (*easing)(float));
void tween_step(tween *t, float dt);
void tween_repeat(tween *t, int times, int yoyo);
tween * tween_release(tween *t);
void tween_pos(tween *t, float *pos);
float tween_pos_i(tween *t, int i);
void tween_start(tween *t);
float tween_tweenfunc(float start, float end, float time, float duration, float (*easing)(float));
void tween_destroy();

/**
 * float tween
 */
tween * tween_float_is_done_remove(tween *t, float *a );

/*
 * cpVect tween
 */
tween * tween_cpv_create(cpVect start, cpVect end, float duration, float (*easing)(float));
tween * tween_cpv_is_done_remove(tween *t, cpVect *a);
cpVect tween_cpv_pos(tween *t);

#endif /* TWEEN_H_ */
