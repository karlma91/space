#include "chipmunk.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "player.h"


static void player_init(object *obj);
static void player_render(object *obj, float dt);
static void player_update(object *obj, float dt);
static void player_destroy(object *obj);
static void tmp_shoot(object *obj);

object player = {
	NULL,
	NULL,
	player_init,
	player_update,
	player_render,
	player_destroy,
	NULL
};



static void player_init(object *obj)
{
	cpFloat radius = 40;
	//cpFloat mass = 1;
	/* make and add new body */
	obj->body = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForBox(1.0f, 30.0f, 30.0f)));
	cpBodySetPos(obj->body, cpv(30*20,10+1*30));
	cpBodySetVelLimit(obj->body,1500);
	/* make and connect new shape to body */
	obj->shape = cpSpaceAddShape(space, cpBoxShapeNew(obj->body, radius, radius));
	cpShapeSetFriction(obj->shape, 0.7);
	cpShapeSetUserData(obj->shape, draw_boxshape);
	cpShapeSetElasticity(obj->shape, 1.0f);
	
}

static void player_render(object *obj, float dt)
{
	//float s = 0.001;
	float dir = cpBodyGetAngle(obj->body);
	setTextAlign(TEXT_LEFT); // \n is currently only supported by left aligned text
	setTextSize(8);
	setTextAngleRad(dir);
	font_drawText(obj->body->p.x,obj->body->p.y, "-THE QUICK BROWN FOX\n+JUMPS OVER\nTHE LAZY DOG\n0123456789.");
}

static void player_update(object *obj, float dt)
{
	cpFloat pangvel = cpBodyGetAngVel(obj->body);
	cpBodySetAngVel(obj->body, pangvel*0.9);
	//cpVect pvel = cpBodyGetVel(obj->body);
	
	//update physics and player
	cpVect rot = cpBodyGetRot(obj->body);
	rot = cpvmult(rot, 10000);
	cpBodySetForce(obj->body, cpv(0,0));
	cpBodySetTorque(obj->body, 0);
	if (obj->body->p.x <= -7950) obj->body->p.x += 7950*2; //tmp wraparound	
	if (obj->body->p.x >= 7950) obj->body->p.x -= 7950*2;
	
	/* Player movement */
	if(keys[SDLK_w]) cpBodySetForce(obj->body, rot);
	if(keys[SDLK_s]) cpBodySetForce(obj->body, cpvneg(rot));
	if(keys[SDLK_d]) cpBodySetTorque(obj->body, -5000);
	if(keys[SDLK_a]) cpBodySetTorque(obj->body, 5000);
	
	if(keys[SDLK_g]){
		keys[SDLK_g] = 0;
		cpVect gravity = cpv(0, -2);
		cpSpaceSetGravity(space, gravity);
	}
	
	if (keys[SDLK_q]){
		cam_zoom /= dt+1.1f;
	}
	if (keys[SDLK_e]){
		cam_zoom *= dt+1.1f;
		if (keys[SDLK_q])
			cam_zoom = 1;  
	}
	if (keys[SDLK_r]){
		obj->body->p.x=0;
		obj->body->p.y=1000; 
	}

	if (keys[SDLK_h]) {
		cpBodySetVelLimit(obj->body,5000);
		cpBodySetAngVelLimit(obj->body,2);
		cpBodySetVel(obj->body, cpvzero);
		cpBodySetAngVel(obj->body, 0);
	}
	
	if (keys[SDLK_SPACE]) {
		tmp_shoot(obj);
		keys[SDLK_SPACE] = 0;
	}
	
	if (keys[SDLK_x]) {
		paricles_add_explosion(cpBodyGetPos(obj->body), 40);
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
	paricles_add_explosion(cpBodyGetPos(cpShapeGetBody(b)), 40);
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, b, NULL);
	return 0;
}


static void tmp_shoot(object *obj)
{
	cpFloat radius = 10;
	cpFloat mass = 1;
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	cpVect prot = cpBodyGetRot(obj->body);
	
	cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(ballBody, cpvadd(cpv(obj->body->p.x, obj->body->p.y), cpvmult(prot,radius+radius)));
	
	cpBodySetVel(ballBody,cpvmult(prot,2000));
	
	cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
	cpShapeSetFriction(ballShape, 0.7);
	cpShapeSetUserData(ballShape, draw_ballshape);
	// Sets bullets collision type to 2
	cpShapeSetCollisionType(ballShape, 2);
	// runs callback begin when bullet (2) hits ground (1) 
	// this will make bullet b and ground a in begin callback
	cpSpaceAddCollisionHandler(space, 1, 2, begin, NULL, NULL, NULL, NULL);
	
}

static void player_destroy(object *obj)
{

}

