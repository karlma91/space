/*
 * llist.c
 *
 *	General purpose Linked List implementation in C.
 *
 *	NB! Does not support more than INT_MAX number
 *	of calls to llist_create().
 *
 *  Created on: Apr 26, 2013
 *      Author: Mathias H. Wilhelmsen
 */

#include <stdlib.h>
#include <stdio.h>
#include "llist.h"

/* Define LLIST_DEBUG 1 to debug llist */

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
	node *iteration_stack[MAX_CONCURRENT_ITERATIONS];
	node *NULL_TEST;
};


/* private variables */
static node *node_pool = NULL;
static const node node_null = {0,0,0};

#if LLIST_DEBUG
static int stats_max_node_count = 0;
static int stats_node_count = 0;
#endif

/* private function definitions */
static int is_valid(struct llist *list) {
	if (list) {
		if (list->id != list) {
			fprintf(stderr, "Error -> LList's ID test failed for list [%p], got %p!\n", list, list->id);
			exit(-1000);
			return 0;
		}
		else if (list->NULL_TEST) {
			fprintf(stderr, "Error -> LList's NULL test failed for list [%p], got %p instead of %p!\n", list, list->NULL_TEST, NULL);
			exit(-1000);
			return 0;
		}
		return 1;
	}
	return 0;
}

static __inline__ node* new_node() {
	node *node;

#if LLIST_DEBUG
	if (++stats_node_count > stats_max_node_count) {
		stats_max_node_count = stats_node_count;
		fprintf(stderr, "List -> expanding node capacity: %d\n", stats_max_node_count);
	}
#endif

	if (node_pool) {
		node = node_pool;
		node_pool = node_pool->next;
	} else {
		node = malloc(sizeof(*node));
#if LLIST_DEBUG
		fprintf(stderr, "List -> node capacity expanded!\n");
#endif
	}

	*node = node_null;
	return node;
}

static void __inline__ free_node(node *node) {
#if LLIST_DEBUG
	--stats_node_count;
#endif

	*node = node_null;

	if (node_pool) {
		node->next = node_pool;
		node_pool = node;
	} else {
		node_pool = node;
	}
}

/* global function definitions */

LList llist_create()
{
	struct llist *list = malloc(sizeof(struct llist));

	list->id = (LList)list;
	list->head = NULL;
	list->tail = NULL;
	list->iteration_index = -1;
	list->size = 0;
	list->remove_callback = 0;
	list->NULL_TEST = 0;

#if LLIST_DEBUG
	fprintf(stderr,"list [%p]: created with callback: %p\n", list, list->remove_callback);
#endif

	return list->id;
}

int llist_add(LList id, void *p)
{
	struct llist *list = (struct llist *)id;
	node *node;

	if (!p || !is_valid(list))
		return 0;

	node = new_node();

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

#if LLIST_DEBUG
	fprintf(stderr,"list [%p]: added %p\n", id,p);
#endif

	return 1;
}

int llist_remove(LList id, void *p)
{
	struct llist *list = (struct llist *)id;
	void *item;
	node *n = NULL;

	if (!p || !is_valid(list)) {
#if LLIST_DEBUG
		if (p) {
			fprintf(stderr,"Warning -> list [%p]: invalid list (remove)\n", id);
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
#if LLIST_DEBUG
	fprintf(stderr,"list [%p]: adjusting stack#%d -> next = %p\n", id, i, list->iteration_stack[i] ? list->iteration_stack[i]->item : NULL);
#endif
			}
		}
	}
	if (n == NULL) {
		n = list->head;
	}

	while (n) {
		if (n->item == p) {
			if (n->prev) {
				n->prev->next = n->next;
			} else {
				list->head = n->next;
			}
			if (n->next) {
				n->next->prev = n->prev;
			} else {
				if (n->prev) {
					list->tail = n->prev;
				} else {
					list->tail = list->head;
				}
			}

			item = n->item;

			n->item = NULL;
			free_node(n);
			list->size--;
			if (list->remove_callback != NULL) {
				list->remove_callback(item);
			}

#if LLIST_DEBUG
	fprintf(stderr,"list [%p]: removed %p\n", id, p);
#endif

			return 1;
		}
		n = n->next;
	}

#if LLIST_DEBUG
	fprintf(stderr,"Warning -> list [%p]: element not found %p (remove)\n", id, p);
#endif

	return 0;
}


int llist_size(LList id)
{
	struct llist *list = (struct llist *) id;

	if (is_valid(list)) {
		return list->size;
	} else {
		return -1;
	}
}

void* llist_first(LList id)
{
	struct llist *list = (struct llist *) id;

	if (is_valid(list) && list->head) {
		return list->head->item;
	} else {
		return NULL;
	}
}

void* llist_last(LList id)
{
	struct llist *list = (struct llist *) id;

	if (is_valid(list) && list->tail) {
		return list->tail->item;
	} else {
		return NULL;
	}
}

void* llist_at_index(LList id, int n)
{
	int i = 0;
	struct llist *list = (struct llist *) id;

	if (is_valid(list) && n >= 0 && n < list->size) {
		node *node = list->head;

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
	struct llist *list = (struct llist *) id;

	if (is_valid(list)) {
		list->remove_callback = remove_callback;
		return 1;
	}

	return 0;
}

void llist_iterate_func(LList id, void (*func)(void *))
{
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
	struct llist *list = (struct llist *) id;

	if (is_valid(list)) {
		if (list->iteration_index + 1 < MAX_CONCURRENT_ITERATIONS) {
			list->iteration_index++;
			list->iteration_stack[list->iteration_index] = list->head;
			return 1;
		}
	}

	return 0;
}

int llist_hasnext(LList id)
{
	struct llist *list = (struct llist *) id;

	return (is_valid(list) && (list->iteration_index != -1) && list->iteration_stack[list->iteration_index]);
}

void* llist_next(LList id)
{
	struct llist *list = (struct llist *) id;
	node *node;

	if (is_valid(list)) {
		if (list->iteration_index == -1)
			return NULL;

		node = list->iteration_stack[list->iteration_index];

		if (node) {
			list->iteration_stack[list->iteration_index] = node->next;
			return node->item;
		}
	}

	return NULL;
}

int llist_end(LList id)
{
	struct llist *list = (struct llist *) id;

	if (is_valid(list)) {
		if (list->iteration_index > -1) {
			list->iteration_stack[list->iteration_index] = NULL;
			list->iteration_index--;
			return 1;
		}
	}

	return 0;
}


void llist_clear(LList id)
{
#if LLIST_DEBUG
	fprintf(stderr,"list [%p]: clear\n", id);
#endif
	while (llist_remove(id, llist_first(id)));
}

int llist_free(LList id)
{
#if LLIST_DEBUG
	fprintf(stderr,"list [%p]: free\n", id);
#endif
	struct llist *list = (struct llist *) id;

	llist_clear(id);

	if (!is_valid(list))
		return 0;

	free(list);
	return 1;
}

void llist_free_nodes() {
	node *node = node_pool;
	while (node_pool) {
		node_pool = node->next;
#if LLIST_DEBUG
		--stats_max_node_count;
#endif
		free(node);
	}
}
