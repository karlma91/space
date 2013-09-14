/*
 * pool.c
 *
 *  Created on: Sep 14, 2013
 *      Author: karlmka
 */
#include "we_data.h"
#include <stdlib.h>
#include "pool.h"

struct pool {
	int element_size;
	LList available;
};

pool * pool_create(int element_size)
{
	pool *p = calloc(1,sizeof *p);
	p->element_size = element_size;
	p->available = llist_create();
	return p;
}

void * pool_instance(pool *p)
{
	void * i;
	if(llist_size(p->available)){
		i = llist_pop(p->available);
	}else{
		i = calloc(1, p->element_size);
	}
	return i;
}

void pool_release(pool *p, void *i)
{
	llist_add(p->available, i);
}

void pool_destroy(pool *p)
{
	llist_set_remove_callback(p->available,free);
	llist_destroy(p->available);
	free(p);
}
