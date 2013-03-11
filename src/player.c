/* header */
#include "player.h"

/* standard c-libraries */
#include <stdio.h>
#include <time.h>

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


static void init(object *fac);

static void player_render(object *obj);
static void player_update(object *obj);
static void player_destroy(object *obj);
static void tmp_shoot(object *obj);

static int collision_enemy_bullet(cpArbiter *arb, cpSpace *space, void *unused);

static void player_controls(object *obj);
static void playerVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

static int controll_mode = 2;

static void normal_control(); //1
static void arcade_control(); //2
static void tmp_shoot_dir(cpVect pos, cpVect v);

static void pitch_up(cpBody *body, float rotSpeed);
static void pitch_down(cpBody *body, float rotSpeed);


struct obj_type type_player = {
	ID_PLAYER,
	init,
	player_update,
	player_render,
	player_destroy
};

static struct player *temp;
static float timer = 0;

static const texture_map tex_map = {
		0,0,1,1
};

struct player_param default_player = {
		200,
		-1
};

object *player_init()
{
	cpFloat radius = 50;
	cpFloat mass = 10;

	struct player *pl = malloc(sizeof(struct player));

	//(object* pl)

	pl->obj.type = &type_player;
	pl->obj.alive = 1;

	pl->param = &default_player;
	pl->param->tex_id = TEX_PLAYER;

	pl->hp = 200;
	pl->gun_level = 1;
	pl->lives = 3;
	pl->score = 0;
	pl->disable = 0;
	pl->rotation_speed = 8;

	/* make and add new body */
	pl->obj.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, radius, radius/2)));
	cpBodySetPos(pl->obj.body, cpv(0,990));
	cpBodySetVelLimit(pl->obj.body,700);
	pl->obj.body->velocity_func = playerVelocityFunc;
	/* make and connect new shape to body */
	pl->shape = cpSpaceAddShape(space, cpBoxShapeNew(pl->obj.body, radius, radius/2));
	cpShapeSetFriction(pl->shape, 0.8);
	cpShapeSetUserData(pl->shape, draw_boxshape);
	cpShapeSetElasticity(pl->shape, 1.0f);
	cpShapeSetLayers(pl->shape, LAYER_PLAYER);
	cpSpaceAddCollisionHandler(space, ID_PLAYER, ID_BULLET_ENEMY, collision_enemy_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(pl->obj.body, (void*)pl);
	objects_add((object*)pl);
	return (object*)pl;

}

static void init(object *fac)
{
	temp = ((struct player*)fac);
}

static void player_render(object *obj)
{
	temp = (struct player*)obj;
	//float s = 0.001;
	float dir = cpBodyGetAngle(temp->obj.body);
	//Color c = RGBAColor(1,0,0,1);
	//draw_boxshape(temp->shape,RGBAColor(1,1,1,1),c);

	static float hp_timer = 0;
	static float hp_last = 1;
	float hp = temp->hp/temp->param->max_hp;

	if (hp_last != hp) {
		hp_timer = 2;
		hp_last = hp;
	}
	if (hp_timer > 0 || hp < 0.5) {
		hp_timer -= dt;
		draw_hp((obj->body->p).x-25,(obj->body->p).y+30,50,8,hp);
	}

	//TODO: fix player draw texture
	///*
	draw_texture(temp->param->tex_id, &(obj->body->p), &(tex_map), 100, 100, dir*180/M_PI);
	// */
}

static void player_update(object *obj)
{
	timer += dt;
	temp = (struct player*)obj;
	//cpFloat pangvel = cpBodyGetAngVel(temp->body);
	//cpBodySetAngVel(temp->body, pangvel*0.9);
	//cpVect pvel = cpBodyGetVel(obj->body);
	
	//update physics and player
	cpVect rot = cpBodyGetRot(temp->obj.body);
	rot = cpvmult(rot, 10000);
	cpBodySetForce(temp->obj.body, cpv(0,0));
	cpBodySetTorque(temp->obj.body, 0);

	if(temp->disable == 0){
		player_controls(obj);
	}

}

static void player_controls(object *obj)
{


		switch(controll_mode){
			case 1:
				normal_control();
				break;
			case 2:
				arcade_control();
				break;
		}

		if(keys[SDLK_g]){
			keys[SDLK_g] = 0;
			cpVect gravity = cpv(0, -2);
			cpSpaceSetGravity(space, gravity);
		}

		if (keys[SDLK_q]){
			cam_zoom /= dt+1.4f;
		}

		if (keys[SDLK_e]){
			cam_zoom *= dt+1.4f;
			if (keys[SDLK_q])
				cam_zoom = 1;
		}
		if (keys[SDLK_r]){
			temp->obj.body->p.x=0;
			temp->obj.body->p.y=500;
		}

		if (keys[SDLK_h]) {
			cpBodySetVelLimit(temp->obj.body,5000);
		}

		if (keys[SDLK_SPACE]) {
			tmp_shoot(obj);
		}

		if (keys[SDLK_x]) {
			particles_add_explosion(cpBodyGetPos(temp->obj.body),0.5f,4000, 1000,200);
		}
}

static void normal_control()
{
	/* units/sec */
	cpFloat accel = 100000;

	cpFloat cspeed = cpvlength(cpBodyGetVel(temp->obj.body));
	/* Player movement */
	if(keys[SDLK_w] && cspeed > 50) {
		pitch_up((temp->obj.body),temp->rotation_speed);
	}
	if(keys[SDLK_s] && cspeed > 50) {
		pitch_down((temp->obj.body),temp->rotation_speed);
	}
	cpBodySetAngle(temp->obj.body, cpvtoangle(cpBodyGetVel(temp->obj.body)));
	cpSpaceReindexShapesForBody(space, temp->obj.body);

	if(keys[SDLK_d]) cpBodyApplyForce(temp->obj.body,cpvmult(cpBodyGetRot(temp->obj.body),accel*dt),cpvzero);
	if(keys[SDLK_a]) cpBodyApplyForce(temp->obj.body,cpvmult(cpBodyGetRot(temp->obj.body),-accel*dt),cpvzero);
}

static void arcade_control()
{

	cpFloat speed = 700;

	cpVect vel = cpv(0,0.01);
	int pressed = 0;
	if(keys[SDLK_w]) {
		vel = cpvadd(vel, cpvmult(cpv(0,1), speed));
		pressed=1;
	}else if(keys[SDLK_s]) {
		vel = cpvadd(vel,cpvmult(cpv(0,-1), speed));
		pressed=1;
	}
	if(keys[SDLK_a]) {
		vel = cpvadd(vel, cpvmult(cpv(-1,0), speed));
		pressed=1;
	}else if(keys[SDLK_d]) {
		vel = cpvadd(vel, cpvmult(cpv(1,0), speed));
		pressed=1;
	}
	if(pressed){
		cpBodySetVel(temp->obj.body, vel);
	}

	cpBodySetAngle(temp->obj.body, cpvtoangle(cpBodyGetVel(temp->obj.body)));
	cpSpaceReindexShapesForBody(space, temp->obj.body);

	cpVect shoot_dir = cpv(0,0);
	if(keys[SDLK_UP]) {
		shoot_dir = cpvadd(shoot_dir, cpv(0,1));
	}else if(keys[SDLK_DOWN]) {
		shoot_dir = cpvadd(shoot_dir, cpv(0,-1));
	}
	if(keys[SDLK_LEFT]) {
		shoot_dir = cpvadd(shoot_dir, cpv(-1,0));
	}else if(keys[SDLK_RIGHT]) {
		shoot_dir = cpvadd(shoot_dir, cpv(1,0));
	}

	if(shoot_dir.x != 0 || shoot_dir.y != 0){
		tmp_shoot_dir(temp->obj.body->p,cpvrotate(cpBodyGetVel(temp->obj.body),shoot_dir));
	}
}

static void pitch_up(cpBody *body, float rotSpeed)
{
	cpVect dirUp = cpvforangle(-rotSpeed*dt);
	cpBodySetVel(body, cpvrotate(cpBodyGetVel(body),dirUp));
}

static void pitch_down(cpBody *body, float rotSpeed)
{
	cpVect dirDown = cpvforangle(rotSpeed*dt);
	cpBodySetVel(body, cpvrotate(cpBodyGetVel(body),dirDown));
}

/**
 * Velocity function to remove gravity
 */
static void playerVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpVect g = cpv(0,0);
	g = cpvproject(gravity,body->v);
	cpBodyUpdateVelocity(body, g, damping, dt);
}


static int collision_enemy_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	temp = ((struct player*)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->alive = 0;

	//particles_add_explosion(b->body->p,0.3,1500,15,200);
	if(temp->hp <= 0 ){
		particles_add_explosion(a->body->p,1,2000,50,800);
			temp->lives--;
	}else{
		temp->hp -= 10;
	}

	return 0;
}

static void tmp_shoot(object *obj)
{
	temp = (struct player*)obj;
	//TMP shooting settings
	static const float cooldown = 0.15f;
	
	if (timer < cooldown) {
		return;
	}
	timer = 0;
	//bullet_init(temp->body->p,cpvforangle(cpBodyGetAngle(temp->body)),ID_BULLET_PLAYER);
	int i;
	for(i=0; i < temp->gun_level;i++){
		bullet_init(temp->obj.body->p,cpvforangle(cpBodyGetAngle(temp->obj.body) + (M_PI/70)*((i+1) - (temp->gun_level-i))),ID_BULLET_PLAYER);
	}
}
static void tmp_shoot_dir(cpVect pos, cpVect v)
{
	//TMP shooting settings
	static const float cooldown = 0.1f;

	if (timer < cooldown) {
		return;
	}
	timer = 0;
	//bullet_init(temp->body->p,cpvforangle(cpBodyGetAngle(temp->body)),ID_BULLET_PLAYER);
	int i;
	for(i=0; i < temp->gun_level;i++){
		bullet_init(pos,cpvforangle(cpvtoangle(v) + (M_PI/70)*((i+1) - (temp->gun_level-i))),ID_BULLET_PLAYER);
	}
}


static void player_destroy(object *obj)
{
	*obj->remove = 1;
	//free(obj);
}
