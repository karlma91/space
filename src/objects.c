#include "objects.h"
#include "level.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int LAYER_PLAYER          =     1<<1 | 1<<2 |                1<<5;
int LAYER_TANK            =     1<<1 |         1<<3;
int LAYER_WHEEL           =                                        1<<6;
int LAYER_TANK_FACTORY    =            1<<2  |        1<<4;
int LAYER_PLAYER_BULLET   =                    1<<3 | 1<<4;
int LAYER_ENEMY_BULLET    =                                 1<<5;

//FIXME count does not always correspond to actual rendered objects!

typedef struct _node node;
struct _node {
	int remove;
	node *next;
	object_data *obj;
};

//TODO put these static variables into a structure, for easier simultaneous parallel level storage
static int object_actual_count[ID_COUNT];
static int object_count[ID_COUNT];
static int instance_counter[ID_COUNT];
static node *(head_all[ID_COUNT]);
static node **(last_all[ID_COUNT]);

static int i;

/* initializes head and last values */
void objects_init() {
	for (i = 0; i < ID_COUNT; i++) {
		head_all[i] = NULL;
		last_all[i] = &(head_all[i]);
		instance_counter[i] = 0;
		object_count[i] = 0;
	}
}

/* add object into its corresponding list */
void objects_add(object_data* obj)
{
	/* find right list to add object into */
	i = obj->preset->ID;

	/* add object at end of list */
	(*(last_all[i])) = malloc(sizeof(node));
	(*(last_all[i]))->obj = obj;
	((*(last_all[i]))->remove) = 0;
	(*(last_all[i]))->obj->remove = &((*(last_all[i]))->remove);
	(*(last_all[i]))->obj->instance_id = instance_counter[i];
	last_all[i] = &((*(last_all[i]))->next);
	*(last_all[i]) = NULL;
	++instance_counter[i];
	++object_count[i];
}


static void errchk_obj_count(obj_id)
{
	if (object_count[obj_id] != object_actual_count[obj_id]) {
		fprintf(stderr,"Data structure warning for obj_id: %s(%d) expected %d, but got %d. Re-adjusting data count.\n", group_names[obj_id],obj_id, object_count[obj_id], object_actual_count[obj_id]);
		object_count[obj_id] = object_actual_count[obj_id];
	}
}

/* iterate through all lists of objects */
void objects_iterate(void (*f)(object_data *))
{
	int obj_id;
	for (obj_id = 0; obj_id < ID_COUNT; obj_id++) {
		object_actual_count[obj_id] = 0;
		if (head_all[obj_id] == NULL) continue;

		node *n;
		node **prev = &(head_all[obj_id]);

		for (n = (head_all[obj_id]); n != NULL;  ) {
			if (n->remove) {
				(*prev) = n->next;
				free(n);
				n = (*prev);
				--object_count[obj_id];
			} else {
				f(n->obj);

				// TODO FIXME
				if (n->remove) {
					(*prev) = n->next;
					free(n);
					n = (*prev);
					--object_count[obj_id];
				} else {
					prev = &(n->next);
					n = n->next;
					++object_actual_count[obj_id];
				}
			}
		}
		last_all[obj_id] = prev;
		errchk_obj_count(obj_id);
	}
}

/* iterate through one type of object */
void objects_iterate_type(void (*f)(object_data *), int obj_id) {
	/* error check obj_id */
	if (obj_id < 0 || obj_id >= ID_COUNT) {
		fprintf(stderr,"list_iterate_type: Invalid obj_id %d\n", obj_id);
		return;
	}

	object_actual_count[obj_id] = 0;

	/* check if list is not empty */
	if (head_all[obj_id]) {

		node *n;
		node **prev = &(head_all[obj_id]);

		for (n = (head_all[obj_id]); n != NULL;  ) {
			if (n->remove) {
				(*prev) = n->next;
				free(n);
				n = (*prev);
				--object_count[i];
			} else {
				f(n->obj);

				if (n->remove) {
					(*prev) = n->next;
					free(n);
					n = (*prev);
					--object_count[i];
				} else {
					prev = &(n->next);
					n = n->next;
					++object_actual_count[obj_id];
				}
			}
		}
		last_all[obj_id] = prev;
	}
	errchk_obj_count(obj_id);
}

/* frees all nodes in all lists */
void objects_destroy()
{
	for (i = 0; i < ID_COUNT; i++) {
		/* continue if list is empty */
		if (head_all[i] == NULL) continue;

		/* frees all nodes in current list */
		node *n, *a;
		for (n = head_all[i], a = n->next; n != NULL; a = n->next, free(n), n=a);

		object_count[i] = 0;
	}

	objects_init();
}

object_data *objects_nearest(cpVect pos, int obj_id)
{
	cpVect v = head_all[obj_id]->obj->body->p;
	if (head_all[obj_id] == NULL) return NULL;

	node *n = head_all[obj_id];
	node *min_node = n;
	cpFloat min_length = INT_MAX;
	cpFloat length;

	for (n = head_all[obj_id]; n != NULL; n = n->next) {
		v = n->obj->body->p;
		length = (v.x-pos.x)*(v.x-pos.x) + (v.y-pos.y)*(v.y-pos.y);
		if (length < min_length) {
			min_length = length;
			min_node = n;
		}
	}

	return min_node->obj;
}

object_data *objects_first(int obj_id)
{
	if (object_count[obj_id])
		return head_all[obj_id]->obj;
	else
		return NULL;
}

object_data *objects_n(int obj_id, int n)
{
	/* error check obj_id */
	if (obj_id < 0 || obj_id >= ID_COUNT) {
		fprintf(stderr,"list_iterate_type: Invalid obj_id %d\n", obj_id);
		return NULL;
	}

	node *node;

	for (node = head_all[obj_id]; node != NULL; --n, node = node->next) {
		if (n <= 0) {
			return node->obj;
		}
	}
	return NULL;
}

object_data *objects_last(int obj_id)
{
	/* error check obj_id */
	if (obj_id < 0 || obj_id >= ID_COUNT) {
		fprintf(stderr,"list_iterate_type: Invalid obj_id %d\n", obj_id);
		return NULL;
	}

	node *n = head_all[obj_id];

	/* return if list is empty */
	if ((head_all[obj_id]) == NULL) return NULL;

	for (; n != NULL; n = n->next) {
		if (n->next == NULL) {
			return n->obj;
		}
	}
	return NULL;
}

object_data *objects_by_id(int obj_id, int instance_id)
{
	/* error check obj_id */
	if (obj_id < 0 || obj_id >= ID_COUNT) {
		fprintf(stderr,"list_iterate_type: Invalid obj_id %d\n", obj_id);
		return NULL;
	}

	node *node;

	for (node = head_all[obj_id]; node != NULL; node = node->next) {
		if (node->obj->instance_id == instance_id) {
			return node->remove ? NULL : node->obj;
		}
	}

	return NULL;
}

int objects_count(int obj_id)
{
	return object_count[obj_id];
}

inline void objects_remove(object_data *obj)
{
	*obj->remove = 1;
}

//TODO compute the shortest distance considering wrap-around
void objects_nearest_x_two(object_data *object, int obj_id, object_data **left, object_data **right, cpFloat *left_distance, cpFloat *right_distance)
{
	if (head_all[obj_id] == NULL) return;

	cpVect v = head_all[obj_id]->obj->body->p;
	node *n = head_all[obj_id];
	cpFloat min_length_left = INT_MIN;
	cpFloat min_length_right = INT_MAX;
	cpFloat length;

	int check_id = (obj_id == object->preset->ID);

	for (n = head_all[obj_id]; n != NULL; n = n->next) {
		if (check_id && object->instance_id == n->obj->instance_id)
			continue; //skip self

		v = n->obj->body->p;
		length = v.x - object->body->p.x;
		if (length < 0) { //left side
			if (length > min_length_left) {
				min_length_left = length;
				*left = n->obj;
			}
		} else { //right side
			if (length < min_length_right) {
				min_length_right = length;
				*right = n->obj;
			}
		}
	}
	if (min_length_left == INT_MIN) {
		*left = NULL;
		*left_distance =  INT_MAX;
	} else {
		*left_distance =  -min_length_left;
	}
	if (min_length_right == INT_MAX) {
		*right = NULL;
		*right_distance = INT_MAX;
	} else {
		*right_distance = min_length_right;
	}
}

//TODO add help methods here?
