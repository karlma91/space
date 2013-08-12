#include "object.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "../io/waffle_utils.h"

#define INS_MAGIC_COOKIE 0xA2F4C681

int object_count = 0;
object_info *objs = NULL;

static void destroy_func(instance* obj)
{
	obj->TYPE->call.on_destroy(obj);
}

int object_register(object *obj)
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

instance *instance_create(object *type, float x, float y, float hs, float vs)
{
	instance *ins = instance_super_malloc(type);
	instance_add(ins);
	type->call.on_create(ins);

	//TODO REMOVE TMP test
	instance_remove(ins);

	return ins;
}


#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

void object_init() {

}

instance *instance_super_malloc(object *type)
{
	LList list = objs[type->ID].pool;
	instance *ins = llist_first(list);

	int size = type->SIZE;

	if (ins == NULL) {
#ifdef DEBUG_MEMORY
		SDL_Log( "Info: Allocating new object id %d of size %u\n", id, size);
#endif
		ins = calloc(1, size);
	} else {
		llist_remove(list, (void *)ins);
		memset(ins,0,size);
	}

	/* set read-only instance data */
	*((object **)&ins->TYPE) = type;
	*((int *)&ins->INS_IDENTIFIER) = INS_MAGIC_COOKIE;

	return ins;
}

#define  err_ins(x) \
	(x == NULL || *((int *) x) != INS_MAGIC_COOKIE)
#define  err_obj(x) \
	(x == NULL || *((int *) x) != OBJ_MAGIC_COOKIE)

void instance_super_free(instance *ins)
{
	if (err_ins(ins)) {
		SDL_Log("ERROR: Invalid instance %p", ins);
		exit(-1);
	}

	int obj_id = ins->TYPE->ID;
	LList list = objs[obj_id].pool;
	llist_add(list, (void *)ins);
#ifdef DEBUG_MEMORY
	SDL_Log( "Info: object id %d has now %d unused allocations\n", TYPE, llist_size(active));
#endif
}


/* add object into its corresponding list */
void instance_add(instance* ins)
{
	int obj_id = ins->TYPE->ID;
	LList active = objs[obj_id].active;

	if (llist_add(active, ins)) {
		int count = objs[obj_id].count++;
		ins->instance_id = count;
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
void instance_iterate_type(void (*f)(instance *), object *type) {
	if (err_obj(type)) {
		SDL_Log("ERROR: in list_iterate_type: Invalid object type %p\n", type);
		exit(-1);
	} else {
		object_info *obj = objs + type->ID;
		llist_iterate_func(obj->active, (void (*)(void *))f);
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

	//TODO remove all object definitions
}


//TODO swap arguments?
instance *instance_nearest(cpVect pos, object *type)
{
	//TODO error check obj_id

	instance *target = NULL;
	float min_length = FLT_MAX;
	float length;
	LList list = objs[type->ID].active;

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

instance *instance_first(object *type)
{
	//TODO error check obj_id
	return (instance *) llist_first(objs[type->ID].active);
}

instance *instance_n(object *type, int n)
{
	//TODO error check obj_id
	return (instance *) llist_at_index(objs[type->ID].active, n);
}

instance *instance_last(object *type)
{
	//TODO error check obj_id
	return (instance *) llist_last(objs[type->ID].active);
}

instance *instance_by_id(object *type, int instance_id)
{
	//TODO error check obj_id
	LList list = objs[type->ID].active;

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

int instance_count(object *type)
{
	//TODO error check obj_id
	return llist_size(objs[type->ID].active);
}

void instance_remove(instance *obj)
{
	//TODO restructure removal method completely
	llist_remove(objs[obj->TYPE->ID].active, obj);
}


