#include "object_types.h"

#include "../level.h"
#include "we_data.h"

/* Bit-layers */
#define L1 (1 << 0)
#define L2 (1 << 1)
#define L3 (1 << 2)
#define L4 (1 << 3)
#define L5 (1 << 4)
#define L6 (1 << 5)
#define L7 (1 << 6)
#define L8 (1 << 7)

/* Collision Layer Setup */
int LAYER_PLAYER        = L1 | L2 | L3 | L4;
int LAYER_ENEMY         = L1 | L5;
int LAYER_BUILDING      = L2 | L6 | L7 | L8;
int LAYER_BULLET_PLAYER = L5 | L6;
int LAYER_BULLET_ENEMY  = L3;
int LAYER_SHIELD_PLAYER = 0;
int LAYER_SHIELD_ENEMY  = 0;
int LAYER_PICKUP        = L4 | L8;

/* registration of all objects */
void object_types_init(void)
{
	OBJECT_REGISTER(template);

	OBJECT_REGISTER(bullet);
	OBJECT_REGISTER(player);
	OBJECT_REGISTER(factory);
	OBJECT_REGISTER(tank);
	OBJECT_REGISTER(turret);
	OBJECT_REGISTER(rocket);
	OBJECT_REGISTER(robotarm);
	OBJECT_REGISTER(spikeball);

	OBJECT_REGISTER(coin);
}

#define FLOAT_LARGE 9999999

void instance_get2nearest(instance *object, object_id *obj_id, instance **left, instance **right, cpFloat *left_distance, cpFloat *right_distance)
{
	LList list = object_get_instances(obj_id);

	cpFloat min_length_left = FLOAT_LARGE;
	cpFloat min_length_right = FLOAT_LARGE;
	cpFloat length;

	int left_set = 0;
	int right_set = 0;

	int check_id = (obj_id == object->TYPE);

	cpVect p1 = object->body->p;
	cpVect rot1 = object->body->rot;

	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		instance *obj = (instance *) llist_next(list);
		if (check_id && object->instance_id == obj->instance_id)
			continue; //skip self

		cpVect p2 = obj->body->p;

		length = cpvdistsq(p1, p2);
		cpVect rot2 = cpvnormalize_safe(cpvsub(p2,p1));
		cpFloat dot = cpvdot(rot1,rot2);

		if (dot < 0) { //left side
			if (length < min_length_left) {
				left_set = 1;
				min_length_left = length;
				*left = obj;
			}
		} else { //right side
			if (length < min_length_right) {
				right_set = 1;
				min_length_right = length;
				*right = obj;
			}
		}
	}
	llist_end_loop(list);

	if (!left_set) {
		*left = NULL;
		*left_distance = FLOAT_LARGE;
	} else {
		*left_distance = sqrtf(min_length_left);
	}
	if (!right_set) {
		*right = NULL;
		*right_distance = FLOAT_LARGE;
	} else {
		*right_distance = sqrtf(min_length_right);
	}
}
