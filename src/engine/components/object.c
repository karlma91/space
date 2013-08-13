#include "object.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "../io/waffle_utils.h"

#define INS_MAGIC_COOKIE 0xA2F4C681

int object_count = 0;
object_info *objects_meta = NULL;

static void destroy_func(instance* obj)
{
	obj->TYPE->call.on_destroy(obj);
}

int object_register(object *obj)
{
	int id = object_count++;

	/* set read-only object info*/
	*(int *)(&obj->ID) = id;

	objects_meta = realloc(objects_meta, object_count * obj->ID);

	obj->info = objects_meta + id;
	objects_meta[id].obj = obj;
	objects_meta[id].count = 0;
	objects_meta[id].active = llist_create();
	objects_meta[id].pool = llist_create();

	llist_set_remove_callback(objects_meta[id].active, (void (*) (void *))destroy_func);

	return id;
}

object *object_by_name(const char *obj_name)
{
	int obj_id;
	object_info *obj_meta = objects_meta;

	for (obj_id = 0; obj_id < object_count; obj_id++) {
		if (strcasecmp(obj_name, obj_meta->obj->NAME) == 0) {
			return obj_meta->obj;
		}
		++obj_meta;
	}

	return NULL;
}

instance *instance_create(object *type, const void *param, float x, float y, float hs, float vs)
{
	instance *ins = instance_super_malloc(type);

	ins->x = x;
	ins->y = y;
	ins->hs = hs;
	ins->vs = vs;

	//TODO remove x,y,hs,vs in ins.
	//TODO init body
	//TODO set body pos and vel

	/* sets default values */
	ins->alive = 1;
	ins->active_components = type->components_mask;

	if (param) {
		instance_set_param(ins, param);
	}

	instance_add(ins);

	type->call.on_create(ins);

	return ins;
}


#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

void object_init() {

}

instance *instance_super_malloc(object *type)
{
	LList list = objects_meta[type->ID].pool;
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
	LList list = objects_meta[obj_id].pool;
	llist_add(list, (void *)ins);
#ifdef DEBUG_MEMORY
	SDL_Log( "Info: object id %d has now %d unused allocations\n", TYPE, llist_size(active));
#endif
}

struct instance_dummy {
	instance ins;
	struct {

	} params;
};

int instance_set_param(instance *ins, const void *param)
{
	if (err_ins(ins)) {
		SDL_Log("ERROR: instance_copy, instance not recognized!");
		return 1;
	} else if (!param) {
		SDL_Log("ERROR: instance_copy, param == NULL");
		return 1;
	}

	//int offset = sizeof *to;
	size_t size = ins->TYPE->P_SIZE;
	memcpy(&((struct instance_dummy *)ins)->params, param, size);

	return 0;
}

/* add object into its corresponding list */
void instance_add(instance* ins)
{
	int obj_id = ins->TYPE->ID;
	LList active = objects_meta[obj_id].active;

	if (llist_add(active, ins)) {
		int count = objects_meta[obj_id].count++;
		ins->instance_id = count;
	}
}

/* iterate through all lists of object */
void instance_iterate(void (*f)(instance *))
{
	int obj_id;
	object_info *obj = objects_meta;

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
		object_info *obj = objects_meta + type->ID;
		llist_iterate_func(obj->active, (void (*)(void *))f);
	}
}

/* removes all nodes from all lists */
void object_clear()
{
	int obj_id;
	object_info *obj = objects_meta;

	for (obj_id = 0; obj_id < object_count; ++obj_id, ++obj) {
		llist_clear(obj->active);
		obj->count = 0;
	}
}

/* frees all nodes in all object lists */
void object_destroy()
{
	int obj_id;
	object_info *obj = objects_meta;

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
	LList list = objects_meta[type->ID].active;

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
	return (instance *) llist_first(objects_meta[type->ID].active);
}

instance *instance_n(object *type, int n)
{
	//TODO error check obj_id
	return (instance *) llist_at_index(objects_meta[type->ID].active, n);
}

instance *instance_last(object *type)
{
	//TODO error check obj_id
	return (instance *) llist_last(objects_meta[type->ID].active);
}

instance *instance_by_id(object *type, int instance_id)
{
	//TODO error check obj_id
	LList list = objects_meta[type->ID].active;

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
	return llist_size(objects_meta[type->ID].active);
}

void instance_remove(instance *obj)
{
	//TODO restructure removal method completely
	llist_remove(objects_meta[obj->TYPE->ID].active, obj);
}


