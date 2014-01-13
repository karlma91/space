#ifndef LEVEL_H_
#define LEVEL_H_

#include "tilemap.h"
#include "../engine/components/object.h"
#include "../engine/components/shape.h"
#include "../engine/audio/sound.h"
#include "cJSON.h"

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

typedef struct param_defs {
	hashmap * param;
} param_defs;

int level_safe_parse_int(cJSON *param, char *name );
double level_safe_parse_float(cJSON *param, char *name );
char* level_safe_parse_char(cJSON *param, char *name );

object_id* level_safe_parse_object_id(cJSON *param, char *name);
polyshape level_safe_parse_shape(cJSON *param, char *name);
SPRITE_ID level_safe_parse_sprite(cJSON *param, char *name);
EMITTER_ID level_safe_parse_emitter(cJSON *param, char *name);
Mix_Chunk * level_safe_parse_sound(cJSON *param, char *name);
int level_safe_parse_texture(cJSON *param, char *name );


extern int level_init(void);
extern void level_unload(level *lvl);
extern void level_destroy(void);
extern void level_get_ships(level_ship **,int *);

void level_write_to_file(level *lvl);

extern int level_get_station_count(void);
extern int level_get_level_count(int station);

void level_add_object_recipe_name(level * lvl, const char * obj_type, const char * param_name, cpVect pos, float rotation);
void level_add_object_recipe(level * lvl, object_id *obj_id, const char * param_name,  void * param, cpVect pos, float rotation);

void * level_get_param(hashmap * h, char *type, char * name);
level *level_load(char * filename);
void level_clear_objects(level *lvl);
void level_start_level(level *lvl);
level_ship* level_get_world();

#endif /* LEVEL_H_ */
