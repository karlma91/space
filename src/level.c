#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "level.h"
#include "objects.h"
#include "tank.h"
#include "tankfactory.h"
#include "player.h"
#include "bullet.h"

static int station_count;
static level_ship *worlds;

static char *(group_names[ID_COUNT]);

static int (count[ID_COUNT]);
static char (*(names[ID_COUNT]))[21];
static void *(params[ID_COUNT]);

static int i;

static FILE * file;

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
	group_names[ID_TANK_FACTORY]  = "FACTORY";
	//group_names[ID_BULLET_PLAYER] = "BULLET_P";
	//group_names[ID_BULLET_ENEMY]  = "BULLET_E";

	//TODO add new object types here /\


	/* read space station data */
	file = fopen("bin/data/space","r");

	if (file == NULL) {
		fprintf(stderr, "Could not load level data!\n");
		return 1;
	}

	ret = fscanf(file, "%d\n", &station_count);
	worlds = calloc(station_count,sizeof(level_ship));

	for (i = 0; i < station_count; i++) {
		int x, y, count, radius;
		float spd;
		ret = fscanf(file, "%d %d %d %d %f\n", &x, &y, &count, &radius, &spd);

		if (ret == EOF) {
			i = station_count;
			fprintf(stderr, "Error while loading level data!\n");
			return 2;
		}

		//fprintf(stderr, "%d %d %d %d %f\n", x, y, count, radius, spd);
		worlds[i].x = x;
		worlds[i].y = y;
		worlds[i].count = count;
		worlds[i].radius = radius;
		worlds[i].rotation_speed = spd;
	}
	fclose(file);
	if (i != station_count || i <= 0) {
		i = station_count;
		fprintf(stderr, "Error while loading level data!\n");
		return 2;
	}

	/* read object sub groups / object sub types */
	file = fopen("bin/data/objects","r");

	if (file == NULL) {
		fprintf(stderr, "Could not load object data!\n");
		return 3;
	}

	char group[21];
	char subtype[21];

	for (;;) {
		ret = fscanf(file, "%s %s ", &group[0], &subtype[0]);
		if (ret == EOF) {
			break;
		}

		fprintf(stderr, "%s %s \n", group, subtype); //DEBUG

		/* find object type id */
		int group_id;
		for (group_id = 0; group_id < ID_COUNT; group_id++) {
			if (strcasecmp(group, group_names[group_id]) == 0) {
				/* object name recognised */
				break;
			}
		}
		if (group_id == ID_COUNT) {
			fprintf(stderr, "Unrecognised object group: '%s'\n", group);
			return 4;
		}

		/* check if subtype allready exists */
		for (i = 0; i < count[group_id]; i++) {
			if (strcmp(names[group_id][i], subtype) == 0) {
				/* subtype allready in list */
				fprintf(stderr, "Duplicate sub object definitions: %s %s\n", group, subtype);
				return 5;
			}
		}

		/* reallocates arrays so they have room for one more object */
		++(count[group_id]);
		names[group_id] = realloc(names[group_id], sizeof(char[count[group_id]][21]));
		strcpy(names[group_id][count[group_id]-1], subtype);

		/* add new sub object definition */
		float a, b, c, d;
		switch (group_id) {
		case ID_PLAYER:

			break;
		case ID_TANK:
			fscanf(file, "%d\n", &a);
			params[group_id] = realloc(params[group_id], sizeof(struct tank_param[count[group_id]]));
			((struct tank_param *)params)[group_id].max_hp = a;
			//static void *(params[ID_COUNT]);
			break;
		case ID_TANK_FACTORY: //TMP
			fscanf(file, "%d\n", &a);

			break;
		case ID_BULLET_PLAYER:

			break;
		case ID_BULLET_ENEMY:

			break;
		}
	}


	fclose(file);
	return 0;
}

level *level_load(int space_station, int deck)
{
	if (space_station < 1 || space_station > station_count) {
		fprintf(stderr, "Space station no. %d does not exist!\n", space_station);
		return NULL;
	}

	if (deck < 1 || deck > worlds[space_station].count) {
		fprintf(stderr, "Deck no. %d on space station no. %d does not exist!\n", deck, space_station);
		return NULL;
	}



	return NULL;
}

void level_unload(level *lvl)
{

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
