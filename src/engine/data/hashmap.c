/*
 * hashmap.c
 *
 *  Created on: 22. aug. 2013
 *      Author: Karl
 */
#include "stdlib.h"
#include "string.h"
#include "hashmap.h"

#define MAX_KEY_LENGTH 64
#define INITIAL_SIZE 128

struct hashmap{
	int size;
	int count;
	hashnode **buckets;
	hashnode * pool;
};

struct hashnode {
	char key[MAX_KEY_LENGTH];
	void *data;
	hashnode *next;
};

struct hashiterator {
	hashmap *hm;
	char * key;
	int index;
	hashnode *node;
};

/*http://www.cse.yorku.ca/~oz/hash.html*/
static unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

hashmap * hm_create(void)
{
	hashmap *hm = malloc(sizeof(hashmap));
	if(hm){
		hm->size = INITIAL_SIZE;
		hm->count = 0;
		hm->pool = NULL;
		hm->buckets = malloc(hm->size * sizeof(hashnode *));
		if(hm->buckets){
			int i;
			for(i=0; i < hm->size; i++){
				hm->buckets[i] = NULL;
			}
		}
		return hm;
	}else{
		return NULL;
	}
}

int hm_size(hashmap *hm)
{
	return hm->count;
}

static int hm_add_internal(hashnode **buckets, hashnode *node, int size)
{
	int index = hash((unsigned char*)node->key) % size;
	if( buckets[index] == NULL){
		buckets[index] = node;
	}else{
		hashnode *temp = buckets[index];
		while(temp != NULL){
			if(strcmp(temp->key, node->key) == 0){
				return 1;
			}
			if(temp->next == NULL){
				temp->next = node;
				return 0;
			}
			temp = temp->next;
		}
	}
	return 0;
}

static void hm_resize(hashmap *hm)
{
	int size = hm->size * 2;
	hashnode **buckets = calloc(size, sizeof(hashnode *));
	int i;
	for(i=0; i < hm->size; i++){
		hashnode *node;
		for(node = hm->buckets[i]; node != NULL; node = node->next){
			hm_add_internal(buckets, node, size);
		}
	}
	free(hm->buckets);
	hm->buckets = buckets;
	hm->size = size;
}

static hashnode * hm_create_node(hashmap *hm, const char *key, void *data)
{
	hashnode *node;
	if(hm->pool != NULL){
		node = hm->pool;
		hm->pool = hm->pool->next;
	}else{
		node = malloc(sizeof(hashnode));
	}
	strcpy(node->key, key);
	node->data = data;
	node->next = NULL;
	return node;
}

void* hm_remove(hashmap *hm, const char *key){
	int index = hash((unsigned char*)key) % hm->size;
	hashnode *prev =NULL;
	hashnode *node = hm->buckets[index];
	while(node != NULL){
		if(strcmp(key, node->key) == 0){
			if(prev ==NULL){
				hm->buckets[index] = node->next;
			}else{
				prev->next = node->next;
			}
			hm->count--;
			void *data = node->data;
			node->data = NULL;
			node->next = hm->pool;
			hm->pool = node;
			return data;
		}
		prev = node;
		node = node->next;
	}
	return NULL;
}

int hm_add(hashmap *hm, const char *key, void *data)
{
	if(hm->count == hm->size){
		hm_resize(hm);
	}
	hashnode *node = hm_create_node( hm, key, data );
	if(hm_add_internal(hm->buckets,node,hm->size) == 0){
		hm->count += 1;
	}else{
		free(node);
		return 1;
	}
	return 0;
}

void * hm_get(hashmap *hm, const char *key)
{
	int index = hash((unsigned char*)key) % hm->size;
	hashnode *node = hm->buckets[index];
	while(node != NULL){
		if(strcmp(key, node->key) == 0){
			return node->data;
		}
		node = node->next;
	}
	return NULL;
}

hashiterator * hm_get_iterator(hashmap *hm)
{
	hashiterator *it = malloc(sizeof(hashiterator));
	it->hm = hm;
	it->node = NULL;
	it->index = 0;
	if(hm->count == 0){
		it->index = hm->size;
		return it;
	}
	while(it->node == NULL){
		it->node = hm->buckets[it->index];
		if(it->node == NULL){
			it->index++;
		}
	}
	return it;
}

void * hm_iterator_next(hashiterator *it)
{
	hashnode *data = NULL;
	if(it->index < it->hm->size){
		data = it->node;
		it->key = data->key;
		it->node = it->node->next;
		while(it->node == NULL && it->index < it->hm->size){
			it->index++;
			if(it->index < it->hm->size){
				it->node = it->hm->buckets[it->index];
			}
		}
	}else{
		return NULL;
	}
	return data->data;
}

int hm_iterator_hasnext(hashiterator *it)
{
	return (it->index < it->hm->size);
}
char * hm_iterator_get_key(hashiterator *it)
{
 return it->key;
}

void hm_destroy_iterator(hashiterator *it)
{
	free(it);
}

void hm_destroy(hashmap *hm)
{

	int i;
	/* free all used nodes */
	for(i=0; i < hm->size; i++){
		hashnode *node = hm->buckets[i];
		while(node != NULL){
			hashnode *next = node->next;
			free(node);
			node = next;
		}
	}

	/* free unused nodes */
	while(hm->pool != NULL){
		hashnode *node = hm->pool;
		hm->pool = hm->pool->next;
		free(node);
	}
	free(hm->buckets);
	free(hm);
}
