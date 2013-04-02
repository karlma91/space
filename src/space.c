/* header */
#include "space.h"

/* standard c-libraries */
#include <stdio.h>

/* SDL */
#include "SDL_opengl.h"

/* Game state */
#include "main.h"
#include "menu.h"
#include "gameover.h"

/* Drawing */
#include "draw.h"
#include "font.h"
#include "particles.h"
#include "tilemap.h"

/* Game components */
#include "objects.h"
#include "player.h"
#include "tankfactory.h"
#include "robotarm.h"
#include "level.h"
#include "tank.h"

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
static void update_objects(object_data *obj);
static void render_objects(object_data *obj);
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

int cam_left;
int cam_right;

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
	object_group_player *player = (object_group_player*)objects_first(ID_PLAYER);
	if(player->hp_bar.value <= 0){
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
	object_group_player *player = (object_group_player*)objects_first(ID_PLAYER);
	player->hp_bar.value = 0;
	player->disable = 1;
	if (state_timer > 3) {
		change_state(LEVEL_PLAYER_DEAD);
	}
}
int lvl_cleared = 0; //TODO tmp lvl cleared;
static void level_player_dead()
{
	update_all();
	if(state_timer > 3){
		lvl_cleared=0;
		currentState = &state_gameover;
		gameover_setstate(enter_name);
		//change_state(LEVEL_TRANSITION);
	}
}
static void level_cleared()
{
	//TODO: add a 3 seconds animation of remaining time being added to score
	int time_remaining = (currentlvl->timelimit - game_time + 0.5f);

	if (time_remaining > 0) {
		object_group_player *player = (object_group_player *)objects_first(ID_PLAYER);
		int time_bonus = time_remaining * 25;
		player->score += time_bonus; //Time bonus (25 * sec)
		particles_add_score_popup(player->data.body->p, time_bonus);
		game_time = currentlvl->timelimit;
	}

	update_all();

	if(state_timer > 3){
		lvl_cleared=1;
		change_state(LEVEL_TRANSITION);
	}
}
static void level_transition()
{
	if(state_timer > 1){
		//space_init_level(1,1);
		if (lvl_cleared==1) {
			//TODO remove tmp next level
			int next_lvl = currentlvl->deck + 1;
			//TODO WARNING: final level index hard-coded!
			if (next_lvl > 3) {
				currentState = &state_gameover;
				gameover_setstate(GAMEOVER_WIN);
			} else {
				space_init_level(1,next_lvl); //TODO TMP
			}
		} else {
			//space_init_level(1,1); //TODO TMP
		}
		/* update objects to move shapes to same position as body */
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
	}else if(keys[SDLK_F11]){
		game_time = currentlvl->timelimit;
		return;
	}else if(keys[SDLK_F8]){
		particles_reload_particles();
		keys[SDLK_F8] = 0;
	}

	/*
	 * Opens the pause menu
	 */
	if(keys[SDLK_ESCAPE] && gamestate == LEVEL_RUNNING){
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

static void empty(){

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
	objects_iterate(empty);

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
static object_group_tank *temptank = NULL;
static void update_objects(object_data *obj)
{
	if(obj->alive){

		//TODO: fix this shit
		if(obj->preset->ID == ID_TANK){
			temptank = ((object_group_tank*)obj);
			if (obj->body->p.x < currentlvl->left ){
				obj->body->p.x = currentlvl->right - abs(currentlvl->left -obj->body->p.x );
				temptank->wheel1->p.x = currentlvl->right - abs(currentlvl->left - temptank->wheel1->p.x );
				temptank->wheel2->p.x = currentlvl->right - abs(currentlvl->left - temptank->wheel2->p.x );

			}
			if (obj->body->p.x > currentlvl->right){
				obj->body->p.x = currentlvl->left + (obj->body->p.x - currentlvl->right);
				temptank->wheel1->p.x = currentlvl->left + (temptank->wheel1->p.x - currentlvl->right);
				temptank->wheel2->p.x = currentlvl->left + (temptank->wheel2->p.x - currentlvl->right);
			}
		}else{
			if (obj->body->p.x < currentlvl->left ) obj->body->p.x = currentlvl->right - abs(currentlvl->left -obj->body->p.x );
			if (obj->body->p.x > currentlvl->right) obj->body->p.x = currentlvl->left + (obj->body->p.x - currentlvl->right);
		}

		obj->preset->update(obj);
	}else{
		objects_remove(obj);
		obj->preset->destroy(obj);
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
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

	/* dynamic camera zoom */
	float py = player->data.body->p.y / currentlvl->height;
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
		cam_center_y = player->data.body->p.y;
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

	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));
	/* dynamic camera pos */
	static const float pos_delay = 0.99f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
	static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
	static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
	static float cam_dx;
	cam_dx = cam_dx * pos_delay + ((player->data.body->rot.x * pos_rel_x - pos_rel_offset_x) * WIDTH) * (1 - pos_delay) / cam_zoom;

	cam_center_x = player->data.body->p.x + cam_dx;

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

	/* draw tilemap */
	if(!second_draw){
		tilemap_render(currentlvl->tiles);
	}

	setTextAngle(0);
	/* draw all objects */
	objects_iterate(render_objects);

	/* draw particle effects */
	particles_draw(dt);

	if(!second_draw){

		/* reset transform matrix */
		glLoadIdentity();

		/* draw GUI */
		setTextAngle(0); // TODO don't use global variables for setting font properties
		setTextAlign(TEXT_LEFT);
		setTextSize(40);

		glColor3f(1,1,1);
		//font_drawText(-WIDTH/2+15,HEIGHT/2 - 10,"WASD     MOVE\nQE       ZOOM\nSPACE   SHOOT\nH        STOP\nESCAPE   QUIT");

		object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

		/* simple score animation */
		char score_temp[20];
		static int score_anim = 0;
		static int score_adder = 1;
		if (score_anim + score_adder < player->score) {
			score_anim += score_adder;
			score_adder += 11;
		} else {
			score_anim = player->score;
			score_adder = 1;
		}
		sprintf(score_temp,"%d",score_anim);
		font_drawText(-WIDTH/2+20,HEIGHT/2 - 45,score_temp);


		setTextSize(20);
		char particles_temp[20];
		char particles2_temp[20];
		char particles3_temp[20];
		sprintf(particles_temp,"%d",particles_active);
		sprintf(particles2_temp,"%d",available_particle_counter);
		sprintf(particles3_temp,"%d",(available_particle_counter + particles_active));
		font_drawText(-WIDTH/2+20,HEIGHT/2 - 100,particles_temp);
		font_drawText(-WIDTH/2+20,HEIGHT/2 - 140,particles2_temp);
		font_drawText(-WIDTH/2+20,HEIGHT/2 - 180,particles3_temp);

		char pos_temp[20];
		sprintf(pos_temp,"X: %4.0f Y: %4.0f",player->data.body->p.x,player->data.body->p.y);
		font_drawText(-WIDTH/2+15,-HEIGHT/2+12,pos_temp);

		setTextAlign(TEXT_RIGHT);
		font_drawText(WIDTH/2-25,-HEIGHT/2+15,game_state_names[gamestate]);

		setTextSize(15);
		char level_temp[20];
		setTextAlign(TEXT_CENTER);
		sprintf(level_temp,"STATION: %d DECK: %d",currentlvl->station, currentlvl->deck);
		font_drawText(0, -HEIGHT/2+8, level_temp);


		setTextAlign(TEXT_RIGHT);
		font_drawText(WIDTH/2 - 15, HEIGHT/2 - 20, fps_buf);

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
			font_drawText(0, 0, "GAME OVER");
			setTextSize(120);font_drawText(0,-180,"\x49\x4e\x46\x33\x34\x38\x30");
			break;
		case LEVEL_STATE_COUNT:
			/* invalid value */
			break;
		}
	}
}

static void render_objects(object_data *obj)
{
	if(obj->body->p.x > cam_left - 200 && obj->body->p.x < cam_right + 200){
		obj->preset->render(obj);
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

static void func(object_data* obj)
{
	obj->preset->destroy(obj);
}



void space_init_level(int space_station, int deck)
{
	particles_clear();

	static object_group_player *player;


	if(player==NULL){
		player = (object_group_player*)object_create_player();
	} else {
		player->hp_bar.value = player->param->max_hp;
		player->disable = 0;
		if (space_station == 1 && deck == 1) { // reset player score if level 1 is initializing
			player->score = 0;
		}
	}

	objects_iterate(func);
	objects_destroy();

	objects_add((object_data*)player);

	if (currentlvl != NULL) {
		level_unload(currentlvl);
	}

	currentlvl = level_load(space_station,deck);

	if (currentlvl == NULL) {
		fprintf(stderr, "space_level_init failed!\n");
		exit(-1);
	}


	/* SETS the gamestate */
	change_state(LEVEL_START);

	player->data.body->p.x = currentlvl->left + 50;
	player->data.body->p.y = currentlvl->height - 50;
	player->hp_bar.value = player->param->max_hp;
	player->data.body->v.x = 0;
	player->data.body->v.y = -10;

	//objects_add(robotarm_init(200,&robot_temp));

	/* static ground */
	cpBody  *staticBody = space->staticBody;
	static cpShape *floor;
	static cpShape *ceiling;
	/* remove floor and ceiling */
	if(floor != NULL && ceiling != NULL){
		cpSpaceRemoveStaticShape(space,floor);
		cpSpaceRemoveStaticShape(space,ceiling);
	}

	floor = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left,0), cpv(currentlvl->right,0), 15.0f)); // ground level at 0
	cpShapeSetFriction(floor, 1.0f);
	cpShapeSetCollisionType(floor, ID_GROUND);

	ceiling = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left,currentlvl->height), cpv(currentlvl->right,currentlvl->height), 15.0f));
	cpShapeSetFriction(ceiling, 1.0f);
	cpShapeSetCollisionType(ceiling, ID_GROUND);

	/*
	 * puts all shapes in correct position
	 */
	update_all();

}

static void SPACE_init()
{
	objects_init();

	state_timer = 10;
	change_state(LEVEL_START);

	cpVect gravity = cpv(0, -600);
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

float getGameTime()
{
	return game_time;
}

int getPlayerScore()
{
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));
	if (player != NULL)
		return player->score;
	else
		return -1;
}
