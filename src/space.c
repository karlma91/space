#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "player.h"


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
cpSpace *space;

static float x,y,r;

/* camera settings */
static int cam_relative = 1;
static float cam_dx = 0;
static float cam_dy = 0;
float cam_center_x = 0;
float cam_center_y = 0;
float cam_zoom = 1;

state spaceState = {
	SPACE_draw,
	SPACE_update,
	NULL
};

void SPACE_update(float dt)
{
	accumulator += dt;

	static int F1_pushed = 0;
	if(keys[SDLK_F1]){
		if (!F1_pushed) { 
			F1_pushed = 1;
			cam_relative = !cam_relative;
		}
	} else F1_pushed = 0;

	player.update(&player, dt);

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
	cam_zoom = (1.0f * (HEIGHT / 2 + player.body->p.y * zoom_slow_grow) / (player.body->p.y+(HEIGHT * zoom_slow_shrink)) ) * (1-zoom_delay) + cam_zoom * zoom_delay;
	
	/* dynamic camera pos */
	static const float pos_delay = 0.9f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
	static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
	static const float pos_rel_y = 0.1f; // default = 0.1
	static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
	static const float pos_rel_offset_y = 0.2f; // default = 0.2
	cam_dx = cam_dx * pos_delay + ((player.body->rot.x * pos_rel_x - pos_rel_offset_x) * WIDTH) * (1 - pos_delay) / cam_zoom;
	cam_dy = cam_dy * pos_delay + ((player.body->rot.y * pos_rel_y - pos_rel_offset_y) * HEIGHT) * (1 - pos_delay) / cam_zoom;
	
	cam_center_x = player.body->p.x + cam_dx;
	cam_center_y = player.body->p.y + cam_dy;
	
	/* camera rotation */
	if (!cam_relative) glRotatef(-cpBodyGetAngle(player.body) * MATH_180PI,0,0,1);
	
	/* draw background */
	drawStars();
	
	/* translate view */
	glScalef(cam_zoom,cam_zoom,1);
	glTranslatef(-cam_center_x, -cam_center_y, 0.0f);
	
	/* draw player */
	player.render(&player,dt);

	/* draw all objects */
	draw_space(space);

	/* draw particle effects */
	paricles_draw(dt);
	
	/* something */
	glLoadIdentity();
	
	/* draw GUI */
	setTextAngle(0);
	setTextSize(80);
	setTextAlign(TEXT_CENTER);
	glColor_from_color(draw_rainbow_col((int)((player.body->p.x+8000))));
	font_drawText(0,0.8f*HEIGHT/2, "SPACE");
	
	setTextAlign(TEXT_LEFT);
	setTextSize(10);

	glColor3f(1,1,1);
	font_drawText(-WIDTH/2+15,HEIGHT/2 - 10,"WASD     MOVE\nQE       ZOOM\nSPACE   SHOOT\nH        STOP\nESCAPE   QUIT");
	
	setTextAlign(TEXT_RIGHT);
	font_drawText(WIDTH/2 - 15, HEIGHT/2 - 10, fps_buf);
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


