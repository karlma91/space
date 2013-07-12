/*
 * touch.h
 *
 * define THIS_IS_A_TOUCH_OBJECT 1 to automatically map touch functions to calls,
 * then add a touch_calls pointer to calls as the first element in touch object's struct.
 * Remember to call REGISTER_CALLS when creating a new instance of touch object.
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

#ifndef TOUCH_H_
#define TOUCH_H_

#include "SDL_touch.h"

typedef struct {
	void (*update) (void *);
	void (*render) (void *);

	/* touch_down, touch_motion, and touch_up should return 1 if touch is consumed */
	int (*touch_down) (void *, SDL_TouchFingerEvent *finger);
	int (*touch_motion) (void *, SDL_TouchFingerEvent *finger);
	int (*touch_up) (void *, SDL_TouchFingerEvent *finger);
} touch_calls;

//TODO create general struct for all touchable objects
//typedef struct...

#if THIS_IS_A_TOUCH_OBJECT
static void update(void *);
static void render(void *);
static int touch_down(void *, SDL_TouchFingerEvent *finger);
static int touch_motion(void *, SDL_TouchFingerEvent *finger);
static int touch_up(void *, SDL_TouchFingerEvent *finger);

static const touch_calls calls = {update,render,touch_down,touch_motion,touch_up};

#define REGISTER_CALLS( touchable ) \
		*((void **) touchable) = (void *) &calls; \
		if (touchable->calls != &calls) { \
			fprintf(stderr,"ERROR: touch listener registration failed\n"); \
			exit(-1);\
		}

#endif

#endif /* TOUCH_H_ */
