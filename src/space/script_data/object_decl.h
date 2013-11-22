#ifndef OBJECT_DECL_H_
#define OBJECT_DECL_H_

#include "../game.h"

OBJ_START(staticpolygon)
	SPRITE_ID spr_id;
	float texture_scale;
	int outline;
	float scale;
	polyshape shape_id;
	cpBody *body;
OBJ_END

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

OBJ_START(bullet)
	int bullet_type;
	cpFloat speed;
	Color color;
	float energy;
OBJ_END

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

OBJ_START(factory)
	cpFloat timer;
	void *spawn_param;
	int cur;
	hpbar hp_bar;
	float max_distance;
	minimap radar_image;
	/* animation */
	float rot;
	emitter *smoke;
OBJ_END

OBJ_START(turret)
	float timer;
	int bullets;
	float max_distance;
	int shooting;
	float rate;
	hpbar hp_bar;
	minimap radar_image;
	cpBody *tower;
	sprite spr_gun;
OBJ_END

OBJ_START(rocket)
	float timer;
	int active;
	float rot_speed;
	hpbar hp_bar;
	emitter *flame;
	minimap radar_image;
	float fuel; //TODO move into params
OBJ_END

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

OBJ_START(spikeball)
	float timer;
	int down;
	cpBody *dolly;
	cpConstraint *winch;
	float winch_length;
OBJ_END

OBJ_START(coin)
	cpBool pulled; //TODO create pickup component
OBJ_END



#endif /* OBJECT_DECL_H_ */
