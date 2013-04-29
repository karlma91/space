/*
 * llist.c
 *
 *	General purpose Linked List implementation in C
 *
 *  Created on: Apr 26, 2013
 *      Author: Mathias H. Wilhelmsen
 */

#include <stdlib.h>
#include <stdio.h>
#include "llist.h"
#include "assert.h"

#define DEBUG
//#define DEBUG_3
//#define DEBUG_4

#define MAX_CONCURRENT_ITERATIONS 10

typedef struct node node;

struct node {
	node *next;
	node *prev;
	void *item;
};

struct llist {
	LList id;
	int size;

	node *head;
	node *tail;

	void (*remove_callback)(void *);

	int iteration_index;
	node *iteration_stack[MAX_CONCURRENT_ITERATIONS]; //FIXME possibly causing random memory writes!!
	node *NULL_TEST;
};


/* private variables */
static struct llist **all_lists;
static int list_count = 0;

/* private function definitions */
static int is_valid(int index) {
	if ((index >= 0 && index < list_count) && (all_lists[index])) {
		if (all_lists[index]->NULL_TEST) {
			fprintf(stderr, "Error -> LList's NULL test failed for list#%d, got %p instead of %p!\n", index, all_lists[index]->NULL_TEST, NULL);
			exit(-1000);
			return 0;
		}
		return 1;
	}
	return 0;
}


/* global function definitions */

int llist_create()
{
	struct llist *list;
	int i = list_count++;
	all_lists = realloc(all_lists, sizeof(struct llist *[list_count]));
	all_lists[i] = malloc(sizeof(struct llist));

	list = all_lists[i];
	list->id = i;
	list->head = NULL;
	list->tail = NULL;
	list->iteration_index = -1;
	list->size = 0;
	list->remove_callback = 0;
	list->NULL_TEST = 0;

#ifdef DEBUG
	fprintf(stderr,"list#%02d: created with callback: %p\n", i, list->remove_callback);
#endif

	return list->id;
}

int llist_add(LList id, void *p)
{
	struct llist *list;
	node *node;

	if ((p && is_valid(id)) == 0)
		return 0;

	list = all_lists[id];

	node = malloc(sizeof(*node));

	node->item = p;
	node->next = NULL;
	node->prev = list->tail;

	if (list->head && list->tail) {
		list->tail->next = node;
	} else {
		list->head = node;
	}

	list->tail = node;
	list->size++;

#ifdef DEBUG
	fprintf(stderr,"list#%02d: added %p\n", id,p);
#endif

	return 1;
}

int llist_remove(LList id, void *p)
{
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 0\n", id, p);
#endif

	struct llist *list;
	void *item;
	node *n = NULL;

	if (p && is_valid(id))
		list = all_lists[id];
	else {
#ifdef DEBUG
		if (p) {
			fprintf(stderr,"Warning -> list#%02d: invalid list (remove)\n", id);
		}
#endif
		return 0;
	}


	if (list->iteration_index != -1) {
		// make sure that all current iterators are updated
		int i;
		for (i = 0; i <= list->iteration_index; i++) {
			if (list->iteration_stack[i] && list->iteration_stack[i]->item == p) {
				n = list->iteration_stack[i];
				list->iteration_stack[i] = n->next;
#ifdef DEBUG
	fprintf(stderr,"list#%02d: adjusting stack#%d -> next = %p\n", id, i, list->iteration_stack[i] ? list->iteration_stack[i]->item : NULL);
#endif
			}
		}
	}
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 1\n", id, p);
#endif
	if (n == NULL) {
		n = list->head;
	}

	while (n) {
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 2 comparing with: %p\n", id, p, n->item);
#endif
		if (n->item == p) {
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 3 pointer identified\n", id, p);
#endif
			if (n->prev) {
				n->prev->next = n->next;
			} else {
				list->head = n->next;
			}
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 4 prev and head adjusted\n", id, p);
#endif
			if (n->next) {
				n->next->prev = n->prev;
			} else {
				if (n->prev) {
					list->tail = n->prev;
				} else {
					list->tail = list->head;
				}
			}
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 5 next and tail adjusted\n", id, p);
#endif

			item = n->item;

			n->item = NULL;
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 5 free...\n", id, p);
#endif
			free(n); /*todo move node into unused node list*/
			list->size--;
#ifdef DEBUG_4
	fprintf(stderr,"removing -> list#%02d: removal of %p step 6 remove callback to func: %p\n", id, p, list->remove_callback);
#endif
			if (list->remove_callback != NULL) {
				list->remove_callback(item);
			}

#ifdef DEBUG
	fprintf(stderr,"list#%02d: removed %p\n", id, p);
#endif

			return 1;
		}
		n = n->next;
	}

#ifdef DEBUG
	fprintf(stderr,"Warning -> list#%02d: element not found %p (remove)\n", id, p);
#endif

	return 0;
}



int llist_size(LList id)
{
	if (is_valid(id)) {
		return (all_lists[id])->size;
	} else {
		return -1;
	}
}

void* llist_first(LList id)
{
	if (is_valid(id) && all_lists[id]->head) {
		return all_lists[id]->head->item;
	} else {
		return NULL;
	}
}

void* llist_last(LList id)
{
	if (is_valid(id) && all_lists[id]->tail) {
		return all_lists[id]->tail->item;
	} else {
		return NULL;
	}
}

void* llist_at_index(LList id, int n)
{
	int i = 0;

	if (is_valid(id) && n >= 0 && n < all_lists[id]->size) {
		node *node = all_lists[id]->head;

		while (node) {
			if (i == n) {
				return node->item;
			}

			++i;
			node = node->next;
		}
	}

	return NULL;
}

int llist_set_remove_callback(LList id, void (*remove_callback)(void *))
{
	if (is_valid(id)) {
		all_lists[id]->remove_callback = remove_callback;
		return 1;
	}

	return 0;
}

void llist_iterate_func(LList id, void (*func)(void *))
{
#ifdef DEBUG_3
	fprintf(stderr,"list#%02d: iterate function %p\n", id, *func);
#endif

	if (is_valid(id) && func) {
		node *n;

		llist_begin(id);
		while (llist_hasnext(id)) {
			n = llist_next(id);
			func(n);
		}
		llist_end(id);
	}
}


int llist_begin(LList id)
{
	struct llist *list;
	if (is_valid(id)) {
		list = all_lists[id];
		if (list->iteration_index < MAX_CONCURRENT_ITERATIONS) {
			list->iteration_index++;
			list->iteration_stack[list->iteration_index] = list->head;
#ifdef DEBUG_3
	fprintf(stderr,"list#%02d: begin %d\n", id, list->iteration_index);
#endif
			return 1;
		}
	}

	return 0;
}

int llist_hasnext(LList id)
{
#ifdef DEBUG_3
	fprintf(stderr,"list#%02d: has next = %d\n", id, (is_valid(id) && (all_lists[id]->iteration_index != -1) &&
			all_lists[id]->iteration_stack[all_lists[id]->iteration_index]));
#endif
	return (is_valid(id) && (all_lists[id]->iteration_index != -1) &&
			all_lists[id]->iteration_stack[all_lists[id]->iteration_index]);
}

void* llist_next(LList id)
{
	struct llist *list;
	node *node;

	if (is_valid(id)) {
		list = all_lists[id];

		if (list->iteration_index == -1)
			return NULL;

		node = list->iteration_stack[list->iteration_index];

		if (node) {
			list->iteration_stack[list->iteration_index] = node->next;
#ifdef DEBUG_3
	fprintf(stderr,"list#%02d: next = %p\n", id, node->item);
#endif
			return node->item;
		}
	}

	return NULL;
}

int llist_end(LList id)
{
	struct llist *list;
	if (is_valid(id)) {
		list = all_lists[id];
		if (list->iteration_index >= -1) {
			list->iteration_stack[list->iteration_index] = NULL;
#ifdef DEBUG_3
	fprintf(stderr,"list#%02d: end %d\n", id, list->iteration_index);
#endif
			list->iteration_index--;
			return 1;
		}
	}

	return 0;
}


void llist_clear(LList id)
{
#ifdef DEBUG
	fprintf(stderr,"list#%02d: clear\n", id);
#endif
	while (llist_remove(id, llist_first(id)));
}

int llist_free(LList id)
{
#ifdef DEBUG
	fprintf(stderr,"list#%02d: free\n", id);
#endif
	llist_clear(id);

	if (is_valid(id)) {
		free(all_lists[id]);
		all_lists[id] = NULL;

		return 1;
	}

	return 0;
}

void llist_freeall()
{
#ifdef DEBUG
	fprintf(stderr,"list: free all\n");
#endif
	int i;
	for (i = 0; i < list_count; i++) {
		llist_free(i);
	}

	list_count = 0;
	free(all_lists);
}
