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
//static node **previous = &head;

void list_add(object* obj)
{
		(*last) = malloc(sizeof(node));
		(*last)->obj = obj;
		last = &((*last)->next);
		(*last) = NULL;
	//TODO set remove-pointer in object
}

void list_iterate(void (*f)(object *))
{
	if (head == NULL) {
		return;
	}

	node *n;
	node **prev = &head;

	for (n = head; n != NULL; prev = &(n->next), n = n->next) {
		if (n->remove) {
			(*prev)->next = n->next;
			free(n);
		} else {
			f(n->obj);
		}
	}
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
