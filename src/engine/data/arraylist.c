#include "stdlib.h"
#include "string.h"
#include "arraylist.h"
#include "we_utils.h"

#define INITIAL_SIZE 32

struct arraylist {
	int size;
	int count;
	void **data;
};

arraylist * alist_new(void)
{
	arraylist *al = calloc(1, sizeof(arraylist));
	if (al){
		al->count = 0;
		al->data = (void**)calloc(INITIAL_SIZE, sizeof(void*));
		al->size = INITIAL_SIZE;
		return al;
	} else {
		return NULL;
	}
}

int alist_size(arraylist *hm)
{
	return hm->size;
}

int alist_last_index(arraylist *al)
{
	return al->count;
}

static void alist_resize(arraylist *al)
{
	int size = al->size;
	int new_size = size * 2;
	al->data = realloc(al->data, new_size * sizeof(void*));
	memset(&al->data[size], 0, size * sizeof(void*));
	al->size = new_size;
}

int alist_add(arraylist *al, void *data)
{
	if (al->count >= al->size) {
		alist_resize(al);
	}
	al->data[al->count] = data;
	int index = al->count;
	al->count += 1;
	return index;
}


void alist_set_safe(arraylist *al, int index, void *data)
{
	while(index >= al->size) {
		alist_resize(al);
	}

	alist_set(al, index, data);
}
void alist_set(arraylist *al, int index, void *data)
{
	if (index >= 0 && index < al->size) {
		al->data[index] = data;
	} else {
		we_error("ERROR: index out of bounds");
	}
}

void * alist_get(arraylist *al, int index)
{
	if (index >= 0 && index < al->size ) {
		return al->data[index];
	} else{
		return NULL;
	}
}

void* alist_get_last(arraylist* al)
{
	if (al->count > 0) {
		return al->data[al->count - 1];
	} else {
		return NULL;
	}
}

void * alist_get_first(arraylist *al)
{
	return al->data[0];
}

void alist_destroy(arraylist *al)
{
	free(al->data);
	free(al);
}
