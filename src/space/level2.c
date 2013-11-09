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


void str_to_upper(char * str)
{
	while(*str){
		*str = toupper(*str);
		str++;
	}
}

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
	str_to_upper(type);
	str_to_upper(name);

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
		arg.turret.rot_speed =      cJSON_GetObjectItem(param,"rot_speed")->valuedouble;
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

		char *type = cJSON_GetObjectItem(object,"type")->valuestring;
		char *name = cJSON_GetObjectItem(object,"name")->valuestring;
		str_to_upper(type);
		str_to_upper(name);

		SDL_Log("ADDING OBJECT TYPE: %s name: %s ", type, name);
		cJSON *pos = cJSON_GetObjectItem(object,"pos");
		cpVect p;
		p.x = cJSON_GetObjectItem(pos,"x")->valuedouble;
		p.x = cJSON_GetObjectItem(pos,"y")->valuedouble;
		level_add_object_recipe_name(lvl, type, name, p, 0);

	}

	return lvl;
}

void level_add_object_recipe_name(level * lvl, char * obj_type, char * param_name, cpVect pos, float rotation)
{
	object_recipe * rec = calloc(1,sizeof(object_recipe));
	rec->obj_type = object_by_name(obj_type);
	rec->param = level_get_param(lvl->param_list, obj_type, param_name);
	strcpy(rec->param_name, param_name);
	rec->pos = pos;
	rec->rotation = rotation;
	llist_set_remove_callback(lvl->level_data, free);
	llist_add(lvl->level_data, rec);
}

void level_add_object_recipe(level * lvl, object_id *obj_id, char * param_name, void * param, cpVect pos, float rotation)
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

void level_write_to_file(level *lvl)
{
	cJSON *root,*fmt;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name", cJSON_CreateString(lvl->name));
	cJSON_AddItemToObject(root, "tilemap", cJSON_CreateString("level02_01.tmx"));
	cJSON_AddNumberToObject(root, "timelimit",100);

	cJSON * param_array = cJSON_CreateArray();

	hashiterator *it = hm_get_iterator(lvl->param_list);
	hashmap * h = hm_iterator_next(it);
	while(h != NULL) {
		object_id *obj_id = object_by_name(hm_iterator_get_key(it));
		hashiterator *it2 = hm_get_iterator(h);
		void * data = hm_iterator_next(it2);
		while(data != NULL) {

			SDL_Log("Adding param to array");
			cJSON *param = cJSON_CreateObject();
			//cJSON_AddItemReferenceToArray(param_array, param);

			cJSON_AddItemToObject(param, "type", cJSON_CreateString(hm_iterator_get_key(it)));
			cJSON_AddItemToObject(param, "name", cJSON_CreateString(hm_iterator_get_key(it2)));

			if ( obj_id == obj_id_tank) {
				obj_param_tank * tank = (obj_param_tank*) data;
				cJSON_AddNumberToObject(param, "max_hp",tank->max_hp);
				cJSON_AddNumberToObject(param, "coins",tank->coins);
			} else if (obj_id == obj_id_turret) {
				obj_param_turret * turret = (obj_param_turret*) data;
				cJSON_AddNumberToObject(param, "max_hp",turret->max_hp);
				cJSON_AddNumberToObject(param, "coins",turret->coins);
				cJSON_AddNumberToObject(param, "shoot_interval", turret->shoot_interval);
				cJSON_AddNumberToObject(param, "burst_number",turret->burst_number);
				cJSON_AddNumberToObject(param, "rot_speed",turret->rot_speed);
				//cJSON_AddItemToObject(param, "texture_name", cJSON_CreateString(turret->)); TODO: ADD sprite to turret
			} else if (obj_id == obj_id_rocket) {
				obj_param_rocket *rocket = (obj_param_rocket*) data;
				cJSON_AddNumberToObject(param, "max_hp",rocket->max_hp);
				cJSON_AddNumberToObject(param, "coins",rocket->coins);
				//cJSON_AddItemToObject(param, "texture_name", cJSON_CreateString(turret->)); TODO: ADD sprite to rocket
			} else if (obj_id == obj_id_factory) {
				obj_param_factory *factory = (obj_param_factory *) data;
				cJSON_AddNumberToObject(param, "max_hp",factory->max_hp);
				cJSON_AddNumberToObject(param, "coins",factory->coins);
				cJSON_AddNumberToObject(param, "max_tanks",factory->max_tanks);
				cJSON_AddNumberToObject(param, "spawn_delay",factory->spawn_delay);
				cJSON_AddNumberToObject(param, "coins",factory->coins);
				cJSON_AddItemToObject(param, "sprite", cJSON_CreateString(factory->sprite_name));
				cJSON_AddItemToObject(param, "shape", cJSON_CreateString(factory->shape_name));
				cJSON *object_spawn = cJSON_CreateObject();
				cJSON_AddItemToObject(object_spawn, "type", cJSON_CreateString(factory->type_name));
				cJSON_AddItemToObject(object_spawn, "name", cJSON_CreateString(factory->param_name));
				cJSON_AddItemToObject(param, "object_spawn", object_spawn);
			}else if (obj_id == obj_id_robotarm) {
				obj_param_robotarm * robotarm = (obj_param_robotarm*) data;
				cJSON_AddNumberToObject(param, "max_hp",robotarm->max_hp);
				cJSON_AddNumberToObject(param, "coins",robotarm->coins);
				//cJSON_AddItemToObject(param, "texture_name", cJSON_CreateString(turret->)); TODO: ADD sprite to robotarm
			}
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
