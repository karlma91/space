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

#include "SDL_events.h"
#include "SDL_touch.h"

struct touch_calls;

typedef struct {
	const struct touch_calls *calls;

	enum {
		CTRL_UNDEFINED,
		CTRL_JOYSTICK,
		CTRL_BUTTON,
		CTRL_SCROLL
	} type;

	int enabled;
	int visible;

	struct { /* READ ONLY */
		float x, y; /* center position of touchable */
		float width, height; /* dimensions of touchable area */

		float margin; /* touch margin */

		/* computed touch region */
		float t1x, t1y; /* upper left corner of touch region */
		float t2x, t2y; /* lower right corner of touch region */
	} get; /* READ ONLY! Use touch_place and touch_area to perform changes */
} touchable;

typedef struct touch_calls{
	void (*update) (touchable *);
	void (*render) (touchable *);

	/* touch_down, touch_motion, and touch_up should return 1 if touch is consumed */
	int (*touch_down) (touchable *, SDL_TouchFingerEvent *finger);
	int (*touch_motion) (touchable *, SDL_TouchFingerEvent *finger);
	int (*touch_up) (touchable *, SDL_TouchFingerEvent *finger);

	/*
	 * on_down callback?
	 * on_motion callback?
	 * on_up callback?
	 */
} touch_calls;

void touch_place(touchable *t, float x, float y);
void touch_area(touchable *t, float w, float h);
void touch_margin(touchable *t, float margin);

int touch_is_inside(touchable *t, float x, float y);


#endif /* TOUCH_H_ */

#if THIS_IS_A_TOUCH_OBJECT
static void update(touchable *);
static void render(touchable *);
static int touch_down(touchable *, SDL_TouchFingerEvent *finger);
static int touch_motion(touchable *, SDL_TouchFingerEvent *finger);
static int touch_up(touchable *, SDL_TouchFingerEvent *finger);

static const touch_calls calls = {update,render,touch_down,touch_motion,touch_up};

#define REGISTER_CALLS( t ) \
		((touchable *)t)->calls = &calls; \
		if (t->touch_data.calls != &calls) { \
			fprintf(stderr,"ERROR: touch listener registration failed\n"); \
			exit(-1);\
		} \
		((touchable *)t)->visible=1; \
		((touchable *)t)->enabled=1; \
		((touchable *)t)->get.margin=0;

#define INSIDE( t, x, y) touch_is_inside((touchable *) t, x, y)

#endif
