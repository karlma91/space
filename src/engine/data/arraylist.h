/*
 * hashmap.h
 *
 *  Created on: 22. aug. 2013
 *      Author: Karl
 */

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

typedef struct arraylist arraylist;

arraylist * alist_new();
int alist_size(arraylist *hm);
int alist_counter(arraylist *al);
int alist_add(arraylist *hm, void *data);
void alist_set_safe(arraylist *al, unsigned int index, void *data);
void alist_set(arraylist *hm, unsigned int index, void *data);
void * alist_get(arraylist *hm, unsigned int index);
void* alist_get_last(arraylist* al);
void * alist_get_first(arraylist *hm);
void alist_destroy(arraylist *hm);

#endif /* ARRAYLIST_H_ */
