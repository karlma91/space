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


//TODO split active instances from list of objects?

static int component_index = 0;

int object_count = 0;

object_id *object_defs[OBJECT_MAX_OBJECTS];

int object_getcount(void)
{
	return object_count;
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

	object_defs[id] = obj;

	return id;
}


object_id *object_by_name(const char *obj_name)
{
	int obj_id;
	object_info *obj_meta = current_objects->objects_meta;

	for (obj_id = 0; obj_id < object_count; obj_id++) {
		if (strcasecmp(obj_name, object_defs[obj_id]->NAME) == 0) {
			return object_defs[obj_id];
		}
		++obj_meta;
	}

	return NULL;
}

object_id *object_by_id(int id)
{
	if (id >= 0 && id < object_count)
		return object_defs[id];
	else
		return NULL;
}

LList object_get_instances(const object_id *type)
{
	return current_objects->objects_meta[type->ID].active;
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

void object_init(void) {
}

/* frees all nodes in all object lists */
void object_destroy(void)
{
	//int obj_id;
	//TODO remove instances
	/*
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


	//TODO destroy llist_destroy(object_system);
	//TODO remove all object definitions
	 */
	//SDL_Log("INFO: %d of %d instances freed!", debug_frees, debug_allocs);
}


/*****
 * Hjelpemetoder for opprettelse av bodies
 *****/

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


/*****
 * Hjelpemetoder for fjerning av bodies
 *****/

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
