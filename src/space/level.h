#ifndef LEVEL_H_
#define LEVEL_H_

#include "tilemap.h"
#include "../engine/components/object.h"
#include "../engine/components/shape.h"
#include "../engine/audio/sound.h"
#include "cJSON.h"

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
	int timelimit;
	float inner_radius;
	float outer_radius;
	float height;

	param_list params;
	LList level_data;

	tilemap2 tilemap;
} level;



extern param_list param_defs;


we_bool level_safe_parse(cJSON *param, char *field_name, char *type, void *field, void *def);

void level_write_solar_file(LList world);
void level_load_solar();

void level_load_levels_from_folder(LList world);
void level_load_params(param_list *defs, cJSON *root);

int level_init(void);
void level_unload(level *lvl);
void level_destroy(void);
void level_write_to_file(level *lvl);

void level_destry_param_list(param_list *params);


void level_add_object_recipe_name(level * lvl, const char * obj_type, const char * param_name, cpVect pos, float rotation);
void level_add_object_recipe(level * lvl, object_id *obj_id, const char * param_name,  void * param, cpVect pos, float rotation);

void * level_get_param(param_list *params, const char *type, const char * name);
level *level_load(int folder, char * filename);
void level_clear_objects(level *lvl);
void level_start_level(level *lvl);
LList level_get_world();

#endif /* LEVEL_H_ */
