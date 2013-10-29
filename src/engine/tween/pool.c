/*
 * pool.c
 *
 *  Created on: Sep 14, 2013
 *      Author: karlmka
 */
#include "we_data.h"
#include <stdlib.h>
#include "pool.h"

pool * pool_create(int element_size)
{
	pool *p = calloc(1,sizeof *p);
	p->element_size = element_size;
	p->available = llist_create();
	//p->in_use = llist_create();
	return p;
}

void * pool_instance(pool *p)
{
	void *elem = NULL;
	int size = llist_size(p->available);
	if (size == 0) {
		elem = calloc(1, p->element_size);
	} else if (size > 0) {
		elem = llist_pop(p->available);
	} else {
		SDL_Log("Error: reading pool's llist");
	}
	return elem;
}

void pool_release_rmcall(void *i, pool *p)
{
	//llist_remove(p->in_use,i);
	llist_add(p->available, i);
}

void pool_release(pool *p, void *i)
{
	pool_release_rmcall(i,p);
}

void pool_destroy(pool *p)
{
	llist_set_remove_callback(p->available, free);
	//llist_set_remove_callback(p->in_use, free);
	llist_destroy(p->available);
	//llist_destroy(p->in_use);
	free(p);
}
