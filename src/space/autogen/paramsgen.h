/* GENERATED FILE DO NOT EDIT */
#ifndef PARAMS_GENERATED_H_
#define PARAMS_GENERATED_H_

#include "we_object.h"
#include "cJSON.h"
#include "../level/spacelvl.h"
#define MAX_STRING_SIZE 256

void* parse_generated(cJSON *param, char* type, char *name); 
cJSON * write_generated(object_id * obj_id, void *data, char *type, char *name); 
OBJECT_DECLARE(tank);
OBJECT_DECLARE(rocket);
OBJECT_DECLARE(bullet);
OBJECT_DECLARE(turret);
OBJECT_DECLARE(spiky);
OBJECT_DECLARE(crate);
OBJECT_DECLARE(factory);
OBJECT_DECLARE(robotarm);
OBJECT_DECLARE(player);
OBJECT_DECLARE(spikeball);
OBJECT_DECLARE(staticpolygon);
OBJECT_DECLARE(explosion);
OBJECT_DECLARE(coin);
PARAM_START(tank)
	float shoot_vel;
	char bullet_param[MAX_STRING_SIZE];
	float mass_wheel;
	int coins;
	object_id *bullet_type;
	float mass_body;
	int max_hp;
	float mass_barrel;
	char expl_pname[MAX_STRING_SIZE];
PARAM_END
PARAM_START(rocket)
	char expl_pname[MAX_STRING_SIZE];
	int coins;
	float force;
	float damage;
	float max_hp;
PARAM_END
PARAM_START(bullet)
	float alive_time;
	char expl_pname[MAX_STRING_SIZE];
	SPRITE_ID spr_id;
	float fade_time;
	Color col;
	float radius;
	float mass;
	float render_size;
	int render_stretch;
	float damage;
	int friendly;
PARAM_END
PARAM_START(turret)
	float shoot_vel;
	char bullet_param[MAX_STRING_SIZE];
	char expl_pname[MAX_STRING_SIZE];
	int coins;
	object_id *bullet_type;
	float turret_size;
	int burst_number;
	float mass;
	int max_hp;
	polyshape shape_id;
	float rot_speed;
	float shoot_interval;
PARAM_END
PARAM_START(spiky)
PARAM_END
PARAM_START(crate)
	SPRITE_ID sprite;
	int coins;
	int invinc;
	int max_hp;
	char expl_pname[MAX_STRING_SIZE];
	float size;
PARAM_END
PARAM_START(factory)
	char param_name[MAX_STRING_SIZE];
	object_id *spawn_type;
	SPRITE_ID sprite;
	int coins;
	float spawn_delay;
	char expl_pname[MAX_STRING_SIZE];
	polyshape shape;
	float max_hp;
	int max_tanks;
PARAM_END
PARAM_START(robotarm)
	float force;
	float max_vel;
	int seg_length;
	int segments;
	int coins;
	int max_hp;
	char expl_pname[MAX_STRING_SIZE];
PARAM_END
PARAM_START(player)
	int player_id;
PARAM_END
PARAM_START(spikeball)
	float top_delay;
	float bot_delay;
	float up_speed;
	float damage;
	float down_force;
	float radius;
PARAM_END
PARAM_START(staticpolygon)
	float texture_scale;
	polyshape shape_id;
	char tex_name[MAX_STRING_SIZE];
	int outline;
	float scale;
PARAM_END
PARAM_START(explosion)
	Mix_Chunk *snd;
	float force;
	float dmg;
	float seconds;
	EMITTER_ID em_expl;
	EMITTER_ID em_frag;
	float size;
PARAM_END
PARAM_START(coin)
	float explo_fmax;
PARAM_END
#include "../script_data/object_decl.h"

#endif /* end of PARAMS_GENERATED_H_ */