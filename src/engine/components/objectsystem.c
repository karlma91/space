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


#define DEBUG_MEMORY

static int debug_allocs = 0;
static int debug_frees = 0;

int objsys_terminating = 0;

static LList ll_systems = NULL;


static void destroy_func(instance* obj)
{
	obj->TYPE->call.on_remove(obj);
}

static void free_dead_func(void *obj)
{
#ifdef DEBUG_MEMORY
	SDL_Log("TMP: removing %p[%s]",obj, ((instance *)obj)->TYPE->NAME);
#endif
	if (!((instance *)obj)->destroyed) {
		((instance *)obj)->TYPE->call.on_destroy(obj);
	}
	destroy_func(obj);
	free(obj);
	++debug_frees;
}

static void free_active_func(instance *obj)
{
	if (!((instance*)obj)->destroyed) {
		obj->TYPE->call.on_destroy(obj);
	}
	free_dead_func(obj);
}

/* removes all nodes from all lists */
void objectsystem_clear(void)
{
	int obj_id;
	object_info *obj = current_objects->objects_meta;

	int term_state = objsys_terminating;
	objsys_terminating = 1;

	for (obj_id = 0; obj_id < object_getcount(); ++obj_id, ++obj) {
		llist_iterate_func(obj->active, instance_destroy, NULL);
		llist_iterate_func(obj->active, instance_remove, NULL);
		obj->count = 0;
	}
	void instance_poststep(void);
	instance_poststep();

	objsys_terminating = term_state;
}

static void instance_remove_dead(instance *ins, void *unused)
{
	if (!ins->alive) {
		llist_remove(current_objects->objects_meta[ins->TYPE->ID].active, ins);
		extern void instance_super_free(instance *ins);
		instance_super_free(ins);
	}
}
void instance_poststep(void) // "hidden" method called from statesystem
{
	instance *ins;
	while ((ins = llist_pop(current_objects->ins2destroy)) != NULL) {
		ins->TYPE->call.on_destroy(ins);
	}

	instance_iterate(instance_remove_dead, NULL);
}

void objectsystem_init(void)
{
	ll_systems = llist_create();
	objsys_terminating = 0;
}

void objectsystem_destroy(void)
{
	objsys_terminating = 1;
	llist_destroy(ll_systems);
#if !ARCADE_MODE
	SDL_Log("INFO: %d of %d instances freed!", debug_frees, debug_allocs);
#endif
}


object_system *objectsystem_new(void)
{
	object_system *system = calloc(1, sizeof *system);
	system->ins2destroy = llist_create();

	llist_add(ll_systems, system);

	int id;
	for (id=0; id < object_getcount(); id++) {
		system->objects_meta[id].active = llist_create();
		system->objects_meta[id].pool = llist_create();
		llist_set_remove_callback(system->objects_meta[id].active, (ll_rm_callback)destroy_func);
	}

	system->space = cpSpaceNew();
	current_space = system->space;

	return system;
}

void objectsystem_free(object_system *system)
{
	if (!system)
		return;

	//TODO call objectsystem_clear first
	current_objects = system;
	//objectsystem_clear();

	int term_state = objsys_terminating;
	objsys_terminating = 1;


	llist_destroy(system->ins2destroy);

	int id;
	for (id=0; id < object_getcount(); id++) {
		llist_set_remove_callback(system->objects_meta[id].active, (ll_rm_callback)free_active_func);
		llist_set_remove_callback(system->objects_meta[id].pool, (ll_rm_callback)free_dead_func);
		llist_destroy(system->objects_meta[id].active);
		llist_destroy(system->objects_meta[id].pool);
	}

	cpSpaceDestroy(system->space);
	llist_remove(ll_systems, system);

	objsys_terminating = term_state;
	free(system);
}


instance *instance_super_malloc(object_id *type)
{
	LList list = current_objects->objects_meta[type->ID].pool;
	instance *ins = llist_first(list);

	int size = type->SIZE;

	if (ins == NULL) {
		ins = calloc(1, size);
		++debug_allocs;
#ifdef DEBUG_MEMORY
		SDL_Log("TMP: allocating %p[%s] size: %u",ins, type->NAME, size);
#endif
	} else {
		llist_remove(list, (void *)ins);
		memset(ins,0,size);
	}

	/* set read-only instance data */
	*((object_id **)&ins->TYPE) = type;
	*((int *)&ins->INS_IDENTIFIER) = INS_MAGIC_COOKIE;

	return ins;
}

#define  err_ins(x) \
	(x == NULL || *((int *) x) != INS_MAGIC_COOKIE)
void instance_super_free(instance *ins)
{
	if (err_ins(ins)) {
		SDL_Log("ERROR: Invalid instance %p", ins);
		exit(-1);
	}

	int obj_id = ins->TYPE->ID;
	LList list = current_objects->objects_meta[obj_id].pool;
	llist_add(list, (void *)ins);
#ifdef DEBUG_MEMORY
	SDL_Log( "Info: obj_%s got %d unused allocations\n", ins->TYPE->NAME, llist_size(list));
#endif
}


