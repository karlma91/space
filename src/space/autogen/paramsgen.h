/* GENERATED FILE DO NOT EDIT */
#ifndef PARAMS_GENERATED_H_
#define PARAMS_GENERATED_H_

#include "we_object.h"
#include "cJSON.h"
#include "../level.h"
#define MAX_STRING_SIZE 64

void* parse_generated(cJSON *param, char* type, char *name); 
OBJECT_DECLARE(staticpolygon);
OBJECT_DECLARE(player);
OBJECT_DECLARE(bullet);
OBJECT_DECLARE(tank);
OBJECT_DECLARE(factory);
OBJECT_DECLARE(turret);
OBJECT_DECLARE(rocket);
OBJECT_DECLARE(robotarm);
OBJECT_DECLARE(spikeball);
OBJECT_DECLARE(coin);
PARAM_START(staticpolygon)
	char spr_name[MAX_STRING_SIZE];
	int outline;
	float scale;
	float texture_scale;
	char shape_name[MAX_STRING_SIZE];
	polyshape shape_id;
PARAM_END
PARAM_START(player)
	float max_hp;
	int tex_id;
	int player_id;
	float gun_cooldown;
	float cash_radius;
PARAM_END
PARAM_START(bullet)
	int friendly;
	float damage;
PARAM_END
PARAM_START(tank)
	int max_hp;
	int coins;
PARAM_END
PARAM_START(factory)
	int max_tanks;
	float max_hp;
	float spawn_delay;
	int coins;
	object_id *type;
	char param_name[MAX_STRING_SIZE];
	char type_name[MAX_STRING_SIZE];
	char sprite_name[MAX_STRING_SIZE];
	char shape_name[MAX_STRING_SIZE];
PARAM_END
PARAM_START(turret)
	int max_hp;
	int coins;
	float rot_speed;
	float shoot_interval;
	int burst_number;
	int tex_id;
	char shape_name[MAX_STRING_SIZE];
	polyshape *shape_id;
PARAM_END
PARAM_START(rocket)
	float max_hp;
	int coins;
	int tex_id;
	float force;
	float damage;
PARAM_END
PARAM_START(robotarm)
	int max_hp;
	int coins;
PARAM_END
PARAM_START(spikeball)
	float radius;
	float up_speed;
	float down_force;
	float top_delay;
	float bot_delay;
	float damage;
PARAM_END
PARAM_START(coin)
	float explo_fmax;
PARAM_END
#include "../script_data/object_decl.h"

#endif /* end of PARAMS_GENERATED_H_ */