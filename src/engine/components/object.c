#include "object.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../io/waffle_utils.h"


int object_count = 0;
object_info *objs = NULL;


static void destroy_func(instance* obj)
{
	obj->obj_id->call.on_destroy(obj);
}

int object_priv_register(object_id *obj)
{
	int id = object_count++;

	/* set read-only object info*/
	*(int *)(&obj->ID) = id;

	objs = realloc(objs, object_count * obj->ID);

	obj->info = objs + id;
	objs[id].obj = obj;
	objs[id].count = 0;
	objs[id].active = llist_create();
	objs[id].pool = llist_create();

	llist_set_remove_callback(objs[id].active, (void (*) (void *))destroy_func);

	return id;
}

//TODO pointer to object_id rather than obj_id
//TODO read about macro with use of ## and #
instance *instance_create(int obj_id, float x, float y, float hs, float vs)
{
	instance *ins = instance_super_malloc(obj_id, objs[obj_id].obj->size);
	instance_add(ins);
	objs[obj_id].obj->call.on_create(ins);

	//TMP test
	instance_remove(ins);

	return ins;
}


#ifndef INT_MIN
#define INT_MIN -2147483648
#endif


/*
static LList lists[ID_COUNT];
static LList lists_pool[ID_COUNT];
static int instance_counter[ID_COUNT];
*/



void object_init() {

}

instance *instance_super_malloc(int obj_id, size_t size)
{
	LList list = objs[obj_id].pool;
	instance *obj = llist_first(list);

	if (obj == NULL) {
#ifdef DEBUG_MEMORY
		SDL_Log( "Info: Allocating new object id %d of size %u\n", id, size);
#endif
		obj = calloc(1, size);
	} else {
		llist_remove(list, (void *)obj);
		memset(obj,0,size);
	}

	obj->obj_id = objs[obj_id].obj;

	return obj;
}

void object_super_free(instance *obj)
{
	int obj_id = obj->obj_id->ID;
	LList list = objs[obj_id].pool;
	llist_add(list, (void *)obj);
#ifdef DEBUG_MEMORY
	SDL_Log( "Info: object id %d has now %d unused allocations\n", obj_id, llist_size(active));
#endif
}


/* add object into its corresponding list */
void instance_add(instance* obj)
{
	int obj_id = obj->obj_id->ID;
	LList active = objs[obj_id].active;

	if (llist_add(active, obj)) {
		int count = objs[obj_id].count++;
		obj->instance_id = count;
	}
}

/* iterate through all lists of object */
void instance_iterate(void (*f)(instance *))
{
	int obj_id;
	object_info *obj = objs;

	for (obj_id = 0; obj_id < object_count; obj_id++) {
		llist_iterate_func(obj->active, (void (*)(void *))f);
		++obj;
	}
}

/* iterate through one type of object */
void instance_iterate_type(void (*f)(instance *), int obj_id) {
	object_info *obj = objs + obj_id;

	/* simple error check of obj_id */
	if (obj_id >= 0 && obj_id < object_count) {
		llist_iterate_func(obj->active, (void (*)(void *))f);
	} else {
		SDL_Log("ERROR: in list_iterate_type: Invalid obj_id %d\n", obj_id);
		exit(-1);
		return;
	}
}

/* removes all nodes from all lists */
void object_clear()
{
	int obj_id;
	object_info *obj = objs;

	for (obj_id = 0; obj_id < object_count; ++obj_id, ++obj) {
		llist_clear(obj->active);
		obj->count = 0;
	}
}

/* frees all nodes in all object lists */
void object_destroy()
{
	int obj_id;
	object_info *obj = objs;

	for (obj_id = 0; obj_id < object_count; ++obj_id, ++obj) {
		llist_destroy(obj->active);
		obj->count = 0;
	}
}



instance *instance_nearest(cpVect pos, int obj_id)
{
	//TODO error check obj_id

	instance *target = NULL;
	float min_length = FLT_MAX;
	float length;
	LList list = objs[obj_id].active;

	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		instance *obj = (instance *) llist_next(list);
		cpVect v = obj->body->p;

		float dx = v.x-pos.x;
		float dy = v.y-pos.y;
		length = dx*dx + dy*dy;

		if (length < min_length) {
			min_length = length;
			target = obj;
		}
	}
	llist_end_loop(list);

	return target;
}

instance *instance_first(int obj_id)
{
	//TODO error check obj_id
	return (instance *) llist_first(objs[obj_id].active);
}

instance *instance_n(int obj_id, int n)
{
	//TODO error check obj_id
	return (instance *) llist_at_index(objs[obj_id].active, n);
}

instance *instance_last(int obj_id)
{
	//TODO error check obj_id
	return (instance *) llist_last(objs[obj_id].active);
}

instance *instance_by_id(int obj_id, int instance_id)
{
	//TODO error check obj_id
	LList list = objs[obj_id].active;

	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		instance *obj = (instance *) llist_next(list);
		if (obj->instance_id == instance_id) {
			return obj;
		}
	}
	llist_end_loop(list);

	return NULL;
}

int instance_count(int obj_id)
{
	//TODO error check obj_id
	return llist_size(objs[obj_id].active);
}

void instance_remove(instance *obj)
{
	//TODO restructure removal method completely
	llist_remove(objs[obj->obj_id->ID].active, obj);
}


