/*
 * level2.c
 *
 *  Created on: Nov 8, 2013
 *      Author: karlmka
 */


//TODO SPLIT INTO SMALLER MODULES

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "we_object.h"

#include "level.h"
#include "obj/object_types.h"

#include "tilemap.h"

#include "we_utils.h"

#include "../engine/engine.h"
#include "../engine/io/waffle_utils.h"

#include "states/space.h"

#define FILE_SIZE_BUFFER 128000

param_list param_defs;

int level_init(void)
{
	srand(0x9b3a09fa);
	param_list *l = param_load(WAFFLE_ZIP, "level/object_defaults.json");
	param_defs = *l;

	if(param_defs.param == NULL) {
		we_error("could not load object_defaults");
	}
	return 1;
}

static void * level_get_param_direct(param_list *params, char *type, char * name)
{
	hashmap *names = (hashmap*)hm_get(params->param,type);

	if(names) {
		void *data = hm_get(names, name);
		if(data){
			return data;
		}else{
			//SDL_Log("LEVEL: could not find %s of type %s", name, type);
		}
	} else {
		//SDL_Log("LEVEL: Could not find type %s", type);
	}
	return NULL;
}

void * level_get_param(param_list *params, const char *type, const char * name)
{
	char l_type[40];
	char l_name[40];

	strtolower(l_type, type);
	strtolower(l_name, name);
	void * param = level_get_param_direct(&(param_defs), l_type, l_name);
	if (param) return param;

	//SDL_Log("LEVEL: Loading from param_DEFS");
	param = level_get_param_direct(&(param_defs), l_type, l_name);
	if (param) return param;

	param = level_get_param_direct(&(param_defs), l_type, "def");
	if (param) return param;

	//SDL_Log("LEVEL: Could not find param: %s for type: %s", l_name, l_type);
	return NULL;
}

static void load_tilemap(cJSON *t, level *lvl)
{
	int def = 0;
	int def_col = 48;
	int cols;

	int def_ii = 8;
	int def_oi = 32;
	int ii, oi;
	float min_rad;
	float min_rad_def = 250;

	jparse_parse(t, "t_layers", "int", &(lvl->tilemap.layers), &def);
	jparse_parse(t, "t_cols", "int", &cols, &def_col);
	jparse_parse(t, "t_inner_i", "int", &ii, &def_ii);
	jparse_parse(t, "t_outer_i", "int", &oi, &def_oi);
	jparse_parse(t, "t_min_rad", "float", &min_rad, &min_rad_def);

	lvl->tilemap.grid = grid_create(cols, min_rad, GRID_MAXROW-1, ii, oi);

	cJSON *data = cJSON_GetObjectItem(t,"tilemaptest");
	if(data) {
		int i,j,k;
		for (i=0; i < lvl->tilemap.layers; i++) {
			cJSON * row = cJSON_GetArrayItem(data,i);
			for (j= 0; j < lvl->tilemap.grid->pol.rows; j++) {
				cJSON * col = cJSON_GetArrayItem(row,j);
				for (k = 0; k < lvl->tilemap.grid->pol.cols; k++) {
					lvl->tilemap.data[i][j+ii][k] = cJSON_GetArrayItem(col,k)->valueint;
				}
			}
		}
	}

	meta_tile meta_def;
	meta_def.block = NULL;
	meta_def.destroyable = WE_FALSE;
	meta_def.hp = 0;

	int j,k;
	for (j= 0; j < lvl->tilemap.grid->pol.rows; j++) {
		meta_def.y_row = j;
		for (k = 0; k < lvl->tilemap.grid->pol.cols; k++) {
			meta_def.x_col = k;
			lvl->tilemap.metadata[j][k] = meta_def;
		}
	}
}

level *level_load(int folder, char * filename)
{
	SDL_Log("PARSING LEVEL : %s", filename);
	char file_path[100];
	char buff[FILE_SIZE_BUFFER]; // TODO: stor nok? (10kb ikke stort nok!)
	int filesize;

	if(folder == WAFFLE_DOCUMENTS||
			folder == WAFFLE_LIBRARY){
		FILE *f = waffle_fopen(WAFFLE_DOCUMENTS, filename, "r");
		filesize = waffle_read(f, buff, FILE_SIZE_BUFFER);
	}else if(folder == WAFFLE_ZIP ) {
		sprintf(file_path,"level/%s.json", filename);
		filesize = waffle_read_file_zip(file_path, buff, FILE_SIZE_BUFFER);
	}


	if (filesize == 0) {
		SDL_Log("Could not load level %s filesize = 0", filename);
		return NULL;
	}

	level *lvl = calloc(1, sizeof(level));
	lvl->level_data = llist_create();
	llist_set_remove_callback(lvl->level_data, free);

	cJSON *root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", filename);
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return NULL;
	}

	// GET level name
	strcpy(lvl->name, cJSON_GetObjectItem(root,"name")->valuestring);

	load_tilemap(root, lvl);

	/* Calculate level size based on grid data */
	lvl->inner_radius = lvl->tilemap.grid->pol.rad[lvl->tilemap.grid->pol.inner_i];
	lvl->outer_radius = lvl->tilemap.grid->pol.rad[lvl->tilemap.grid->pol.outer_i-1];;
	lvl->height = lvl->outer_radius - lvl->inner_radius;


	cJSON * object_array = cJSON_GetObjectItem(root,"objects");

	objrecipe_load_objects(lvl->level_data,object_array,&(param_defs));
	return lvl;
}

void level_add_object_recipe_name(level * lvl, const char * obj_type, const char * param_name, cpVect pos, float rotation)
{

	if(!obj_type){
		SDL_Log("LEVEL: INVALID OBJECT TYPE");
		return;
	}

	if(!param_name){
		SDL_Log("LEVEL: INVALID param NAME");
		return;
	}

	char data_type[32];
	char data_name[32];

	strtolower(data_type, obj_type);
	strtolower(data_name, param_name);

	SDL_Log("LEVEL: ADDING %s,  %s", data_type, data_name);

	object_recipe * rec = calloc(1, sizeof(object_recipe));
	rec->obj_type = object_by_name(data_type);
	rec->param = level_get_param(&(param_defs), data_type, data_name);
	strcpy(rec->param_name, data_name);
	rec->pos = pos;
	rec->rotation = rotation;
	llist_set_remove_callback(lvl->level_data, free);
	llist_add(lvl->level_data, rec);
}

void level_add_object_recipe(level * lvl, object_id *obj_id, const char * param_name, void * param, cpVect pos, float rotation)
{
	object_recipe * rec = calloc(1,sizeof(object_recipe));
	rec->obj_type = obj_id;
	rec->param = param;
	rec->pos = pos;
	rec->rotation = rotation;
	llist_add(lvl->level_data, rec);
}

void level_start_level(level *lvl)
{
	cpResetShapeIdCounter();
	llist_begin_loop(lvl->level_data);
	while(llist_hasnext(lvl->level_data)) {
		object_recipe * data = llist_next(lvl->level_data);
		SDL_Log("INSTANTIATING TYPE: %s at: %f ", data->obj_type->NAME, data->pos.x);
		instance_create(data->obj_type, data->param, data->pos, cpvzero);
	}
	llist_end_loop(lvl->level_data);
}

void level_clear_objects(level *lvl)
{
	llist_clear(lvl->level_data);
}

void level_unload(level *lvl)
{
	free(lvl);
}

void level_write_to_file(level *lvl)
{
	char filename[200];
	sprintf(filename, "levels/karlmka/%s.json",lvl->name);
	FILE *file = waffle_fopen(WAFFLE_DOCUMENTS, filename,"w");
	if (file == NULL) {
		SDL_Log( "Could not open %s\n",filename);
		return;
	}

	cJSON *root;//,*fmt;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name", cJSON_CreateString(lvl->name));
	cJSON_AddItemToObject(root, "tilemap", cJSON_CreateString("level02_01.tmx"));
	cJSON_AddNumberToObject(root, "timelimit", 100);

	cJSON_AddNumberToObject(root, "t_layers", lvl->tilemap.layers);
	cJSON_AddNumberToObject(root, "t_cols", lvl->tilemap.grid->pol.cols);
	cJSON_AddNumberToObject(root, "t_inner_i", lvl->tilemap.grid->pol.inner_i);
	cJSON_AddNumberToObject(root, "t_outer_i", lvl->tilemap.grid->pol.outer_i);
	cJSON_AddNumberToObject(root, "t_min_rad", lvl->tilemap.grid->pol.min_rad);

	cJSON * tilemap = cJSON_CreateArray();
	int ii = lvl->tilemap.grid->pol.inner_i;
	int i,j,k;
	for (i=0; i < lvl->tilemap.layers; i++) {
		cJSON * row = cJSON_CreateArray();
		for (j= 0; j < lvl->tilemap.grid->pol.rows; j++) {
			int temp[lvl->tilemap.grid->pol.cols];
			for (k = 0; k < lvl->tilemap.grid->pol.cols; k++) {
				temp[k] = lvl->tilemap.data[i][j+ii][k];
			}
			cJSON * col = cJSON_CreateIntArray(temp, lvl->tilemap.grid->pol.cols);
			cJSON_AddItemToArray(row, col);
		}
		cJSON_AddItemToArray(tilemap, row);
	}

	cJSON * param_array = cJSON_CreateArray();

	cJSON * object_array = cJSON_CreateArray();
	llist_begin_loop(lvl->level_data);
	while(llist_hasnext(lvl->level_data)) {
		object_recipe * data = llist_next(lvl->level_data);
		cJSON *object = cJSON_CreateObject();
		cJSON_AddItemToObject(object, "type", cJSON_CreateString(data->obj_type->NAME));
		cJSON_AddItemToObject(object, "name", cJSON_CreateString(data->param_name));
		cJSON *pos = cJSON_CreateObject();
		cJSON_AddNumberToObject(pos,"x",data->pos.x);
		cJSON_AddNumberToObject(pos,"y",data->pos.y);
		cJSON_AddNumberToObject(pos,"r",data->rotation);
		cJSON_AddItemToObject(object,"pos", pos);
		cJSON_AddItemToArray(object_array, object);
	}

	cJSON_AddItemToObject(root,"tilemaptest", tilemap);
	cJSON_AddItemToObject(root,"params", param_array);
	cJSON_AddItemToObject(root,"objects", object_array);

	char * rendered = cJSON_Print(root);
	//SDL_Log("%s", rendered);
	fprintf(file,"%s",rendered);
	fclose(file);
	// TODO: Write to file
	cJSON_Delete(root);
}


void level_destroy(void)
{

}
