#include "object_types.h"
#include "chipmunk.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"

#define OBJ_NAME tank
#include "we_defobj.h"

/* static prototypes */
static cpBody *addChassis(cpSpace *space, obj_tank *tank, cpVect pos, cpGroup group);
static cpBody *addWheel(cpSpace *space, cpVect pos, cpGroup group);

#define MASS 5.0f
#define MASS_WHEEL 10.0f
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

	sprite_create(&(tank->wheel_sprite), SPRITE_TANK_WHEEL, 120, 120, 0);
	sprite_create(&(tank->data.spr), SPRITE_TANK_BODY, 200, 100, 0);
	sprite_create(&(tank->turret_sprite), SPRITE_TANK_TURRET, 150, 150, 0);

	tank->max_distance = 800;
	tank->rot_speed = M_PI/2;

	cpFloat start_height = tank->data.p_start.y - 50;

	cpFloat width = 80;
	cpFloat height = 30;
	tank->data.body = cpSpaceAddBody(space, cpBodyNew(MASS, cpMomentForBox(MASS, width, height)));
	cpBodySetPos(tank->data.body, tank->data.p_start);
	cpBodySetUserData(tank->data.body, tank);
	se_tangent_body(tank->data.body);
	se_velfunc(tank->data.body, 1);

	cpShape *shape = we_add_box_shape(space, tank->data.body, width, height, 0.6, 0.0);
	we_shape_collision(shape, &this, LAYER_ENEMY, tank);

	// Make a car with some nice soft suspension
	float wheel_offset = 40;
	cpVect posA = cpv(tank->data.body->p.x - wheel_offset, start_height);
	cpVect posB = cpv(tank->data.body->p.x + wheel_offset, start_height);

	tank->wheel1 = addWheel(space, posA, tank);
	tank->wheel2 = addWheel(space, posB, tank);

	tank->data.components[CMP_BODIES] = tank->wheel1;
	tank->data.components[CMP_BODIES+1] = tank->wheel2;

	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel1 , cpv(-30, -10), cpv(-wheel_offset, -30), cpvzero));
	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel2, cpv( 30, -10), cpv( wheel_offset, -30), cpvzero));

	cpSpaceAddConstraint(space, cpDampedSpringNew(tank->data.body, tank->wheel1 , cpv(-30, 0), cpvzero, 50.0f, 600.0f, 0.5f));
	cpSpaceAddConstraint(space, cpDampedSpringNew(tank->data.body, tank->wheel2, cpv( 30, 0), cpvzero, 50.0f, 600.0f, 0.5f));

	hpbar_init(&tank->hp_bar,tank->param.max_hp,80,16,0,60,&(tank->data.body->p));
}

static void set_wheel_velocity(obj_tank *tank, float velocity)
{
	cpBodySetTorque(tank->wheel1, velocity);
	cpBodySetTorque(tank->wheel2, velocity);
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
	obj_player *player = ((obj_player*)instance_first(obj_id_player));

	static float ptx = 0;

	if (player) {
	cpVect pl = player->data.body->p;
	cpVect rc = tank->data.body->p;
	ptx = se_arcdist2player(tank->data.body->p.x);

	cpFloat best_angle = se_get_best_shoot_angle(rc,tank->data.body->v, pl, player->data.body->v, SHOOT_VEL);

	best_angle = best_angle - cpvtoangle(tank->data.body->rot);
	if(best_angle < 0){
		best_angle += 2*M_PI;
	}else if(best_angle>2*M_PI){
		best_angle -= 2*M_PI;
	}
	tank->barrel_angle=turn_toangle(tank->barrel_angle, best_angle, tank->rot_speed * dt);


	if(tank->barrel_angle > M_PI && tank->barrel_angle < 3*(M_PI/2)){
		tank->barrel_angle = M_PI;
	}else if(tank->barrel_angle<0 || tank->barrel_angle > 3*(M_PI/2)){
		tank->barrel_angle = 0;
	}

	if(tank->timer > 1 + (3.0f*we_randf) && se_arcdist2player(tank->data.body->p.x)<tank->max_distance){
		//TODO hent ut lik kode for skyting og lag en metode av det
		cpVect shoot_vel = cpvforangle(tank->barrel_angle + cpBodyGetAngle(tank->data.body));
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
	instance_nearest_x_two((instance *)tank, obj_id_tank, &left, &right, &left_dist, &right_dist);

	int left_clear = (left_dist > 250);
	int right_clear = (right_dist > 250);

	float velocity_low = 50000;
	float velocity_heigh = 200000;

	//TMP DEBUG OVERSTYRING AV TANK
	if (keys[SDL_SCANCODE_LCTRL]) {
		if (keys[SDL_SCANCODE_K])
			set_wheel_velocity(tank, 500000);
		else if (keys[SDL_SCANCODE_L])
			set_wheel_velocity(tank, -500000);
		else
			set_wheel_velocity(tank, 0);
	} else {
		if (ptx < 0) {
			if (left_clear)
				set_wheel_velocity(tank, left_dist > 400 ? velocity_heigh : velocity_low);
			else if (right_clear)
				set_wheel_velocity(tank, right_dist > 400 ? -velocity_heigh : -velocity_low);
			else
				set_wheel_velocity(tank, 0);
		} else if (ptx > 0) {
			if (right_clear)
				set_wheel_velocity(tank, right_dist > 400 ? -velocity_heigh : -velocity_low);
			else if (left_clear)
				set_wheel_velocity(tank, left_dist > 400 ? velocity_heigh : velocity_low);
			else
				set_wheel_velocity(tank, 0);
		} else {
			set_wheel_velocity(tank, 0);
		}
	}
}

/*
 * make a wheel
 */
static cpBody * addWheel(cpSpace *space, cpVect pos, cpGroup group) {
	cpFloat radius = 40.0f;
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
	GLfloat dir = cpBodyGetAngle(tank->data.body);
	GLfloat rot = cpBodyGetAngle(tank->wheel1);
	GLfloat barrel_angle = (tank->barrel_angle + dir);

	hpbar_draw(&tank->hp_bar);

	draw_color4f(1,1,1,1);

	sprite_render_body(&(tank->wheel_sprite), tank->wheel1);
	sprite_render_body(&(tank->wheel_sprite), tank->wheel2);

	if (tank->param.max_hp >= 100) {//TODO add color into param
		draw_color4f(1,0.2,0,1);
	}

	cpVect pos = tank->data.body->p;
	sprite_render_body(&(tank->data.spr), tank->data.body);
	sprite_render(&(tank->turret_sprite), pos, barrel_angle);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	particles_get_emitter_at(parti, EMITTER_FRAGMENTS, tank->data.body->p);
	se_spawn_coins((instance *)tank);
	we_body_remove_constraints(space, tank->data.body);
	sound_play(SND_TANK_EXPLODE);
	//instance_remove(tank); //TODO don't call this method, but just remove constraints
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(space, &tank->data.body);
	we_body_remove(space, &tank->wheel1);
	we_body_remove(space, &tank->wheel2);
	factory_remove_child(tank);
}
