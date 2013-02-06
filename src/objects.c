#include "objects.h"

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

static node *(head_all[ID_COUNT]);
static node **(last_all[ID_COUNT]);

static int i;

/* initializes head and last values */
void list_init() {
	for (i = 0; i < ID_COUNT; i++) {
		head_all[i] = NULL;
		last_all[i] = &(head_all[i]);
	}
}

/* add object into its corresponding list */
void list_add(object* obj)
{
	/* find right list to add object into */
	i = obj->type->ID;

	/* add object at end of list */
	(*(last_all[i])) = malloc(sizeof(node));
	(*(last_all[i]))->obj = obj;
	((*(last_all[i]))->remove) = 0;
	(*(last_all[i]))->obj->remove = &((*(last_all[i]))->remove);
	last_all[i] = &((*(last_all[i]))->next);
	*(last_all[i]) = NULL;
}

/* iterate through all lists of objects */
void list_iterate(void (*f)(object *))
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
void list_iterate_type(void (*f)(object *), int type_id)
{
	/* error check type_id */
	if (type_id < 0 || type_id >= ID_COUNT) {
		fprintf(stderr,"list_iterate_type: Invalid type_id %d\n", type_id);
		return;
	}

	/* return if list is empty */
	if ((head_all[type_id]) == NULL) return;

	node *n;
	node **prev = &(head_all[type_id]);

	for (n = (head_all[type_id]); n != NULL;  ) {
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
	last_all[type_id] = prev;
}

/* frees all nodes in all lists */
void list_destroy()
{
	for (i = 0; i < ID_COUNT; i++) {
		/* continue if list is empty */
		if (head_all[i] == NULL) continue;

		/* frees all nodes in current list */
		node *n, *a;
		for (n = head_all[i], a = n->next; n != NULL; a = n->next, free(n), n=a);
	}

	list_init();
}
