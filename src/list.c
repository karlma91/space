#include "stdlib.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "math.h"

typedef struct node_ node;
struct node_ {
	int remove;
	node *next;
	object *obj;
};

static node *head = NULL;
static node **last = &head;

void list_add(object* obj)
{
		fprintf(stderr,"0x%010x\n",(*last) = malloc(sizeof(node)));

		(*last)->obj = obj;
		((*last)->remove) = 0;
		(*last)->obj->remove = &((*last)->remove);
		last = &((*last)->next);
		(*last) = NULL;
}

void list_iterate(void (*f)(object *))
{
	if (head == NULL) {
		return;
	}

	node *n;
	node **prev = &head;

	for (n = head; n != NULL;  ) {
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
	last = prev;
}

void list_iterate_type(void (*f)(object *), int type_id)
{

}

void list_destroy()
{
	if (head == NULL) {
		return;
	}
	node *n, *a;
	for (n = head, a = n->next; n != NULL; a = n->next, free(n), n=a);
}
