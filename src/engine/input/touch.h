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

#include "../graphics/camera.h"
#include "SDL_events.h"
#include "SDL_touch.h"

typedef int touch_unique_id;

void finger_init(void);
touch_unique_id finger_bind(SDL_FingerID finger_id); /* binds finger_id to the returned unique touch_id, returns -1 if id is already bound */
touch_unique_id finger_bind_force(SDL_FingerID finger_id); /* same as finger_bind but captures earlier binding if any */
int finger_status(touch_unique_id touch_id, SDL_FingerID finger_id); /* returns whether if touch_id is active and corresponds to finger_id, or not. Set finger_id to -1 to check only if touch_id is active */
int finger_active(SDL_FingerID finger_id); /* return if finger_id is already bound */
void finger_release(SDL_FingerID finger_id); /* unbinds the unique_touch_id associated with SDL_fingerID */
void finger_release_all(void); /* releases all bindings */
void finger_unbind(touch_unique_id touch_id); /* unbinds the given unique_touch_id */
touch_unique_id finger_get_touch_id(SDL_FingerID finger_id);

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

	view *container;
} touchable;

typedef struct touch_calls{
	void (*update) (touchable *);
	void (*render) (touchable *);

	/* touch_down, touch_motion, and touch_up should return 1 if touch is consumed */
	int (*touch_down) (touchable *, SDL_TouchFingerEvent *finger);
	int (*touch_motion) (touchable *, SDL_TouchFingerEvent *finger);
	int (*touch_up) (touchable *, SDL_TouchFingerEvent *finger);

	/* return 1 if keypress was handled */
	int (*touch_keypress) (touchable *, SDL_Scancode key);

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
#ifndef TOUCH_OBJECT_H_
#define TOUCH_OBJECT_H_
static void update(touchable *);
static void render(touchable *);
static int touch_down(touchable *, SDL_TouchFingerEvent *finger);
static int touch_motion(touchable *, SDL_TouchFingerEvent *finger);
static int touch_up(touchable *, SDL_TouchFingerEvent *finger);
static int keypress_down(touchable *, SDL_Scancode key);

static const touch_calls calls = {update,render,touch_down,touch_motion,touch_up, keypress_down};

#define REGISTER_CALLS( t ) \
		((touchable *)t)->calls = &calls; \
		((touchable *)t)->visible=1; \
		((touchable *)t)->enabled=1; \
		((touchable *)t)->get.margin=0;

#define REGISTER_BUTTON_CALLS( t ) \
		((touchable *)t)->calls = &calls; \
		if (t->touch_data.calls != &calls) { \
			SDL_Log("ERROR: touch listener registration failed"); \
			exit(-1);\
		} \
		((touchable *)t)->visible=1; \
		((touchable *)t)->enabled=1; \
		((touchable *)t)->get.margin=0;

#define INSIDE( t, x, y) touch_is_inside((touchable *) t, x, y)
#endif /* TOUCH_OBJECT_H_ */
#endif
