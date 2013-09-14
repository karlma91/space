#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "level.h"
#include "obj/object_types.h"

#include "tilemap.h"

#include "../engine/engine.h"
#include "../engine/io/waffle_utils.h"

#include "states/space.h"


#define FILE_PERCENT (100.0f* offset / filesize)

static int station_count;
static level_ship *worlds;

static int (count[OBJECT_MAX_OBJECTS]);
static char (*(names[OBJECT_MAX_OBJECTS]))[21];
static char *(params[OBJECT_MAX_OBJECTS]);

static int i;

#include "zzip/lib.h"

static char buf[21];
static char object_buf[21];
static char group[21];
static char subtype[21];
static char fname[51];

#define FILE_BUFFER_SIZE 8192 /* NB! make sure buffer is large enough */
static char buffer[FILE_BUFFER_SIZE];
static int offset = 0;
static int offset_add = 0;
static int filesize = 0;

/** return sub_name's index in params, or -1 if not found  */
int get_sub_index(const object_id *obj_id, const char* sub_name) {
	int sub_id;
	int group_id = obj_id->ID;

	for (sub_id = 0; sub_id < count[group_id]; sub_id++) {
		if (strcmp(names[group_id][sub_id], sub_name) == 0) {
			/* subtype allready in list */
			return sub_id;
		}
	}
	return -1;
}

//TODO: rewrite object parsing: general and automatic object parsing
int level_init(void)
{
	int ret;

	for (i = 0; i < OBJECT_MAX_OBJECTS; i++) {
		object_id *obj_id = object_by_id(i);
		count[i] = 0;
		level_add_param(obj_id, NULL, "NULL");
	}

	/* read space station data */
	filesize = waffle_read_file("space", &buffer[0], FILE_BUFFER_SIZE);
	if (!filesize) {
		SDL_Log("Could not load level data!");
		exit(1);
	}

	ret = sscanf(buffer, "%d\n%n", &station_count, &offset);
	if (ret != 1) {
		SDL_Log("Could not find station count!");
		exit(1);
	}

	worlds = calloc(station_count,sizeof(level_ship));

	for (i = 0; i < station_count; i++) {
		int x, y, count, radius;
		float spd;
		ret = sscanf(&buffer[offset], "%d %d %d %d %f\n%n", &x, &y, &count, &radius, &spd, &offset_add);
		offset += offset_add;

		if (ret == EOF) {
			i = station_count;
			SDL_Log("Error while loading level data, reached EOF!\n");
			exit(2);
		}

		SDL_Log("Data read: %d %d %d %d %f\n", x, y, count, radius, spd);
		worlds[i].x = x;
		worlds[i].y = y;
		worlds[i].count = count;
		worlds[i].radius = radius;
		worlds[i].rotation_speed = spd;
		worlds[i].id = i + 1;
	}

	if (i != station_count || i <= 0) {
		SDL_Log("Error while loading level data, could not load all stations! (%d of %d loaded)\n", i, station_count);
		exit(2);
	}

	/* read object definitions (sub groups / object sub types) */
	filesize = waffle_read_file("objects", &buffer[0], FILE_BUFFER_SIZE);
	if (!filesize) {
		SDL_Log("Could not load object data!\n");
		exit(3);
	}

	/* read file contents into buffer */
	offset = 0;
	offset_add = 0;

	SDL_Log("DEBUG: read_buffer:\n%s", buffer);

	for (;offset < filesize;) {
		ret = sscanf(&buffer[offset], "%s %s %n", &group[0], &subtype[0], &offset_add);
		offset += offset_add;

		if (ret == EOF) {
			break;
		}

		SDL_Log("DEBUG: %5.1f %% - Parsing %s %s - characters left: %d\n", FILE_PERCENT, group, subtype, filesize - offset); //DEBUG

		/* find object type id */
		object_id *obj_id =  object_by_name(group);
		if (obj_id == NULL) {
			SDL_Log("Unrecognized object group: '%s'\n", group);
			exit(4);
		}

		/* check if subtype allready exists */
		int sub_id = get_sub_index(obj_id, subtype);
		if (sub_id != -1) {
			SDL_Log("Duplicate sub object definitions: %s %s\n", group, subtype);
			exit(5);
		}


		//TODO use LList instead!

		int group_id = obj_id->ID;

		/* reallocates arrays so they have room for one more object */
		++(count[group_id]);
		names[group_id] = realloc(names[group_id], sizeof(char[count[group_id]][21]));
		strcpy(names[group_id][count[group_id]-1], subtype);

		int expected = 0;
		int obj_index = count[group_id] - 1;

		/* add new sub object definition */
		union {
			obj_param_tank tank;
			obj_param_rocket rocket;
			obj_param_turret turret;
			obj_param_factory factory;
		} arg;

		const int paramsize = obj_id->P_SIZE;

		if (obj_id == obj_id_tank) {
			expected = 3;
			ret = sscanf(&buffer[offset], "%f %d %s%n\n", &arg.tank.max_hp, &arg.tank.coins, &fname[0], &offset_add);
			offset += offset_add;
		} else if (obj_id == obj_id_turret) {
			expected = 6;
			ret = sscanf(&buffer[offset], "%f %d %f %f %d %s%n\n", &arg.turret.max_hp, &arg.turret.coins, &arg.turret.rot_speed, &arg.turret.shoot_interval,&arg.turret.burst_number, &fname[0], &offset_add);
			offset += offset_add;
			arg.turret.tex_id = texture_load(fname);
		} else if (obj_id == obj_id_rocket) {
			expected = 4;
			ret = sscanf(&buffer[offset], "%f %d %s %f%n\n", &arg.rocket.max_hp, &arg.rocket.coins, &fname[0], &arg.rocket.force, &offset_add);
			offset += offset_add;
			arg.rocket.tex_id = texture_load(fname);
		} else if (obj_id == obj_id_factory) {
			expected = 8;
			ret = sscanf(&buffer[offset], "%d %f %f %d %s %s %s %s%n\n", &arg.factory.max_tanks, &arg.factory.max_hp, &arg.factory.spawn_delay, &arg.factory.coins, object_buf, buf, &fname[0], &arg.factory.shape_name[0], &offset_add);
			offset += offset_add;

			arg.factory.sprite_id = sprite_link(fname);
			arg.factory.shape_id = shape_read(arg.factory.shape_name);

			int sub_id = -1;
			/* find arg.tank subtype */
			object_id * arg_obj = object_by_name(object_buf);
			if (arg_obj){
				sub_id = get_sub_index(arg_obj,buf);
				arg.factory.type = arg_obj;
			} else {
				SDL_Log("ERROR while reading factory data, object '%s' not defined\n", object_buf);
				exit(7);
			}

			if (sub_id == -1) {
				SDL_Log("ERROR while reading factory data, TANK %s not defined before\n", buf);
				exit(7);
			}

			arg.factory.param = params[arg.factory.type->ID] + arg.factory.type->P_SIZE * sub_id;
		}

		/* check if all expected parameters were defined */
		if (ret != expected) {
			SDL_Log("Wrong number of parameters for %s %s got: %d expected %d\n", group, subtype, ret, expected);
			exit(6);
		}

		/* reallocate current array */
		params[group_id] = realloc(params[group_id], paramsize * count[group_id]);

		/* store new param into current array */
		char *to = params[group_id] + paramsize * obj_index;
		memcpy(to, &arg, paramsize);
	}

	SDL_Log("DEBUG: Finished parsing Game Object Definitions");

	return 0;
}

level *level_load(int space_station, int deck)
{
	SDL_Log("DEBUG: Starting to load level %d.%d", space_station, deck);
	int ret;
	if (space_station < 1 || space_station > station_count) {
		SDL_Log("Space station no. %d does not exist!\n", space_station);
		return NULL;
	}

	if (deck < 1 || deck > worlds[space_station - 1].count) {
		SDL_Log("Deck no. %d on space station no. %d does not exist!\n", deck, space_station);
		return NULL;
	}

	char levelpath[32];
	sprintf(levelpath, "level/%02d_%02d.lvl", space_station, deck);

	level *lvl = calloc(1, sizeof *lvl);
	currentlvl = lvl;

	/* open and read level data */
	filesize = waffle_read_file(levelpath, &buffer[0], FILE_BUFFER_SIZE);
	if (!filesize) {
		SDL_Log("Could not find level %d.%d\n",space_station,deck);
		return NULL;
	}

	offset = 0;
	offset_add = 0;

	char tilemap_name[100];
	ret = sscanf(&buffer[offset],"%s%n\n", tilemap_name, &offset_add);
	offset += offset_add;

	int retExp = 0;
	lvl->tiles = calloc(1, sizeof *lvl->tiles);
	ret = tilemap_create(lvl->tiles,tilemap_name);
	if (ret != retExp) {
		SDL_Log("Error while parsing level header. Could not load tilemap %s.\n", tilemap_name);
		return NULL;
	}
	lvl->height = lvl->tiles->height*lvl->tiles->tile_height;
	lvl->left = -(lvl->tiles->width*lvl->tiles->tile_width)/2;
	lvl->right = (lvl->tiles->width*lvl->tiles->tile_width)/2;
	lvl->width = (lvl->tiles->width*lvl->tiles->tile_width);

	// level render transformation data
	//lvl->outer_radius = (currentlvl->right - currentlvl->left)/(2*M_PI);//2100;
	//lvl->inner_radius = lvl->outer_radius - currentlvl->height;
	lvl->inner_radius = currentlvl->width/(2*M_PI);//2100;
	lvl->outer_radius = lvl->inner_radius + currentlvl->height;
	lvl->theta_max = atan2f(GAME_WIDTH/2, lvl->inner_radius);//M_PI/8;


	/* read level specific data */
	retExp = 3;
	ret = sscanf(&buffer[offset],"%d %d %d%n\n", &(lvl->station),&(lvl->deck), &(lvl->timelimit), &offset_add);
	offset += offset_add;

	if (ret != retExp) {
		SDL_Log("Error while parsing level header. Wrong number of arguments. Got %d, expected %d.\n", ret, retExp);
		return NULL;
	}

	SDL_Log("DEBUG: Starting to add objects to level");
	int x;
	/* add objects */
	while (offset < filesize) {
		SDL_Log("DEBUG: Adding objects - %5.1f %% - characters left: %d", FILE_PERCENT, filesize - offset);

		ret = sscanf(&buffer[offset], "%s %s %d%n\n", &group[0], &subtype[0], &x, &offset_add);
		SDL_Log("DEBUG: Characters read: %d", offset_add);
		offset += offset_add;


		if (ret == EOF) {
			break;
		} else if (ret == 0) {
			SDL_Log("WARNING: missing level input before EOF? No arguments\n");
			break;
		} else if (ret != 3) {
			SDL_Log("Error while parsing level data. Wrong number of arguments: %d\n"
					"%s:%s:%d", ret, group, subtype, x);
			return NULL;
		}

		object_id *obj_id = object_by_name(group);
		int sub_id = get_sub_index(obj_id, subtype);
		SDL_Log("Adding object: %s_%s x=%d\n", group, subtype, x);

		void* args = params[obj_id->ID] + obj_id->P_SIZE * sub_id;

		float r = lvl->outer_radius-lvl->height/2;
		float a = (WE_2PI * x / lvl->width);
		instance_create(obj_id, args, WE_P2C(r,a), cpvzero);
	}

	SDL_Log("DEBUG: Finished adding objects to level");

	return lvl;
}

void level_unload(level *lvl)
{
	tilemap_destroy(lvl->tiles);
	free(lvl->tiles);
	free(lvl);
}

void level_get_ships(level_ship **ship, int *count)
{
	*ship = worlds;
	*count = station_count;
}

void level_destroy(void)
{
	station_count = 0;
	free(worlds);
}

int level_get_station_count(void)
{
	return station_count;
}

int level_get_level_count(int station)
{
	return (station > 0 && station <= station_count) ? worlds[station-1].count : 0;
}

int level_add_param(object_id *obj_id, void *param, const char *name)
{
	if (!obj_id)
		return -1;

	int id = obj_id->ID;
	int paramsize = obj_id->P_SIZE;

	++count[id];
	names[id] = realloc(names[id], sizeof(char[count[id]][21]) * count[id]);
	strcpy(names[id][count[id]-1], name);

	params[id] = realloc(params[id], paramsize);

	if (param) {
		memcpy(params[id], param, paramsize);
	} else {
		memset(params[id], 0, paramsize);
	}

	return count[id];
}
