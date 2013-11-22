/*
 * hashmap.h
 *
 *  Created on: 22. aug. 2013
 *      Author: Karl
 */

#ifndef HASHMAP_H_
#define HASHMAP_H_

typedef struct hashnode hashnode;
typedef struct hashiterator hashiterator;

typedef struct hashmap hashmap;

hashmap * hm_create(void);
int hm_size(hashmap *hm);
void hm_destroy(hashmap *hm);
int hm_add(hashmap *hm, const char *key, void *data);
void* hm_remove(hashmap *hm, const char *key);
void * hm_get(hashmap *hm, const char *key);
hashiterator * hm_get_iterator(hashmap *hm);
void * hm_iterator_next(hashiterator *iterator);
int hm_iterator_hasnext(hashiterator *it);
char * hm_iterator_get_key(hashiterator *it);
void hm_iterator_destroy(hashiterator *iterator);

#endif /* HASHMAP_H_ */
