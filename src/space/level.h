#ifndef LEVEL_H_
#define LEVEL_H_

#include "tilemap.h"
#include "../engine/components/object.h"

typedef struct level_ship {
	int id;
	cpVect pos;
	float radius;
	float rotation;
	float rotation_speed;
	char  level_name[32];
} level_ship;

typedef struct object_recipe {
	object_id * obj_type;
	char param_name[32];
	void * param;
	cpVect pos;
	cpVect vel;
	float rotation;
}object_recipe;

typedef struct level {
	char name[30];
	int station;
	int deck;
	int height;
	int left;
	int right;
	int width;
	int timelimit;
	float inner_radius;
	float outer_radius;

	hashmap * param_list;
	LList level_data;

	tilemap *tiles;
} level;

extern int level_init(void);
extern void level_unload(level *lvl);
extern void level_destroy(void);
extern void level_get_ships(level_ship **,int *);

extern int level_get_station_count(void);
extern int level_get_level_count(int station);

void level_add_object_recipe_name(level * lvl, char * obj_type, char * param_name, cpVect pos, float rotation);
void level_add_object_recipe(level * lvl, object_id *obj_id,char * param_name,  void * param, cpVect pos, float rotation);

void * level_get_param(hashmap * h, char *type, char * name);
level *level_load(char * filename);
void level_start_level(level *lvl);
level_ship* level_get_world();

#endif /* LEVEL_H_ */
