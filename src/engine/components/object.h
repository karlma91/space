#include <stdlib.h>
#include "chipmunk.h"

#include "hpbar.h"
#include "../graphics/sprite.h"
#include "../data/llist.h"

#ifndef GENERAL_OBJECT_FUNCS
#define GENERAL_OBJECT_FUNCS 1


#define OBJ_TYPE_3( type ) \
	obj_ ## type
#define OBJ_TYPE_2( name ) \
	OBJ_TYPE_3( name )

#define OBJ_ID_3( id ) \
	obj_id_ ## id
#define OBJ_ID_2( name ) \
	OBJ_ID_3( name )

//#define INSTANCE_DATA_SIZE 1024;
#define OBJ_STRUCT_START typedef struct {instance ins;

void object_init();
void object_clear();
void object_destroy();

typedef struct instance instance;
typedef struct object object;

typedef struct { //TODO move: WARNING: exposed internal data structure
	object *obj;
	int count;
	LList active;
	LList pool;
} object_info;

struct object {
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
	object *obj_id;

	int active_components;
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

int object_register(object *obj);

#define OBJECT_REGISTER(name) object_register (&OBJ_ID_2(name))
#define OBJECT_DECLARE(name) extern object OBJ_ID_2(name)

#endif /* GENERAL_OBJECT_FUNCS */

/*** PER-OBJECT ***/
#ifdef OBJ_NAME
#ifndef PER_OBJECT_CODEBLOCK
#define PER_OBJECT_CODEBLOCK 1

#define OBJ_TYPE OBJ_TYPE_2(OBJ_NAME)
#define OBJ_ID OBJ_ID_2(OBJ_NAME)

static void on_create(OBJ_TYPE *obj);
static void on_update(OBJ_TYPE *obj);
static void on_render(OBJ_TYPE *obj);
static void on_destroy(OBJ_TYPE *obj);

object OBJ_ID = {
	.ID = -1,
	.size = sizeof(OBJ_TYPE),
	.call = {
		on_create,
		on_update,
		on_render,
		on_destroy
	}
};

#endif /* PER_OBJECT_CODEBLOCK */
#endif /* OBJ_TYPE_NAME */

