/*
 * level2.c
 *
 *  Created on: Nov 8, 2013
 *      Author: karlmka
 */


#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "level.h"
#include "obj/object_types.h"

#include "tilemap.h"

#include "we_utils.h"

#include "../engine/engine.h"
#include "../engine/io/waffle_utils.h"

#include "states/space.h"

static int station_count;
//static level_ship *world;

LList solar_systems;

//TODO: Set all default values to something usefull
static char DEF_STRING[10] = "HELLO";

param_list param_defs;

int level_init(void)
{
	srand(0x9b3a09fa);
	solar_systems = llist_create();
	llist_set_remove_callback(solar_systems, solarsystem_destroy);
	SPRITE_ID spr_sun = sprite_link("sun01");
	we_diriter *wed = waffle_get_diriter(WAFFLE_DOCUMENTS, "levels");
	int sun = 0;
	while(waffle_dirnext(wed)){
		if(waffle_isdir(wed)) {
			int stations = 0;
			float rnd = rand() & 0x1f;
			Color base = {0x70-rnd,0x30,0x30+rnd,0xff};
			Color glow = {0xff-rnd,0xa0,0x70+rnd,0x80};
			Color add1 = {0x90-rnd,0x80,0x40+rnd,0x00};
			Color add2 = {0xb0-rnd,0x70,0x40+rnd,0x00};
			solarsystem *sy = solarsystem_create(sun, (500 + we_randf*(sun*1500/2 + (sun>1?1000:0)) + 300*sun/2), spr_sun, base, glow, add1, add2);
			sun++;
			we_diriter *wed2 = waffle_get_diriter(WAFFLE_DOCUMENTS, wed->cur_path);
			while(waffle_dirnext(wed2)) {
				if(waffle_isfile(wed2)) {
					SDL_Log("LEVELS ADDING: %s", wed2->cur_path);
					char temp[64];
					sscanf(wed2->dir->d_name, "%s.json",temp);
					temp[strlen(temp) - 5] = 0;
					SDL_Log("%s", temp);
					solarsystem_add_station(sy, NULL, wed2->dir_type, temp , wed2->cur_path);
					stations++;
				}
			}
			llist_add(solar_systems, sy);
		}
	}
	waffle_free_diriter(wed);


	/* read space station data */
	char buff[10000]; // TODO: stor nok?
	int filesize, i;
	cJSON *root;

	/*filesize = waffle_read_file_zip("space_1.json", &buff[0], 10000);
	if (!filesize) {
		SDL_Log("Could not load level data!");
		exit(1);
	}

	root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", "space_1");
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return NULL;
	}
	cJSON *station_array = cJSON_GetObjectItem(root,"stations");
	station_count = cJSON_GetArraySize(station_array);
	world = calloc(station_count,sizeof(level_ship));

	for (i = 0; i < cJSON_GetArraySize(station_array); i++) {
		cJSON *station = cJSON_GetArrayItem(station_array, i);
		world[i].id = i+1;
		world[i].radius = 100;
		world[i].rotation = 1;
		world[i].rotation_speed = 1;
		world[i].pos.x = cJSON_GetObjectItem(station,"x")->valuedouble;
		world[i].pos.y = cJSON_GetObjectItem(station,"y")->valuedouble;
		strcpy(world[i].level_path, cJSON_GetObjectItem(station,"level")->valuestring);
	}

	if (i != station_count || i <= 0) {
		SDL_Log("Error while loading level data, could not load all stations! (%d of %d loaded)\n", i, station_count);
		exit(2);
	}

	cJSON_Delete(root);*/

	filesize = waffle_read_file_zip("level/object_defaults.json", &buff[0], 10000);
	if (!filesize) {
			SDL_Log("Could not load level data!");
			exit(1);
		}
	root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", "level/object_defaults.json");
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return NULL;
	}
	level_load_params(&(param_defs), root);
	cJSON_Delete(root);

	return 1;
}

LList level_get_world(){
	return solar_systems;
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
	void * param = level_get_param_direct(params, l_type, l_name);
	if (param) return param;

	//SDL_Log("LEVEL: Loading from param_DEFS");
	param = level_get_param_direct(&(param_defs), l_type, l_name);
	if (param) return param;

	param = level_get_param_direct(&(param_defs), l_type, "def");
	if (param) return param;

	//SDL_Log("LEVEL: Could not find param: %s for type: %s", l_name, l_type);
	return NULL;
}

static void parse_param_object(cJSON *param, hashmap * param_list)
{
	char type[32], name[32];
	pl_parse(param,"type","char",type, DEF_STRING);
	pl_parse(param,"name","char",name, DEF_STRING);
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

static void load_tilemap(cJSON *t, level *lvl)
{
	int def = 0;
	int def_col = 48;
	int cols, rows;

	pl_parse(t, "t_cols", "int", &cols, &def_col);
	pl_parse(t, "t_rows", "int", &rows, &def);
	pl_parse(t, "t_layers", "int", &(lvl->tilemap.layers), &def);

	lvl->tilemap.grid = grid_create(cols, lvl->inner_radius, lvl->outer_radius);
	/*
	if (lvl->tilemap.grid->rows != rows) {
        fprintf(stderr, "WARNING load_tilemap: inconsistent row count. Got %d, expected %d! Setting row count to expected value\n", lvl->tilemap.grid->rows, rows);
        lvl->tilemap.grid->rows = rows;
    }
	*/
	cJSON *data = cJSON_GetObjectItem(t,"tilemaptest");
	if(data) {
		int i,j,k;
		for (i=0; i < lvl->tilemap.layers; i++) {
			cJSON * row = cJSON_GetArrayItem(data,i);
			for (j= 0; j < lvl->tilemap.grid->rows; j++) {
				cJSON * col = cJSON_GetArrayItem(row,j);
				for (k = 0; k < lvl->tilemap.grid->cols; k++) {
					lvl->tilemap.data[i][j][k] = cJSON_GetArrayItem(col,k)->valueint;
				}
			}
		}
	}

}

#define FILE_SIZE_BUFFER 128000
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

	int i;
	cJSON *root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", filename);
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return NULL;
	}

	// GET level name
	strcpy(lvl->name, cJSON_GetObjectItem(root,"name")->valuestring);

	/*//old tilemap
	//GET tilemap name
	char tilemap_name[30];
	cJSON *tilemap = cJSON_GetObjectItem(root,"tilemap");
	if(tilemap){
		strcpy(tilemap_name, tilemap->valuestring);
		lvl->tiles = calloc(1, sizeof *lvl->tiles);
		if (tilemap_create(lvl->tiles, tilemap_name)) {
			SDL_Log("Error while parsing level header. Could not load tilemap %s.\n", tilemap_name);
			return NULL;
		}
		lvl->height = lvl->tiles->height*lvl->tiles->tile_height;
		lvl->width = (lvl->tiles->width*lvl->tiles->tile_width);
	} else{
			SDL_Log("LEVEL: No field with name: tilemap");
	}
	*/
	float def = 0;
	float def_m = 500;
	float def_M = 2000;
	pl_parse(root,"innrad","float",&(lvl->inner_radius),&def_m);
	pl_parse(root,"outrad","float",&(lvl->outer_radius),&def_M);
	load_tilemap(root, lvl);

	level_load_params(&(lvl->params), root);

	cJSON * object_array = cJSON_GetObjectItem(root,"objects");

	for (i = 0; i < cJSON_GetArraySize(object_array); i++){
		cJSON *object = cJSON_GetArrayItem(object_array, i);


		char type[128], name[128];
		pl_parse(object,"type", "char", type, DEF_STRING);
		pl_parse(object,"name", "char", name, DEF_STRING);

		strtolower(type, type);
		strtolower(name, name);

		SDL_Log("ADDING OBJECT TYPE: %s name: %s ", type, name);
		cJSON *pos = cJSON_GetObjectItem(object,"pos");
		cpVect p = cpvzero;
		if(pos != NULL){
			float def = 100;
			float x,y;
			//&(p.x) do not work because its a double
			pl_parse(pos,"x","float",&(x),&def);
			pl_parse(pos,"y","float",&(y),&def);
			p.x = x;
			p.y = y;
		} else {
			SDL_Log("LEVEL PARSING ERROR: Cannot find field pos in object");
		}

		level_add_object_recipe_name(lvl, type, name, p, 0);
	}
	return lvl;
}


void level_load_params(param_list *defs, cJSON *root)
{
	cJSON *param_array = cJSON_GetObjectItem(root,"params");
	defs->param = hm_create();
	int i;
	for (i = 0; i < cJSON_GetArraySize(param_array); i++){
		cJSON *param = cJSON_GetArrayItem(param_array, i);
		parse_param_object(param, defs->param);
	}
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
	rec->param = level_get_param(&(lvl->params), data_type, data_name);
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
		instance *ins = instance_create(data->obj_type, data->param, data->pos, cpvzero);
	}
	llist_end_loop(lvl->level_data);
}

void level_clear_objects(level *lvl)
{
	llist_clear(lvl->level_data);
}

void level_unload(level *lvl)
{
	tilemap_destroy(lvl->tiles);
	level_destry_param_list(&(lvl->params));
	free(lvl->tiles);
	free(lvl);
}

void level_destry_param_list(param_list *params)
{
	hashiterator *it = hm_get_iterator(params->param);
	hashmap * h = hm_iterator_next(it);
	while(h != NULL) {
		hashiterator *it2 = hm_get_iterator(h);
		void * data = hm_iterator_next(it2);
		while(data != NULL) {
			free(data);
			data = hm_iterator_next(it2);
		}
		hm_iterator_destroy(it2);
		hm_destroy(h);
		h = hm_iterator_next(it);
	}
	hm_iterator_destroy(it);
	hm_destroy(params->param);
}

void level_write_to_file(level *lvl)
{
	char filename[200] = "levels/karlmka/test.json";
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
	cJSON_AddNumberToObject(root, "innrad", lvl->inner_radius);
	cJSON_AddNumberToObject(root, "outrad", lvl->outer_radius);

	cJSON_AddNumberToObject(root, "t_layers", lvl->tilemap.layers);
	cJSON_AddNumberToObject(root, "t_rows", lvl->tilemap.grid->rows);
	cJSON_AddNumberToObject(root, "t_cols", lvl->tilemap.grid->cols);

	cJSON * tilemap = cJSON_CreateArray();
	int i,j,k;
	for (i=0; i < lvl->tilemap.layers; i++) {
		cJSON * row = cJSON_CreateArray();
		for (j= 0; j < lvl->tilemap.grid->rows; j++) {
			int temp[lvl->tilemap.grid->cols];
			for (k = 0; k < lvl->tilemap.grid->cols; k++) {
				temp[k] = lvl->tilemap.data[i][j][k];
			}
			cJSON * col = cJSON_CreateIntArray(temp, lvl->tilemap.grid->cols);
			cJSON_AddItemToArray(row, col);
		}
		cJSON_AddItemToArray(tilemap, row);
	}

	cJSON * param_array = cJSON_CreateArray();

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
	station_count = 0;
}
