#include "object_types.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "chipmunk.h"

#define OBJ_NAME player
#include "we_defobj.h"

int player_assisted_steering = 0;
int player_cheat_invulnerable = 0;

static void controls(obj_player *);
static void action_shoot(obj_player *);

//#define IMPULSE_FORCE 160 //prev value: 60

static void init(OBJ_TYPE *OBJ_NAME)
{
	instance_revive((instance *)player);
	player->gun_level = 1;
	player->lives = 3;
	player->coins = 0;
	player->rotation_speed = 2.5;
	player->aim_angle = 0;
	player->aim_speed = 0.5;
	player->gun_timer = 0;
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	cpShape *shape;
	cpFloat radius = 30;
	cpFloat mass = 2;

	sprite_create(&(player->gun), SPRITE_PLAYER_GUN, 120, 120, 0);
	sprite_create(&(player->data.spr), SPRITE_PLAYER, 120, 120, 0);

	player->data.components[CMP_HPBAR] = &player->hp_bar;
	player->data.components[CMP_MINIMAP] = &player->radar_image;
	player->radar_image = cmp_new_minimap(10, COL_GREEN);

	player->flame = particles_get_emitter(parti, EMITTER_FLAME);
	player->disable=0;
	init(player);

	/* make and add new body */
	player->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, radius, radius/2,cpvzero)));
	cpBodySetPos(player->data.body, cpv(0,990));
	cpBodySetVelLimit(player->data.body,450); //700
	cpBodySetUserData(player->data.body, (void*)player);

	/* make and connect new shape to body */
	shape = we_add_circle_shape(space, player->data.body,radius,0.8,0.9);
	we_shape_collision(shape, &this, LAYER_PLAYER, player);

	shape = we_add_circle_shape(space, player->data.body, player->param.cash_radius, 0,0);
	we_shape_collision(shape, &this, LAYER_PICKUP, player);
	cpShapeSetSensor(shape, 1);

	hpbar_init(&(player->hp_bar), 100, 120, 25, 0, 50, &(player->data.body->p));

	//FIXME cleanup
	player->gunwheel = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(player->gunwheel, player->data.body->p);
	cpBodySetUserData(player->gunwheel, (void*)player);

	shape = we_add_circle_shape(space, player->gunwheel,radius,0.9,0.8);
	we_shape_collision(shape, &this, LAYER_PLAYER, player);
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	cpVect pos_body = player->data.body->p;
	cpVect pos_gun = player->gunwheel->p;
	draw_color4f(1,1,1,1);
	sprite_render(&(player->gun), &(pos_gun), player->aim_angle * 180/M_PI);
	sprite_render(&(player->data.spr), &(pos_body), player->direction * 180/M_PI);
	hpbar_draw(&player->hp_bar);
}

#include <time.h>
static void on_update(OBJ_TYPE *OBJ_NAME)
{
	if (player_cheat_invulnerable)
		player->hp_bar.value = 10000;

	player->gun_timer += dt;

	//update physics and player
	if (player->hp_bar.value > 0) { //alive
		player->direction = turn_toangle(player->direction_target, player->direction, 2 * M_PI * dt / 1000);

		cpBodySetAngVel(player->data.body,0);
		player->flame->p = player->data.body->p;
		player->flame->angular_offset = player->direction * (180/M_PI)+90;

		player->gunwheel->p = player->data.body->p;
		player->gunwheel->rot = cpvforangle(player->aim_angle);
		player->gunwheel->v = player->data.body->v;

		if (player->disable == 0){
			controls(player);
		}
	} else {
		float body_angle = cpvtoangle(player->data.body->rot);
		player->direction = turn_toangle(body_angle, player->direction, 2 * M_PI * dt / 1000);
		player->aim_angle = cpvtoangle(player->gunwheel->rot);
		player->flame->disable = 1;
	}

}

static void controls(obj_player *player)
{
	//float player_angle = cpBodyGetAngle(player->data.body);
	float dir_step;

	//cpFloat speed = 700;
	int instant = 1; //tmp instant direction

	if (joy_p1_left->amplitude) {
		/*
		if (!instant) {
			dir_step = (player->rotation_speed * 2*M_PI)*dt; // 2.5 rps
			player_angle = turn_toangle(player_angle,player_angle_target,dir_step);
		} else {
			player_angle = player_angle_target;
		}
		*/

		player->direction_target = joy_p1_left->direction;

		cpVect player_dir = cpv(joy_p1_left->axis_x, joy_p1_left->axis_y);
		cpVect j = cpvmult(player_dir, player->force);

		if (player_assisted_steering) {
			cpVect F = cpvmult(cpSpaceGetGravity(space),cpBodyGetMass(player->data.body)*dt);
			j.x -= F.x;
			j.y -= F.y;

			float length = cpvlength(j);
			float max_length = player->force * joy_p1_left->amplitude;
			if (length > max_length) {
				cpvmult(j, max_length / length);
			}
		}

		player->direction_target = cpvtoangle(j);

		cpBodyApplyImpulse(player->data.body, j, cpvmult(player_dir, -100)); // applies impulse from rocket
		//cpBodySetForce(player->data.body, cpvmult(player_dir, speed*30)); //*600

		player->flame->disable = 0;
	} else {
		player->flame->disable = 1;
		float vel_angle = cpvtoangle(cpBodyGetVel(player->data.body));
		player->direction = turn_toangle(vel_angle, player->direction, 2 * M_PI * dt / 10000);
	}


	/*
	cpVect vel = cpBodyGetVel(player->data.body);
	if (cpvlengthsq(vel) > 1)
		cpBodySetAngle(player->data.body, cpvtoangle(vel_angle));
	 */

	float aim_angle_target = 0;

	if (joy_p1_right->amplitude) {
		aim_angle_target = joy_p1_right->direction;
		if (!instant) {
			dir_step = (player->aim_speed * 2*M_PI) * dt; // 0.5 rps
			player->aim_angle = turn_toangle(player->aim_angle, aim_angle_target, dir_step);
		} else {
			player->aim_angle = aim_angle_target;
		}
		action_shoot(player);
	}

	/* DEBUG KEYS */
#if !ARCADE_MODE
		if (keys[SDL_SCANCODE_E]){
			player->data.body->p.x=0;
			player->data.body->p.y=0;
		}

		if (keys[SDL_SCANCODE_H]) {
			cpBodySetVelLimit(player->data.body,5000);
		}
#endif
}


//TODO lage en generell skyte-struct
static void action_shoot(obj_player *player)
{
	if (player->gun_timer >= player->param.gun_cooldown) {
		int i;

		sound_play(SND_LASER_1);

		for(i=0; i < player->gun_level;i++){
			//obj_bullet *b = object_create_bullet(player->data.body->p, cpvforangle(player->aim_angle + (M_PI/70)*((i+1) - (player->gun_level-i))), player->data.body->v, ID_BULLET_PLAYER);
			cpVect shoot_vel = cpvforangle(player->aim_angle + (M_PI/70)*((i+1) - (player->gun_level-i))); //TODO remove and split into another weapon
			cpVect shoot_pos = cpvadd(player->data.body->p, cpvmult(shoot_vel,40));
			shoot_vel = cpvmult(shoot_vel, 1400);
			shoot_vel = cpvadd(shoot_vel, player->data.body->v);


			obj_param_bullet opb = {.friendly = 1, .damage = player->bullet_dmg};
			void *params = NULL;
			if (player->bullet_type == obj_id_bullet) {
				params = &opb;
			}
			instance *instance = instance_create(player->bullet_type, params, shoot_pos.x, shoot_pos.y, shoot_vel.x, shoot_vel.y);
		}

		player->gun_timer = 0;
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	sound_play(SND_FACTORY_EXPLODE);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	//free(player); // does not work since player is currently static inside space.c
}
