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

param_list param_defs;

void* parse_generated(cJSON *param, char* type, char *name);

static void parse_param_object(cJSON *param)
{
	char type[32], name[32];
	jparse_parse(param,"type","char",type, "NULL");
	jparse_parse(param,"name","char",name, NULL);
	strtolower(type, type);
	strtolower(name, name);

	SDL_Log("PARSING TYPE: %s name: %s ", type, name);

	hashmap * names;
	names = hm_get(param_defs.hm_param, type);
	if (names == NULL) {
		names = hm_create();
		hm_add(param_defs.hm_param, type, names);
	}

	void * data = parse_generated(param, type, name);

	if (hm_add(names, name, data)) {
		SDL_Log("param of type %s with name %s was already in leveldata", type, name);
	}
}

static void load_params(cJSON *root)
{
	cJSON *param_array = cJSON_GetObjectItem(root,"params");
	param_defs.hm_param = hm_create();
	int i;
	for (i = 0; i < cJSON_GetArraySize(param_array); i++){
		cJSON *param = cJSON_GetArrayItem(param_array, i);
		parse_param_object(param);
	}
}


void param_load(int dir_type, char *file)
{
	cJSON *root = jparse_open(dir_type, file);
	if(root == NULL) {
		return;
	}
	load_params(root);
	jparse_close(root);
}

void param_list_destroy(void)
{
	if (param_defs.hm_param) {
		hashiterator *ih = hm_get_iterator(param_defs.hm_param);
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
		hm_destroy(param_defs.hm_param);
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
	param_load(WAFFLE_ZIP, "level/object_defaults.json");

	if(param_defs.hm_param == NULL) {
		we_error("could not load object_defaults");
	}
}

void *param_get(const char *type, const char * name)
{
	void *param = NULL;
	char l_type[40], l_name[40];
	strtolower(l_type, type);
	strtolower(l_name, name);

	hashmap *hm_names = (hashmap*)hm_get(param_defs.hm_param,type);

	if (hm_names) {
		param = hm_get(hm_names, l_name);
		if (param) return param;
		param = hm_get(hm_names, "def");
		if (param) return param;
	}
	//SDL_Log("LEVEL: Could not find param: %s for type: %s", l_name, l_type);
	return param;
}
