/* header */
#include "player.h"

/* standard c-libraries */
#include <stdio.h>
#include <time.h>

/* Game state */
#include "space.h"

/* Drawing */
#include "draw.h"
#include "font.h"
#include "particles.h"

/* Game components */
#include "objects.h"
#include "tank.h"
#include "bullet.h"


static void init(object *fac);

static void player_render(object *obj);
static void player_update(object *obj);
static void player_destroy(object *obj);
static void tmp_shoot(object *obj);

struct obj_type type_player = {
	ID_PLAYER,
	init,
	player_update,
	player_render,
	player_destroy
};

static struct player *temp;
static float timer = 0;

object *player_init()
{
	cpFloat radius = 10;
	//cpFloat mass = 1;

	struct player *pl = malloc(sizeof(struct player));

	pl->type = &type_player;
	pl->alive = 1;
	pl->max_hp = 200;
	pl->hp = 200;
	/* make and add new body */
	pl->body = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForBox(1.0f, radius, radius)));
	cpBodySetPos(pl->body, cpv(0,990));
	cpBodySetVelLimit(pl->body,700);
	/* make and connect new shape to body */
	pl->shape = cpSpaceAddShape(space, cpBoxShapeNew(pl->body, radius, radius));
	cpShapeSetFriction(pl->shape, 0.7);
	cpShapeSetUserData(pl->shape, draw_boxshape);
	cpShapeSetElasticity(pl->shape, 1.0f);

	cpBodySetUserData(pl->body, (object*)pl);
	list_add((object*)pl);
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
	float dir = cpBodyGetAngle(temp->body);
	setTextAlign(TEXT_CENTER);
	setTextSize(10);
	setTextAngleRad(dir);
	static char text[20];

	sprintf(text, " SPEED: %.3f",cpvlength(cpBodyGetVel(temp->body)));
	glColor3f(1,1,1);
	font_drawText(temp->body->p.x,temp->body->p.y, text);
	draw_hp(temp->body->p.x-20,temp->body->p.y+15,40,10,temp->hp/temp->max_hp);
}

static void player_update(object *obj)
{

	timer += dt;
	temp = (struct player*)obj;
	cpFloat pangvel = cpBodyGetAngVel(temp->body);
	cpBodySetAngVel(temp->body, pangvel*0.9);
	//cpVect pvel = cpBodyGetVel(obj->body);
	
	//update physics and player
	cpVect rot = cpBodyGetRot(temp->body);
	rot = cpvmult(rot, 10000);
	cpBodySetForce(temp->body, cpv(0,0));
	cpBodySetTorque(temp->body, 0);
	if (temp->body->p.x < level_left + 50) temp->body->p.x = level_right - 50;
	if (temp->body->p.x > level_right - 50) temp->body->p.x = level_left + 50;


	/*
	cpVect cpBodyGetVel(const cpBody *body)
	void cpBodySetVel(cpBody *body, const cpVect value)
	cpVect cpvforangle(const cpFloat a)  Returns the unit length vector for the given angle (in radians).
	cpVect cpvrotate(const cpVect v1, const cpVect v2) Uses complex multiplication to rotate v1 by v2. Scaling will occur if v1 is not a unit vector.
	*/

	/* units/sec */
	cpFloat rotSpeed = 5.0;
	cpFloat accel = 50000;

	cpVect dirUp = cpvforangle(-rotSpeed*dt);
	cpVect dirDown = cpvforangle(rotSpeed*dt);

	cpFloat cspeed = cpvlength(cpBodyGetVel(temp->body));

	/* Player movement */
	if(keys[SDLK_w] && cspeed > 100) {
		cpBodySetVel(temp->body, cpvrotate(cpBodyGetVel(temp->body),dirUp));
	}

	if(keys[SDLK_s] && cspeed > 100) {
		cpBodySetVel(temp->body, cpvrotate(cpBodyGetVel(temp->body),dirDown));
	}

	cpBodySetAngle(temp->body, cpvtoangle(cpBodyGetVel(temp->body)));
	cpSpaceReindexShapesForBody(space, temp->body);


	if(keys[SDLK_d]) cpBodyApplyForce(temp->body,cpvmult(cpBodyGetRot(temp->body),accel*dt),cpvzero);
	if(keys[SDLK_a]) cpBodyApplyForce(temp->body,cpvmult(cpBodyGetRot(temp->body),-accel*dt),cpvzero);
	
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
		temp->body->p.x=0;
		temp->body->p.y=500;
	}

	if (keys[SDLK_h]) {
		cpBodySetVelLimit(temp->body,5000);
		cpBodySetAngVelLimit(temp->body,2);
		cpBodySetVel(temp->body, cpvzero);
		cpBodySetAngVel(temp->body, 0);
	}
	
	if (keys[SDLK_SPACE]) {
		tmp_shoot(obj);
	}
	
	if (keys[SDLK_x]) {
		particles_add_explosion(cpBodyGetPos(temp->body),0.5f,3000, 40,200);
	}
}


static void tmp_shoot(object *obj)
{
	temp = (struct player*)obj;
	//TMP shooting settings
	static const float cooldown = 0.1f;
	
	if (timer < cooldown) {
		return;
	}
	timer = 0;
	bullet_init(temp->body->p,cpBodyGetRot(temp->body),1);
}

static void player_destroy(object *obj)
{
	*obj->remove = 1;
	//free(obj);
}
