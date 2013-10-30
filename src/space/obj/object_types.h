/*
 * object_types.h
 *
 *  Created on: 8. aug. 2013
 *      Author: Mathias
 */

#ifndef OBJECT_TYPES_H_
#define OBJECT_TYPES_H_

#include "../game.h"

void object_types_init(void);
void instance_get2nearest(instance *ins, object_id *obj_id, instance **left, instance **right, cpFloat *instance, cpFloat *right_distance);


/* COLLISION LAYERS */
extern int LAYER_PLAYER;
extern int LAYER_ENEMY;
extern int LAYER_BUILDING;
extern int LAYER_BULLET_PLAYER;
extern int LAYER_BULLET_ENEMY;
extern int LAYER_SHIELD_PLAYER;
extern int LAYER_SHIELD_ENEMY;
extern int LAYER_PICKUP;

/*
 * OBJECT DECLARATION
 */
OBJECT_DECLARE(template);

OBJECT_DECLARE(staticpolygon);
OBJECT_DECLARE(player);
OBJECT_DECLARE(bullet);
OBJECT_DECLARE(tank);
OBJECT_DECLARE(factory);
OBJECT_DECLARE(turret);
OBJECT_DECLARE(rocket);
OBJECT_DECLARE(robotarm);
OBJECT_DECLARE(spikeball); /* Hengende Jernpiggball i taket, faller ned, og trekkes sakte opp */
OBJECT_DECLARE(coin);

//TODO implementere følgende objekter:
OBJECT_DECLARE(enemy_bomb);
OBJECT_DECLARE(bombtower);
OBJECT_DECLARE(explosion);
OBJECT_DECLARE(tankshield); /* Treig tanks med saktegående turret med lite skjold */
OBJECT_DECLARE(tankrocket); /* Kjøretøy som må stoppe for å skyte rockets */
OBJECT_DECLARE(box);

//OBJECT_DECLARE(box_group); /* Gruppe med små bokser? */


/*
 * OBJECT DEFINITIONS
 */
PARAM_START(template)
	int a;
	int b;
PARAM_END
OBJ_START(template)
	float x, y;
OBJ_END

PARAM_START(staticpolygon)
	int texture;
	int outline;
	float scale;
	float texture_scale;
	char shape_name[32];
	polyshape shape_id;
PARAM_END
OBJ_START(staticpolygon)
	int texture;
	float texture_scale;
	int outline;
	float scale;
	polyshape shape_id;
	cpBody *body;
OBJ_END

PARAM_START(player)
	float max_hp;
	int tex_id;
	int player_id; // 1 or 2
	float gun_cooldown;
	float cash_radius;
PARAM_END
OBJ_START(player)
	int player_id; // 1 or 2
	joystick *joy_left;
	joystick *joy_right;
	hpbar hp_bar;
	int lives;
	float lost_life_timer;
	int disable; //todo move this out to instance?
	int gun_level;
	int coins;
	float rotation_speed;
	float direction; /* in radians [0, 2*PI]*/ //TODO remove this
	float direction_target; /* in radians [0, 2*PI]*/
	float gun_timer;
	float aim_angle; //TODO remove this
	float aim_speed;
	minimap radar_image;
	float force;
	float bullet_dmg;
	object_id *bullet_type;

	emitter *flame;
	cpBody *gunwheel;

	sprite gun;
OBJ_END

PARAM_START(bullet)
	int friendly;
	float damage;
PARAM_END
OBJ_START(bullet)
	int bullet_type;
	cpFloat speed;
	Color color;
	float energy;
OBJ_END

PARAM_START(tank)
	float max_hp;
	int coins;
PARAM_END
OBJ_START(tank)
	float timer;
	float rot_speed;
	float max_distance;
	hpbar hp_bar;
	minimap radar_image;
	cpBody *wheel1;
	cpBody *wheel2;
	cpBody *barrel;
	int state;
	sprite wheel_sprite;
	sprite turret_sprite;
	/* tmp debug variables */
	cpFloat debug_left_dist;
	cpFloat debug_right_dist;
OBJ_END

void factory_remove_child(instance *child);
PARAM_START(factory)
	int max_tanks;
	float max_hp;
	float spawn_delay;
	int coins;
	object_id *type;
	void *param;
	SPRITE_ID sprite_id;
	char shape_name[32];
	polyshape shape_id;
PARAM_END
OBJ_START(factory)
	cpFloat timer;
	int cur;
	hpbar hp_bar;
	float max_distance;
	minimap radar_image;
	/* animation */
	float rot;
	emitter *smoke;
OBJ_END

PARAM_START(turret)
	float max_hp;
	int coins;
	float rot_speed;
	float shoot_interval;
	int burst_number;
	int tex_id;
	char shape_name[32];
	polyshape *shape_id;
PARAM_END
OBJ_START(turret)
	float timer;
	int bullets;
	float max_distance;
	int shooting;
	float rate;
	hpbar hp_bar;
	minimap radar_image;
	cpBody *tower;
OBJ_END

PARAM_START(rocket)
	float max_hp;
	int coins;
	int tex_id;
	float force;
	float damage;
PARAM_END
OBJ_START(rocket)
	float timer;
	int active;
	float rot_speed;
	hpbar hp_bar;
	emitter *flame;
	minimap radar_image;
	float fuel; //TODO move into params
OBJ_END

PARAM_START(robotarm)
	float max_hp;
	int coins;
PARAM_END
OBJ_START(robotarm)
	cpBody *saw;
	sprite saw_sprite;
	float timer;
	float hp;
	int segments;
	float seg_length;
	float *x;
	float *y;
	float *angle;
	hpbar hp_bar;
	minimap radar_image;
OBJ_END


PARAM_START(spikeball)
	float radius;
	float up_speed;
	float down_force;
	float top_delay;
	float bot_delay;
	float damage;
PARAM_END
OBJ_START(spikeball)
	float timer;
	int down;
	cpBody *dolly;
	cpConstraint *winch;
	float winch_length;
OBJ_END


PARAM_START(coin)
	float explo_fmax;
PARAM_END
OBJ_START(coin)
	cpBool pulled; //TODO create pickup component
OBJ_END



#endif /* OBJECT_TYPES_H_ */
