#include "objects.h"
#include "../level.h"
#include "../../engine/data/llist.h"
#include "../../engine/io/waffle_utils.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define DEBUG_MEMORY

int LAYER_PLAYER          =     1<<1 | 1<<2 |                1<<5;
int LAYER_TANK            =     1<<1 |         1<<3;
int LAYER_WHEEL           =                                        1<<6;
int LAYER_TANK_FACTORY    =            1<<2  |        1<<4;
int LAYER_PLAYER_BULLET   =                    1<<3 | 1<<4;
int LAYER_ENEMY_BULLET    =                                 1<<5;

#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

static LList lists[ID_COUNT];
static LList lists_pool[ID_COUNT];
static int instance_counter[ID_COUNT];

static void destroy_func(object_data* obj)
{
	obj->preset->destroy(obj);
}


/* initializes head and last values */
void objects_init() {
	int i;
	for (i = 0; i < ID_COUNT; i++) {
		lists[i] = llist_create();
		llist_set_remove_callback(lists[i], (void (*) (void *))destroy_func);
		instance_counter[i] = 0;
	}

	for (i = 0; i < ID_COUNT; i++) {
		lists_pool[i] = llist_create(); //TODO use lists_pool
	}
}

object_data *objects_super_malloc(enum OBJECT_ID id, size_t size)
{
	LList list = lists_pool[id];
	object_data *obj = llist_first(list);

	if (obj == NULL) {
#ifdef DEBUG_MEMORY
		SDL_Log( "Info: Allocating new object id %d of size %u\n", id, size);
#endif
		obj = calloc(1, size);
	} else {
		llist_remove(list, (void *)obj);
		memset(obj,0,size);
	}

	return obj;
}

void objects_super_free(object_data *obj)
{
	int id = obj->preset->ID;
	LList list = lists_pool[id];
	llist_add(list, (void *)obj);
#ifdef DEBUG_MEMORY
	SDL_Log( "Info: object id %d has now %d unused allocations\n", id, llist_size(list));
#endif
}


/* add object into its corresponding list */
void objects_add(object_data* obj)
{
	int obj_id;
	/* find right list to add object into */
	obj_id = obj->preset->ID;

	if (llist_add(lists[obj_id], obj)) {
		obj->instance_id = instance_counter[obj_id];
		++instance_counter[obj_id];
	}
}

/* iterate through all lists of objects */
void objects_iterate(void (*f)(object_data *))
{
	int obj_id;
	for (obj_id = 0; obj_id < ID_COUNT; obj_id++) {
		llist_iterate_func(lists[obj_id], (void (*)(void *))f);
	}
}

/* iterate through one type of object */
void objects_iterate_type(void (*f)(object_data *), int obj_id) {
	/* error check obj_id */
	if (obj_id >= 0 && obj_id < ID_COUNT) {
		llist_iterate_func(lists[obj_id], (void (*)(void *))f);
	} else {
		SDL_Log("list_iterate_type: Invalid obj_id %d\n", obj_id);
		return;
	}
}

/* removes all nodes from all lists */
void objects_clear()
{
	int obj_id;
	for (obj_id = 0; obj_id < ID_COUNT; obj_id++) {
		llist_clear(lists[obj_id]);
		instance_counter[obj_id] = 0;
	}
}

/* frees all nodes in all object lists */
void objects_destroy()
{
	int obj_id;
	for (obj_id = 0; obj_id < ID_COUNT; obj_id++) {
		llist_destroy(lists[obj_id]);
		instance_counter[obj_id] = 0;
	}
}

object_data *objects_nearest(cpVect pos, int obj_id)
{
	//TODO error check obj_id

	object_data *target = NULL;
	cpFloat min_length = INT_MAX;
	cpFloat length;
	LList list = lists[obj_id];

	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		object_data *obj = (object_data *) llist_next(list);
		cpVect v = obj->body->p;
		length = (v.x-pos.x)*(v.x-pos.x) + (v.y-pos.y)*(v.y-pos.y);

		if (length < min_length) {
			min_length = length;
			target = obj;
		}
	}
	llist_end_loop(list);

	return target;
}

object_data *objects_first(int obj_id)
{
	//TODO error check obj_id
	return (object_data *) llist_first(lists[obj_id]);
}

object_data *objects_n(int obj_id, int n)
{
	//TODO error check obj_id
	return (object_data *) llist_at_index(lists[obj_id], n);
}

object_data *objects_last(int obj_id)
{
	//TODO error check obj_id
	return (object_data *) llist_last(lists[obj_id]);
}

object_data *objects_by_id(int obj_id, int instance_id)
{
	//TODO error check obj_id
	LList list = lists[obj_id];

	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		object_data *obj = (object_data *) llist_next(list);
		if (obj->instance_id == instance_id) {
			return obj;
		}
	}
	llist_end_loop(list);

	return NULL;
}

int objects_count(int obj_id)
{
	//TODO error check obj_id
	return llist_size(lists[obj_id]);
}

void objects_remove(object_data *obj)
{
	//TODO restructure removal method completely
	llist_remove(lists[obj->preset->ID], obj);
}

//TODO compute the shortest distance considering wrap-around
void objects_nearest_x_two(object_data *object, int obj_id, object_data **left, object_data **right, cpFloat *left_distance, cpFloat *right_distance)
{
	//TODO error check obj_id

	LList list = lists[obj_id];

	cpFloat min_length_left = INT_MIN;
	cpFloat min_length_right = INT_MAX;
	cpFloat length;

	int check_id = (obj_id == object->preset->ID);

	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		object_data *obj = (object_data *) llist_next(list);
		if (check_id && object->instance_id == obj->instance_id)
			continue; //skip self

		cpVect v = obj->body->p;
		length = v.x - object->body->p.x;
		if (length < 0) { //left side
			if (length > min_length_left) {
				min_length_left = length;
				*left = obj;
			}
		} else { //right side
			if (length < min_length_right) {
				min_length_right = length;
				*right = obj;
			}
		}
	}
	llist_end_loop(list);


	if (min_length_left == INT_MIN) {
		*left = NULL;
		*left_distance =  INT_MAX;
	} else {
		*left_distance =  -min_length_left;
	}
	if (min_length_right == INT_MAX) {
		*right = NULL;
		*right_distance = INT_MAX;
	} else {
		*right_distance = min_length_right;
	}
}
