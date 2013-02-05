#ifndef PLAYER_H_
#define PLAYER_H_

#include "chipmunk.h" /* Chipmunk physics library */
#include "objects.h"  /* Game components */


struct player{
	struct obj_type *type;
	int instance_id;
	int *remove;
	int alive;
	cpBody *body;
	cpShape *shape;
	float max_hp;
	float hp;
};

extern struct obj_type type_player;
struct player *player;
object *player_init();


#endif /* PLAYER_H_ */
