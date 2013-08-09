#include <stdlib.h>
#include "chipmunk.h"

#ifndef GENERAL_OBJECT_FUNCS
#define GENERAL_OBJECT_FUNCS 1

//#define INSTANCE_DATA_SIZE 1024;

void object_init();
void object_destroy();

typedef struct _instance instance;

typedef struct {
	const int ID;
	//const int DATA_SIZE;

	const struct {
		void (*on_create)(instance *obj);
		void (*on_update)(instance *obj);
		void (*on_render)(instance *obj);
		void (*on_destroy)(instance *obj);
	} call;
} obj_id; /* per-object type variables */

struct _instance {
	obj_id * object_id;

	struct {

	} components;

	//char instance_data[INSTANCE_DATA_SIZE]; ?
}; /* per-instance variables */

instance *instance_create(obj_id id, cpVect pos, cpVect speed);
//object_create(obj_id id, cpVect pos, cpVect speed, argument struct *?);



#endif /* GENERAL_OBJECT_FUNCS */

/*** PER-OBJECT ***/
#ifdef OBJ_VAR_NAME

void object_priv_register(obj_id *id);

//TODO use OBJ_VAR_NAME as type for argument obj?
static void on_create(instance *obj);
static void on_update(instance *obj);
static void on_render(instance *obj);
static void on_destroy(instance *obj);

static obj_id this = {
	.ID = -1,
	.call = {
		on_create,
		on_update,
		on_render,
		on_destroy
	}
};

#define REGISTER_OBJ() object_priv_register(&this)

#endif

