#ifndef LIST_H_
#define LIST_H_

#include "chipmunk.h"

typedef struct object_ object;

struct obj_type {
	int ID;
	void (*init)(object *obj);
	void (*update)(object *obj);
	void (*render)(object *obj);
	void (*destroy)(object *obj);
};

struct object_ {
	struct obj_type *type;
	int instance_id;
	int *remove;
	int alive;
	cpBody *body;
};

/* All collision types for objects */
enum OBJECT_ID {
	ID_PLAYER,
	ID_GROUND,
	ID_TANK_FACTORY,
	ID_TANK,
	ID_BULLET_PLAYER,
	ID_BULLET_ENEMY,

	ID_COUNT
};

/* layers for objects */
extern  int LAYER_PLAYER;
extern  int LAYER_TANK;
extern  int LAYER_TANK_FACTORY;
extern  int LAYER_PLAYER_BULLET;
extern  int LAYER_ENEMY_BULLET;

extern void list_init();
extern void list_add(object *);
extern void list_iterate(void (*f)(object *));
extern void list_iterate_type(void (*f)(object *), int ID);
extern void list_destroy();

extern object *objects_nearest(cpVect pos, int obj_id);
extern object *objects_first(int obj_id);
extern object *objects_n(int obj_id, int n);
extern object *objects_last(int obj_id);
extern object *objects_by_id(int obj_id, int instance_id);

#endif /* LIST_H_ */
