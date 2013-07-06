#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "level.h"
#include "objects.h"
#include "tank.h"
#include "turret.h"
#include "rocket.h"
#include "tankfactory.h"
#include "player.h"
#include "bullet.h"

#include "waffle_utils.h"

#define FILE_PERCENT (100.0f* offset / filesize)

static int station_count;
static level_ship *worlds;

char *(group_names[ID_COUNT]);

static int (count[ID_COUNT]);
static char (*(names[ID_COUNT]))[21];
static void *(params[ID_COUNT]);

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

/** return group_name's index in group_names, or -1 if not found  */
int get_group_index(const char* group_name) {
	int group_id;
	for (group_id = 0; group_id < ID_COUNT; group_id++) {
		if (strcasecmp(group_name, group_names[group_id]) == 0) {
			return group_id;
		}
	}
	return -1;
}

/** return sub_name's index in params, or -1 if not found  */
int get_sub_index(int group_id, const char* sub_name) {
	int sub_id;
	for (sub_id = 0; sub_id < count[group_id]; sub_id++) {
		if (strcmp(names[group_id][sub_id], sub_name) == 0) {
			/* subtype allready in list */
			return sub_id;
		}
	}
	return -1;
}

int level_init()
{
	int ret;

	for (i = 0; i < ID_COUNT; i++) {
		count[i] = 0;
		group_names[i] = "";
		names[i] = NULL;
		params[i] = NULL;
	}

	/* set group names */
	//group_names[ID_PLAYER]        = "PLAYER";
	group_names[ID_TANK]          = "TANK";
	group_names[ID_ROCKET]        = "ROCKET";
	group_names[ID_FACTORY]       = "FACTORY";
	group_names[ID_TURRET]       = "TURRET";
	//group_names[ID_BULLET_PLAYER] = "BULLET_P";
	//group_names[ID_BULLET_ENEMY]  = "BULLET_E";

	//TODO add new object types here


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
		int group_id = get_group_index(group);
		if (group_id == -1) {
			SDL_Log("Unrecognized object group: '%s'\n", group);
			exit(4);
		}

		/* check if subtype allready exists */
		int sub_id = get_sub_index(group_id, subtype);
		if (sub_id != -1) {
			SDL_Log("Duplicate sub object definitions: %s %s\n", group, subtype);
			exit(5);
		}

		/* reallocates arrays so they have room for one more object */
		++(count[group_id]);
		names[group_id] = realloc(names[group_id], sizeof(char[count[group_id]][21]));
		strcpy(names[group_id][count[group_id]-1], subtype);

		int expected = 0;
		int paramsize = 0;
		int obj_index = count[group_id] - 1;

		/* add new sub object definition */ //TODO add new param structs here
		object_param_tank tank;
		object_param_rocket rocket;
		object_param_turret turret;
		object_param_factory factory;

		switch (group_id) {
		case ID_PLAYER:
			/* currently unsupported */
			break;
		case ID_TANK:
			expected = 3;
			paramsize = sizeof(object_param_tank);
			ret = sscanf(&buffer[offset], "%f %d %s%n\n", &tank.max_hp, &tank.score, &fname[0], &offset_add);
			offset += offset_add;
			break;
		case ID_TURRET:
			expected = 6;
			paramsize = sizeof(object_param_turret);
			ret = sscanf(&buffer[offset], "%f %d %f %f %d %s%n\n", &turret.max_hp, &turret.score, &turret.rot_speed, &turret.shoot_interval,&turret.burst_number, &fname[0], &offset_add);
			offset += offset_add;
			turret.tex_id = texture_load(fname);
			break;
		case ID_ROCKET:
			expected = 4;
			paramsize = sizeof(object_param_rocket);
			ret = sscanf(&buffer[offset], "%f %d %s %f%n\n", &rocket.max_hp, &rocket.score, &fname[0], &rocket.force, &offset_add);
			offset += offset_add;
			rocket.tex_id = texture_load(fname);
			break;
		case ID_FACTORY:
			expected = 7;
			paramsize = sizeof(object_param_factory);
			ret = sscanf(&buffer[offset], "%d %f %f %d %s %s %s%n\n", &factory.max_tanks, &factory.max_hp, &factory.spawn_delay, &factory.score, object_buf, buf, &fname[0], &offset_add);
			offset += offset_add;

			int sub_id = -1;
			/* find tank subtype */
			if(strcmp(object_buf,"TANK") == 0){
				sub_id = get_sub_index(ID_TANK,buf);
				factory.type = ID_TANK;
			}else{
				sub_id = get_sub_index(ID_ROCKET,buf);
				factory.type = ID_ROCKET;
			}
			if (sub_id == -1) {
				SDL_Log("ERROR while reading tank factory data, TANK %s not defined before\n", buf);
				exit(7);
			}
			factory.r_param = NULL;
			factory.t_param = NULL;
			if(factory.type == ID_ROCKET){
				factory.r_param = &(((object_param_rocket *)params[ID_ROCKET])[sub_id]);
			}else{
				factory.t_param = &(((object_param_tank *)params[ID_TANK])[sub_id]);
			}
			factory.tex_id = texture_load(fname);
			break;
		case ID_BULLET_PLAYER:
			/* currently unsupported */
			break;
		case ID_BULLET_ENEMY:
			/* currently unsupported */
			break;
		}

		/* check if all expected parameters were defined */
		if (ret != expected) {
			SDL_Log("Wrong number of parameters for %s %s got: %d expected %d\n", group, subtype, ret, expected);
			exit(6);
		}

		/* reallocate current array */
		params[group_id] = realloc(params[group_id], paramsize * count[group_id]);

		/* store new param into current array */
		switch (group_id) {
		case ID_PLAYER:
			/* currently unsupported */ break;
		case ID_TANK:
			((object_param_tank *)params[group_id])[obj_index] = tank; break;
		case ID_TURRET:
			((object_param_turret *)params[group_id])[obj_index] = turret; break;
		case ID_ROCKET:
			((object_param_rocket *)params[group_id])[obj_index] = rocket; break;
		case ID_FACTORY:
			((object_param_factory *)params[group_id])[obj_index] = factory; break;
		case ID_BULLET_PLAYER:
			/* currently unsupported */ break;
		case ID_BULLET_ENEMY:
			/* currently unsupported */ break;
		}
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

	level *lvl = malloc(sizeof(*lvl));
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
	lvl->tiles = malloc(sizeof(tilemap));
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
	lvl->inner_radius = 4*(currentlvl->right - currentlvl->left)/(2*M_PI);//2100;
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

		int group_id = get_group_index(group);
		int sub_id = get_sub_index(group_id, subtype);
		SDL_Log("Adding object: %s_%s x=%d\n", group, subtype, x);

		switch (group_id) {
		case ID_PLAYER:
			/* currently unsupported */
			break;
		case ID_TANK:
			object_create_tank(x,NULL, &(((object_param_tank *)params[group_id])[sub_id]));
			break;
		case ID_TURRET:
			object_create_turret(x, &(((object_param_turret *)params[group_id])[sub_id]));
			break;
		case ID_FACTORY:
			object_create_factory(x, &(((object_param_factory *)params[group_id])[sub_id]));
			break;
		case ID_BULLET_PLAYER:
			/* currently unsupported */
			break;
		case ID_BULLET_ENEMY:
			/* currently unsupported */
			break;
		}
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

void level_destroy()
{
	station_count = 0;
	free(worlds);
}

int level_get_station_count()
{
	return station_count;
}

int level_get_level_count(int station)
{
	return (station > 0 && station <= station_count) ? worlds[station-1].count : 0;
}
