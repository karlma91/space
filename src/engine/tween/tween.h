/*
 * tween2.h
 *
 *  Created on: 6. mars 2014
 *      Author: Karl
 */

#ifndef TWEEN2_H_
#define TWEEN2_H_

#include "easing.h"
#include "../data/llist.h"

#define TWEEN_FORWARD 1
#define TWEEN_BACKWARD (-1)

#define TWEEN_MAX_DIMENSION 6
#define TWEEN_NONRELATIVE 3
#define TWEEN_RELATIVE 2
#define TWEEN_YOYO 1
#define TWEEN_TIMELINE 0
#define TWEEN_TWEEN 1
#define TWEEN_SET 0
#define TWEEN_GET 1
#define TWEEN_FALSE 0
#define FALSE 0
#define TRUE 1
#define TWEEN_INFINITY (-1)




typedef struct tween_instance timeline;
typedef struct tween_instance tween;
typedef struct tween_instance tween_instance;

typedef int (*tween_accessor)(int action, void *data, double *values); // return dimension
typedef float (*easing_func)(float);
typedef void (*tween_callback)(tween_instance * t, void *userdata);

extern tween_accessor cpvect_accessor;
extern tween_accessor float_accessor;
extern tween_accessor color_accessor;
extern tween_accessor sprite_accessor;

typedef struct {
	double start[TWEEN_MAX_DIMENSION];
	double end[TWEEN_MAX_DIMENSION];
	void * accessor_data;
	easing_func easing;
	tween_accessor accessor;
} inner_tween;

typedef struct {
	LList tweens;
} inner_timeline;

struct tween_instance {
	int type;          // timeline or tween
	int autofree;    // gets automaticaly removed and freed from system
	int yoyo;          // running to end and back or start from start
	int running;       // running? used for pause
	int done;          // if 1 done and ready for removal
	int repeats;       // num of repeats
	int repeats_left;  // num of repeats left
	int dims;	       // elements in start and end
	int dir;           // bacwards or forwards
	float time;        // time runned
	float duration;    // duration of a tween
	float startdelay;  // delay before start
	float repeatdelay; // delay before repeat
	void *userdata;    // userdata for callback
	tween_callback callback; // callback
	union {
		inner_timeline tl;
		inner_tween tw;
	};
};

typedef struct tween_system_ {
	float time;
	LList tweens;
} tween_system;

void tween_init(void);
tween_system * tween_new_system(void);
tween * tween_system_new_tween(tween_system *s, tween_accessor accessor, void * data, float duration);
void tween_add(tween_system *s, tween_instance *t);
void tween_update(tween_system *s, float delta);
void tween_system_clear(tween_system *s);
void tween_system_destoy(tween_system *s);

void tween_delay(tween_instance *t, float delay);
void tween_set_callback(tween_instance *t, tween_callback callback, void *userdata);
void tween_call_callback(tween_instance *t);
void tween_repeat(tween_instance *t, int yoyo, int times, float delay);
void tween_set_dir(tween_instance *t, int dir);

tween * tween_new_tween(tween_accessor accessor, void * data, float duration);
void tween_tween_reset(tween *t);
void tween_tween_set_start(tween *t);
void tween_dimtochange(tween *t, int dim);
void tween_easing(tween *t, easing_func e);
void tween_target(tween*t, int relative, double x, ...);
void tween_tween_update(tween *t, float delta);

timeline * tween_new_timeline(void);
void tween_timeline_reset(timeline *t);
void tween_push(timeline *t, tween *tween);
void tween_timeline_update(timeline *t, float delta);

void tween_destroy(void);

#endif /* TWEEN2_H_ */
