#include "object_types.h"
#include "chipmunk.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"

#define OBJ_NAME tank
#include "we_defobj.h"

/* static prototypes */
static cpBody *addWheel(cpSpace *space, cpVect pos, cpGroup group);

#define MASS 4.0f
#define MASS_WHEEL 4.0f
#define MASS_BARREL 1.0f
#define SHOOT_VEL 1500

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(tank, HPBAR, &tank->hp_bar);
	COMPONENT_SET(tank, COINS, &tank->param.coins);
	COMPONENT_SET(tank, MINIMAP, &tank->radar_image);
	tank->radar_image = cmp_new_minimap(10, COL_RED);

	sprite_create(&(tank->wheel_sprite), SPRITE_TANKWHEEL001, 120, 120, 0);
	sprite_create(&(tank->data.spr), SPRITE_TANKBODY001, 200, 100, 0);
	sprite_create(&(tank->turret_sprite), SPRITE_TANKGUN001, 150, 150, 0);

	tank->max_distance = 800;
	tank->rot_speed = M_PI/2;

	cpVect polar_start = we_cart2pol(tank->data.p_start);
	//cpFloat start_height = polar_start. - 50;

	cpFloat width = 80;
	cpFloat height = 30;
	tank->data.body = cpSpaceAddBody(current_space, cpBodyNew(MASS, cpMomentForBox(MASS, width, height)));
	cpBodySetPos(tank->data.body, tank->data.p_start);
	cpBodySetUserData(tank->data.body, tank);
	se_tangent_body(tank->data.body);
	se_velfunc(tank->data.body, 1);

	cpShape *shape = we_add_box_shape(current_space, tank->data.body, width, height, 0.6, 0.0);
	we_shape_collision(shape, &this, LAYER_ENEMY, tank);

	// Make a car with some nice soft suspension
	float wheel_offset = 30;

	cpVect posA = cpvadd(tank->data.body->p, cpvrotate(cpv(-wheel_offset, -30),tank->data.body->rot));
	cpVect posB = cpvadd(tank->data.body->p, cpvrotate(cpv(wheel_offset, -30), tank->data.body->rot));

	tank->wheel1 = addWheel(current_space, posA, tank);
	tank->wheel2 = addWheel(current_space, posB, tank);

	tank->data.components[CMP_BODIES] = tank->wheel1;
	tank->data.components[CMP_BODIES+1] = tank->wheel2;

	cpSpaceAddConstraint(current_space, cpGrooveJointNew(tank->data.body, tank->wheel1 , cpv(-30, -10), cpv(-wheel_offset, -30), cpvzero));
	cpSpaceAddConstraint(current_space, cpGrooveJointNew(tank->data.body, tank->wheel2, cpv( 30, -10), cpv( wheel_offset, -30), cpvzero));

	cpSpaceAddConstraint(current_space, cpDampedSpringNew(tank->data.body, tank->wheel1 , cpv(-30, 0), cpvzero, 50.0f, 600.0f, 0.5f));
	cpSpaceAddConstraint(current_space, cpDampedSpringNew(tank->data.body, tank->wheel2, cpv( 30, 0), cpvzero, 50.0f, 600.0f, 0.5f));

	tank->barrel = cpSpaceAddBody(current_space, cpBodyNew(MASS_BARREL, cpMomentForBox(MASS_BARREL, width/2,height/2)));
	cpBodySetPos(tank->barrel, tank->data.p_start);
	cpBodySetUserData(tank->barrel, tank);
	se_tangent_body(tank->barrel);
	se_velfunc(tank->barrel, 1);
	shape_add_shapes(current_space, POLYSHAPE_TANK, tank->barrel, 150, cpvzero, 0.8, 0.7, tank, &this, LAYER_ENEMY, 0);
	cpSpaceAddConstraint(current_space, cpSimpleMotorNew(tank->data.body, tank->barrel, 0));
	cpSpaceAddConstraint(current_space, cpPivotJointNew(tank->data.body, tank->barrel, tank->data.body->p));

	hpbar_init(&tank->hp_bar,tank->param.max_hp,80,16,0,60,&(tank->data.body->p));
}

static void set_wheel_torque(obj_tank *tank, float torque)
{
	cpBodySetTorque(tank->wheel1, torque);
	cpBodySetTorque(tank->wheel2, torque);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
#if !ARCADE_MODE
	if (keys[SDL_SCANCODE_F2]) { //DEBUG
		tank->data.body->p.x = 0;
		tank->data.body->p.y = 200;
	}
#endif

	tank->timer +=dt;
	/* gets the player from the list */
	obj_player *player = ((obj_player*)instance_nearest(tank->data.body->p, obj_id_player));

	static float ptx = 0;

	if (player) {
		cpFloat best_angle = se_get_best_shoot_angle(tank->data.body, player->data.body, SHOOT_VEL);

		tank->barrel->a = turn_toangle(tank->barrel->a, best_angle, tank->rot_speed * dt);

		//limit barrel between [0, PI]
		cpVect barrel_angle = cpvforangle(tank->barrel->a - tank->data.body->a);
		tank->barrel->a = cpvdot(barrel_angle, cpv(0,1)) < 0 ? (tank->data.body->a +
				(cpvdot(barrel_angle, cpv(1,0)) > 0 ? 0 : WE_PI))  : tank->barrel->a;

		cpBodySetAngle(tank->barrel, tank->barrel->a);

		if(tank->timer > 1 + (3.0f*we_randf) && se_arcdist2player(tank->data.body->p)<tank->max_distance){
			//TODO hent ut lik kode for skyting og lag en metode av det
			cpVect shoot_vel = tank->barrel->rot;
			cpVect shoot_pos = cpvadd(tank->data.body->p, cpvmult(shoot_vel,55));

			shoot_vel = cpvmult(shoot_vel,SHOOT_VEL);

			obj_param_bullet opb = {.friendly = 0, .damage = 10};
			instance_create(obj_id_bullet, &opb, shoot_pos, shoot_vel);
			sound_play(SND_LASER_2);
			tank->timer = 0;
		}
	}

	instance *left, *right;
	cpFloat left_dist, right_dist;
	instance_get2nearest((instance *)tank, obj_id_tank, &left, &right, &left_dist, &right_dist);

	int left_clear = (left_dist > 300);
	int right_clear = (right_dist > 300);

	float velocity_high = 100000;
	float velocity_low = 10000;
	ptx = se_arcdist2player(tank->data.body->p);

	//TODO use motors
	//TMP DEBUG OVERSTYRING AV TANK
	if (keys[SDL_SCANCODE_LCTRL]) {
		if (keys[SDL_SCANCODE_K])
			set_wheel_torque(tank, 500000);
		else if (keys[SDL_SCANCODE_L])
			set_wheel_torque(tank, -500000);
		else
			set_wheel_torque(tank, 0);
	} else {
		if (ptx < 0) {
			if (left_clear)
				set_wheel_torque(tank, left_dist > 500 ? velocity_high : velocity_low);
			else if (right_clear)
				set_wheel_torque(tank, right_dist > 500 ? -velocity_high : -velocity_low);
			else
				set_wheel_torque(tank, 0);
		} else if (ptx > 0) {
			if (right_clear)
				set_wheel_torque(tank, right_dist > 500 ? -velocity_high : -velocity_low);
			else if (left_clear)
				set_wheel_torque(tank, left_dist > 500 ? velocity_high : velocity_low);
			else
				set_wheel_torque(tank, 0);
		} else {
			set_wheel_torque(tank, 0);
		}
	}
}

/*
 * make a wheel
 */
static cpBody * addWheel(cpSpace *space, cpVect pos, cpGroup group) {
	cpFloat radius = 30.0f;
	cpBody *body = cpSpaceAddBody(space,
			cpBodyNew(MASS_WHEEL, cpMomentForCircle(MASS_WHEEL, 0.0f, radius, cpvzero)));
	cpBodySetPos(body, pos);
	cpBodySetAngVelLimit(body, 20);
	se_tangent_body(body);
	se_velfunc(body, 1);

	cpShape *shape = we_add_circle_shape(space, body, radius, 0.9, 0.5);
	cpShapeSetGroup(shape, &this);
	cpShapeSetLayers(shape, LAYER_BULLET_ENEMY);

	return body;
}


static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float alpha = 1;
	if (tank->data.time_destroyed > 2) { //TODO automatically? or in its own destroyed_tick?
		instance_remove((instance *)tank);
		alpha = 0;
	} else if (tank->data.destroyed) {
		alpha = maxf(0, 1 - tank->data.time_destroyed / 2);
	}

	hpbar_draw(RLAY_GUI_BACK, &tank->hp_bar,cpvtoangle(tank->data.body->p));

	if (1) { //TODO REMOVE TMP TEST
		instance *left, *right;
		cpFloat left_dist, right_dist;
		instance_get2nearest((instance *)tank, obj_id_tank, &left, &right, &left_dist, &right_dist);
		draw_color_rgbmulta4f(1,(left_dist > 300),(left_dist > 300),alpha);
		sprite_render_body(RLAY_GAME_FRONT, &(tank->wheel_sprite), tank->wheel1);
		draw_color_rgbmulta4f(1,(right_dist > 300),(right_dist > 300),alpha);
		sprite_render_body(RLAY_GAME_FRONT, &(tank->wheel_sprite), tank->wheel2);
	} else {
	draw_color_rgbmulta4f(1,1,1,alpha);
	sprite_render_body(RLAY_GAME_FRONT, &(tank->wheel_sprite), tank->wheel1);
	sprite_render_body(RLAY_GAME_FRONT, &(tank->wheel_sprite), tank->wheel2);
	}

	if (tank->param.max_hp >= 100) {//TODO add color into param
		draw_color_rgbmulta4f(1,0.2,0,alpha);
	} else {
		draw_color_rgbmulta4f(1,1,1,alpha);
	}

	sprite_render_body(RLAY_GAME_FRONT, &(tank->data.spr), tank->data.body);
	sprite_render_body(RLAY_GAME_FRONT, &(tank->turret_sprite), tank->barrel);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	particles_get_emitter_at(current_particles,RLAY_GAME_FRONT, EMITTER_FRAGMENTS, tank->data.body->p);
	particles_get_emitter_at(current_particles, RLAY_GAME_FRONT, EMITTER_EXPLOSION, tank->data.body->p);
	se_spawn_coins((instance *)tank);
	we_body_remove_constraints(current_space, tank->data.body);
	cpBodySetTorque(tank->wheel1, 0);
	cpBodySetTorque(tank->wheel2, 0);
	sound_play(SND_TANK_EXPLODE);
	//instance_remove(tank); //TODO don't call this method, but just remove constraints
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &tank->data.body);
	we_body_remove(current_space, &tank->barrel);
	we_body_remove(current_space, &tank->wheel1);
	we_body_remove(current_space, &tank->wheel2);
	factory_remove_child((instance *)tank);
}
