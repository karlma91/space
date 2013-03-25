#ifndef OBJECTS_H_
#define OBJECTS_H_

#include "chipmunk.h"

/* ID number of each object group*/
enum OBJECT_ID {
	ID_PLAYER,
	ID_GROUND,
	ID_TANK_FACTORY,
	ID_TANK,
	ID_BULLET_PLAYER,
	ID_BULLET_ENEMY,
	ID_ROBOTARM,

	ID_COUNT
};

typedef struct _object_data object_data;

typedef struct {
	enum OBJECT_ID ID;
	void (*init)(object_data *obj);
	void (*update)(object_data *obj);
	void (*render)(object_data *obj);
	void (*destroy)(object_data *obj);
} object_group_preset;

struct _object_data {
	object_group_preset *preset;
	int instance_id;
	int *remove;
	int alive;
	cpBody *body;
};

//TODO? move all object_param_* and object_group_* typedefs in here? (to avoid circular dependencies)
/* object parameters */
typedef struct _object_param_player object_param_player;
typedef struct _object_param_tank object_param_tank;
typedef struct _object_param_tankfactory object_param_tankfactory;


/* layers for objects */
extern int LAYER_PLAYER;
extern int LAYER_TANK;
extern int LAYER_TANK_FACTORY;
extern int LAYER_PLAYER_BULLET;
extern int LAYER_ENEMY_BULLET;

extern void objects_init();
extern void objects_add(object_data *);
extern void objects_iterate(void (*f)(object_data *));
extern void objects_iterate_type(void (*f)(object_data *), int ID);
extern void objects_destroy();
extern inline void objects_remove(object_data *);

extern void objects_nearest_x_two(object_data *object, int obj_id, object_data **left, object_data **right, cpFloat *left_distance, cpFloat *right_distance);
extern object_data *objects_nearest(cpVect pos, int obj_id);
extern object_data *objects_first(int obj_id);
extern object_data *objects_n(int obj_id, int n);
extern object_data *objects_last(int obj_id);
extern object_data *objects_by_id(int obj_id, int instance_id);

extern int objects_count(int obj_id);

#endif /* OBJECTS_H_ */
