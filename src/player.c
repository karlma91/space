/* header */
#include "player.h"

/* standard c-libraries */
#include <stdio.h>
#include <time.h>
#include <assert.h>

/* Game state */
#include "space.h"
#include "levelselect.h"

/* Drawing */
#include "draw.h"
#include "font.h"
#include "particles.h"

#include "objects.h"
/* Game components */
#include "objects.h"
#include "tank.h"
#include "bullet.h"

#include "chipmunk.h"
#include "spaceengine.h"

#include "waffle_utils.h"

static void init(object_data *);

static void player_render(object_group_player *);
static void player_update(object_group_player *);
static void player_destroy(object_group_player *);

static void player_controls(object_group_player *);

static void arcade_control(object_group_player *); //2
static void action_shoot(object_group_player *);

object_group_preset object_type_player = {
	ID_PLAYER,
	init,
	player_update,
	player_render,
	player_destroy
};

//static float timer = 0;

static const float tex_map[2][8] = {
		{0,1, 0.5,1, 0,0, 0.5,0},
		{0.5,1, 1,1, 0.5,0, 1,0}
};

object_param_player default_player = {
		.max_hp = 200,
		.tex_id = -1,
		.gun_cooldown = 0.125f
};

object_group_player *object_create_player()
{
	cpFloat radius = 25;
	cpFloat mass = 2;

	object_group_player *player = (object_group_player *)objects_super_malloc(ID_PLAYER, sizeof(object_group_player));


	player->data.preset = &object_type_player;
	player->data.alive = 1;

	player->param = &default_player;
	player->param->tex_id = TEX_PLAYER;

	player->data.components.hp_bar = &player->hp_bar;
	player->data.components.body_count = 0;

	player->flame = particles_get_emitter(EMITTER_FLAME);
	player->disable=0;
	init((object_data*)player);

	/* make and add new body */
	player->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, radius, radius/2,cpvzero)));
	cpBodySetPos(player->data.body, cpv(0,990));
	cpBodySetVelLimit(player->data.body,550); //700

	/* make and connect new shape to body */
	player->shape = se_add_circle_shape(player->data.body,radius,0.8,0.9);

	cpShapeSetLayers(player->shape, LAYER_PLAYER);
	cpShapeSetCollisionType(player->shape, ID_PLAYER);

	cpBodySetUserData(player->data.body, (void*)player);
	objects_add((object_data*)player);

	hpbar_init(&(player->hp_bar), player->param->max_hp, 80, 16, -40, 30, &(player->data.body->p));

	cpShapeSetGroup(player->shape, 341); // use a group to keep the car parts from colliding

	//FIXME cleanup
	player->gunwheel = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(player->gunwheel, player->data.body->p);

	cpShape *shape = se_add_circle_shape(player->gunwheel,radius,0.9,0.8);

	cpShapeSetGroup(shape, 341); // use a group to keep the car parts from colliding
	cpShapeSetLayers(shape,LAYER_PLAYER_BULLET);

	return player;
}

static void init(object_data *pl)
{
	object_group_player *player = (object_group_player*) pl;
	player->gun_level = 1;
	player->lives = 3;
	player->score = 0;
	player->rotation_speed = 2.5;
	player->aim_angle = 0;
	player->aim_speed = 0.5;
	player->gun_timer = 0;
}

static void player_render(object_group_player *player)
{
	//float s = 0.001;
	float dir = cpBodyGetAngle(player->data.body);

	cpVect pos_body = player->data.body->p;
	cpVect pos_gun = player->gunwheel->p;

	draw_texture(player->param->tex_id, &(pos_gun), &(tex_map[0]), 100, 100, player->aim_angle * 180/M_PI);
	draw_texture(player->param->tex_id, &(pos_body), &(tex_map[1]), 100, 100, dir*180/M_PI);

	hpbar_draw(&player->hp_bar);
}

#include <time.h>
static void player_update(object_group_player *player)
{
	if (keys[SDL_SCANCODE_I] && !config.arcade_keys) //CHEAT
		player->hp_bar.value = 1000000;

	player->gun_timer += dt;

	cpBodySetForce(player->data.body, cpv(0,0));
	//update physics and player
	if (player->hp_bar.value > 0) { //alive
		//cpBodySetAngVel(player->data.body,cpBodyGetAngVel(player->data.body)*0.8);
		cpBodySetAngVel(player->data.body,0);
		player->flame->p = player->data.body->p;
		player->flame->angular_offset = cpvtoangle(player->data.body->v) * (180/M_PI)+90;

		player->gunwheel->p = player->data.body->p;
		player->gunwheel->rot = cpvforangle(player->aim_angle);
		player->gunwheel->v = player->data.body->v;

		if(player->disable == 0){
			player_controls(player);
		}
	} else {
		player->aim_angle = cpvtoangle(player->gunwheel->rot);
		player->flame->disable = 1;
	}

}

static void player_controls(object_group_player *player)
{
	arcade_control(player);

	/* DEBUG KEYS */
	if (!config.arcade_keys) {
		if (keys[SDL_SCANCODE_E]){
			player->data.body->p.x=0;
			player->data.body->p.y=500;
		}

		if (keys[SDL_SCANCODE_H]) {
			cpBodySetVelLimit(player->data.body,5000);
		}
	}
}

static void arcade_control(object_group_player *player)
{
	float player_angle = cpBodyGetAngle(player->data.body);
	float player_angle_target;
	float dir_step;

	cpFloat speed = 700;
	int instant = 1; //tmp instant direction

	if (joy_left.amplitude) {
		player_angle_target = joy_left.direction;

		if (!instant) {
			dir_step = (player->rotation_speed * 2*M_PI)*dt; // 2.5 rps
			player_angle = turn_toangle(player_angle,player_angle_target,dir_step);
		} else {
			player_angle = player_angle_target;
		}
		//TODO use impulses instead?
		cpBodySetForce(player->data.body, cpvmult(cpvforangle(player_angle),speed*300)); //*600

		player->flame->disable = 0;
	} else {
		player->flame->disable = 1;
	}

	cpBodySetAngle(player->data.body, cpvtoangle(cpBodyGetVel(player->data.body)));

	float aim_angle_target = 0;

	if (joy_right.amplitude) {
		aim_angle_target = joy_right.direction;
		if (!instant) {
			dir_step = (player->aim_speed * 2*M_PI) * dt; // 0.5 rps
			player->aim_angle = turn_toangle(player->aim_angle, aim_angle_target, dir_step);
		} else {
			player->aim_angle = aim_angle_target;
		}
		action_shoot(player);
	}
}



static void action_shoot(object_group_player *player)
{
	if (player->gun_timer >= player->param->gun_cooldown) {
		int i;

		for(i=0; i < player->gun_level;i++){
			object_create_bullet(player->data.body->p, cpvforangle(player->aim_angle + (M_PI/70)*((i+1) - (player->gun_level-i))), player->data.body->v, ID_BULLET_PLAYER);
		}

		player->gun_timer = 0;
	}
}


static void player_destroy(object_group_player *player)
{
	//free(player); // does not work since player is currently static inside space.c
}
