/*
 * param.c
 *
 *  Created on: Feb 13, 2014
 *      Author: karlmka
 */

#include <stdlib.h>
#include "we_io.h"
#include "we_data.h"
#include "param.h"

void* parse_generated(cJSON *param, char* type, char *name);

static void parse_param_object(cJSON *param, hashmap * param_list)
{
	char type[32], name[32];
	jparse_parse(param,"type","char",type, "NULL");
	jparse_parse(param,"name","char",name, NULL);
	strtolower(type, type);
	strtolower(name, name);

	SDL_Log("PARSING TYPE: %s name: %s ", type, name);

	hashmap * names;
	names = hm_get(param_list, type);
	if (names == NULL) {
		names = hm_create();
		hm_add(param_list, type, names);
	}

	void * data = parse_generated(param, type, name);

	if (hm_add(names, name, data)) {
		SDL_Log("param of type %s with name %s was already in leveldata", type, name);
	}
}

static void load_params(param_list *defs, cJSON *root)
{
	cJSON *param_array = cJSON_GetObjectItem(root,"params");
	defs->param = hm_create();
	int i;
	for (i = 0; i < cJSON_GetArraySize(param_array); i++){
		cJSON *param = cJSON_GetArrayItem(param_array, i);
		parse_param_object(param, defs->param);
	}
}


param_list * param_load(int dir_type, char *file)
{
	cJSON *root = jparse_open(dir_type, file);
	if(root == NULL) {
		return NULL;
	}
	param_list *pl = calloc(1,sizeof(param_list));

	load_params(pl, root);
	jparse_close(root);

	return pl;
}

void param_list_destroy(param_list *pl)
{
	if (pl) {
		if (pl->param) {
			hashiterator *ih = hm_get_iterator(pl->param);
			while(hm_iterator_hasnext(ih)) {
				hashmap * hm = hm_iterator_next(ih);
				hashiterator *ih2 = hm_get_iterator(hm);
				while(hm_iterator_hasnext(ih2)) {
					hashmap * hm2 = hm_iterator_next(ih2);
					hashiterator *ih3 = hm_get_iterator(hm2);
					while(hm_iterator_hasnext(ih3)) {
						void * param = hm_iterator_next(ih3);
						free(param);
					}
					hm_iterator_destroy(ih3);
					hm_destroy(hm2);
				}
				hm_iterator_destroy(ih2);
				hm_destroy(hm);
			}
			hm_iterator_destroy(ih);
			hm_destroy(pl->param);
		}
	}
}

/*void params_write()
{
	hashiterator *it = hm_get_iterator(lvl->params.param);
	hashmap * h = hm_iterator_next(it);
	while(h != NULL) {
		object_id *obj_id = object_by_name(hm_iterator_get_key(it));
		hashiterator *it2 = hm_get_iterator(h);
		void * data = hm_iterator_next(it2);
		while(data != NULL) {

			SDL_Log("Adding param to array");

			cJSON * param = write_generated(obj_id, data, hm_iterator_get_key(it), hm_iterator_get_key(it2));
			cJSON_AddItemToArray(param_array, param);
			data = hm_iterator_next(it2);
		}
		h = hm_iterator_next(it);
	}
}*/

void param_init()
{

}

