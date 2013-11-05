#include "stdlib.h"
#include "string.h"
#include "array.h"
#include "we_utils.h"

#define INITIAL_SIZE 8

struct array {
	int size;
	int last_elem;
	unsigned int elem_size;
	char *data;
};

array * array_new(unsigned int element_size)
{
	array *al = calloc(1, sizeof(array));
	if (al) {
		al->elem_size = element_size;
		al->last_elem = 0;
		al->size = INITIAL_SIZE;
		al->data = (char*)calloc(INITIAL_SIZE, element_size);
		return al;
	} else {
		return NULL;
	}
}

int array_size(array *a)
{
	return a->size;
}

int array_count(array *a)
{
	return a->last_elem + 1;
}

static void array_resize(array *al)
{
	int size = al->size;
	int new_size = size * 2;
	al->data = realloc(al->data, new_size * al->elem_size);
	memset( (al->data + size*al->elem_size), 0, size * al->elem_size);
	al->size = new_size;
}

void array_set_safe(array *al, unsigned int index, void *data)
{
	while(index >= al->size) {
		array_resize(al);
	}

	array_set(al, index, data);
}
void array_set(array *al, unsigned int index, void *data)
{
	if(index < al->size) {
		memcpy(((void*)(al->data + index * al->elem_size)), data, al->elem_size);
		if (index > al->last_elem) {
			al->last_elem = index;
		}
	} else {
		we_error("INDEX OUT OF BOUNDS");
	}
}

void * array_get(array *al, unsigned int index)
{
	if ( index < al->size ) {
		return (void*)(al->data + index * al->elem_size);
	} else{
		return NULL;
	}
}
void array_destroy(array *al)
{
	free(al->data);
	free(al);
}
