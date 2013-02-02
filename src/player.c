#include "chipmunk.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "player.h"
#include "tank.h"
#include "list.h"

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

object *player_init()
{
	cpFloat radius = 10;
	//cpFloat mass = 1;

	struct player *pl = malloc(sizeof(struct player));

	pl->type = &type_player;
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



// from chipmunk docs
// removes the object from the space
static void
postStepRemove(cpSpace *space, cpShape *shape, void *unused)
{
	cpSpaceRemoveBody(space, shape->body);
	//cpBodyFree(shape->body);
  
	cpSpaceRemoveShape(space, shape);
	//cpShapeFree(shape);
}

// from chipmunk docs
static int
begin(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	particles_add_explosion(cpBodyGetPos(cpShapeGetBody(b)), 0.3f, 1200, 20,300);
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, b, NULL);
	return 0;
}


static void tmp_shoot(object *obj)
{
	temp = (struct player*)obj;
	//TMP shooting settings
	static const float cooldown = 0.1f;
	static float time = 0;
	
	if (time < cooldown) {
		time += dt;
		return;
	}
	
	time = 0;
	
	static const cpFloat radius = 5;
	static const cpFloat mass = 1;
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	cpVect prot = cpBodyGetRot(temp->body);
	
	cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(ballBody, cpvadd(cpv(temp->body->p.x, temp->body->p.y), cpvmult(prot,radius)));
	
	cpBodySetVel(ballBody,cpvmult(prot,1500));
	
	cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
	cpShapeSetFriction(ballShape, 0.7);
	cpShapeSetUserData(ballShape, draw_ballshape);
	// Sets bullets collision type to 2
	cpShapeSetCollisionType(ballShape, ID_PLAYER_BULLET);
	// runs callback begin when bullet (2) hits ground (1) 
	// this will make bullet b and ground a in begin callback
	cpSpaceAddCollisionHandler(space, ID_GROUND, ID_PLAYER_BULLET, begin, NULL, NULL, NULL, NULL);
	
}

static void player_destroy(object *obj)
{
	*obj->remove = 0;
	free(obj);
}

