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

typedef struct param_defs {
	hashmap * param;
} param_list;

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

	param_list params;
	LList level_data;

	tilemap *tiles;
} level;



extern param_list param_defs;

int level_safe_parse_int(cJSON *param, char *name, void *def);
double level_safe_parse_float(cJSON *param, char *name,   void *def);
char* level_safe_parse_char(cJSON *param, char *name,   void *def);
object_id* level_safe_parse_object_id(cJSON *param, char *name,  void *def);
polyshape level_safe_parse_shape(cJSON *param, char *name,  void *def);
SPRITE_ID level_safe_parse_sprite(cJSON *param, char *name,  void *def);
EMITTER_ID level_safe_parse_emitter(cJSON *param, char *name,  void *def);
Mix_Chunk * level_safe_parse_sound(cJSON *param, char *name,  void *def);
int level_safe_parse_texture(cJSON *param, char *name,  void *def);
Color level_safe_parse_Color(cJSON *param, char *name,  void *def);


void level_load_params(param_list *defs, cJSON *root);

int level_init(void);
void level_unload(level *lvl);
void level_destroy(void);
void level_get_ships(level_ship **,int *);

void level_write_to_file(level *lvl);

void level_destry_param_list(param_list *params);

int level_get_station_count(void);
int level_get_level_count(int station);

void level_add_object_recipe_name(level * lvl, const char * obj_type, const char * param_name, cpVect pos, float rotation);
void level_add_object_recipe(level * lvl, object_id *obj_id, const char * param_name,  void * param, cpVect pos, float rotation);

void * level_get_param(param_list *params, const char *type, const char * name);
level *level_load(char * filename);
void level_clear_objects(level *lvl);
void level_start_level(level *lvl);
level_ship* level_get_world();

#endif /* LEVEL_H_ */
