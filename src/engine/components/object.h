#include <stdlib.h>
#include "chipmunk.h"

#include "hpbar.h"
#include "../graphics/sprite.h"
#include "../data/llist.h"

#ifndef GENERAL_OBJECT_FUNCS
#define GENERAL_OBJECT_FUNCS 1

//#define INSTANCE_DATA_SIZE 1024;
#define OBJ_STRUCT typedef struct {instance ins;

void object_init();
void object_clear();
void object_destroy();

typedef struct instance instance;
typedef struct object_id object_id;

typedef struct { //TODO move: WARNING: exposed internal data structure
	object_id *obj;
	int count;
	LList active;
	LList pool;
} object_info;

struct object_id {
	const int ID;
	const size_t size;
	const object_info *info;

	const struct {
		void (*on_create)(instance *obj);
		void (*on_update)(instance *obj);
		void (*on_render)(instance *obj);
		void (*on_destroy)(instance *obj);
	} call;
}; /* per-object type variables */

struct instance {
	object_id *obj_id;

	struct {
		hpbar hp;
	} components;

	int alive;
	int instance_id;
	int disabled;
	int destroyed;
	cpBody *body;
	sprite spr;

}; /* per-instance variables */


instance *instance_create(int obj_id, float x, float y, float hs, float vs);
//object_create(obj_id id, cpVect pos, cpVect speed, argument struct *?);

extern instance *instance_super_malloc(int obj_id, size_t); /* allocates and initializes a new object of size_t */
extern void object_super_free(instance *);


extern void instance_add(instance *);
extern void instance_iterate(void (*f)(instance *));
extern void instance_iterate_type(void (*f)(instance *), int ID);
extern void instance_remove(instance *);

extern void instance_nearest_x_two(instance *object, int obj_id, instance **left, instance **right, cpFloat *left_distance, cpFloat *right_distance);
extern instance *instance_nearest(cpVect pos, int obj_id);
extern instance *instance_first(int obj_id);
extern instance *instance_n(int obj_id, int n);
extern instance *instance_last(int obj_id);
extern instance *instance_by_id(int obj_id, int instance_id);

extern int instance_count(int obj_id);


#endif /* GENERAL_OBJECT_FUNCS */

/*** PER-OBJECT ***/
#ifdef OBJ_TYPE_NAME
#ifndef PER_OBJECT_CODEBLOCK
#define PER_OBJECT_CODEBLOCK 1

int object_priv_register(object_id *id);

//TODO use OBJ_VAR_NAME as type for argument obj?
static void on_create(instance *obj);
static void on_update(instance *obj);
static void on_render(instance *obj);
static void on_destroy(instance *obj);

static object_id this = {
	.ID = -1,
	.size = sizeof(OBJ_TYPE_NAME),
	.call = {
		on_create,
		on_update,
		on_render,
		on_destroy
	}
};

#define REGISTER_OBJ() object_priv_register(&this)

#endif /* PER_OBJECT_CODEBLOCK */
#endif /* OBJ_TYPE_NAME */

