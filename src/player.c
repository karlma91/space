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
static void tmp_shoot_dir(object *obj, float v);

//static void pitch_up(cpBody *body, float rotSpeed);
//static void pitch_down(cpBody *body, float rotSpeed);


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
	pl->e = particles_get_emitter(EMITTER_FLAME);

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
	cpBodySetForce(obj->body, cpv(0,0));
	cpBodySetTorque(obj->body, 0);

	temp->e->x = obj->body->p.x;
	temp->e->y = obj->body->p.y;

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

		if (keys[SDLK_x]) {
			particles_add_explosion(cpBodyGetPos(temp->obj.body),0.5f,4000, 1000,200);
		}
		if (keys[SDLK_b]) {
			emitter *t = particles_get_emitter(EMITTER_EXPLOTION);
			if(t != NULL){
				t->x = obj->body->p.x;
				t->y = obj->body->p.y;
			}
			keys[SDLK_b] = 0;
		}
}

static void normal_control()
{
	/* units/sec */
	cpFloat accel = 100000;

	cpFloat cspeed = cpvlength(cpBodyGetVel(temp->obj.body));
	/* Player movement */
	if(keys[SDLK_w] && cspeed > 50) {
		//pitch_up((temp->obj.body),temp->rotation_speed);
	}
	if(keys[SDLK_s] && cspeed > 50) {
		//pitch_down((temp->obj.body),temp->rotation_speed);
	}
	cpBodySetAngle(temp->obj.body, cpvtoangle(cpBodyGetVel(temp->obj.body)));
	cpSpaceReindexShapesForBody(space, temp->obj.body);

	if(keys[SDLK_d]) cpBodyApplyForce(temp->obj.body,cpvmult(cpBodyGetRot(temp->obj.body),accel*dt),cpvzero);
	if(keys[SDLK_a]) cpBodyApplyForce(temp->obj.body,cpvmult(cpBodyGetRot(temp->obj.body),-accel*dt),cpvzero);

	if (keys[SDLK_SPACE]) {
		tmp_shoot(temp);
	}
}

//TODO move this method to objects.c?
static float turn_toangle(float from_angle, float to_angle, float step_size)
{
	//TODO fix oscillation when to_angle is around 0
	//TODO possible fix?: consider including step_size in some way in the ternary tests
	from_angle += from_angle >= (2*M_PI) ? -(2*M_PI) : from_angle < 0 ? (2*M_PI) : 0;

	if (to_angle < from_angle - step_size) {
		if ((from_angle - to_angle) < M_PI) {
			from_angle -= step_size;
		} else {
			if (2*M_PI - (from_angle - to_angle) < step_size) {
				from_angle = to_angle;
			} else {
				from_angle += step_size;
			}
		}
	} else if (to_angle > from_angle + step_size) {
		if ((to_angle - from_angle) < M_PI) {
			from_angle += step_size;
		} else {
			if (2*M_PI - (to_angle - from_angle) < step_size) {
				from_angle = to_angle;
			} else {
				from_angle -= step_size;
			}
		}
	} else {
		from_angle = to_angle;
	}

	//fprintf(stderr,"angle: %0.4f\n",from_angle*180/M_PI);
	return from_angle;
}

typedef enum {
	DIR_E, DIR_NE, DIR_N, DIR_NW, DIR_W, DIR_SW, DIR_S, DIR_SE, DIR_NONE = -1
} Direction;

static const float dir8[8] = {
		M_PI/4*0,
		M_PI/4*1,
		M_PI/4*2,
		M_PI/4*3,
		M_PI/4*4,
		M_PI/4*5,
		M_PI/4*6,
		M_PI/4*7,
};

static inline Direction angle_index_fromkeys(SDLKey key_left, SDLKey key_up, SDLKey key_right, SDLKey key_down)
{
	return keys[key_up] ? (keys[key_right] ? DIR_NE : (keys[key_left] ? DIR_NW : DIR_N)) :
		 keys[key_down] ? (keys[key_right] ? DIR_SE : (keys[key_left] ? DIR_SW : DIR_S)) :
		(keys[key_right] ? DIR_E : (keys[key_left] ? DIR_W : DIR_NONE));
}

static void arcade_control()
{
	float player_angle = cpBodyGetAngle(temp->obj.body);
	float player_angle_target;
	Direction angle_index = -1;

	angle_index = angle_index_fromkeys(SDLK_a, SDLK_w, SDLK_d, SDLK_s);

	cpFloat speed = 700;

	if (angle_index != DIR_NONE) {
		player_angle_target = dir8[angle_index];
		float dir_step = (1 * 2*M_PI) * dt; // 1 rps
		player_angle = turn_toangle(player_angle,player_angle_target,dir_step);

		cpBodySetVel(temp->obj.body, cpvmult(cpvforangle(player_angle),speed));
	}

	cpBodySetAngle(temp->obj.body, cpvtoangle(cpBodyGetVel(temp->obj.body)));
	cpSpaceReindexShapesForBody(space, temp->obj.body);

	static float aim_angle = 0;
	float aim_angle_target = 0;

	angle_index = angle_index_fromkeys(SDLK_LEFT, SDLK_UP, SDLK_RIGHT, SDLK_DOWN);

	if (angle_index != DIR_NONE) {
		aim_angle_target = dir8[angle_index];
		float dir_step = (0.5f * 2*M_PI) * dt; // 0.5 rps
		aim_angle = turn_toangle(aim_angle,aim_angle_target,dir_step);
		tmp_shoot_dir((object *)temp, aim_angle);
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
		bullet_init(temp->obj.body->p,cpvforangle(cpBodyGetAngle(temp->obj.body) + (M_PI/70)*((i+1) - (temp->gun_level-i))),obj->body->v, ID_BULLET_PLAYER);
	}
}
static void tmp_shoot_dir(object *obj, float v)
{
	//TMP shooting settings
	static const float cooldown = 0.05f;

	if (timer < cooldown) {
		return;
	}
	timer = 0;
	//bullet_init(temp->body->p,cpvforangle(cpBodyGetAngle(temp->body)),ID_BULLET_PLAYER);
	int i;
	for(i=0; i < temp->gun_level;i++){
		bullet_init(obj->body->p, cpvforangle(v + (M_PI/70)*((i+1) - (temp->gun_level-i))), obj->body->v, ID_BULLET_PLAYER);
	}
}


static void player_destroy(object *obj)
{
	*obj->remove = 1;
	//free(obj);
}
