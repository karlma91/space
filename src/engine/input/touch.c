/*
 * touch.c
 *
 *  Created on: 16. juli 2013
 *      Author: Mathias
 */


/* private struct */

#include "touch.h"
#include "we_utils.h"
#include "SDL.h"

static void update_touch_region(touchable *t)
{
	float x, y, s_2;

	x = t->get.x;
	y = t->get.y;

	s_2 = t->get.width / 2 + t->get.margin;
	t->get.t1x = x - s_2;
	t->get.t2x = x + s_2;

	s_2 = t->get.height / 2 + t->get.margin;
	t->get.t1y = y - s_2;
	t->get.t2y = y + s_2;
}

void touch_place(touchable *t, float x, float y)
{
	t->get.x = x;
	t->get.y = y;

	update_touch_region(t);
}

void touch_area(touchable *t, float w, float h)
{
	t->get.width = w;
	t->get.height = h;

	update_touch_region(t);
}

void touch_margin(touchable *t, float margin)
{
	t->get.margin = margin;

	update_touch_region(t);
}

int touch_is_inside(touchable *t, float x, float y)
{
	return WE_INSIDE_RECT(x,y,t->get.t1x,t->get.t1y,t->get.t2x,t->get.t2y);
}


#define TOUCH_TIMEOUT 2000
#define MAX_FINGERS 40

static int touch_count = 0;
static int next_touch_id = 0;

typedef struct finger_data {
	SDL_FingerID finger_id;
	touch_unique_id touch_id;
	Uint32 timestamp_bind;
	Uint32 timestamp_status;
	//cpVect position;
} finger_data;

static finger_data fingers[MAX_FINGERS];


void finger_init(void)
{
	finger_release_all();
}

/* binds finger_id to the returned unique touch_id, returns -1 if id is already bound */
touch_unique_id finger_bind(SDL_FingerID finger_id)
{
	int i;
	int i_open = -1;
	Uint32 timestamp = SDL_GetTicks();
	for (i = 0; i < MAX_FINGERS; i++) {
		if (fingers[i].finger_id == finger_id) {
			if (timestamp - fingers[i].timestamp_bind > TOUCH_TIMEOUT) {
				i_open = i;
				--touch_count;
				fprintf(stderr, "touch_count: %2d, timeout -> removes binding [%llx => %x]@%02d\n", touch_count, fingers[i].finger_id, fingers[i].touch_id, i_open);
				break;
			}
			return -1;
		}
		if (fingers[i].finger_id == -1) {
			i_open = i;
		}
	}
	if (i_open >= 0) {
		if (next_touch_id == -1) next_touch_id = 0;
		++touch_count;
		if (touch_count > MAX_FINGERS) {
			SDL_Log("ERROR: finger bind -> touch_count > %d!", MAX_FINGERS);
		}
		fingers[i_open].finger_id = finger_id;
		fingers[i_open].touch_id = next_touch_id;
		fingers[i_open].timestamp_bind = timestamp;
		fingers[i_open].timestamp_status = timestamp;
		fprintf(stderr, "touch_count: %2d, binding [%llx => %x]@%02d\n", touch_count, finger_id, next_touch_id, i_open);
		return next_touch_id++;
	} else {
		SDL_Log("WARNING: No fingers available!");
		return -1;
	}
}

/* same as finger_bind but captures earlier binding if any */
touch_unique_id finger_bind_force(SDL_FingerID finger_id)
{
	int unused = 0;
	int i;
	int i_open = -1;
	Uint32 timestamp = SDL_GetTicks();
	for (i = 0; (i < MAX_FINGERS); i++) {
		if (fingers[i].finger_id == -1) {
			unused = 1;
			i_open = i;
		} else if (fingers[i].finger_id == finger_id) {
			unused = 0;
			i_open = i;
			break;
		}
	}
	if (unused) {
		++touch_count;
		if (touch_count > MAX_FINGERS) {
			SDL_Log("ERROR: finger bind force -> touch_count > %d!", MAX_FINGERS);
		}
	} else {
		fprintf(stderr, "force: removing old binding [%llx => %x]@%02d\n", fingers[i_open].finger_id, fingers[i_open].touch_id, i_open);
	}
	if (i_open >= 0) {
		if (next_touch_id == -1) next_touch_id = 0;
		fprintf(stderr, "touch_count: %2d, force binding [%llx => %x]@%02d\n", touch_count, finger_id, next_touch_id, i_open);
		fingers[i_open].finger_id = finger_id;
		fingers[i_open].touch_id = next_touch_id;
		fingers[i_open].timestamp_bind = timestamp;
		fingers[i_open].timestamp_status = timestamp;
		return next_touch_id++;
	} else {
		SDL_Log("WARNING: No fingers available!");
		return -1;
	}
}

/* returns whether if touch_id is active and corresponds to finger_id, or not. finger_id = -1 checks only if touch_id is active */
int finger_status(touch_unique_id touch_id, SDL_FingerID finger_id)
{
	int i;
	//Uint32 timestamp = SDL_GetTicks();
	for (i = 0; i < MAX_FINGERS; i++) {
		//fingers[i].finger_id = (timestamp - fingers[i].timestamp_bind > TOUCH_TIMEOUT) ? -1 : fingers[i].finger_id;
		if (((finger_id == -1) ^ (fingers[i].finger_id == finger_id)) && fingers[i].touch_id == touch_id) { //FIXME
			fingers[i].timestamp_status = SDL_GetTicks();
			return 1;
		}
	}
	return 0;
}

/* return if finger_id is already bound */
int finger_active(SDL_FingerID finger_id)
{
	int i;
	for (i = 0; i < MAX_FINGERS; i++) {
		if (fingers[i].finger_id == finger_id) {
			return 1;
		}
	}
	return 0;
}

/* unbinds the unique_touch_id associated with SDL_fingerID */
void finger_release(SDL_FingerID finger_id)
{
	int i;
	for (i = 0; i < MAX_FINGERS; i++) {
		if (fingers[i].finger_id == finger_id) {
			--touch_count;
			fprintf(stderr, "touch_count: %2d, removes binding  [%llx => %x]@%02d\n", touch_count, finger_id, fingers[i].touch_id, i);
			fingers[i].finger_id = -1;
			fingers[i].touch_id = -1;
			//TODO clear binding?
			if (touch_count < 0) {
				SDL_Log("ERROR: touch release -> touch_count < 0!");
			}
			//break;
		}
	}
}

/* releases all bindings */
void finger_release_all(void)
{
	int i;
	for (i = 0; i < MAX_FINGERS; i++) {
		fingers[i].finger_id = -1;
		fingers[i].touch_id = -1;
	}
	touch_count = 0;
	fprintf(stderr, "all bindings removed\n");
}

/* unbinds the given unique_touch_id */
void finger_unbind(touch_unique_id touch_id)
{
	if (touch_id == -1) return;
	int i;
	for (i = 0; i < MAX_FINGERS; i++) {
		if (fingers[i].finger_id != -1 && fingers[i].touch_id == touch_id) {
			--touch_count;
			fprintf(stderr, "touch_count: %2d, unbinding [%llx => %x]@%02d\n", touch_count, fingers[i].finger_id, touch_id, i);
			fingers[i].finger_id = -1;
			fingers[i].touch_id = -1;
			if (touch_count < 0) {
				SDL_Log("ERROR: finger unbind -> touch_count < 0!");
			}
			//break;
		}
	}
}

touch_unique_id finger_get_touch_id(SDL_FingerID finger_id)
{
	int i;
	for (i = 0; i < MAX_FINGERS; i++) {
		if (fingers[i].finger_id == finger_id) {
			return fingers[i].touch_id;
		}
	}
	return 0;
}


