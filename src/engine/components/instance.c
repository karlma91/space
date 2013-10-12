#include "object.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <signal.h>

#include "../io/waffle_utils.h"
#include "../engine.h"
#include "../state/statesystem.h"

#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

#define  err_ins(x) \
	(x == NULL || *((int *) x) != INS_MAGIC_COOKIE)
#define  err_obj(x) \
	(x == NULL || *((int *) x) != OBJ_MAGIC_COOKIE)


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


instance *instance_create(object_id *type, const void *param, cpVect p, cpVect v)
{
	if (objsys_terminating) {
		return NULL;
	}


	extern instance *instance_super_malloc(object_id *type);
	instance *ins = instance_super_malloc(type);

	if (p.x != p.x || p.y != p.y || v.x != v.x || v.y != v.y) {
		SDL_Log("ERROR in instance_create: params NaN!");
		//raise(SIGKILL);
	}

	ins->p_start = p;
	ins->v_start = v;

	//TODO init body
	//TODO set body pos and vel

	/* sets default values */
	*((int *) &ins->alive) = 1;

	if (param) {
		instance_set_param(ins, param);
	}

	instance_add(ins);

	type->call.on_create(ins);

	return ins;
}


/* add object into its corresponding list */
void instance_add(instance* ins)
{
	int obj_id = ins->TYPE->ID;
	LList active = current_objects->objects_meta[obj_id].active;

	if (llist_add(active, ins)) {
		int count = current_objects->objects_meta[obj_id].count++;
		ins->instance_id = count;
	}
}

/* iterate through all lists of object */
void instance_iterate(void (*f)(instance *, void *data), void *data)
{
	int obj_id;
	object_info *obj = current_objects->objects_meta;

	for (obj_id = 0; obj_id < object_getcount(); obj_id++) {
		llist_iterate_func(obj->active, (void (*)(void *, void *))f, data);
		++obj;
	}
}

/* iterate through one type of object */
void instance_iterate_type(object_id *type, void (*f)(instance *, void *data), void *data) {
	if (type) {
		if (err_obj(type)) {
			SDL_Log("ERROR: in list_iterate_type: Invalid object type %p\n", type);
			exit(-1);
		} else {
			object_info *obj = current_objects->objects_meta + type->ID;
			llist_iterate_func(obj->active, (void (*)(void *, void *))f, data);
		}
	}
}

void instance_iterate_comp(int comp_index, void (*f)(instance *, void *data), void *data)
{
	int obj_id;
	object_info *obj = current_objects->objects_meta;

	for (obj_id = 0; obj_id < object_getcount(); obj_id++) {
		//TODO make use of bitsets in object, much more reliable than just checking first instance
		instance *ins = llist_first(obj->active);
		if (ins) {
			if (ins->components[comp_index]) {
				llist_iterate_func(obj->active, (void (*)(void *, void *))f, data);
			}
		}
		++obj;
	}
}

void instance_update(instance *ins) {
	if (!ins->destroyed) { //TODO avgjøre om destroyed skal bestemme on update skal kalles eller ikke
		ins->TYPE->call.on_update(ins);
	} else {
		ins->time_destroyed += dt;
	}
}


//TODO swap arguments?
instance *instance_nearest(cpVect pos, object_id *type)
{
	//TODO error check obj_id

	instance *target = NULL;
	float min_length = FLT_MAX;
	float length;
	LList list = current_objects->objects_meta[type->ID].active;

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


instance *instance_first(object_id *type)
{
	//TODO error check obj_id
	return (instance *) llist_first(current_objects->objects_meta[type->ID].active);
}

instance *instance_n(object_id *type, int n)
{
	//TODO error check obj_id
	return (instance *) llist_at_index(current_objects->objects_meta[type->ID].active, n);
}

instance *instance_last(object_id *type)
{
	//TODO error check obj_id
	return (instance *) llist_last(current_objects->objects_meta[type->ID].active);
}

instance *instance_by_id(object_id *type, int instance_id)
{
	//TODO error check obj_id
	LList list = current_objects->objects_meta[type->ID].active;

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

int instance_count(object_id *type)
{
	//TODO error check obj_id
	return llist_size(current_objects->objects_meta[type->ID].active);
}

void instance_destroy(instance *ins)
{
	if (!ins->destroyed) {
		*((int *) &ins->destroyed) = 1;
		llist_add(current_objects->ins2destroy, ins);
	}
}

void instance_revive(instance *ins)
{
	*((int *) &ins->destroyed) = 0;
	*((int *) &ins->alive) = 1;
}

void instance_remove(instance *ins)
{
	*((int *) &ins->alive) = 0;
}





