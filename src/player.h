#ifndef PLAYER_H_
#define PLAYER_H_

#include "chipmunk.h" /* Chipmunk physics library */
#include "objects.h"  /* Game components */
#include "particles.h"
#include "hpbar.h"

struct _object_param_player {
	float max_hp;
	int tex_id;
	float gun_cooldown;
};

typedef struct {
	/* standard */
	object_data data;

	/* player's instance variables */
	cpShape *shape;
	hpbar hp_bar;
	int lives;
	float lost_life_timer;
	int disable; //todo move this out to object_data?
	int gun_level;
	int score;
	float rotation_speed;
	float direction; /* in radians [0, 2*PI]*/
	float direction_target; /* in radians [0, 2*PI]*/
	float gun_timer;
	float aim_angle;
	float aim_speed;
	emitter *flame;
	cpBody *gunwheel;

	sprite gun;

	/* parameters */
	object_param_player *param;
} object_group_player;

extern object_group_preset object_type_player;

extern object_group_player *object_create_player();

#endif /* PLAYER_H_ */
