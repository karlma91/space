#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "level.h"
#include "objects.h"

static int station_count;
static level_ship *worlds;

static char *(group_names[ID_COUNT]);

static int (count[ID_COUNT]);
static char *(names[ID_COUNT]);
static void *(params[ID_COUNT]);

static int i;

static FILE * file;

int level_init()
{
	for (i = 0; i < ID_COUNT; i++) {
		count[i] = 0;
		group_names[i] = NULL;
		names[i] = NULL;
		params[i] = NULL;
	}

	/* set group names */
	group_names[ID_PLAYER]        = "PLAYER";
	group_names[ID_TANK]          = "TANK";
	group_names[ID_TANK_FACTORY]  = "FACTORY";
	group_names[ID_BULLET_PLAYER] = "BULLET_P";
	group_names[ID_BULLET_ENEMY]  = "BULLET_E";

	//TODO add new object types here /\


	/* read space station data */
	file = fopen("bin/data/space","r");

	if (file == NULL) {
		fprintf(stderr, "could not load level data!\n");
		return 1;
	}

	fscanf(file, "%d\n", &station_count);
	worlds = calloc(station_count,sizeof(level_ship));

	for (i = 0; i < station_count; i++) {
		int x, y, count, radius;
		float spd;
		fscanf(file, "%d %d %d %d %f\n", &x, &y, &count, &radius, &spd);
		//fprintf(stderr, "%d %d %d %d %f\n", x, y, count, radius, spd);
		worlds[i].x = x;
		worlds[i].y = y;
		worlds[i].count = count;
		worlds[i].radius = radius;
		worlds[i].rotation_speed = spd;
	}
	fclose(file);

	/* read object sub groups / object sub types */
	file = fopen("bin/data/objects","r");

	if (file == NULL) {
		fprintf(stderr, "could not load object data!\n");
		return 2;
	}

	int ret;
	char group[255];
	char type[255];

	for (;;) {
		ret = fscanf(file, "%s %s ", &group[0], &type[0]);
		if (ret == EOF) {
			break;
		}

		//TODO parse input and store subname and create corresponding obj_struct
		//...
		fprintf(stderr, "%s %s \n", group, type);
	}


	fclose(file);

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
