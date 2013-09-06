#include <stdlib.h>
#include "chipmunk.h"

#include "hpbar.h"
#include "../graphics/sprite.h"
#include "../data/llist.h"

#define OBJ_MAGIC_COOKIE 0xB305D7A2

#ifndef GENERAL_OBJECT_FUNCS
#define GENERAL_OBJECT_FUNCS 1
#endif

#ifndef OBJECT_MAX_COMPONENTS
#define OBJECT_MAX_COMPONENTS 30
#endif

#ifndef OBJECT_MAX_OBJECTS
#define OBJECT_MAX_OBJECTS 20 //FIXME TMP define in port progress


void object_init(void);
void object_destroy(void);

typedef struct instance instance;
typedef struct object object_id;

struct object {
	const int OBJ_IDENTIFIER;
	const int ID;
	const char *NAME;
	const size_t SIZE;
	const size_t P_SIZE;

	//const int components_mask;
	const struct {
		void (*init)(instance *obj);
		void (*on_create)(instance *obj);
		void (*on_update)(instance *obj);
		void (*on_render)(instance *obj);
		void (*on_destroy)(instance *obj);
		void (*on_remove)(instance *obj);
	} call;
}; /* per-object type variables */

struct instance {
	const int INS_IDENTIFIER;
	const object_id *TYPE;

	int active_components;

	void *components[OBJECT_MAX_COMPONENTS];

	const int alive;
	const int destroyed;
	int disabled;
	int instance_id;

	cpVect p_start, v_start;

	cpBody *body;
	sprite spr;

}; /* per-instance variables */

int component_register(int pointer_count);

//TODO support filtered iterator

instance *instance_create(object_id *type, const void *param, float x, float y, float hs, float vs);
instance *instance_super_malloc(object_id *type); //TODO hide from user?
void instance_super_free(instance *);

void instance_add(instance *);
void instance_iterate(void (*f)(instance *, void *data), void *data);
void instance_iterate_type(object_id *type, void (*f)(instance *, void *data), void *data);
void instance_iterate_comp(int comp_index, void (*f)(instance *, void *data), void *data);
void instance_update(instance *);
void instance_destroy(instance *);
void instance_revive(instance *);
void instance_remove(instance *);
int instance_set_param(instance *, const void *param);
void instance_clear(void);

instance *instance_nearest(cpVect pos, object_id *type);
instance *instance_first(object_id *type);
instance *instance_n(object_id *type, int n);
instance *instance_last(object_id *type);
instance *instance_by_id(object_id *type, int instance_id);

#define instance_render(ins) ins->TYPE->call.on_render(ins)

//int instance_count_active(object_id *type); //TODO implement this method, returning the number of instances that are not destroyed
int instance_count(object_id *type);
int object_register(object_id *obj);
object_id *object_by_name(const char *obj_name);
object_id *object_by_id(int id);
LList object_get_instances(const object_id *type);

#define OBJ_TYPE_3( name ) obj_ ## name
#define OBJ_TYPE_2( name ) OBJ_TYPE_3( name )

#define OBJ_PARAM_3( name ) obj_param_ ## name
#define OBJ_PARAM_2( name ) OBJ_PARAM_3( name )

#define OBJ_ID_3( name ) obj_id_ ## name
#define OBJ_ID_2( name ) OBJ_ID_3( name )


#define OBJECT_REGISTER(name) object_register (OBJ_ID_2(name))
#define OBJECT_DECLARE(name) \
	extern object_id *OBJ_ID_2(name); \
	typedef struct OBJ_TYPE_2(name) OBJ_TYPE_2(name); \
	typedef struct OBJ_PARAM_2(name) OBJ_PARAM_2(name)


#define OBJ_START(name) struct OBJ_TYPE_2(name) {instance data; struct OBJ_PARAM_2(name) param;
#define OBJ_END };

#define PARAM_START(name) struct OBJ_PARAM_2(name) {
#define PARAM_END };
#define PARAM_EMPTY(name) PARAM_START(name) PARAM_END

#define COMPONENT(obj, cmp, type) ((type) (((instance *)obj)->components[CMP_##cmp]))
#define COMPONENT_SET(obj, cmp, value) (((instance *)obj)->components[CMP_##cmp]) = value
#define REGISTER_CMP(cmp, count) CMP_##cmp = component_register(count);


/* Chipmunk shape and body helper functions */
cpShape *we_add_circle_shape(cpSpace *space, cpBody *body, cpFloat radius, cpFloat friction, cpFloat elasticity);
cpShape *we_add_box_shape(cpSpace *space, cpBody *body, cpFloat width, cpFloat height, cpFloat friction, cpFloat elasticity);
void we_shape_collision(cpShape *shape, cpCollisionType type, cpLayers layers, cpGroup group);

void we_body_remove(cpSpace *space, cpBody **body);
void we_body_remove_shapes(cpSpace *space, cpBody *body);
void we_body_remove_constraints(cpSpace *space, cpBody *body);

#endif /* GENERAL_OBJECT_FUNCS */

