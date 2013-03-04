#ifndef PLAYER_H_
#define PLAYER_H_

#include "chipmunk.h" /* Chipmunk physics library */
#include "objects.h"  /* Game components */

struct player_param;

struct player{
	/* standard */
	object obj;

	/* player's instance variables */
	cpShape *shape;
	float hp;
	int lives;
	float lost_life_timer;
	int disable;
	int gun_level;
	int score;

	/* parameters */
	struct player_param *param;
};

struct player_param {
	float max_hp;
	int tex_id;
};

extern struct obj_type type_player;

object *player_init();

#endif /* PLAYER_H_ */
