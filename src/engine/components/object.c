#include "object.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <signal.h>

#include "../io/waffle_utils.h"

#define INS_MAGIC_COOKIE 0xA2F4C681

//TODO split active instances from list of objects?
typedef struct {
	object_id *obj;
	int count;

	//LList alive;
	LList active;
	//LList destroyed;

	LList pool;
} object_info;

static int component_index = 0;

int object_count = 0;
static int debug_allocs = 0;
static int debug_frees = 0;
object_info objects_meta[OBJECT_MAX_OBJECTS];

static LList ins2destroy;
static int terminating = 0;

static void destroy_func(instance* obj)
{
	obj->TYPE->call.on_remove(obj);
}

static void free_dead_func(void *obj)
{
#ifdef DEBUG_MEMORY
	SDL_Log("TMP: removing %p[%s]",obj, ((instance *)obj)->TYPE->NAME);
#endif
	free(obj);
	++debug_frees;
}

static void free_active_func(instance *obj)
{
	if (!((instance*)obj)->destroyed) {
		obj->TYPE->call.on_destroy(obj);
	}
	destroy_func(obj);
	free_dead_func(obj);
}

int object_register(object_id *obj)
{
	int id = object_count++;

	if (object_count >= OBJECT_MAX_OBJECTS) {
		fprintf(stderr, "MAXIMUM NUMBER OF OBJECT DEFINITIONS EXCEEDED!\n");
		exit(-1);
	}

	/* set read-only object info*/
	*(int *)(&obj->ID) = id;

	objects_meta[id].obj = obj;
	objects_meta[id].count = 0;

	LList alive, active, destroyed;

	//alive = llist_create_group();
	active = llist_create();
	//destroyed = llist_create();

	//llist_add(alive, active);
	//llist_add(alive, destroyed);

	//objects_meta[id].alive = alive;
	objects_meta[id].active = active;
	//objects_meta[id].destroyed = destroyed;

	objects_meta[id].pool = llist_create();

	llist_set_remove_callback(objects_meta[id].active, (void (*) (void *))destroy_func);

	return id;
}

object_id *object_by_name(const char *obj_name)
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

object_id *object_by_id(int id)
{
	if (id >= 0 && id < object_count)
		return objects_meta[id].obj;
	else
		return NULL;
}

LList object_get_instances(const object_id *type)
{
	return objects_meta[type->ID].active;
}

int component_register(int pointer_count)
{
	int empty_index = component_index;
	component_index += pointer_count;

	if (component_index < OBJECT_MAX_COMPONENTS) {
		return empty_index;
	} else {
		component_index = empty_index;
		fprintf(stderr, "ERROR: COMPONENT HOLDER FULL!\n");
		return -1;
	}
}

instance *instance_super_malloc(object_id *type)
{
	LList list = objects_meta[type->ID].pool;
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

instance *instance_create(object_id *type, const void *param, cpVect p, cpVect v)
{
	if (terminating) {
		return NULL;
	}

	instance *ins = instance_super_malloc(type);

	if (p.x != p.x || p.y != p.y || v.x != v.x || v.y != v.y) {
		SDL_Log("ERROR in instance_create: params NaN!");
		//raise(SIGKILL);
	}

	ins->p_start = p;
	ins->v_start = v;

	//TODO remove x,y,hs,vs in ins.
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


#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

void object_init(void) {
	ins2destroy = llist_create();
	terminating = 0;
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
void instance_iterate(void (*f)(instance *, void *data), void *data)
{
	int obj_id;
	object_info *obj = objects_meta;

	for (obj_id = 0; obj_id < object_count; obj_id++) {
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
			object_info *obj = objects_meta + type->ID;
			llist_iterate_func(obj->active, (void (*)(void *, void *))f, data);
		}
	}
}

void instance_iterate_comp(int comp_index, void (*f)(instance *, void *data), void *data)
{
	int obj_id;
	object_info *obj = objects_meta;

	for (obj_id = 0; obj_id < object_count; obj_id++) {
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

/* removes all nodes from all lists */
void instance_clear(void)
{
	int obj_id;
	object_info *obj = objects_meta;

	for (obj_id = 0; obj_id < object_count; ++obj_id, ++obj) {
		llist_clear(obj->active);
		obj->count = 0;
	}
}

/* frees all nodes in all object lists */
void object_destroy(void)
{
	int obj_id;
	object_info *obj = objects_meta;

	terminating = 1;

	for (obj_id = 0; obj_id < object_count; ++obj_id, ++obj) {
		//llist_destroy(obj->alive);
		llist_set_remove_callback(obj->active, free_active_func);
		llist_destroy(obj->active);
		llist_set_remove_callback(obj->pool, free_dead_func);
		llist_destroy(obj->pool);
		obj->count = 0;
	}

	SDL_Log("INFO: %d of %d instances freed!", debug_frees, debug_allocs);

	//TODO remove all object definitions
}

void instance_update(instance *ins) {
	if (!ins->destroyed) { //TODO avgjøre om destroyed skal bestemme on update skal kalles eller ikke
		ins->TYPE->call.on_update(ins);
	}
}


//TODO swap arguments?
instance *instance_nearest(cpVect pos, object_id *type)
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

instance *instance_first(object_id *type)
{
	//TODO error check obj_id
	return (instance *) llist_first(objects_meta[type->ID].active);
}

instance *instance_n(object_id *type, int n)
{
	//TODO error check obj_id
	return (instance *) llist_at_index(objects_meta[type->ID].active, n);
}

instance *instance_last(object_id *type)
{
	//TODO error check obj_id
	return (instance *) llist_last(objects_meta[type->ID].active);
}

instance *instance_by_id(object_id *type, int instance_id)
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

int instance_count(object_id *type)
{
	//TODO error check obj_id
	return llist_size(objects_meta[type->ID].active);
}

void instance_destroy(instance *ins)
{
	if (!ins->destroyed) {
		*((int *) &ins->destroyed) = 1;
		llist_add(ins2destroy, ins);
	}
}

void instance_revive(instance *ins)
{
	*((int *) &ins->destroyed) = 0;
}

void instance_remove(instance *ins)
{
	*((int *) &ins->alive) = 0;
}

static void instance_remove_dead(instance *ins, void *unused)
{
	if (!ins->alive) {
		llist_remove(objects_meta[ins->TYPE->ID].active, ins);
		instance_super_free(ins);
	}
}

void instance_poststep(void) // "hidden" method called from statesystem
{
	instance *ins;
	while ((ins = llist_pop(ins2destroy)) != NULL) {
		ins->TYPE->call.on_destroy(ins);
	}

	instance_iterate(instance_remove_dead, NULL);
}



static void add_shape(cpSpace *space, cpShape *shape, cpFloat friction, cpFloat elasticity)
{
	cpSpaceAddShape(space, shape);
	cpShapeSetFriction(shape, friction);
	cpShapeSetElasticity(shape, elasticity);
}

void we_shape_collision(cpShape *shape, cpCollisionType type, cpLayers layers, cpGroup group)
{
	cpShapeSetCollisionType(shape, type);
	cpShapeSetLayers(shape, layers);
	cpShapeSetGroup(shape, group);
}

cpShape *we_add_circle_shape(cpSpace *space, cpBody *body, cpFloat radius, cpFloat friction, cpFloat elasticity)
{
	cpShape *shape = cpCircleShapeNew(body, radius, cpvzero);
	add_shape(space, shape, friction, elasticity);
	return shape;
}

cpShape *we_add_box_shape(cpSpace *space, cpBody *body, cpFloat width, cpFloat height, cpFloat friction, cpFloat elasticity)
{
	cpShape *shape = cpBoxShapeNew(body, width, height);
	add_shape(space, shape, friction, elasticity);
	return shape;
}



void we_space_remove_shape(cpBody *body, cpShape *shape, void *space)
{
    cpSpaceRemoveShape((cpSpace *) space, shape);
    cpShapeFree(shape);
}
void we_space_remove_constraint(cpBody *body, cpConstraint *constraint, void *space)
{
    cpSpaceRemoveConstraint((cpSpace *) space, constraint);
    cpConstraintFree(constraint);
}

void we_body_remove_shapes(cpSpace *space, cpBody *body)
{
	cpBodyEachShape(body, we_space_remove_shape, space);
}

void we_body_remove_constraints(cpSpace *space, cpBody *body)
{
	cpBodyEachConstraint(body, we_space_remove_constraint, space);
}

void we_body_remove(cpSpace *space, cpBody **body)
{
	we_body_remove_shapes(space, *body);
	we_body_remove_constraints(space, *body);
	cpSpaceRemoveBody(space, *body);
	cpBodyFree(*body);
	*body = NULL;
}

void we_body_remove_static(cpSpace *space, cpBody **body)
{
	we_body_remove_shapes(space, *body);
	we_body_remove_constraints(space, *body);
	cpBodyFree(*body);
	*body = NULL;
}
