/*
 * hashmap.h
 *
 *  Created on: 22. aug. 2013
 *      Author: Karl
 */

#ifndef HASHMAP_H_
#define HASHMAP_H_

typedef struct _hashnode hashnode;
typedef struct _hashiterator hashiterator;

typedef struct {
	int size;
	int count;
	hashnode **buckets;

} hashmap;

hashmap * hm_create();
void hm_destroy(hashmap *hm);
int hm_add(hashmap *hm, const char *key, void *data);
void* hm_remove(hashmap *hm, const char *key);
void * hm_get(hashmap *hm, const char *key);
hashiterator * hm_get_iterator(hashmap *hm);
void * hm_iterator_next(hashiterator *iterator);
void hm_destroy_iterator(hashiterator *iterator);

#endif /* HASHMAP_H_ */
