/* header */
#include "space.h"

/* standard c-libraries */
#include <stdio.h>

/* SDL */
#include "SDL_opengl.h"

/* Game state */
#include "main.h"
#include "menu.h"

/* Drawing */
#include "draw.h"
#include "font.h"
#include "particles.h"

/* Game components */
#include "objects.h"
#include "player.h"
#include "tankfactory.h"
#include "level.h"

#define star_count 1000
static int stars_x[star_count];
static int stars_y[star_count];
static float stars_size[star_count];
static void drawStars();
static float accumulator = 0;

/* state functions */
static void SPACE_init();
static void SPACE_update();
static void space_render();
static void SPACE_destroy();

static void SPACE_draw();
static void update_objects(object *obj);
static void render_objects(object *obj);

/* helper */
int i,j;
static int second_draw = 0;

// Chipmunk
static cpFloat phys_step = 1/60.0f;
cpSpace *space;

/* camera settings */
static int cam_mode = 6;
float cam_center_x = 0;
float cam_center_y = 0;
float cam_zoom = 1;

static int cam_left;
static int cam_right;



/* level data */
level *currentlvl;

state state_space = {
	SPACE_init,
	SPACE_update,
	space_render,
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
	}else if(keys[SDLK_F6]){
		cam_mode = 6;
	}

	if(keys[SDLK_ESCAPE]){
		state_menu.parentState = &state_space;
		currentState = &state_menu;
		keys[SDLK_ESCAPE] = 0;
	}

	objects_iterate(update_objects);

	particles_update(dt);
	while(accumulator >= phys_step)
		{
			cpSpaceStep(space, phys_step);
			accumulator -= phys_step;
		}
}

static void update_objects(object *obj)
{
	if(obj->alive){
		if (obj->body->p.x < currentlvl->left ) obj->body->p.x = currentlvl->right - abs(currentlvl->left -obj->body->p.x );
		if (obj->body->p.x > currentlvl->right) obj->body->p.x = currentlvl->left + (obj->body->p.x - currentlvl->right) ;
		obj->type->update(obj);
	}else{
		obj->type->destroy(obj);
		*(obj->remove) = 1;
	}
}

static void space_render()
{
	/* dynamic camera zoom */
	float py = player->body->p.y / currentlvl->height;
	float scrlvl, zoomlvl;
	switch (cam_mode) {
	case 1:
	case 2: /* fanzy zoom camera */

		scrlvl = 1.0f * HEIGHT/currentlvl->height;

		zoomlvl = cam_mode == 1 ? 4 : 12;
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
			cam_center_y = currentlvl->height / (2);
		} else if (py < 0.8) {
			cam_zoom = (1 - cos(5*M_PI * (py - 0.4 + 1))) / zoomlvl + scrlvl;
			cam_center_y = currentlvl->height - HEIGHT / (2*(cam_zoom));
		} else if (py <= 1.0) {
			cam_zoom = 2 / zoomlvl + scrlvl;
			cam_center_y = currentlvl->height - HEIGHT / (2*cam_zoom);
		} else {
			/* undefined zoom! Reset/fix player position? */
		}
		break;

	case 3:
	case 4:/* simple zoomed camera */
		if(cam_mode == 3){
			cam_zoom = 2;
		}else{
			cam_zoom = 1.3;
		}
		cam_center_y = player->body->p.y;
		if(cam_center_y > currentlvl->height - HEIGHT/(2*cam_zoom)){
			cam_center_y = currentlvl->height - HEIGHT/(2*cam_zoom);
		}else if(cam_center_y <  HEIGHT/(2*cam_zoom)){
			cam_center_y = HEIGHT/(2*cam_zoom);
		}
		break;
	case 5:
		cam_zoom = 1.0f*HEIGHT/currentlvl->height;
		cam_center_y = 1.0f*currentlvl->height/2;
		break;
	case 6:
		scrlvl = 1.0f * HEIGHT/currentlvl->height;
		/* parameters to change */
		zoomlvl = 4; /* amount of zoom less is more zoom */
		float startlvl = 0.8;
		float endlvl = 0.2;

		float freq = startlvl-endlvl;
		if (py < 0) {
			/* undefined zoom! Reset/fix player position? */
		} else if ( py < endlvl) {
			cam_zoom = 2 / zoomlvl + scrlvl;
			cam_center_y = HEIGHT / (2*cam_zoom);
		} else if (py < startlvl) {
			cam_zoom = (1 - cos( (1/freq)*M_PI*(py + (freq-endlvl) ))) / zoomlvl + scrlvl;
			cam_center_y = HEIGHT / (2*cam_zoom);
		} else if (py < 1) {
			cam_zoom = scrlvl;
			cam_center_y = currentlvl->height / (2);
		}else{
			/* undefined zoom! Reset/fix player position? */
		}
		break;
	default:
		cam_zoom = 1.0f*HEIGHT/currentlvl->height;
		cam_center_y = 1.0f*currentlvl->height/2;
		break;
	}

	/* dynamic camera pos */
	static const float pos_delay = 0.9f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
	static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
	static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
	static float cam_dx;
	cam_dx = cam_dx * pos_delay + ((player->body->rot.x * pos_rel_x - pos_rel_offset_x) * WIDTH) * (1 - pos_delay) / cam_zoom;

	cam_center_x = player->body->p.x + cam_dx;

	/* camera constraints */

	static float camera_width;
	camera_width = WIDTH / (2 * cam_zoom);

	static float cam_left_limit, cam_right_limit;

	cam_left_limit = currentlvl->left + camera_width;
	cam_right_limit = currentlvl->right - camera_width;

	cam_left = cam_center_x - camera_width;
	cam_right = cam_center_x + camera_width;

	SPACE_draw();

	if(cam_center_x < cam_left_limit){
		second_draw = 1;
		cam_center_x += currentlvl->right + abs(currentlvl->left) ;
		cam_left = cam_center_x - camera_width;
		cam_right = cam_center_x + camera_width;
		SPACE_draw();
	}else if(cam_center_x > cam_right_limit){
		second_draw = 1;
		cam_center_x -= currentlvl->right + abs(currentlvl->left);
		cam_left = cam_center_x - camera_width;
		cam_right = cam_center_x + camera_width;
		SPACE_draw();

	}

	second_draw = 0;

}

static void SPACE_draw()
{
	
	/* draw background */
	if(!second_draw){
		drawStars();
	}

	/* translate view */
	glLoadIdentity();
	glScalef(cam_zoom,cam_zoom,1);
	glTranslatef(-cam_center_x, -cam_center_y, 0.0f);
	

	/* draw all objects */
	setTextAngle(0);

	/* super slow
	 * TODO: make draw function for ground and roof
	 * */
	//draw_space(space);
	objects_iterate(render_objects);

	/* draw particle effects */
	particles_draw(dt);

	if(!second_draw){
		/* something */
		glLoadIdentity();

		/* draw GUI */
		setTextAngle(0); // TODO don't use global variables for setting font properties
		setTextAlign(TEXT_LEFT);
		setTextSize(10);

		glColor3f(1,1,1);
		font_drawText(-WIDTH/2+15,HEIGHT/2 - 10,"WASD     MOVE\nQE       ZOOM\nSPACE   SHOOT\nH        STOP\nESCAPE   QUIT");

		setTextAlign(TEXT_RIGHT);
		font_drawText(WIDTH/2 - 15, HEIGHT/2 - 10, fps_buf);
	}
	
}

static void render_objects(object *obj)
{
	if(obj->body->p.x > cam_left - 200 && obj->body->p.x < cam_right + 200){
		obj->type->render(obj);
	}
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

static void func(object* obj)
{
	obj->type->destroy(obj);
}


//TODO temp
struct tank_factory_param t = {5,200,3};
level templevel = {1,1,1200,-1000,1000};

void space_init_level(int space_station, int deck)
{
	objects_iterate(func);
	objects_destroy();

	objects_add((object*)player);

	if (currentlvl != NULL) {
		level_unload(currentlvl);
	}

	currentlvl = level_load(space_station,deck);

	if (currentlvl == NULL) {
		fprintf(stderr, "space_level_init failed!\n");
		exit(-1);
	}

	//currentlvl = &templevel;

		/* static ground */

		cpBody  *staticBody = space->staticBody;
		cpShape *shape;
		shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left,0), cpv(currentlvl->right,0), 10)); // ground level at 0
		cpShapeSetFriction(shape, 0.8f);
		cpShapeSetCollisionType(shape, ID_GROUND);

		shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left,currentlvl->height), cpv(currentlvl->right,currentlvl->height), 10.0f));
		cpShapeSetFriction(shape, 0.8f);
		cpShapeSetCollisionType(shape, ID_GROUND);

	/*
		tankfactory_init(500,&t);
		tankfactory_init(100,&t);
		tankfactory_init(-500,&t);
	*/
}

static void SPACE_init(){
	objects_init();
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
	
	player = ((struct player*)player_init());
	space_init_level(1,1);
}

static void SPACE_destroy()
{
	cpSpaceDestroy(space);
	objects_destroy();
}
