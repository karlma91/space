#include <stdlib.h>
#include <stdio.h>
#include "level.h"
#include "objects.h"

static int station_count;
static level_ship *worlds;

static int (count[ID_COUNT]);
static char *(names[ID_COUNT]);
static void *(params[ID_COUNT]);

static int i;

static FILE * file;

void level_init()
{
	for (i = 0; i < ID_COUNT; i++) {
		count[i] = 0;
		names[i] = NULL;
		params[i] = NULL;
	}

	/* read space station data */
	file = fopen("bin/data/space","r");

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

extern void level_get_ships(level_ship **ship, int *count)
{
	*ship = worlds;
	*count = station_count;
}

void level_destroy()
{
	station_count = 0;
	free(worlds);
}
