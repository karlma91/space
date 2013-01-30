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
#include "menu.h"
#include "math.h"
#include "tankfactory.h"


#define star_count 10000
static int stars_x[star_count];
static int stars_y[star_count];
static float stars_size[star_count];
static void drawStars();
static float accumulator = 0;

/* state functions */
static void SPACE_init();
static void SPACE_update();
static void SPACE_draw();
static void SPACE_destroy();

static void update_objects(cpBody *body, void *unused);

/* helper */
static int i,j;

// Chipmunk
static cpFloat phys_step = 1/60.0f;
cpSpace *space;

/* camera settings */
static int cam_mode = 1;
static float cam_dx = 0;
static float cam_dy = 0;
float cam_center_x = 0;
float cam_center_y = 0;
float cam_zoom = 1;

/* level data */
static const int level_height = 1200;
static const int level_left = -2000;
static const int level_right = 2000;

state state_space = {
	SPACE_init,
	SPACE_update,
	SPACE_draw,
	SPACE_destroy,
	NULL
};

static void SPACE_update()
{
	accumulator += dt;
	if(keys[SDLK_F1]){
		cam_mode = 1;
	}else if(keys[SDLK_F2]){
		cam_mode = 2;
	}else if(keys[SDLK_F3]){
		cam_mode = 3;
	}else if(keys[SDLK_F4]){
		cam_mode = 4;
	}else if(keys[SDLK_F5]){
		cam_mode = 5;
	}

	if(keys[SDLK_ESCAPE]){
		state_menu.parentState = &state_space;
		currentState = &state_menu;
		keys[SDLK_ESCAPE] = 0;
	}

	cpSpaceEachBody(space, update_objects, NULL);

	player.update(&player);
	particles_update(dt);
	static int debug;
	while(accumulator >= phys_step)
		{
			cpSpaceStep(space, phys_step);
			accumulator -= phys_step;
		}
}

static void update_objects(cpBody *body, void *unused)
{
	if(body->data != NULL){
		object *obj = ((object*)body->data);
		obj->type->update(obj);
	}
}

static void SPACE_draw()
{
	//TODO to make dynamic camera zoom and pos depend on velocity (e.g. higher velocity -> less delay)

	/* dynamic camera zoom */
	float py = player.body->p.y / level_height;
	if(cam_mode == 1 || cam_mode == 2){ /* fanzy zoom camera */

		float scrlvl = 1.0f * HEIGHT/level_height;

		float zoomlvl = cam_mode == 1 ? 4 : 12;
		if (py < 0) {
			/* undefined zoom! Reset/fix player position? */
		} else if ( py < 0.2) {
			cam_zoom = 2 / zoomlvl + scrlvl;
			cam_center_y = HEIGHT / (2*cam_zoom);
		} else if (py < 0.4) {
			cam_zoom = (1 + cos(5*M_PI * (py + 1))) / zoomlvl + scrlvl;
			cam_center_y = HEIGHT / (2*cam_zoom);
		} else if (py < 0.6) {
			cam_zoom = scrlvl;
			cam_center_y = level_height / (2);
		} else if (py < 0.8) {
			cam_zoom = (1 - cos(5*M_PI * (py - 0.4 + 1))) / zoomlvl + scrlvl;
			cam_center_y = level_height - HEIGHT / (2*(cam_zoom));
		} else if (py <= 1.0) {
			cam_zoom = 2 / zoomlvl + scrlvl;
			cam_center_y = level_height - HEIGHT / (2*cam_zoom);
		} else {
			/* undefined zoom! Reset/fix player position? */
		}
	}else if(cam_mode == 3 || cam_mode == 4){ /* simple zoomed camera */
		if(cam_mode == 3){
			cam_zoom = 2;
		}else{
			cam_zoom = 1.3;
		}
		cam_center_y = player.body->p.y;
		if(cam_center_y > level_height - HEIGHT/(2*cam_zoom)){
			cam_center_y = level_height - HEIGHT/(2*cam_zoom);
		}else if(cam_center_y <  HEIGHT/(2*cam_zoom)){
			cam_center_y = HEIGHT/(2*cam_zoom);
		}

	}else if(cam_mode == 5){
		cam_zoom = 1.0f*HEIGHT/level_height;
		cam_center_y = 1.0f*level_height/2;
	}else{ /* whole map camera */
		cam_zoom = 1.0f*HEIGHT/level_height;
		cam_center_y = 1.0f*level_height/2;
	}
	
	/* dynamic camera pos */
	static const float pos_delay = 0.9f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
	static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
	static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
	cam_dx = cam_dx * pos_delay + ((player.body->rot.x * pos_rel_x - pos_rel_offset_x) * WIDTH) * (1 - pos_delay) / cam_zoom;

	cam_center_x = player.body->p.x + cam_dx;

	/* camera constraints */
	static float cam_left_limit, cam_right_limit;
	cam_left_limit = level_left + WIDTH / (2 * cam_zoom);
	cam_right_limit = level_right - WIDTH / (2 * cam_zoom);
	if (cam_center_x < cam_left_limit) cam_center_x = cam_left_limit;
	if (cam_center_x > cam_right_limit) cam_center_x = cam_right_limit;

	/* camera rotation */
	//if (!cam_relative) glRotatef(-cpBodyGetAngle(player.body) * MATH_180PI,0,0,1);
	
	/* draw background */
	drawStars();
	
	/* translate view */
	glScalef(cam_zoom,cam_zoom,1);
	glTranslatef(-cam_center_x, -cam_center_y, 0.0f);
	
	/* draw player */
	player.render(&player);

	/* draw all objects */
	draw_space(space);

	/* draw particle effects */
	particles_draw(dt);
	
	/* something */
	glLoadIdentity();
	
	/* draw GUI */
	setTextAngle(0); // TODO don't use global variables for setting font properties
	setTextAlign(TEXT_LEFT);
	setTextSize(10);

	static float pptest = 1.0;
	pptest -= rand()%(100) / 10000.0f;
	if (pptest < 0) pptest = 1;
	draw_hp(-400, 100, 80, 10, pptest);

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

static void SPACE_init(){
	cpVect gravity = cpv(0, -200);
	
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	//cpSpaceSetDamping(space, 0.999);
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
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(level_left,0), cpv(level_right,0), 10)); // ground level at 0
	cpShapeSetUserData(shape, draw_segmentshape);
	cpShapeSetElasticity(shape, 0.2f);
	cpShapeSetFriction(shape, 0.8f);
	// sets collision type to 1
	cpShapeSetCollisionType(shape, 1);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(level_left,level_height), cpv(level_right,level_height), 10.0f));
	cpShapeSetUserData(shape, draw_segmentshape);
	cpShapeSetElasticity(shape, 0.2f);
	cpShapeSetFriction(shape, 0.8f);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(level_left,0), cpv(level_left,level_height), 10.0f));
	cpShapeSetUserData(shape, NULL);
	cpShapeSetElasticity(shape, 1.0f);
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(level_right,level_height), cpv(level_right,level_height), 10.0f));
	cpShapeSetUserData(shape, NULL);
	cpShapeSetElasticity(shape, 1.0f);

	cpFloat boxSize = 10.0f;
	cpFloat mass = 1.0f;
	
	player.init(&player);
	
	object *tankFac = malloc(sizeof(object));
	tankfactory_init(tankFac,500,10,100);

	//cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	
	for(i = 1; i<5; i++){
		for(j = 1; j<5; j++){
			cpBody *boxBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, boxSize, boxSize)));
			cpBodySetPos(boxBody, cpv(j*(100), i*(boxSize)));
			cpShape *boxShape = cpSpaceAddShape(space, cpBoxShapeNew(boxBody, boxSize, boxSize));
			cpShapeSetFriction(boxShape, 0.7);
			cpShapeSetUserData(boxShape,  draw_boxshape);
		}
	}
}

static void SPACE_destroy()
{
	cpSpaceDestroy(space);
}


