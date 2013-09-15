/*
 * tween.h
 *
 *  Created on: Sep 13, 2013
 *      Author: karlmka
 */

#ifndef TWEEN_H_
#define TWEEN_H_

#include "we_data.h"
#include "chipmunk.h"
#include "easing.h"

#define TWEEN_DIMS 4
typedef struct tween tween;

struct tween {
	int dims;			// elements in start and end
	int done; 			// how many times it have runned
	int running; 		// sets to on in tween_start
	int repeat; 		// how many times to run
	int yoyo; 			// how many times to run
	int dir;
	float start[TWEEN_DIMS];
	float end[TWEEN_DIMS];
	float (*easing)(float);
	float duration;
	float time;
	void (*callback)(void *data);
};

void tween_init();
tween * tween_create(float *start, float *end, int num, float duration, float (*easing)(float));
tween * tween_cpv_create(cpVect start, cpVect end, float duration, float (*easing)(float));
void tween_update(float dt);
void tween_step(tween *t, float dt);
void tween_repeat(tween *t, int times, int yoyo);
tween * tween_cpv_is_done_remove(tween *t, cpVect *a);
tween * tween_float_is_done_remove(tween *t, float *a );
tween * tween_release(tween *t);
cpVect tween_cpv_pos(tween *t);
void tween_pos(tween *t, float *pos);
float tween_pos_i(tween *t, int i);
void tween_start(tween *t);

float tween_tweenfunc(float start, float end, float time, float duration, float (*easing)(float));

void tween_destroy();

#endif /* TWEEN_H_ */
