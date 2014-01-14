/*
 * level2.c
 *
 *  Created on: Nov 8, 2013
 *      Author: karlmka
 */


#include <stdlib.h>
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
static level_ship *world;

//TODO: Set all default values to something usefull
static char DEF_STRING[10] = "HELLO";
static SPRITE_ID DEF_SPRITE = NULL;
static EMITTER_ID DEF_EMITTER = NULL;
static polyshape DEF_SHAPE = NULL;

param_list param_defs;

int level_init(void)
{
	/* read space station data */
	char buff[10000]; // TODO: stor nok?
	int filesize = waffle_read_file("space_1.json", &buff[0], 10000);
	if (!filesize) {
		SDL_Log("Could not load level data!");
		exit(1);
	}

	int i;
	cJSON *root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", "space_1");
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return NULL;
	}
	cJSON *station_array = cJSON_GetObjectItem(root,"stations");
	station_count = cJSON_GetArraySize(station_array);
	world = calloc(station_count,sizeof(level_ship));

	for (i = 0; i < cJSON_GetArraySize(station_array); i++){
		cJSON *station = cJSON_GetArrayItem(station_array, i);
		world[i].id = i+1;
		world[i].radius = 100;
		world[i].rotation = 1;
		world[i].rotation_speed = 1;
		world[i].pos.x = cJSON_GetObjectItem(station,"x")->valuedouble;
		world[i].pos.y = cJSON_GetObjectItem(station,"y")->valuedouble;
		strcpy(world[i].level_name, cJSON_GetObjectItem(station,"level")->valuestring);
	}

	if (i != station_count || i <= 0) {
		SDL_Log("Error while loading level data, could not load all stations! (%d of %d loaded)\n", i, station_count);
		exit(2);
	}

	cJSON_Delete(root);

	filesize = waffle_read_file("level/object_defaults.json", &buff[0], 10000);
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



static void * level_get_param_direct(param_list *params, char *type, char * name)
{
	hashmap *names = (hashmap*)hm_get(params->param,type);
	return hm_get(names, name);
}

void * level_get_param(param_list *params, const char *type, const char * name)
{
	char l_type[40], l_name[40];
	strtolower(l_type, type);
	strtolower(l_name, name);

	void * param = level_get_param_direct(params, l_type, l_name);
	if (param) return param;

	param = level_get_param_direct(&(param_defs), l_type, l_name);
	if (param) return param;

	param = level_get_param_direct(&(param_defs), l_type, "def");
	if (param) return param;

	SDL_Log("LEVEL: Could not find type crash %s", l_type);
	return NULL;
}



/**
 * Parse an int from cJSON struct
 */
int level_safe_parse_int(cJSON *param, char *name )
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if(t!=NULL){
		return t->valueint;
	}
	SDL_Log("Could not load param int %s", name);
	return 0;
}

/**
 * Parse an double from cJSON struct
 */
double level_safe_parse_float(cJSON *param, char *name )
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if(t!=NULL){
		return t->valuedouble;
	}
	SDL_Log("Could not load param int %s", name);
	return 0;
}

/**
 * Parse an string char * from cJSON struct
 */
char* level_safe_parse_char(cJSON *param, char *name )
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if(t!=NULL){
		return t->valuestring;
	}
	SDL_Log("Could not load param char %s", name);
	return DEF_STRING;
}

/**
 * Parse an SPRITE_ID from cJSON struct
 */
SPRITE_ID level_safe_parse_sprite(cJSON *param, char *name )
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if (t != NULL) {
		return sprite_link(t->valuestring);
	}
	SDL_Log("Could not load sprite %s", name);
	return DEF_SPRITE;
}

/**
 * Parse an EMITTER_ID from cJSON struct
 */
EMITTER_ID level_safe_parse_emitter(cJSON *param, char *name )
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if (t != NULL) {
		return particles_bind_emitter(t->valuestring);
	}
	SDL_Log("Could not load emitter %s", name);
	return DEF_EMITTER;
}

/**
 * Parse an Mix_Chunk from cJSON struct
 */
Mix_Chunk * level_safe_parse_sound(cJSON *param, char *name )
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if (t != NULL) {
		return sound_loadchunk(t->valuestring);
	}
	SDL_Log("Could not load sound %s", name);
	return NULL;
}

/**
 * Parse an texture from cJSON struct
 */
int level_safe_parse_texture(cJSON *param, char *name )
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if (t != NULL) {
		return texture_load(t->valuestring);
	}
	SDL_Log("Could not load texture %s", name);
	return NULL;
}

/**
 * Parse an polyshape from cJSON struct
 */
polyshape level_safe_parse_shape(cJSON *param, char *name)
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if(t != NULL){
		return shape_read(t->valuestring);
	}
	SDL_Log("LEVEL: Could not load shape %s", name);
	return DEF_SHAPE;
}

/**
 * Parse an object_id from cJSON struct
 */
object_id* level_safe_parse_object_id(cJSON *param, char *name)
{
	cJSON *t = cJSON_GetObjectItem(param, name);
	if (t != NULL) {
			return object_by_name(t->valuestring);
	}
	SDL_Log("Could not load object_id %s", name);
	return NULL;
}

static void parse_param_object(cJSON *param, hashmap * param_list)
{
	char * type = level_safe_parse_char(param,"type");
	char * name = level_safe_parse_char(param,"name");
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

level *level_load(char * filename)
{
	SDL_Log("PARSING LEVEL : %s", filename);
	char file_path[100];
	sprintf(file_path,"level/%s.json", filename);
	char buff[10000]; // TODO: stor nok?
	int filesize = waffle_read_file(file_path, buff, 10000);

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

	//GET tilemap name
	char tilemap_name[30];
	strcpy(tilemap_name, cJSON_GetObjectItem(root,"tilemap")->valuestring);
	lvl->tiles = calloc(1, sizeof *lvl->tiles);
	if (tilemap_create(lvl->tiles, tilemap_name)) {
		SDL_Log("Error while parsing level header. Could not load tilemap %s.\n", tilemap_name);
		return NULL;
	}
	lvl->height = lvl->tiles->height*lvl->tiles->tile_height;
	lvl->left = -(lvl->tiles->width*lvl->tiles->tile_width)/2;
	lvl->right = (lvl->tiles->width*lvl->tiles->tile_width)/2;
	lvl->width = (lvl->tiles->width*lvl->tiles->tile_width);

	lvl->inner_radius = lvl->width/(WE_2PI);
	//lvl->outer_radius = lvl->inner_radius + lvl->height;
	lvl->outer_radius = lvl->inner_radius + lvl->height;


	level_load_params(&(lvl->params), root);

	cJSON * object_array = cJSON_GetObjectItem(root,"objects");

	for (i = 0; i < cJSON_GetArraySize(object_array); i++){
		cJSON *object = cJSON_GetArrayItem(object_array, i);

		char *type = level_safe_parse_char(object,"type");
		char *name = level_safe_parse_char(object,"name");
		strtolower(type, type);
		strtolower(name, name);

		SDL_Log("ADDING OBJECT TYPE: %s name: %s ", type, name);
		cJSON *pos = cJSON_GetObjectItem(object,"pos");
		cpVect p = cpvzero;
		if(pos != NULL){
			p.x = level_safe_parse_float(pos,"x");
			p.y = level_safe_parse_float(pos,"y");
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
	cJSON *root;//,*fmt;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name", cJSON_CreateString(lvl->name));
	cJSON_AddItemToObject(root, "tilemap", cJSON_CreateString("level02_01.tmx"));
	cJSON_AddNumberToObject(root, "timelimit",100);

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

	cJSON_AddItemToObject(root,"params", param_array);
	cJSON_AddItemToObject(root,"objects", object_array);

	char * rendered = cJSON_Print(root);
	SDL_Log("%s", rendered);
	// TODO: Write to file
	cJSON_Delete(root);
}

void level_get_ships(level_ship **ship, int *count)
{
	*ship = world;
	*count = station_count;
}

level_ship* level_get_world()
{
	return world;
}

int level_get_station_count(void)
{
	return station_count;
}

void level_destroy(void)
{
	station_count = 0;
	free(world);
}
