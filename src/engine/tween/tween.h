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
typedef struct manager manager;

void tween_init();
manager * tween_manager();
void tween_update_manager(manager *m, float dt);
tween * tween_create(float *start, float *end, int num, float duration, float (*easing)(float));
void tween_step(tween *t, float dt);
cpVect tween_cpv_pos(tween *t);
void tween_pos(tween *t, float *pos);
float tween_pos_i(tween *t, int i);

float tween_move_f(float start, float end, float time, float duration, float (*easing)(float));

void tween_destroy();

#endif /* TWEEN_H_ */
