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
#include "robotarm.h"
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
/**
 * The global space state
 */
state state_space = {
	SPACE_init,
	SPACE_update,
	space_render,
	SPACE_destroy,
	NULL
};

static void SPACE_draw();
static void update_objects(object *obj);
static void render_objects(object *obj);
static void update_camera_zoom(int cam_mode);
static void update_camera_position();

static int second_draw = 0;

static float game_time;

// Chipmunk
static cpFloat phys_step = 1/60.0f;
/* extern */
cpSpace *space;

/* camera settings */
float cam_center_x = 0;
float cam_center_y = 0;
float cam_zoom = 1;

static int cam_left;
static int cam_right;

static float camera_width;
static float cam_left_limit;
static float cam_right_limit;


/* level data */
level *currentlvl;

/*
 * The ingame states for level transition and delays
 */
enum game_state{
	LEVEL_START,
	LEVEL_RUNNING,
	LEVEL_TIMESUP,
	LEVEL_PLAYER_DEAD,
	LEVEL_CLEARED,
	LEVEL_TRANSITION,

	LEVEL_STATE_COUNT
};

char *game_state_names[] = {
	"LEVEL_START",
	"LEVEL_RUNNING",
	"LEVEL_TIMESUP",
	"LEVEL_PLAYER_DEAD",
	"LEVEL_CLEARED",
	"LEVEL_TRANSITION"
};

/*
 * the current state of the game
 */
enum game_state gamestate = LEVEL_START;

static void level_start();
static void level_running();
static void level_timesup();
static void level_player_dead();
static void level_cleared();
static void level_transition();
static void change_state(int state);
static void update_all();

void (*state_functions[])(void) = {
		level_start,
		level_running,
		level_timesup,
		level_player_dead,
		level_cleared,
		level_transition
};


/* The state timer */
static float state_timer = 0;
/**
 * Inner state functions
 */

static void level_start()
{
	game_time = 0;
	if(state_timer > 1.5){
		change_state(LEVEL_RUNNING);
	}
}
static void level_running()
{
	/* update game time */
	game_time += dt;

	update_all();
	struct player *player = (struct player*)objects_first(ID_PLAYER);
	if(player->hp <= 0){
		player->disable = 1;
		change_state(LEVEL_PLAYER_DEAD);
	}
	if(objects_count(ID_TANK_FACTORY) == 0){
		change_state(LEVEL_CLEARED);
	}

	if (game_time >= currentlvl->timelimit) {
		change_state(LEVEL_TIMESUP);
	}
}
static void level_timesup()
{
	update_all();
	struct player *player = (struct player*)objects_first(ID_PLAYER);
	player->hp = 0;
	player->disable = 1;
	if (state_timer > 3) {
		change_state(LEVEL_PLAYER_DEAD);
	}
}
static void level_player_dead()
{
	update_all();
	if(state_timer > 3){
		change_state(LEVEL_TRANSITION);
	}
}
static void level_cleared()
{
	update_all();
	if(state_timer > 3){
		change_state(LEVEL_TRANSITION);
	}
}
static void level_transition()
{

	if(state_timer > 1){
		particles_removeall();
		space_init_level(1,1);
		/* update objects to move shapes to same position as body */
		update_all();
		change_state(LEVEL_START);
	}
}

/**
 * Resets timer and changes state
 */
static void change_state(int state)
{
	state_timer = 0;
	gamestate = state;
	fprintf(stderr,"DEBUG: entering state[%d]: %s\n",state,game_state_names[state]);
}

/**
 * Main space update function
 */
static void SPACE_update()
{
	/*
	 * Camera modes
	 */
	static int cam_mode = 6;
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

	/*
	 * Opens the pause menu
	 */
	if(keys[SDLK_ESCAPE]){
		state_menu.parentState = &state_space;
		currentState = &state_menu;
		keys[SDLK_ESCAPE] = 0;
	}

	/* runs the current state */
	state_timer+=dt; /* updates the timer */
	state_functions[gamestate]();

	/* Updating camera zoom an position */
	update_camera_zoom(cam_mode);
	update_camera_position();
}

/**
 * Updates all the objects in objects and in chipmunk
 */
static void update_all()
{
	/* chipmunk timestep counter */
	accumulator += dt;

	/*
	 * Calls update_opbject for all objects in objects
	 */
	objects_iterate(update_objects);

	/*
	 * update the particle engine
	 */
	particles_update(dt);


	/*
	 * update all chipmunk objects 60 times per second
	 */
	while(accumulator >= phys_step)
	{
		cpSpaceStep(space, phys_step);
		accumulator -= phys_step;
	}

}


/**
 * Used by object_iterate to update all objects
 */
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

/**
 * draws everything twice to make infinite loop world
 */
static void space_render()
{
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

static void update_camera_zoom(int cam_mode)
{
	struct player *player = ((struct player*)objects_first(ID_PLAYER));

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
}

static void update_camera_position()
{

	struct player *player = ((struct player*)objects_first(ID_PLAYER));
	/* dynamic camera pos */
	static const float pos_delay = 0.9f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
	static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
	static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
	static float cam_dx;
	cam_dx = cam_dx * pos_delay + ((player->body->rot.x * pos_rel_x - pos_rel_offset_x) * WIDTH) * (1 - pos_delay) / cam_zoom;

	cam_center_x = player->body->p.x + cam_dx;

	/* camera constraints */

	camera_width = WIDTH / (2 * cam_zoom);


	cam_left_limit = currentlvl->left + camera_width;
	cam_right_limit = currentlvl->right - camera_width;

	cam_left = cam_center_x - camera_width;
	cam_right = cam_center_x + camera_width;
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
		setTextSize(40);

		glColor3f(1,1,1);
		//font_drawText(-WIDTH/2+15,HEIGHT/2 - 10,"WASD     MOVE\nQE       ZOOM\nSPACE   SHOOT\nH        STOP\nESCAPE   QUIT");

		struct player *player = ((struct player*)objects_first(ID_PLAYER));

		char score_temp[20];
		sprintf(score_temp,"%d",player->highscore);
		font_drawText(-WIDTH/2+20,HEIGHT/2 - 45,score_temp);

		setTextSize(10);
		char pos_temp[20];
		sprintf(pos_temp,"X: %4.0f Y: %4.0f",player->body->p.x,player->body->p.y);
		font_drawText(-WIDTH/2+15,-HEIGHT/2+8,pos_temp);

		font_drawText(WIDTH/2-250,-HEIGHT/2+8,game_state_names[gamestate]);

		setTextAlign(TEXT_RIGHT);
		font_drawText(WIDTH/2 - 15, HEIGHT/2 - 10, fps_buf);

		switch(gamestate) {
		case LEVEL_START:
			setTextSize(60);
			glColor3f(1,1,1);
			setTextAlign(TEXT_CENTER);
			font_drawText(0, 0, "GET READY!");
			break;
		case LEVEL_RUNNING: case LEVEL_TIMESUP:
			glColor3f(1,1,1);
			char time_temp[20];
			int time_remaining, min, sec;
			time_remaining = (currentlvl->timelimit - game_time + 0.5f);
			//if (time_remaining < 0) time_remaining = 0;
			min = time_remaining / 60;
			sec = time_remaining % 60;
			sprintf(time_temp,"%01d:%02d",min,sec);
			int extra_size = (time_remaining < 10 ? 10 - time_remaining : 0) * 30;
			if (time_remaining < 10) {
				if (time_remaining % 2 == 0) {
					glColor3f(1,0,0);
				} else {
					glColor3f(1,1,1);
				}
			}
			setTextAlign(TEXT_CENTER);
			setTextSize(40 + extra_size);
			font_drawText(0, HEIGHT/2 - 45 - extra_size*1.5, time_temp);
			break;
		case LEVEL_CLEARED:
			setTextSize(60);
			glColor3f(1,1,1);
			setTextAlign(TEXT_CENTER);
			font_drawText(0, 0, "LEVEL CLEARED!");
			break;
		case LEVEL_TRANSITION:
			setTextSize(60);
			glColor3f(0.8f,0.8f,0.8f);
			setTextAlign(TEXT_CENTER);
			font_drawText(0, 0, "LOADING LEVEL...");
			break;
		case LEVEL_PLAYER_DEAD:
			setTextSize(60);
			glColor3f(1,0,0);
			setTextAlign(TEXT_CENTER);
			font_drawText(0, 0, "GAME OVER!");
			break;
		}
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
	int i;
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
struct robotarm_param robot_temp = {200};

void space_init_level(int space_station, int deck)
{

	static struct player *player;

	if(player==NULL){
		player = (struct player*)player_init();
	} else {
		player->hp = player->max_hp;
		player->disable = 0;
	}

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

	player->body->p.x = currentlvl->left + 50;
	player->body->p.y = currentlvl->height - 50;
	player->hp = player->max_hp;
	player->body->v.x = 0;
	player->body->v.y = -10;

	objects_add(robotarm_init(200,&robot_temp));

	/* static ground */
	cpBody  *staticBody = space->staticBody;
	static cpShape *floor;
	static cpShape *ceiling;
	/* remove floor and ceiling */
	if(floor != NULL && ceiling != NULL){
		cpSpaceRemoveStaticShape(space,floor);
		cpSpaceRemoveStaticShape(space,ceiling);
	}

	floor = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left,0), cpv(currentlvl->right,0), 10)); // ground level at 0
	cpShapeSetFriction(floor, 0.8f);
	cpShapeSetCollisionType(floor, ID_GROUND);

	ceiling = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left,currentlvl->height), cpv(currentlvl->right,currentlvl->height), 10.0f));
	cpShapeSetFriction(ceiling, 0.8f);
	cpShapeSetCollisionType(ceiling, ID_GROUND);

}

static void SPACE_init()
{
	objects_init();

	state_timer = 10;
	gamestate = LEVEL_TRANSITION;

	cpVect gravity = cpv(0, -200);
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	//cpSpaceSetDamping(space, 0.999);

	//init stars
	srand(122531);
	int i;
	for (i=0; i<star_count; i++) {
		stars_x[i] = rand()%(SW*2) - SW;
		stars_y[i] = rand()%(SW*2) - SW;
		stars_size[i] = 2 + 4*(rand() % 1000) / 1000.0f;
	}
}

static void SPACE_destroy()
{
	cpSpaceDestroy(space);
	objects_destroy();
}

float getGameTime() {
	return game_time;
}
