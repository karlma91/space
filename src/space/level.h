#ifndef LEVEL_H_
#define LEVEL_H_

#include "tilemap.h"
#include "obj/objects.h"
#include "../engine/components/object.h"

typedef struct level_ship {
	int id;
	float x;
	float y;
	int count;
	float radius;
	float rotation;
	float rotation_speed;
} level_ship;

typedef struct level {
	int station;
	int deck;
	int height;
	int left;
	int right;
	int width;
	int timelimit;
	int ceiling;
	int floor;

	float inner_radius;
	float theta_max;

	tilemap *tiles;
} level;

extern int level_init();
extern level *level_load(int space_station, int deck);
extern void level_unload(level *);
extern void level_destroy();
extern void level_get_ships(level_ship **,int *);

extern int level_get_station_count();
extern int level_get_level_count(int station);

extern char *(group_names[ID_COUNT]);

#endif /* LEVEL_H_ */
