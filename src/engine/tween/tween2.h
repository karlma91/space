/*
 * tween2.h
 *
 *  Created on: 6. mars 2014
 *      Author: Karl
 */

#ifndef TWEEN2_H_
#define TWEEN2_H_
#define TWEEN_MAX_DIMENSION 6
#define TWEEN_TIMELINE 0
#define TWEEN_TWEEN 1

typedef int (*accessor_func)(void *data, float *set); // return dimension
typedef float (*easing_func)(float);
typedef void (*callback_func)(void *data);

typedef struct {
	float start[TWEEN_MAX_DIMENSION];
	float *values[TWEEN_MAX_DIMENSION];
	float end[TWEEN_MAX_DIMENSION];
	void * accessor_data;
	easing_func easing;
	accessor_func accessor;
} tween;

typedef struct {
	LList tweens;
} timeline;

typedef struct tween_instance tween_timeline;
typedef struct tween_instance tween_tween;
typedef struct tween_instance {
	int type;          // timeline or tween
	int yoyo;          // running to end and back or start from start
	int running;       // running?
	int done;          // done?
	int repeats;       // num of repeats
	int dims;	       // elements in start and end
	int dir;           // bacwards or forwards
	float time;        // time runned
	float duration;    // duration of a tween
	float startdelay;  // delay before start
	void *userdata;    // userdata for callback
	callback_func callback; // callback
	union {
		timeline tl;
		tween tw;
	};
} tween_instance;

typedef struct system {
	float time;
	LList timelines;
} tween_system;


void tween_delay(tween_instance *t, float t);
void tween_callback(tween_instance *t, callback_func callback, void *data);
void tween_repeat(tween_instance *t, int yoyo, int times, float delay);

tween_tween * tween_new_tween(accessor_func accessor, void * data, float duration);
void tween_dimtochange(tween_tween *t, int dim);
void tween_easing(tween_tween *t, easing_func e);
void tween_target(tween_tween*t, float x, ...);
void tween_targetrelative(tween_tween*t, float x, ...);

tween_timeline * tween_new_timeline();
void tween_push(tween_timeline *t, tween_tween *tween);

tween_system * tween_new_system();
void tween_add_timeline(tween_system *s, tween_timeline *t);
void tween_add_tween(tween_system *s, tween_tween *t);
void tween_update(tween_system *s);

#endif /* TWEEN2_H_ */
