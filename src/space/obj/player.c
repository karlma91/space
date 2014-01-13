#include "object_types.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "chipmunk.h"
#include "../upgrades.h"
#include "../../engine/graphics/layersystem.h"

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
	player->gun_level = weapons[weapon_index].level + 1;
	player->lives = 3;
	player->coins = 0;
	player->rotation_speed = 2.5;
	player->aim_speed = 4;
	player->gun_timer = 0;

	cpBodySetPos(player->data.body, player->data.p_start);
	cpBodySetPos(player->gunwheel, player->data.p_start);
	cpBodySetAngle(player->data.body,3*(M_PI/2));
	player->data.body->v = cpvzero;
	player->gunwheel->v = cpvzero;

	player->force = engines[engine_index].force;
	cpBodySetVelLimit(player->data.body, engines[engine_index].max_speed);
	cpBodySetMass(player->data.body, upg_total_mass);
	player->gun_cooldown = 1 / weapons[weapon_index].lvls[weapons[weapon_index].level].firerate;
	player->bullet_dmg = weapons[weapon_index].lvls[weapons[weapon_index].level].damage;
	player->bullet_type = object_by_name(weapons[weapon_index].obj_name);
	player->hp_bar.max_hp = armors[armor_index].max_hp;
	player->hp_bar.value = player->hp_bar.max_hp;


	//TODO get bullet type info from shop
	player->bullet_param = level_get_param(&currentlvl->params, "bullet", "player");

	cpSpaceReindexShapesForBody(current_space, player->gunwheel);
	cpSpaceReindexShapesForBody(current_space, player->data.body);
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	cpShape *shape;
	cpFloat radius = 30;
	cpFloat mass = 2;

	player->player_id = player->param.player_id;
	if(player->player_id == 1) {
		player->joy_left = joy_p1_left;
		player->joy_right = joy_p1_right;
	} else if(player->player_id == 2) {
		player->joy_left = joy_p2_left;
		player->joy_right = joy_p2_right;
	} else {
		SDL_Log("PLAYER CREATE PLAYER WRONG ID");
		exit(1);
	}

	sprite_create(&(player->gun), SPRITE_PLAYERGUN001, 120, 120, 0);
	sprite_create(&(player->data.spr), SPRITE_PLAYERBODY001, 120, 120, 0);

	player->data.components[CMP_HPBAR] = &player->hp_bar;
	player->data.components[CMP_MINIMAP] = &player->radar_image;
	player->radar_image = cmp_new_minimap(10, COL_GREEN);

	player->flame = particles_get_emitter(RLAY_BACK_MID, EM_FLAME);
    if (player->flame) {
        player->flame->self_draw = 1;
        player->flame->disable = 1;
    }
	player->disable=0;

	//cpVect player_pos = cpv(0,990);

	/* make and add new body */
	player->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, radius, radius/2,cpvzero)));
	cpBodySetPos(player->data.body, player->data.p_start);
	//cpBodySetVelLimit(player->data.body,2000); //450 //700
	cpBodySetUserData(player->data.body, (void*)player);
	se_velfunc(player->data.body, 1);

	/* make and connect new shape to body */
	shape = we_add_circle_shape(current_space, player->data.body,radius,0.8,0.9);
	we_shape_collision(shape, &this, LAYER_PLAYER, player);

	shape = we_add_circle_shape(current_space, player->data.body, 250, 0,0); //TODO be able to upgrade coin magnet radius (<--250)
	we_shape_collision(shape, &this, LAYER_PICKUP, player);
	cpShapeSetSensor(shape, 1);

	hpbar_init(&(player->hp_bar), 100, 120, 25, 0, 50, &(player->data.body->p));

	//FIXME cleanup
	player->gunwheel = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(player->gunwheel, player->data.p_start);
	cpBodySetUserData(player->gunwheel, (void*)player);
	se_velfunc(player->gunwheel, 1);

	shape = we_add_circle_shape(current_space, player->gunwheel,radius-2,0.9,0.8);
	we_shape_collision(shape, &this, LAYER_PLAYER, player);
	cpSpaceAddConstraint(current_space, cpPivotJointNew(player->data.body, player->gunwheel, player->data.p_start));

	init(player);
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	if (player->flame) {
		player->flame->p = player->data.body->p;
		player->flame->angular_offset = player->direction + WE_PI_2;
		particles_draw_emitter(player->flame);
	}

	draw_color4f(1,1,1,1);
	sprite_render_body(RLAY_GAME_BACK,&(player->data.spr), player->data.body);
	sprite_render_body(RLAY_GAME_BACK, &(player->gun), player->gunwheel);
	particles_draw_emitter(player->smoke);
	hpbar_draw(RLAY_GAME_FRONT, &player->hp_bar, cpvtoangle(player->data.body->p));
}

#include <time.h>
static void on_update(OBJ_TYPE *OBJ_NAME)
{
#if !ARCADE_MODE
	if (player_cheat_invulnerable)
		player->hp_bar.value = 10000;
#endif
	player->gun_timer += dt;

	//update physics and player
	player->direction = turn_toangle(player->direction_target, player->direction, WE_2PI * dt / 1000);
	if (player->joy_left->amplitude) {
		cpBodySetAngVel(player->data.body,cpBodyGetAngVel(player->data.body)/2);
		//cpBodySetAngVel(player->data.body,0);
		cpBodySetAngle(player->data.body, player->direction);
	}

	cpBodySetPos(player->gunwheel, player->data.body->p);
	//cpBodySetVel(player->gunwheel, player->data.body->v);

	if (player->disable == 0){
		controls(player);
	}
}

static void controls(obj_player *player)
{
	float dir_step;
	int instant = 0; //tmp instant direction

	view *pl_view = player->player_id == 1 ? view_p1 : view_p2;

	if (player->joy_left->amplitude) {
		cpVect player_dir = cpvrotate(cpv(player->joy_left->axis_x, player->joy_left->axis_y), cpvforangle(-pl_view->rotation));
		static float FORCE = 200, VELOCITY = 435;
		//cpVect j = cpvmult(player_dir, player->force);
		cpVect j = cpvmult(player_dir, FORCE);
		//cpBodySetVelLimit(player->data.body, VELOCITY);
		//cpBodySetVelLimit(player->gunwheel, VELOCITY);


		if (player_assisted_steering) {
			//TODO get appliedForce instead?
			cpVect gravity = cpSpaceGetGravity(current_space);
			gravity = cpvmult(cpvnormalize_safe(player->data.body->p), cpvlength(gravity));
			cpVect F = cpvmult(gravity,(cpBodyGetMass(player->data.body)+cpBodyGetMass(player->gunwheel))*dt);
			j.x -= F.x;
			j.y -= F.y;

			float length = cpvlength(j);
			float max_length = player->force * player->joy_left->amplitude;
			if (length > max_length) {
				cpvmult(j, max_length / length);
			}
		}

		player->direction_target = cpvtoangle(j);

		cpBodyApplyImpulse(player->data.body, j, cpvmult(player_dir, -1*dt)); // applies impulse from rocket
		if (player->flame) player->flame->disable = 0;
	} else {
		if (player->flame) player->flame->disable = 1;
		float vel_angle = cpvtoangle(cpBodyGetVel(player->data.body));
		player->direction = turn_toangle(vel_angle, player->direction,WE_2PI * dt / 10000);
	}

	float aim_angle_target = 0;
	if (player->joy_right->amplitude) {
		aim_angle_target = player->joy_right->direction -pl_view->rotation;
		if (!instant) {
			dir_step = (player->aim_speed * 2*M_PI) * dt; // 0.5 rps
			cpBodySetAngle(player->gunwheel, turn_toangle(cpBodyGetAngle(player->gunwheel), aim_angle_target, dir_step));
		} else {
			cpBodySetAngle(player->gunwheel, aim_angle_target);
		}
		cpBodySetAngVel(player->gunwheel,cpBodyGetAngVel(player->gunwheel)/2);
		action_shoot(player);
	}

	/* DEBUG KEYS */
#if !ARCADE_MODE
		if (keys[SDL_SCANCODE_E]){
			player->data.body->p.x=0;
			player->data.body->p.y=0;
		}
#endif
}


//TODO lage en generell skyte-struct
static void action_shoot(obj_player *player)
{
	int i;
	if (player->gun_timer >= player->gun_cooldown) {
		sound_play(SND_LASER_1);
		for(i=0; i < player->gun_level;i++){
			//obj_bullet *b = object_create_bullet(player->data.body->p, cpvforangle(player->aim_angle + (M_PI/70)*((i+1) - (player->gun_level-i))), player->data.body->v, ID_BULLET_PLAYER);
			cpVect shoot_vel = cpvforangle(cpBodyGetAngle(player->gunwheel) + (M_PI/70)*((i+1) - (player->gun_level-i))); //TODO remove and split into another weapon
			cpVect shoot_pos = cpvadd(player->gunwheel->p, cpvmult(shoot_vel,40));
			shoot_vel = cpvmult(shoot_vel, 1400);
			//shoot_vel = cpvadd(shoot_vel, player->data.body->v);
			instance_create(player->bullet_type, player->bullet_param, shoot_pos, shoot_vel);
		}
		player->gun_timer = 0;
	}
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
	if (player->data.destroyed && player->data.time_destroyed < 2) {
		player->gun_timer += dt * we_randf*we_randf;
		action_shoot(player);
	}

	if (player->smoke) {
		cpVect pos = player->data.body->p;
		cpVect rot = player->data.body->rot;
		pos = cpvadd(pos,cpvrotate(cpv(20,10),rot));
		player->smoke->p = pos;
		player->smoke->angular_offset = player->data.body->a; //TODO make sure smoke is going in -g direction
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	explosion_create(player->data.body->p, EM_EXPLOSION, EM_FRAGMENTS, SND_BUILDING_EXPLODE, 1300, 180, 0.2);
	player->disable = 1;
    if (player->flame) {
    	player->flame->disable = 1;
    }
	we_body_remove_constraints(current_space, player->data.body);

	player->smoke = particles_get_emitter(RLAY_GAME_BACK, EM_SMOKE);
	particles_self_draw(player->smoke, 1);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	particles_release_emitter(player->flame);
	particles_release_emitter(player->smoke);
	we_body_remove(current_space, &player->gunwheel);
	we_body_remove(current_space, &player->data.body);
}
