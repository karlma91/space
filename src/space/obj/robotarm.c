#include "object_types.h"
#include "chipmunk.h"
#include "../game.h"
#include "../spaceengine.h"
#include "../states/space.h"

#define OBJ_NAME robotarm
#include "we_defobj.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{

	COMPONENT_SET(robotarm, HPBAR, &robotarm->hp_bar);
	COMPONENT_SET(robotarm, COINS, &robotarm->param.coins);
	COMPONENT_SET(robotarm, MINIMAP, &robotarm->radar_image);
	robotarm->radar_image = cmp_new_minimap(12, COL_BLUE);

	cpShape *shape;
	robotarm->timer = 0;

	sprite_create(&(robotarm->saw_sprite), SPRITE_SAW, 300, 300, 30);
	sprite_create(&(robotarm->data.spr), SPRITE_TANK_BODY, 200, 100, 0);
	robotarm->segments = 4;
	robotarm->seg_length = currentlvl->height / (2*robotarm->segments);
	cpVect pos = robotarm->data.p_start;
	pos = we_cart2pol(pos);
	pos.x += 180;
	pos = we_pol2cart(pos);

	robotarm->x = 	  calloc(robotarm->segments, sizeof(float));
	robotarm->y =	  calloc(robotarm->segments, sizeof(float));
	robotarm->angle = calloc(robotarm->segments, sizeof(float));
	int i;
	for(i=0; i<robotarm->segments; i++){
		robotarm->x[i] = pos.x;
		robotarm->y[i] = pos.y;
		robotarm->angle[i] = 0;
	}

	cpFloat radius = 130.0f;
	cpFloat mass = 2.0f;


	robotarm->saw = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(robotarm->saw, pos);
	cpBodySetVelLimit(robotarm->saw, 300);
	cpBodySetUserData(robotarm->saw , (instance*)robotarm);

	shape = we_add_circle_shape(current_space, robotarm->saw, radius, 0.7, 0.0);
	we_shape_collision(shape, ID_GROUND , LAYER_ENEMY, robotarm);

	cpFloat size = 50;
	/* make and add new static body */
	robotarm->data.body = cpBodyNew(200, cpMomentForBox(20.0f, size, size));
	cpBodySetUserData(((instance *) robotarm)->body, (instance*)robotarm);


	cpBodySetPos(((instance *) robotarm)->body, pos);
	se_tangent_body(robotarm->data.body);
	cpBodySetVelLimit(((instance *) robotarm)->body,150);

	/* make and connect new shape to body */
	shape = cpSpaceAddShape(current_space, cpBoxShapeNew(robotarm->data.body, size, size));
	cpShapeSetFriction(shape, 0.01);
	we_shape_collision(shape, &this, LAYER_ENEMY, robotarm);

	//connect sawblade with body
	cpSpaceAddConstraint(current_space, cpSlideJointNew(robotarm->saw, robotarm->data.body, cpv(0,0), cpv(0,0), 1.0f, (robotarm->segments)*robotarm->seg_length));

	hpbar_init(&robotarm->hp_bar, robotarm->param.max_hp,80,16,0,60,&(robotarm->data.body->p));
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	robotarm->timer += dt;
	sprite_update(&(robotarm->saw_sprite));


	float targetx = robotarm->saw->p.x;
	float targety = robotarm->saw->p.y;

	robotarm->x[robotarm->segments-1] = robotarm->data.body->p.x;
	robotarm->y[robotarm->segments-1] = robotarm->data.body->p.y;

	int i;
	for(i=0; i<robotarm->segments; i++){
		float dx = targetx - robotarm->x[i];
		float dy = targety - robotarm->y[i];
		robotarm->angle[i] = atan2f(dy,dx);
		targetx = targetx - cos(robotarm->angle[i])*robotarm->seg_length;
		targety = targety - sin(robotarm->angle[i])*robotarm->seg_length;
	}
	for(i=robotarm->segments-1; i>0; i--){
		robotarm->x[i-1] = robotarm->x[i] + cos(robotarm->angle[i])*robotarm->seg_length;
		robotarm->y[i-1] = robotarm->y[i] + sin(robotarm->angle[i])*robotarm->seg_length;
	}

	instance *player = instance_nearest(robotarm->saw->p,obj_id_player);
	cpVect d = cpvzero;
	if(player){
		d = se_dist_v(robotarm->saw->p, player->body->p);
	}

	cpBodySetForce(robotarm->saw, cpvzero);
	d = cpvnormalize(d);
	cpBodyApplyForce(robotarm->saw, cpvmult(d, 10000), cpvzero);
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float redfade = (1.5 - robotarm->data.time_destroyed) / 1.5;
	redfade = redfade < 0 ? 0 : redfade;
	if (robotarm->data.time_destroyed >= 1.5) { //TODO automatically? or in its own destroyed_tick?
		particles_get_emitter_at(current_particles, RLAY_GAME_FRONT, EMITTER_EXPLOSION_BIG, robotarm->saw->p);
		sound_play(SND_FACTORY_EXPLODE);
		instance_remove((instance *)robotarm);
	} else if (robotarm->data.destroyed) {

	}
	draw_color4f(1,redfade,redfade,1);

	if (!robotarm->data.destroyed) {
		draw_color4f(1,0,0,1);
		cpVect v1 = cpv(0,0);
		cpVect v2 = cpv(0,0);

		v1.x = robotarm->x[0];
		v1.y = robotarm->y[0];
		v2.x = robotarm->x[0] + cos(robotarm->angle[0])*robotarm->seg_length;
		v2.y = robotarm->y[0] + sin(robotarm->angle[0])*robotarm->seg_length;
		draw_glow_line(v1, v2, 200);

		int i;
		for(i=0; i<robotarm->segments-1; i++){
			draw_color4f(1*(1-(1.0f*i/(robotarm->segments-1))),0,0,1);
			v1.x = robotarm->x[i];
			v1.y = robotarm->y[i];
			v2.x = robotarm->x[i+1];
			v2.y = robotarm->y[i+1];
			draw_glow_line(v1, v2, 200);
		}
	}
	sprite_render_body(RLAY_GAME_MID, &(robotarm->saw_sprite), robotarm->saw);
	sprite_render_body(RLAY_GAME_MID, &(robotarm->data.spr), robotarm->data.body);
	hpbar_draw(RLAY_GAME_FRONT, &robotarm->hp_bar,cpvtoangle(robotarm->data.body->p));
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	particles_get_emitter_at(current_particles, RLAY_GAME_FRONT, EMITTER_EXPLOSION, robotarm->data.body->p);
	sound_play(SND_TANK_EXPLODE);
	se_spawn_coins((instance *)robotarm);
	cpBodySetForce(robotarm->saw, cpvzero);
	we_body_remove_constraints(current_space, robotarm->data.body);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	free(robotarm->x);
	free(robotarm->y);
	free(robotarm->angle);

	we_body_remove(current_space, &robotarm->saw);
	we_body_remove_static(current_space, &robotarm->data.body);
}
