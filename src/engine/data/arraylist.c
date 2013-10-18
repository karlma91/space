#include "stdlib.h"
#include "string.h"
#include "arraylist.h"

#define INITIAL_SIZE 32

struct arraylist {
	int size;
	int count;
	void **data;
};

arraylist * al_new(void)
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

int al_size(arraylist *hm)
{
	return hm->count;
}

static void al_resize(arraylist *al)
{
	int size = al->size * 2;
	al->data = realloc(al->data, size * sizeof(void*));
	al->size = size;
}

int al_add(arraylist *al, void *data)
{
	if (al->count >= al->size) {
		al_resize(al);
	}
	al->data[al->count] = data;
	al->count += 1;
	return 1;
}

void al_set(arraylist *al, unsigned int index, void *data)
{
	if(index < al->size) {
		al->data[index] = data;
	} else {
		//we_error("INDEX OUT OF BOUNDS");
	}
}

void * al_get(arraylist *al, unsigned int index)
{
	if ( index < al->count ) {
		return al->data[index];
	} else{
		return NULL;
	}
}

void * al_get_last(arraylist *al)
{
	if ( al->count > 0) {
		return al->data[al->count - 1];
	}else {
		return NULL;
	}
}

void * al_get_first(arraylist *al)
{
	return al->data[0];
}

void al_destroy(arraylist *al) {
	free(al->data);
	free(al);
}
