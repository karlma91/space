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

#include "level.h"
#include "obj/object_types.h"

#include "tilemap.h"

#include "we_utils.h"

#include "../engine/engine.h"
#include "../engine/io/waffle_utils.h"

#include "states/space.h"

#define FILE_SIZE_BUFFER 128000

static int station_count;
//static level_ship *world;

LList user_system;
LList game_system;


//TODO: Set all default values to something usefull
static char DEF_STRING[10] = "HELLO";

param_list param_defs;

LList load_solarsystem_file(cJSON *root);

int level_init(void)
{
	srand(0x9b3a09fa);

	char buff[FILE_SIZE_BUFFER];
	cJSON *root;
	int filesize = waffle_read_file_zip("level/object_defaults.json", &buff[0], 10000);
	if (!filesize) {
		SDL_Log("Could not load level data!");
		exit(1);
	}
	root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", "level/object_defaults.json");
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return 0;
	}
	level_load_params(&(param_defs), root);
	cJSON_Delete(root);

	return 1;
}


void level_load_solar()
{
	char buff[FILE_SIZE_BUFFER];
	int filesize = 0;
	FILE *f = waffle_fopen(WAFFLE_DOCUMENTS, "levels/userlevels.json", "r");
	if(f){
		filesize = waffle_read(f, buff, FILE_SIZE_BUFFER);
	}

	if(!filesize) {
		SDL_Log("DEBUG: Could not load userlevels.json");
		user_system = llist_create();
	} else {
	cJSON *root;
	root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", "level/userlevels.json");
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
	} else {
		user_system = load_solarsystem_file(root);
	}
	}
	level_load_levels_from_folder(user_system);
}


LList load_solarsystem_file(cJSON *root)
{
	SDL_Log("LOADING SOLARSYSTEMS");
	LList world = llist_create();
	llist_set_remove_callback(world, (ll_rm_callback)solarsystem_destroy);


	cJSON *systems = cJSON_GetObjectItem(root,"systems");
	if(systems) {
		int i,j;
		char name[256];
		char folder[256];
		char path[256];
		char filename[256];
		char author[256];
		SPRITE_ID spr;
		Color col;
		int size;
		for (i=0; i < cJSON_GetArraySize(systems); i++) {
			cJSON * sun = cJSON_GetArrayItem(systems,i);
			pl_parse(sun, "name", "char", &name, DEF_STRING);
			pl_parse(sun, "folder", "char", &folder, DEF_STRING);
			SDL_Log("CREATING SUN)");
			solarsystem *sy = solarsystem_create(i, name, folder);

			cJSON * levels = cJSON_GetObjectItem(sun,"levels");
			for (j= 0; j < cJSON_GetArraySize(levels); j++) {
				cJSON * lvl = cJSON_GetArrayItem(levels,j);
				pl_parse(lvl, "author", "char", author, DEF_STRING);
				pl_parse(lvl, "name", "char", name, DEF_STRING);
				pl_parse(lvl, "filename", "char", filename, DEF_STRING);
				pl_parse(lvl, "sprite", "sprite", &(spr), SPRITE_STATION001);
				pl_parse(lvl, "size", "int ", &size, 0);
				pl_parse(lvl, "color", "Color", &col, 0);
				sprintf(path, "levels/%s/%s", folder, filename);
				FILE* f =  waffle_fopen(WAFFLE_DOCUMENTS, path, "r");
				if(f == NULL){
					SDL_Log("%s does not exist", path);
				}else{
					fclose(f);
					SDL_Log("CREATING station %s", path);
					solarsystem_add_station(sy, spr,col, WAFFLE_DOCUMENTS, name , path, author, filename);
				}
			}
			llist_add(world,sy);
		}
		/*if(llist_size(sy->stations)){
				llist_add(world,sy);
			}else{
				solarsystem_destroy(sy);
			}*/
	} else {
		SDL_Log("Could not find field systems");
		SDL_Log("%s", cJSON_GetErrorPtr());
	}
	return world;
}

char * color_to_string(Color c, char *t){
	sprintf(t,"%02X%02X%02X%02X", c.r, c.b, c.g, c.a);
	return t;
}
void level_write_solar_file(LList world)
{
	FILE *file = waffle_fopen(WAFFLE_DOCUMENTS, "levels/userlevels.json","w");
	if (file == NULL) {
		SDL_Log( "Could not open levels/userlevels.json");
		return;
	}
	cJSON *root;//,*fmt;
	root = cJSON_CreateObject();
	cJSON * systems = cJSON_CreateArray();
	llist_begin_loop(world);
	while (llist_hasnext(world)) {
		solarsystem *sy = llist_next(world);

		cJSON *system = cJSON_CreateObject();
		cJSON_AddItemToObject(system, "name", cJSON_CreateString(sy->name));
		cJSON_AddItemToObject(system, "folder", cJSON_CreateString(sy->folder));

		cJSON * levels = cJSON_CreateArray();
		llist_begin_loop(sy->stations);
		while (llist_hasnext(sy->stations)) {
			station *st = llist_next(sy->stations);

			cJSON *station = cJSON_CreateObject();
			cJSON_AddItemToObject(station, "author", cJSON_CreateString(st->author));
			cJSON_AddItemToObject(station, "name", cJSON_CreateString(st->name));
			cJSON_AddItemToObject(station, "filename", cJSON_CreateString(st->filename));
			cJSON_AddItemToObject(station, "sprite", cJSON_CreateString(sprite_get_name(st->spr_id)));
			cJSON_AddNumberToObject(station, "size", st->radius);
			char c[10];
			cJSON_AddItemToObject(station, "color", cJSON_CreateString(color_to_string(st->col, c)));

			cJSON_AddItemToArray(levels, station);
		}
		llist_end_loop(sy->stations);

		cJSON_AddItemToObject(system, "levels", levels);
		cJSON_AddItemToArray(systems, system);
		cJSON_AddItemToObject(root, "systems", systems);
	}
	llist_end_loop(world);
	char * rendered = cJSON_Print(root);
	fprintf(file,"%s",rendered);
	fclose(file);
	cJSON_Delete(root);

}

void level_load_levels_from_folder(LList world)
{
	we_diriter *wed = waffle_get_diriter(WAFFLE_DOCUMENTS, "levels");
	while(waffle_dirnext(wed)){
		if(waffle_isdir(wed)) {
			solarsystem *sy = solarsystem_get_from_folder(world, wed->dir->d_name);
			if(sy == NULL){
				SDL_Log("Creating new solarsystem %s,  %s", wed->dir->d_name, wed->dir->d_name );
				sy = solarsystem_create(llist_size(world), wed->dir->d_name, wed->dir->d_name);
				llist_add(world, sy);
			}else{
				SDL_Log("Found solarsystem for folder %s", wed->cur_path);
			}
			we_diriter *wed2 = waffle_get_diriter(WAFFLE_DOCUMENTS, wed->cur_path);
			while(waffle_dirnext(wed2)) {
				if(waffle_isfile(wed2)) {
					SDL_Log("LOOKING FOR %s",  wed2->dir->d_name);
					station *st = solarsystem_get_station(sy, wed2->cur_path);
					if(st == NULL){
						SDL_Log("LEVELS ADDING: %s", wed2->cur_path);
						char temp[64];
						sscanf(wed2->dir->d_name, "%s.json", temp);
						temp[strlen(temp) - 5] = 0;
						solarsystem_add_station(sy, SPRITE_COIN, COL_BLACK, wed2->dir_type, temp , wed2->cur_path, "author", wed2->dir->d_name);
					}else{
						SDL_Log("Found station %s in folder %s",wed2->dir->d_name, wed2->path);
					}
				}
			}
		}
	}
	waffle_free_diriter(wed);
}


LList level_get_world(){
	return user_system;
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
	int cols;

	int def_ii = 8;
	int def_oi = 32;
	int ii, oi;
	float min_rad;
	float min_rad_def = 250;

	pl_parse(t, "t_layers", "int", &(lvl->tilemap.layers), &def);
	pl_parse(t, "t_cols", "int", &cols, &def_col);
	pl_parse(t, "t_inner_i", "int", &ii, &def_ii);
	pl_parse(t, "t_outer_i", "int", &oi, &def_oi);
	pl_parse(t, "t_min_rad", "float", &min_rad, &min_rad_def);

	lvl->tilemap.grid = grid_create(cols, min_rad, GRID_MAXROW-1, ii, oi);

	cJSON *data = cJSON_GetObjectItem(t,"tilemaptest");
	if(data) {
		int i,j,k;
		for (i=0; i < lvl->tilemap.layers; i++) {
			cJSON * row = cJSON_GetArrayItem(data,i);
			for (j= 0; j < lvl->tilemap.grid->rows; j++) {
				cJSON * col = cJSON_GetArrayItem(row,j);
				for (k = 0; k < lvl->tilemap.grid->cols; k++) {
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
	for (j= 0; j < lvl->tilemap.grid->rows; j++) {
		meta_def.y_row = j;
		for (k = 0; k < lvl->tilemap.grid->cols; k++) {
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
	load_tilemap(root, lvl);

	/* Calculate level size based on grid data */
	lvl->inner_radius = lvl->tilemap.grid->rad[lvl->tilemap.grid->inner_i];
	lvl->outer_radius = lvl->tilemap.grid->rad[lvl->tilemap.grid->outer_i-1];;
	lvl->height = lvl->outer_radius - lvl->inner_radius;

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
	level_destry_param_list(&(lvl->params));
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
	cJSON_AddNumberToObject(root, "t_cols", lvl->tilemap.grid->cols);
	cJSON_AddNumberToObject(root, "t_inner_i", lvl->tilemap.grid->inner_i);
	cJSON_AddNumberToObject(root, "t_outer_i", lvl->tilemap.grid->outer_i);
	cJSON_AddNumberToObject(root, "t_min_rad", lvl->tilemap.grid->min_rad);

	cJSON * tilemap = cJSON_CreateArray();
	int ii = lvl->tilemap.grid->inner_i;
	int i,j,k;
	for (i=0; i < lvl->tilemap.layers; i++) {
		cJSON * row = cJSON_CreateArray();
		for (j= 0; j < lvl->tilemap.grid->rows; j++) {
			int temp[lvl->tilemap.grid->cols];
			for (k = 0; k < lvl->tilemap.grid->cols; k++) {
				temp[k] = lvl->tilemap.data[i][j+ii][k];
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
