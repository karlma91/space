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

#include "../engine/engine.h"
#include "../engine/io/waffle_utils.h"

#include "cJSON.h"

#include "states/space.h"

static int station_count;
static level_ship *world;

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
			world[i].count = 1;
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
}

void * level_get_param(hashmap * h, char *type, char * name)
{
	hashmap *names = (hashmap*)hm_get(h,type);
	if(names) {
		void *data = hm_get(names, name);
		if(data){
			return data;
		}else{
			SDL_Log("could not find %s in type %s names", name, type);
		}
	} else {
		SDL_Log("Could not find type %s", type);
	}
	return NULL;
}

static void parse_param_object(cJSON *param, hashmap * param_list)
{

	char * type = cJSON_GetObjectItem(param,"type")->valuestring;
	char * name = cJSON_GetObjectItem(param,"name")->valuestring;
	SDL_Log("PARSING TYPE: %s name: %s ", type, name);

	hashmap * names;
	names = hm_get(param_list, type);
	if(names == NULL) {
		names = hm_create();
		hm_add(param_list, type, names);
	}

	object_id *obj_id = object_by_name(type);

	/* add new sub object definition */
	union {
		obj_param_tank tank;
		obj_param_rocket rocket;
		obj_param_turret turret;
		obj_param_factory factory;
		obj_param_robotarm robotarm;
	} arg;

	if (obj_id == obj_id_tank) {
		arg.tank.max_hp = cJSON_GetObjectItem(param,"max_hp")->valueint;
		arg.tank.coins =  cJSON_GetObjectItem(param,"coins")->valueint;
	} else if (obj_id == obj_id_turret) {
		arg.turret.max_hp =         cJSON_GetObjectItem(param,"max_hp")->valueint;
		arg.turret.coins =          cJSON_GetObjectItem(param,"coins")->valueint;
		arg.turret.shoot_interval = cJSON_GetObjectItem(param,"shoot_interval")->valuedouble;
		arg.turret.burst_number =   cJSON_GetObjectItem(param,"burst_number")->valuedouble;
	} else if (obj_id == obj_id_rocket) {
		arg.rocket.max_hp =       cJSON_GetObjectItem(param,"max_hp")->valueint;
		arg.rocket.coins =        cJSON_GetObjectItem(param,"coins")->valueint;
	} else if (obj_id == obj_id_factory) {
		arg.factory.max_tanks =   cJSON_GetObjectItem(param,"max_tanks")->valueint;
		arg.factory.max_hp =      cJSON_GetObjectItem(param,"max_hp")->valueint;
		arg.factory.spawn_delay = cJSON_GetObjectItem(param,"spawn_delay")->valueint;
		arg.factory.coins =       cJSON_GetObjectItem(param,"coins")->valueint;
		strcpy(arg.factory.shape_name, cJSON_GetObjectItem(param,"shape")->valuestring);
		strcpy(arg.factory.sprite_name, cJSON_GetObjectItem(param,"sprite")->valuestring);
		cJSON * object_spawn = cJSON_GetObjectItem(param,"object_spawn");
		arg.factory.type = object_by_name(cJSON_GetObjectItem(object_spawn,"type")->valuestring);
		strcpy(arg.factory.param_name, cJSON_GetObjectItem(object_spawn,"name")->valuestring);
		strcpy(arg.factory.type_name, cJSON_GetObjectItem(object_spawn,"type")->valuestring);

	}else if (obj_id == obj_id_robotarm) {
		arg.robotarm.max_hp = cJSON_GetObjectItem(param,"max_hp")->valueint;
		arg.robotarm.coins =  cJSON_GetObjectItem(param,"coins")->valueint;
	}

	const int paramsize = obj_id->P_SIZE;

	void * data = calloc(1, paramsize);

	memcpy(data, &arg, paramsize);

	if(hm_add(names, name, data)) {
		SDL_Log("param of type %s with name %s was already in leveldata", type, name);
	}
}

level *level_load(char * filename)
{

	SDL_Log("PARSING LEVEL : %s", filename);
	SDL_Log("PARSING LEVEL : %s", filename);
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
	lvl->outer_radius = lvl->inner_radius + lvl->height;

	cJSON *param_array = cJSON_GetObjectItem(root,"params");

	lvl->param_list = hm_create();

	for (i = 0; i < cJSON_GetArraySize(param_array); i++){
		cJSON *param = cJSON_GetArrayItem(param_array, i);
		parse_param_object(param, lvl->param_list);
	}

	cJSON * object_array = cJSON_GetObjectItem(root,"objects");

	for (i = 0; i < cJSON_GetArraySize(object_array); i++){
		cJSON *object = cJSON_GetArrayItem(object_array, i);

		object_recipe * rec = calloc(1,sizeof(object_recipe));

		char *type = cJSON_GetObjectItem(object,"type")->valuestring;
		char *param = cJSON_GetObjectItem(object,"param")->valuestring;
		SDL_Log("ADDING OBJECT TYPE: %s name: %s ", type, param);
		rec->obj_type = object_by_name(type);

		rec->param = level_get_param(lvl->param_list, type, param);

		cJSON *pos = cJSON_GetObjectItem(object,"pos");
		rec->pos.x = cJSON_GetObjectItem(pos,"x")->valuedouble;
		rec->pos.y = cJSON_GetObjectItem(pos,"y")->valuedouble;
		rec->rotation = cJSON_GetObjectItem(pos,"r")->valuedouble;

		llist_add(lvl->level_data, rec);
	}

	return lvl;
}

void level_start_level(level *lvl)
{
	llist_begin_loop(lvl->level_data);
	while(llist_hasnext(lvl->level_data)) {
		object_recipe * data = llist_next(lvl->level_data);
		SDL_Log("INSTANCIATING TYPE: %s at: %f ", data->obj_type->NAME, data->pos.x);
		instance_create(data->obj_type, data->param, data->pos, cpvzero);
	}
}

void level_unload(level *lvl)
{
	tilemap_destroy(lvl->tiles);
	// TODO: move to hashmap with remove callback
	hashiterator *it = hm_get_iterator(lvl->param_list);
	hashmap * h = hm_iterator_next(it);
	while(h != NULL) {
		hashiterator *it2 = hm_get_iterator(h);
		void * data = hm_iterator_next(it2);
		while(data != NULL) {
			free(data);
			data = hm_iterator_next(it2);
		}
		hm_destroy_iterator(it2);
		hm_destroy(h);
		h = hm_iterator_next(it);
	}
	hm_destroy_iterator(it);
	hm_destroy(lvl->param_list);
	free(lvl->tiles);
	free(lvl);
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

int level_get_level_count(int station)
{
	return (station > 0 && station <= station_count) ? world[station-1].count : 0;
}

void level_destroy(void)
{
	station_count = 0;
	free(world);
}
