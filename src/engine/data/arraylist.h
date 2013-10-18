/*
 * hashmap.h
 *
 *  Created on: 22. aug. 2013
 *      Author: Karl
 */

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

typedef struct arraylist arraylist;

arraylist * al_new();
int al_size(arraylist *hm);
int al_counter(arraylist *al);
int al_add(arraylist *hm, void *data);
void al_set_safe(arraylist *al, unsigned int index, void *data);
void al_set(arraylist *hm, unsigned int index, void *data);
void * al_get(arraylist *hm, unsigned int index);
void * al_get_last(arraylist *hm);
void * al_get_first(arraylist *hm);
void al_destroy(arraylist *hm);

#endif /* ARRAYLIST_H_ */
