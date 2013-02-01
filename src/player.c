#include "chipmunk.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "player.h"

static void player_init(struct player *obj);
static void player_render(struct player *obj);
static void player_update(struct player *obj);
static void player_destroy(struct player *obj);
static void tmp_shoot(struct player *obj);

struct player player = {
	NULL,
	NULL,
	player_init,
	player_update,
	player_render,
	player_destroy,
	NULL
};



static void player_init(struct player *obj)
{
	cpFloat radius = 10;
	//cpFloat mass = 1;
	/* make and add new body */
	obj->body = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForBox(1.0f, radius, radius)));
	cpBodySetPos(obj->body, cpv(0,990));
	cpBodySetVelLimit(obj->body,700);
	/* make and connect new shape to body */
	obj->shape = cpSpaceAddShape(space, cpBoxShapeNew(obj->body, radius, radius));
	cpShapeSetFriction(obj->shape, 0.7);
	cpShapeSetUserData(obj->shape, draw_boxshape);
	cpShapeSetElasticity(obj->shape, 1.0f);
	
}

static void player_render(struct player *obj)
{
	//float s = 0.001;
	float dir = cpBodyGetAngle(obj->body);
	setTextAlign(TEXT_CENTER);
	setTextSize(10);
	setTextAngleRad(dir);
	static char text[20];
	sprintf(text, " SPEED: %.3f",cpvlength(cpBodyGetVel(obj->body)));
	font_drawText(obj->body->p.x,obj->body->p.y, text);
}

static void player_update(struct player *obj)
{
	cpFloat pangvel = cpBodyGetAngVel(obj->body);
	cpBodySetAngVel(obj->body, pangvel*0.9);
	//cpVect pvel = cpBodyGetVel(obj->body);
	
	//update physics and player
	cpVect rot = cpBodyGetRot(obj->body);
	rot = cpvmult(rot, 10000);
	cpBodySetForce(obj->body, cpv(0,0));
	cpBodySetTorque(obj->body, 0);
	if (obj->body->p.x < level_left + 50) obj->body->p.x = level_right - 50;
	if (obj->body->p.x > level_right - 50) obj->body->p.x = level_left + 50;


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

	cpFloat cspeed = cpvlength(cpBodyGetVel(obj->body));

	/* Player movement */
	if(keys[SDLK_w] && cspeed > 100) {
		cpBodySetVel(obj->body, cpvrotate(cpBodyGetVel(obj->body),dirUp));
	}

	if(keys[SDLK_s] && cspeed > 100) {
		cpBodySetVel(obj->body, cpvrotate(cpBodyGetVel(obj->body),dirDown));
	}

	cpBodySetAngle(obj->body, cpvtoangle(cpBodyGetVel(obj->body)));
	cpSpaceReindexShapesForBody(space, obj->body);


	//TODO: gjøre svinghastighet avhengig av dt
	if(keys[SDLK_d]) cpBodyApplyForce(obj->body,cpvmult(cpBodyGetRot(obj->body),accel*dt),cpvzero);
	if(keys[SDLK_a]) cpBodyApplyForce(obj->body,cpvmult(cpBodyGetRot(obj->body),-accel*dt),cpvzero);
	
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
		obj->body->p.x=0;
		obj->body->p.y=500;
	}

	if (keys[SDLK_h]) {
		cpBodySetVelLimit(obj->body,5000);
		cpBodySetAngVelLimit(obj->body,2);
		cpBodySetVel(obj->body, cpvzero);
		cpBodySetAngVel(obj->body, 0);
	}
	if(keys[SDLK_y]){
		tank_init(200,100);
		keys[SDLK_y] = 0;
	}
	
	if (keys[SDLK_SPACE]) {
		tmp_shoot(obj);
	}
	
	if (keys[SDLK_x]) {
		particles_add_explosion(cpBodyGetPos(obj->body),0.5f,3000, 40,200);
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


static void tmp_shoot(struct player *obj) //TODO change dt to global
{
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
	cpVect prot = cpBodyGetRot(obj->body);
	
	cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(ballBody, cpvadd(cpv(obj->body->p.x, obj->body->p.y), cpvmult(prot,radius)));
	
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

static void player_destroy(struct player *obj)
{

}

