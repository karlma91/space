#include <stdlib.h>
#include "chipmunk.h"

#include "hpbar.h"
#include "../graphics/sprite.h"
#include "../data/llist.h"

#ifndef GENERAL_OBJECT_FUNCS
#define GENERAL_OBJECT_FUNCS 1

#define OBJ_MAGIC_COOKIE 0xB305D7A2

#define ID_COUNT 7 //TMP define in port progress


void object_init();
void object_clear();
void object_destroy();

typedef struct instance instance;
typedef struct object object;

//TODO split active instances from list of objects?
typedef struct { //TODO move: WARNING: exposed internal data structure
	object *obj;
	int count;
	LList active;
	LList pool;
} object_info;

struct object {
	const int OBJ_IDENTIFIER;
	const int ID;
	const char *NAME;
	const size_t SIZE;
	const size_t P_SIZE;
	const object_info *info;

	const struct {
		void (*on_create)(instance *obj);
		void (*on_update)(instance *obj);
		void (*on_render)(instance *obj);
		void (*on_destroy)(instance *obj);
	} call;
}; /* per-object type variables */

struct instance {
	const int INS_IDENTIFIER;
	const object *TYPE;

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

instance *instance_create(object *type, const void *param, float x, float y, float hs, float vs);
instance *instance_super_malloc(object *type); //TODO hide from user?
void instance_super_free(instance *);

void instance_add(instance *);
void instance_iterate(void (*f)(instance *));
void instance_iterate_type(void (*f)(instance *), object *type);
void instance_remove(instance *);
int instance_set_param(instance *, const void *param);

instance *instance_nearest(cpVect pos, object *type);
instance *instance_first(object *type);
instance *instance_n(object *type, int n);
instance *instance_last(object *type);
instance *instance_by_id(object *type, int instance_id);

#define instance_update(ins) ins->TYPE->call.on_update(ins)
#define instance_render(ins) ins->TYPE->call.on_render(ins)

int instance_count(object *type);
int object_register(object *obj);


#define OBJ_TYPE_3( name ) obj_ ## name
#define OBJ_TYPE_2( name ) OBJ_TYPE_3( name )

#define OBJ_PARAM_3( name ) obj_param_ ## name
#define OBJ_PARAM_2( name ) OBJ_PARAM_3( name )

#define OBJ_ID_3( name ) obj_id_ ## name
#define OBJ_ID_2( name ) OBJ_ID_3( name )


#define OBJECT_REGISTER(name) object_register (OBJ_ID_2(name))
#define OBJECT_DECLARE(name) \
	extern object *OBJ_ID_2(name); \
	typedef struct OBJ_TYPE_2(name) OBJ_TYPE_2(name); \
	typedef struct OBJ_PARAM_2(name) OBJ_PARAM_2(name)


#define OBJ_START(name) struct OBJ_TYPE_2(name) {instance ins; struct OBJ_PARAM_2(name) param;
#define OBJ_END };

#define PARAM_START(name) struct OBJ_PARAM_2(name) {
#define PARAM_END };
#define PARAM_EMPTY(name) PARAM_START(name) PARAM_END

#endif /* GENERAL_OBJECT_FUNCS */

/*** PER-OBJECT ***/
#ifdef OBJ_NAME
#ifndef PER_OBJECT_CODEBLOCK
#define PER_OBJECT_CODEBLOCK 1

#define STRINGIFY_2(str) #str
#define STRINGIFY(str) STRINGIFY_2(str)

#define OBJ_TYPE OBJ_TYPE_2(OBJ_NAME)
#define OBJ_PARAM_TYPE OBJ_PARAM_2(OBJ_NAME)
#define OBJ_ID OBJ_ID_2(OBJ_NAME)

static void on_create(OBJ_TYPE *obj);
static void on_update(OBJ_TYPE *obj);
static void on_render(OBJ_TYPE *obj);
static void on_destroy(OBJ_TYPE *obj);

static object this = {
	.OBJ_IDENTIFIER = OBJ_MAGIC_COOKIE,
	.ID = -1,
	.NAME = STRINGIFY(OBJ_NAME),
	.SIZE = sizeof(OBJ_TYPE),
	.P_SIZE = sizeof(OBJ_PARAM_TYPE),
	.call = {
		(void (*)(instance *)) on_create,
		(void (*)(instance *)) on_update,
		(void (*)(instance *)) on_render,
		(void (*)(instance *)) on_destroy
	}
};
object *OBJ_ID = &this;

#endif /* PER_OBJECT_CODEBLOCK */
#endif /* OBJ_TYPE_NAME */

