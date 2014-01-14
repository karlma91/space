#include "object_types.h"
#include "chipmunk.h"
#include "../game.h"
#include "../spaceengine.h"
#include "../states/space.h"

#define OBJ_NAME robotarm
#include "we_defobj.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
	cpVect pos = robotarm->data.p_start;

	cpBodySetPos(((instance *) robotarm)->body, pos);
	se_tangent_body(robotarm->data.body);
	cpBodySetPos(robotarm->saw, pos);

	int i;
	for(i=0; i<robotarm->param.segments; i++){
		robotarm->x[i] = pos.x;
		robotarm->y[i] = pos.y;
		robotarm->angle[i] = 0;
	}

	cpSpaceReindexShapesForBody(current_space, robotarm->saw);
	cpSpaceReindexShapesForBody(current_space, robotarm->data.body);
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	int segments = robotarm->param.segments;
	int seg_length = robotarm->param.seg_length;
	COMPONENT_SET(robotarm, HPBAR, &robotarm->hp_bar);
	COMPONENT_SET(robotarm, COINS, &robotarm->param.coins);
	COMPONENT_SET(robotarm, MINIMAP, &robotarm->radar_image);
	robotarm->radar_image = cmp_new_minimap(12, COL_BLUE);

	cpShape *shape;
	robotarm->timer = 0;

	sprite_create(&(robotarm->saw_sprite), SPRITE_SAW, 300, 300, 30);
	sprite_create(&(robotarm->data.spr), SPRITE_TANKBODY001, 200, 100, 0);
	//segments = 3;
	//robotarm->param.seg_length = currentlvl->height / (2*segments);
	cpVect pos = robotarm->data.p_start;
	robotarm->x = 	  calloc(segments, sizeof(float));
	robotarm->y =	  calloc(segments, sizeof(float));
	robotarm->angle = calloc(segments, sizeof(float));
	int i;
	for(i=0; i<segments; i++){
		robotarm->x[i] = pos.x;
		robotarm->y[i] = pos.y;
		robotarm->angle[i] = 0;
	}
	cpFloat radius = 130.0f;
	cpFloat mass = 2.0f;
	robotarm->saw = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(robotarm->saw, pos);
	cpBodySetVelLimit(robotarm->saw, robotarm->param.max_vel);
	cpBodySetUserData(robotarm->saw, (instance*)robotarm);

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
	cpSpaceAddConstraint(current_space, cpSlideJointNew(robotarm->data.body,robotarm->saw, cpv(0,0), cpv(0,0), 1.0f, segments*seg_length));

	hpbar_init(&robotarm->hp_bar, robotarm->param.max_hp,80,16,0,60,&(robotarm->data.body->p));

	se_velfunc(robotarm->saw, 0);
	se_velfunc(robotarm->data.body, 0);
	init(robotarm);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	int segments = robotarm->param.segments;
	int seg_length = robotarm->param.seg_length;
	int max_reach = segments*seg_length;

	robotarm->timer += dt;
	sprite_update(&(robotarm->saw_sprite));

	float targetx = robotarm->saw->p.x;
	float targety = robotarm->saw->p.y;

	robotarm->x[segments-1] = robotarm->data.body->p.x;
	robotarm->y[segments-1] = robotarm->data.body->p.y;

	int i;
	for(i=0; i<segments; i++){
		float dx = targetx - robotarm->x[i];
		float dy = targety - robotarm->y[i];
		robotarm->angle[i] = atan2f(dy,dx);
		targetx = targetx - cos(robotarm->angle[i])*seg_length;
		targety = targety - sin(robotarm->angle[i])*seg_length;
	}
	for(i=segments-1; i>0; i--){
		robotarm->x[i-1] = robotarm->x[i] + cos(robotarm->angle[i])*seg_length;
		robotarm->y[i-1] = robotarm->y[i] + sin(robotarm->angle[i])*seg_length;
	}



	cpVect dist = cpvzero;
	instance *player = instance_nearest(robotarm->saw->p,obj_id_player);
	if (player) {
		//dist = se_dist2body(robotarm->saw, player->body);
		/*
		cpVect target = player->body->p;
		dist = se_dist2body(robotarm->data.body, player->body);
		float dist_lensq = cpvlengthsq(dist);
		float maxmax = max_reach * max_reach;
		if (dist_lensq > maxmax) {
			target = cpvmult(cpvnormalize(dist), max_reach);
		}
		float len = cpvlength(dist) / max_reach * 10;
		float force = robotarm->param.force * (len > max_reach / 10 ? 1 : len);
		dist = cpvnormalize_safe(cpvsub(target, robotarm->saw->p));
		*/
		dist = cpvnormalize_safe(se_dist2body(robotarm->saw, player->body));
		cpBodySetForce(robotarm->saw, cpvmult(dist, robotarm->param.force));
	} else {
		cpBodySetForce(robotarm->saw, cpvzero);
	}
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
	if (robotarm->data.time_destroyed >= 1.5) {
		explosion_create(robotarm->saw->p, EM_EXPLOSIONBIG, EM_SPARKS, SND_BUILDING_EXPLODE, 1200, 320, 0.3, 10);
		instance_remove((instance *)robotarm);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	int segments = robotarm->param.segments;
	int seg_length = robotarm->param.seg_length;
	float redfade = (1.5 - robotarm->data.time_destroyed) / 1.5;
	redfade = redfade < 0 ? 0 : redfade;

	if (!robotarm->data.destroyed) {
		draw_color4f(1,0,0,1);
		cpVect v1 = cpv(0,0);
		cpVect v2 = cpv(0,0);

		v2.x = robotarm->x[0];
		v2.y = robotarm->y[0];
		v1.x = robotarm->x[0] + cos(robotarm->angle[0])*seg_length;
		v1.y = robotarm->y[0] + sin(robotarm->angle[0])*seg_length;
		draw_line_spr_id(RLAY_GAME_MID, SPRITE_GEAR, v1, v2, 100, 0);

		draw_color4f(0.9, 0.9, 0.9, 1);
		sprite_render_by_id(RLAY_GAME_BACK, SPRITE_GEAR, v2, cpv(200,200), cpvtoangle(cpvsub(v2,v1)));

		int i;
		for(i=0; i<segments-1; i++){
			v1.x = robotarm->x[i];
			v1.y = robotarm->y[i];
			v2.x = robotarm->x[i+1];
			v2.y = robotarm->y[i+1];
			draw_color4f(0.9, 0.9, 0.9, 1);
			sprite_render_by_id(RLAY_GAME_BACK, SPRITE_GEAR, v2, cpv(200,200), -cpvtoangle(cpvsub(v2,v1)));
			draw_color4f(1*(1-(1.0f*i/(segments-1))),0,0,1);
			draw_line_spr_id(RLAY_GAME_MID, SPRITE_GEAR, v1, v2, 100, 0);
		}
	}

	draw_color4f(1,redfade,redfade,1);
	sprite_render_body(RLAY_GAME_FRONT, &(robotarm->saw_sprite), robotarm->saw);
	sprite_render_body(RLAY_GAME_FRONT, &(robotarm->data.spr), robotarm->data.body);
	hpbar_draw(RLAY_GAME_FRONT, &robotarm->hp_bar,cpvtoangle(robotarm->data.body->p));
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	explosion_create(robotarm->data.body->p, EM_EXPLOSION, NULL, SND_UNIT_EXPLODE, 800, 210, 0.3, 10);
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
