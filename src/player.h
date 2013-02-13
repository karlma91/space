#ifndef PLAYER_H_
#define PLAYER_H_

#include "chipmunk.h" /* Chipmunk physics library */
#include "objects.h"  /* Game components */


enum player_state{
	PLAYER_STATE_GAMEOVER,
	PLAYER_STATE_LOST_LIFE,
	PLAYER_STATE_NORMAL
};

struct player{
	struct obj_type *type;
	int instance_id;
	int *remove;
	int alive;
	cpBody *body;
	cpShape *shape;
	float max_hp;
	float hp;
	int highscore;
	int lives;
	enum player_state state;
	float lost_life_timer;
};

extern struct obj_type type_player;

object *player_init();

#endif /* PLAYER_H_ */
