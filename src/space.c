#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"

#define star_count 1000
static int stars_x[star_count];
static int stars_y[star_count];
static float stars_size[star_count];

static void player_draw(cpShape *shape);
static void drawStars();
static void tmp_shoot();
static float accumulator = 0;


/* helper */
static int i,j;

// Chipmunk
static cpFloat phys_step = 1/60.0f;
static cpSpace *space;
static cpBody *player;
static float x,y,r;

/* camera settings */
static int cam_relative = 1;
static float cam_dx = 0;
static float cam_dy = 0;
float cam_center_x = 0;
float cam_center_y = 0;
float cam_zoom = 1;

struct state spaceState = {
	SPACE_draw,
	SPACE_update,
	NULL
};

void SPACE_update(float dt)
{
	accumulator += dt;
	
	cpFloat pangvel = cpBodyGetAngVel(player);
	cpBodySetAngVel(player, pangvel*0.9);
	cpVect pvel =  cpBodyGetVel(player);
	
	//update physics and player
	cpVect rot = cpBodyGetRot(player);
	rot = cpvmult(rot, 10000);
	cpBodySetForce(player, cpv(0,0));
	cpBodySetTorque(player, 0);
	if (player->p.x <= -7950) player->p.x += 7950*2; //tmp wraparound	
	if (player->p.x >= 7950) player->p.x -= 7950*2;
	
	if(keys[SDLK_w]) cpBodySetForce(player, rot);
	if(keys[SDLK_s]) cpBodySetForce(player, cpvneg(rot));
	if(keys[SDLK_d]) cpBodySetTorque(player, -5000);
	if(keys[SDLK_a]) cpBodySetTorque(player, 5000);
	
	static int F1_pushed = 0;
	if(keys[SDLK_F1]){
		if (!F1_pushed) { 
			F1_pushed = 1;
			cam_relative = !cam_relative;
		}
	} else F1_pushed = 0;
	
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
		player->p.x=0;
		player->p.y=1000; 
	}

	if (keys[SDLK_h]) {
		cpBodySetVelLimit(player,5000);
		cpBodySetAngVelLimit(player,2);
		cpBodySetVel(player, cpvzero);
		cpBodySetAngVel(player, 0);
	}
	
	if (keys[SDLK_SPACE]) {
		tmp_shoot();
		keys[SDLK_SPACE] = 0;
	}
	
	if (keys[SDLK_x]) {
		paricles_add_explosion(cpBodyGetPos(player), 40);
	}
	
	while(accumulator >= phys_step)
	{
		cpSpaceStep(space, phys_step);
		accumulator -= phys_step;
	}
	
}

void SPACE_draw(float dt) 
{
	//TODO to make dynamic camera zoom and pos depend on velocity (e.g. higher velocity -> less delay)
	
	/* dynamic camera zoom */
	static const float zoom_delay = 0.9f; // 1.0 = manual zoom, 0.0 = no delay, <0 = oscillerende zoom, >1 = undefined, default = 0.9
	static const float zoom_slow_grow = 0.1f; // bigger = slower outer zoom growth, deafult = 0.1f
	static const float zoom_slow_shrink = 0.5f; // bigger = slower inner zoom growth, default = 0.5f
	cam_zoom = (1.0f * (HEIGHT / 2 + player->p.y * zoom_slow_grow) / (player->p.y+(HEIGHT * zoom_slow_shrink)) ) * (1-zoom_delay) + cam_zoom * zoom_delay;
	
	/* dynamic camera pos */
	static const float pos_delay = 0.9f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
	static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
	static const float pos_rel_y = 0.1f; // default = 0.1
	static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
	static const float pos_rel_offset_y = 0.2f; // default = 0.2
	cam_dx = cam_dx * pos_delay + ((player->rot.x * pos_rel_x - pos_rel_offset_x) * WIDTH) * (1 - pos_delay) / cam_zoom;
	cam_dy = cam_dy * pos_delay + ((player->rot.y * pos_rel_y - pos_rel_offset_y) * HEIGHT) * (1 - pos_delay) / cam_zoom;
	
	cam_center_x = player->p.x + cam_dx;
	cam_center_y = player->p.y + cam_dy;
	
	/* camera rotation */
	if (!cam_relative) glRotatef(-cpBodyGetAngle(player) * MATH_180PI,0,0,1);
	
	/* draw background */
	drawStars();
	
	/* translate view */
	glScalef(cam_zoom,cam_zoom,1);
	glTranslatef(-cam_center_x, -cam_center_y, 0.0f);
	
	/* draw all objects */
	draw_space(space);

	/* draw particle effects */
	paricles_draw(dt);
	
	/* something */
	glLoadIdentity();
	
	/* draw GUI */
	glColor3f(cos((player->p.x/50)),sin((player->p.y/100)),player->p.x/2550.0f*player->p.y/2550.0f);
	
	setTextAngle(0);
	setTextSize(80);
	setTextAlign(TEXT_CENTER);
	font_drawText(0,0.8f*HEIGHT/2, "SPACE");
	
	setTextAlign(TEXT_LEFT);
	setTextSize(10);

	glColor3f(1,1,1);
	font_drawText(-WIDTH/2+15,HEIGHT/2 - 10,"WASD     MOVE\nQE       ZOOM\nSPACE   SHOOT\nH        STOP\nESCAPE   QUIT");
	
	setTextAlign(TEXT_RIGHT);
	font_drawText(WIDTH/2 - 15, HEIGHT/2 - 10, fps_buf);
}


static void player_draw(cpShape *shape)
{
	cpCircleShape *circle = (cpCircleShape *)shape;
	draw_circle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), 15,cam_zoom, RGBAColor(0.95f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
	//draw_ballshape(shape);
	float s = 0.001;
	float dir = cpBodyGetAngle(cpShapeGetBody(shape));
	setTextAlign(TEXT_LEFT); // \n is currently only supported by left aligned text
	setTextSize(8);
	setTextAngleRad(dir);
	font_drawText(circle->tc.x,circle->tc.y, "-THE QUICK BROWN FOX\n+JUMPS OVER\nTHE LAZY DOG\n0123456789.");
}

#define SW (8000)
static void drawStars()
{
	glPushMatrix();
	
	glScalef(0.8f * cam_zoom,0.8f * cam_zoom, 1);
	glTranslatef(-cam_center_x*0.5,-cam_center_y*0.5,0);
	
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	for (i=0;i<star_count;i++) {
		float size = stars_size[i];
		float star_x = (stars_x[i]);
		float star_y = (stars_y[i]);
		glVertex2f(star_x - size, star_y - size);
		glVertex2f(star_x + size, star_y - size);
		glVertex2f(star_x + size, star_y + size);
		glVertex2f(star_x - size, star_y + size);
	}
	glEnd();
	
	glPopMatrix();
}

void SPACE_init(){
	cpVect gravity = cpv(0, -200);
	
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	cpSpaceSetDamping(space, 0.999);
	//init stars
	srand(122531);
	for (i=0;i<star_count;i++) {
		stars_x[i] = rand()%(SW*2) - SW;
		stars_y[i] = rand()%(SW*2) - SW;
		stars_size[i] = 2 + 4*(rand() % 1000) / 1000.0f;
	}
	
	/* static ground */
	cpBody  *staticBody = space->staticBody;
	cpShape *shape;
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-10000,0), cpv(10000,0), 10)); // ground level at 0
	cpShapeSetUserData(shape, draw_segmentshape);
	cpShapeSetElasticity(shape, 0.4f);
	cpShapeSetFriction(shape, 0.4f);
	// sets collision type to 1
	cpShapeSetCollisionType(shape, 1);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-8000,8000), cpv(8000,8000), 10.0f));
	cpShapeSetUserData(shape, draw_segmentshape);
	cpShapeSetElasticity(shape, 1.0f);
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-8000,0), cpv(-8000,8000), 10.0f));
	cpShapeSetUserData(shape, NULL);
	cpShapeSetElasticity(shape, 1.0f);
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(8000,0), cpv(8000,8000), 10.0f));
	cpShapeSetUserData(shape, NULL);
	cpShapeSetElasticity(shape, 1.0f);
	cpFloat radius = 40;
	cpFloat mass = 1;
	
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	//player
	player = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForCircle(2, 0, 15, cpvzero)));
	cpBodySetPos(player, cpv(30*20,10+1*30));
	cpBodySetVelLimit(player,1500);
	
	cpShape *playerShape = cpSpaceAddShape(space, cpCircleShapeNew(player, 15, cpvzero));
	cpShapeSetFriction(playerShape, 0.7);
	cpShapeSetUserData(playerShape, player_draw);
	cpShapeSetElasticity(playerShape, 1.0f);
	
	for(i = 1; i<5; i++){
		for(j = 1; j<5; j++){
			cpBody *boxBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(1.0f, 30.0f, 30.0f)));
			cpBodySetPos(boxBody, cpv(j*42, i*42));
			cpShape *boxShape = cpSpaceAddShape(space, cpBoxShapeNew(boxBody, radius, radius));
			cpShapeSetFriction(boxShape, 0.7);
			cpShapeSetUserData(boxShape,  draw_boxshape);
		}
	}
}

void SPACE_destroy()
{
	cpSpaceDestroy(space);
}



// from chipmunk docs
// removes the object from the space
static void
postStepRemove(cpSpace *space, cpShape *shape, void *unused)
{
  cpSpaceRemoveBody(space, shape->body);
  cpBodyFree(shape->body);
  
  cpSpaceRemoveShape(space, shape);
  cpShapeFree(shape);
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


static void tmp_shoot()
{
	cpFloat radius = 10;
	cpFloat mass = 1;
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	cpVect prot = cpBodyGetRot(player);
	
	cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(ballBody, cpvadd(cpv(player->p.x, player->p.y), cpvmult(prot,radius+radius)));
	
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


