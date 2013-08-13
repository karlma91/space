/*
 * object_types.h
 *
 *  Created on: 8. aug. 2013
 *      Author: Mathias
 */

#ifndef OBJECT_TYPES_H_
#define OBJECT_TYPES_H_

#include "../../engine/components/object.h"
#include "../../engine/graphics/particles.h"
#include "../../engine/graphics/draw.h"

void object_types_init();

/*
 * OBJECT DECLARATION
 */
OBJECT_DECLARE(template);

OBJECT_DECLARE(player);
OBJECT_DECLARE(bullet);
OBJECT_DECLARE(tank);
OBJECT_DECLARE(factory);
OBJECT_DECLARE(turret);
OBJECT_DECLARE(rocket);

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

PARAM_START(player)
	float max_hp;
	int tex_id;
	float gun_cooldown;
PARAM_END

OBJ_START(player)
	cpShape *shape;
	hpbar hp_bar;
	int lives;
	float lost_life_timer;
	int disable; //todo move this out to instance?
	int gun_level;
	int score;
	float rotation_speed;
	float direction; /* in radians [0, 2*PI]*/
	float direction_target; /* in radians [0, 2*PI]*/
	float gun_timer;
	float aim_angle;
	float aim_speed;

	float force;
	float bullet_dmg;

	emitter *flame;
	cpBody *gunwheel;

	sprite gun;
OBJ_END

PARAM_START(bullet)
	int friendly;
PARAM_END
OBJ_START(bullet)
	cpShape *shape;
	int bullet_type;
	float damage;
	cpFloat speed;
	Color color;
	float energy;
OBJ_END

PARAM_START(tank)
	float max_hp;
	int score;
PARAM_END
OBJ_START(tank)
	cpShape *shape;
	float timer;
	float barrel_angle;
	float rot_speed;
	float max_distance;
	hpbar hp_bar;
	obj_factory *factory;
	int factory_id;
	cpBody *wheel1;
	cpBody *wheel2;
	int state;
	sprite wheel_sprite;
	sprite turret_sprite;
	/* tmp debug variables */
	cpFloat debug_left_dist;
	cpFloat debug_right_dist;
OBJ_END

PARAM_START(factory)
	int max_tanks;
	float max_hp;
	float spawn_delay;
	int score;
	object *type;
	void *param;
	SPRITE_ID sprite_id;
PARAM_END
OBJ_START(factory)
	cpShape *shape;
	cpFloat timer;
	int cur;
	hpbar hp_bar;
	float max_distance;

	/* animation */
	float rot;
	emitter *smoke;
OBJ_END

PARAM_START(turret)
	float max_hp;
	int score;
	float rot_speed;
	float shoot_interval;
	int burst_number;
	int tex_id;
PARAM_END
OBJ_START(turret)
	cpShape *shape;
	float timer;
	float barrel_angle;
	int bullets;
	float max_distance;
	int shooting;
	float rate;
	hpbar hp_bar;
OBJ_END

PARAM_START(rocket)
	float max_hp;
	int score;
	int tex_id;
	float force;
PARAM_END
OBJ_START(rocket)
	cpShape *shape;
	obj_factory *factory;
	int factory_id;
	float timer;
	float angle;
	int active;
	float rot_speed;
	hpbar hp_bar;
	emitter *flame;
OBJ_END

#endif /* OBJECT_TYPES_H_ */
