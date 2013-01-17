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

// camera settings
static int cam_relative = 1;

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

	if (keys[SDLK_h]) {
		cpBodySetVelLimit(player,5000);
		cpBodySetAngVelLimit(player,2);
		cpBodySetVel(player, cpvzero);
		cpBodySetAngVel(player, 0);
	}
	
	if (keys[SDLK_SPACE]) {
		tmp_shoot();
		//keys[SDLK_SPACE] = 0;
	}
	if (keys[SDLK_x]) {
		paricles_add_explosion(cpBodyGetPos(player), 10);
	paricles_add_explosion(cpBodyGetPos(player), 10);
	paricles_add_explosion(cpBodyGetPos(player), 10);
	paricles_add_explosion(cpBodyGetPos(player), 10);
	paricles_add_explosion(cpBodyGetPos(player), 10);
		//keys[SDLK_SPACE] = 0;
	}
	
	while(accumulator >= phys_step)
	{
		cpSpaceStep(space, phys_step);
		accumulator -= phys_step;
	}
	
}
static GLfloat testa[12] = {0.5,-0.5,	0.5,0.5,	-0.5,0.5,	-0.5,	-0.5,	-0.5,0	,0.5,0}; //A
void SPACE_draw(float dt) 
{
	//camera zoom
	glScalef(cam_zoom,cam_zoom,cam_zoom);
	static float r;
	if (cam_relative){
		
	}else {
		glRotatef(-r * MATH_180PI  + 90,0,0,1);
	}
	glTranslatef(-player->p.x, -player->p.y, 0.0f);
	drawStars();
	draw_space(space);

	paricles_draw(dt);

	
	glLoadIdentity();
	
	//draw GUI
	glColor3f(cos((player->p.x/50)),sin((player->p.y/100)),player->p.x/2550.0f*player->p.y/2550.0f);
	//draw_line_strip(testa, 12, 16);

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
	/*
	 //Font performance test
	 for (i = 1; i < 100; i++) {
	 setTextAngleRad(dir*i*1.618033988749895f/100.0f);
	 setTextSize(s);
	 drawText(circle->tc.x,circle->tc.y, "THE QUICK BROWN FOX\nJUMPED OVER THE LAZY DOG\n0123456789");
	 s=(10*s+s*1.618033988749895f)/11;
	 }
	 */
}

#define SW (4000)
static void drawStars()
{
	glColor3f(1,1,1);
	
	glPushMatrix();
	glTranslatef(((int)player->p.x+SW/2) / SW * SW,((int)player->p.y+SW/2) / SW * SW,0);
	
	glBegin(GL_QUADS);
	for (i=0;i<star_count;i++) {
		float size = stars_size[i];
		glVertex2f(stars_x[i]-size,stars_y[i]-size);
		glVertex2f(stars_x[i]+size,stars_y[i]-size);
		glVertex2f(stars_x[i]+size,stars_y[i]+size);
		glVertex2f(stars_x[i]-size,stars_y[i]+size);
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
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-2000,-2000), cpv(2000,-2000), 10.0f));
	cpShapeSetUserData(shape, draw_segmentshape);
	cpShapeSetElasticity(shape, 1.0f);
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-2000,2000), cpv(2000,2000), 10.0f));
	cpShapeSetUserData(shape, draw_segmentshape);
	cpShapeSetElasticity(shape, 1.0f);
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-2000,-2000), cpv(-2000,2000), 10.0f));
	cpShapeSetUserData(shape, draw_segmentshape);
	cpShapeSetElasticity(shape, 1.0f);
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(2000,-2000), cpv(2000,2000), 10.0f));
	cpShapeSetUserData(shape, draw_segmentshape);
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

static void tmp_shoot() {
	cpFloat radius = 10;
	cpFloat mass = 1;
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	cpVect prot = cpBodyGetRot(player);
	
	cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(ballBody, cpvadd(cpv(player->p.x, player->p.y), cpvmult(prot,radius+radius)));
	
	cpBodySetVel(ballBody,cpvmult(prot,1000));
	
	cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
	cpShapeSetFriction(ballShape, 0.7);
	cpShapeSetUserData(ballShape, draw_ballshape);
}

