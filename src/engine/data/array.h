/*
 * hashmap.h
 *
 *  Created on: 22. aug. 2013
 *      Author: Karl
 */

#ifndef ARRAY_H_
#define ARRAY_H_

typedef struct array array;

array * array_new(unsigned int element_size);
int array_count(array *a);
int array_size(array *a);
int array_add(array *a, void *data);
void array_set_safe(array *a, unsigned int index, void *data);
void array_set(array *a, unsigned int index, void *data);
void * array_get(array *a, unsigned int index);
void array_destroy(array *a);

#endif /* ARRAY_H_ */
