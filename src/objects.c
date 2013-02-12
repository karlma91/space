#include "objects.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int LAYER_PLAYER          =     1<<1 | 1<<2 |                1<<5;
int LAYER_TANK            =     1<<1 |         1<<3;
int LAYER_TANK_FACTORY    =            1<<2  |        1<<4;
int LAYER_PLAYER_BULLET   =                    1<<3 | 1<<4;
int LAYER_ENEMY_BULLET    =                                 1<<5;


typedef struct node_ node;
struct node_ {
	int remove;
	node *next;
	object *obj;
};

int instance_counter[ID_COUNT];
static node *(head_all[ID_COUNT]);
static node **(last_all[ID_COUNT]);

static int i;

/* initializes head and last values */
void objects_init() {
	for (i = 0; i < ID_COUNT; i++) {
		head_all[i] = NULL;
		last_all[i] = &(head_all[i]);
		instance_counter[i] = 0;
	}
}

/* add object into its corresponding list */
void objects_add(object* obj)
{
	/* find right list to add object into */
	i = obj->type->ID;

	/* add object at end of list */
	(*(last_all[i])) = malloc(sizeof(node));
	(*(last_all[i]))->obj = obj;
	((*(last_all[i]))->remove) = 0;
	(*(last_all[i]))->obj->remove = &((*(last_all[i]))->remove);
	(*(last_all[i]))->obj->instance_id = instance_counter[i];
	last_all[i] = &((*(last_all[i]))->next);
	*(last_all[i]) = NULL;
	++instance_counter[i];
}

/* iterate through all lists of objects */
void objects_iterate(void (*f)(object *))
{
	for (i = 0; i < ID_COUNT; i++) {
		if (head_all[i] == NULL) continue;

		node *n;
		node **prev = &(head_all[i]);

		for (n = (head_all[i]); n != NULL;  ) {
			if (n->remove) {
				(*prev) = n->next;
				free(n);
				n = (*prev);
			} else {
				f(n->obj);
				prev = &(n->next);
				n = n->next;
			}
		}
		last_all[i] = prev;
	}
}

/* iterate through one type of object */
void objects_iterate_type(void (*f)(object *), int obj_id)
{
	/* error check obj_id */
	if (obj_id < 0 || obj_id >= ID_COUNT) {
		fprintf(stderr,"list_iterate_type: Invalid obj_id %d\n", obj_id);
		return;
	}

	/* return if list is empty */
	if ((head_all[obj_id]) == NULL) return;

	node *n;
	node **prev = &(head_all[obj_id]);

	for (n = (head_all[obj_id]); n != NULL;  ) {
		if (n->remove) {
			(*prev) = n->next;
			free(n);
			n = (*prev);
		} else {
			f(n->obj);
			prev = &(n->next);
			n = n->next;
		}
	}
	last_all[obj_id] = prev;
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
	}

	objects_init();
}

object *objects_nearest(cpVect pos, int obj_id)
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

object *objects_first(int obj_id)
{
	return head_all[obj_id]->obj;
}

object *objects_n(int obj_id, int n)
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

object *objects_last(int obj_id)
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

object *objects_by_id(int obj_id, int instance_id)
{
	/* error check obj_id */
	if (obj_id < 0 || obj_id >= ID_COUNT) {
		fprintf(stderr,"list_iterate_type: Invalid obj_id %d\n", obj_id);
		return NULL;
	}

	node *node;

	for (node = head_all[obj_id]; node != NULL; node = node->next) {
		if (node->obj->instance_id == instance_id) {
			return node->obj;
		}
	}

	return NULL;
}


int objects_isempty(int obj_id)
{
	if(head_all[obj_id] == NULL){
		return 1;
	}
	return 0;
}


