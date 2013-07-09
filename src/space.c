/* header */
#include "space.h"

/* standard c-libraries */
#include <stdio.h>

#include "game.h"

/* Game state */
#include "main.h"
#include "menu.h"
#include "gameover.h"
#include "state.h"

/* Drawing */
#include "draw.h"
#include "font.h"
#include "particles.h"
#include "tilemap.h"

/* Game components */
#include "objects.h"
#include "collisioncallbacks.h"
#include "player.h"
#include "tankfactory.h"
#include "robotarm.h"
#include "level.h"
#include "tank.h"
#include "button.h"

#include "waffle_utils.h"

static float accumulator = 0;

STATE_ID state_space;

/**
 * The global space state
 */

static void SPACE_draw();
static void update_objects(object_data *obj);
static void render_objects(object_data *obj);
static void update_camera_zoom(int cam_mode);
static void update_camera_position();

int space_rendering_map = 0;

static int second_draw = 0;

static button btn_pause;
static int game_paused = 0;

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

static int cam_mode = 5;

/* level data */
level *currentlvl;

static void input();

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

joystick *joy_left;
joystick *joy_right;

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

static void draw_gui();

/* The state timer */
static float state_timer = 0;
/**
 * Inner state functions
 */

static void level_start()
{
	game_time = 0;
	if(state_timer > 1.5){
		object_group_player *player = (object_group_player*)objects_first(ID_PLAYER);
		player->disable = 0;
		change_state(LEVEL_RUNNING);
	}
}
static void level_running()
{
	/* update game time */
	game_time += dt;

	input();

	update_all();
	object_group_player *player = (object_group_player*)objects_first(ID_PLAYER);
	if(player->hp_bar.value <= 0){
		player->disable = 1;
		change_state(LEVEL_PLAYER_DEAD);
	}

	if ((objects_count(ID_FACTORY) + objects_count(ID_TANK) + objects_count(ID_TURRET)) == 0) {
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
}
static void level_cleared()
{
	//TODO: add a 3 seconds animation of remaining time being added to score
	int time_remaining = (currentlvl->timelimit - game_time + 0.5f);

	if (time_remaining > 0) {
		object_group_player *player = (object_group_player *)objects_first(ID_PLAYER);
		int score_bonus = time_remaining * 75; // tidligere 25!
		score_bonus += (player->hp_bar.value / player->hp_bar.max_hp) * 100 * 35; // adds score bonus for remaining hp

		player->score += score_bonus; //Time bonus (75 * sec left)
		particles_add_score_popup(player->data.body->p, score_bonus);
		game_time = currentlvl->timelimit;
	}

	update_all();

	if(state_timer > 2){
		lvl_cleared=1;
		change_state(LEVEL_TRANSITION);
	}
}
static void level_transition()
{
	//if(state_timer > 1){
		//space_init_level(1,1);
		if (lvl_cleared==1) {
			//TODO remove tmp next level
			int next_lvl = currentlvl->deck + 1;
			if (next_lvl > level_get_level_count(currentlvl->station)) {
				gameover_setstate(GAMEOVER_WIN);
			    statesystem_set_state(state_gameover);
			} else {
				space_init_level(1,next_lvl); //TODO TMP
			}
		} else {
			//space_init_level(1,1); //TODO TMP
		}

		/* update objects to move shapes to same position as body */
		change_state(LEVEL_START);
	//}
}

/**
 * Resets timer and changes state
 */
static void change_state(int state)
{
	state_timer = 0;
	statesystem_set_inner_state(state_space,state);
	gamestate = state;

#if !ARCADE_MODE
	SDL_Log("DEBUG: entering state[%d]: %s\n",state,game_state_names[state]);
#endif
}

/**
 * Main space update function
 */
static void pre_update()
{
	input();

	/* runs the current state */
	state_timer+=dt; /* updates the timer */
	//state_functions[gamestate]();


}

static void post_update()
{
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
static void update_objects(object_data *obj)
{
	int moved_left = 0;
	int moved_right = 0;

	if(obj->alive){
		if(obj->body->p.y > currentlvl->height  || obj->body->p.y < 0){
			obj->alive = 0;
			SDL_Log("WARNING: object killed outside of level!");
		}

		if (obj->body->p.x < currentlvl->left ){
			obj->body->p.x = currentlvl->right - abs(currentlvl->left -obj->body->p.x );
			moved_left = 1;
		}


		if(obj->destroyed || moved_left){
			int i = 0;
			if (obj->components.body_count) {
			for(i=0;i < obj->components.body_count; i++){
				cpBody *body = obj->components.bodies[i];
				if (body) {
				if(body->p.x < currentlvl->left || moved_left){
					body->p.x = currentlvl->right - (currentlvl->left -body->p.x );
				}
				}
			}
			}
		}

		if (obj->body->p.x > currentlvl->right){
			obj->body->p.x = currentlvl->left + (obj->body->p.x - currentlvl->right);
			moved_right = 1;
		}
		if(obj->destroyed || moved_right){
			int i = 0;
			if (obj->components.body_count) {
			for(i=0;i < obj->components.body_count; i++){
				cpBody *body = obj->components.bodies[i];
				if (body) {
				if(body->p.x > currentlvl->right || moved_right){
					body->p.x = currentlvl->left + (body->p.x - currentlvl->right);
				}
				}
			}
			}
		}

		obj->preset->update(obj);
	}else{
		objects_remove(obj);
	}
}

/**
 * draws everything twice to make infinite loop world
 */
static void draw()
{
	SPACE_draw();
	draw_light_map();

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

		scrlvl = 1.0f * GAME_HEIGHT/currentlvl->height;

		zoomlvl = cam_mode == 1 ? 4 : 12;
		if (py < 0) {
			/* undefined zoom! Reset/fix player position? */
		} else if ( py < 0.2) {
			cam_zoom = 2 / zoomlvl + scrlvl;
			cam_center_y = GAME_HEIGHT / (2*cam_zoom);
		} else if (py < 0.4) {
			cam_zoom = (1 + cos(5*M_PI * (py + 1))) / zoomlvl + scrlvl;
			cam_center_y = GAME_HEIGHT / (2*cam_zoom);
		} else if (py < 0.6) {
			cam_zoom = scrlvl;
			cam_center_y = currentlvl->height / (2);
		} else if (py < 0.8) {
			cam_zoom = (1 - cos(5*M_PI * (py - 0.4 + 1))) / zoomlvl + scrlvl;
			cam_center_y = currentlvl->height - GAME_HEIGHT / (2*(cam_zoom));
		} else if (py <= 1.0) {
			cam_zoom = 2 / zoomlvl + scrlvl;
			cam_center_y = currentlvl->height - GAME_HEIGHT / (2*cam_zoom);
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
		if(cam_center_y > currentlvl->height - GAME_HEIGHT/(2*cam_zoom)){
			cam_center_y = currentlvl->height - GAME_HEIGHT/(2*cam_zoom);
		}else if(cam_center_y <  GAME_HEIGHT/(2*cam_zoom)){
			cam_center_y = GAME_HEIGHT/(2*cam_zoom);
		}
		break;
	case 5:
		cam_zoom = 1.0f*GAME_HEIGHT/currentlvl->height;
		cam_center_y = 1.0f*currentlvl->height/2;
		break;
	case 6:
		scrlvl = 1.0f * GAME_HEIGHT/currentlvl->height;
		/* parameters to change */
		zoomlvl = 4; /* amount of zoom less is more zoom */
		float startlvl = 0.8;
		float endlvl = 0.2;

		float freq = startlvl-endlvl;
		if (py < 0) {
			/* undefined zoom! Reset/fix player position? */
		} else if ( py < endlvl) {
			cam_zoom = 2 / zoomlvl + scrlvl;
			cam_center_y = GAME_HEIGHT / (2*cam_zoom);
		} else if (py < startlvl) {
			cam_zoom = (1 - cos( (1/freq)*M_PI*(py + (freq-endlvl) ))) / zoomlvl + scrlvl;
			cam_center_y = GAME_HEIGHT / (2*cam_zoom);
		} else if (py < 1) {
			cam_zoom = scrlvl;
			cam_center_y = currentlvl->height / (2);
		}else{
			/* undefined zoom! Reset/fix player position? */
		}
		break;
	default:
		cam_zoom = 1.0f*GAME_HEIGHT/currentlvl->height;
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
	cam_dx = cam_dx * pos_delay + ((player->data.body->rot.x * pos_rel_x - pos_rel_offset_x) * GAME_WIDTH) * (1 - pos_delay) / cam_zoom;

	cam_center_x = player->data.body->p.x + cam_dx;

	/* camera constraints */
	camera_width = GAME_WIDTH / (2 * cam_zoom);

	cam_left_limit = currentlvl->left + camera_width;
	cam_right_limit = currentlvl->right - camera_width;

	cam_left = cam_center_x - camera_width;
	cam_right = cam_center_x + camera_width;
}

static void SPACE_draw()
{
	space_rendering_map = 1;
	/* draw background */
	drawStars();

	/* translate view */
	draw_load_identity();
	draw_scale(cam_zoom,cam_zoom,1);
	draw_translate(-cam_center_x, -cam_center_y, 0.0f);

	/* draw tilemap */
	tilemap_render(currentlvl->tiles);

	setTextAngle(0);
	/* draw all objects */
	objects_iterate(render_objects);

	/* draw particle effects */
	particles_draw(dt);

	space_rendering_map = 0;
	if(!second_draw){
		draw_gui();
	}
}

void draw_gui()
{
	/* reset transform matrix */
	draw_load_identity();
	draw_color4f(1,1,1,1);

#if GOT_TOUCH
	draw_color4f(1,1,1,1);
	joystick_render(joy_left);
	joystick_render(joy_right);
#endif

	if (gamestate == LEVEL_RUNNING && !game_paused) {
		button_render(btn_pause);
	}

	/* draw GUI */
	setTextAngle(0); // TODO don't use global variables for setting font properties
	setTextAlign(TEXT_LEFT);
	setTextSize(35);

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
	font_drawText(-GAME_WIDTH/2+20,GAME_HEIGHT/2 - 26,score_temp);

	draw_color4f(1,0,0,1);
	setTextSize(20);
	char goals_left[100];
	sprintf(goals_left, "OBJEKTER: %d",
			objects_count(ID_FACTORY)+
			objects_count(ID_TURRET)+
			objects_count(ID_TANK));
	font_drawText(-GAME_WIDTH/2+20,GAME_HEIGHT/2 - 100,goals_left);

	draw_color4f(1,1,1,1);
	setTextSize(20);
	char particles_temp[20];
	char particles2_temp[20];
	char particles3_temp[20];
	sprintf(particles_temp,"%d",particles_active);
	sprintf(particles2_temp,"%d",available_particle_counter);
	sprintf(particles3_temp,"%d",(available_particle_counter + particles_active));
	//font_drawText(-WIDTH/2+20,HEIGHT/2 - 100,particles_temp);
	//font_drawText(-WIDTH/2+20,HEIGHT/2 - 140,particles2_temp);
	//font_drawText(-WIDTH/2+20,HEIGHT/2 - 180,particles3_temp);

	char pos_temp[20];
	sprintf(pos_temp,"X: %4.0f Y: %4.0f",player->data.body->p.x,player->data.body->p.y);
	//font_drawText(-WIDTH/2+15,-HEIGHT/2+12,pos_temp);

	setTextAlign(TEXT_RIGHT);
	//font_drawText(WIDTH/2-25,-HEIGHT/2+15,game_state_names[gamestate]);

	draw_color4f(1,1,1,1);
	setTextSize(15);
	char level_temp[20];
	setTextAlign(TEXT_CENTER);
	//		sprintf(level_temp,"STATION: %d DECK: %d", currentlvl->station, currentlvl->deck);
	sprintf(level_temp,"LEVEL %d", currentlvl->deck);
	font_drawText(0, -GAME_HEIGHT/2+24, level_temp);



	setTextAlign(TEXT_RIGHT);

#if !ARCADE_MODE
	font_drawText(GAME_WIDTH/2 - 15, GAME_HEIGHT/2 - 20, fps_buf);
#endif

	char time_temp[20];
	switch(gamestate) {
	case LEVEL_START:
		setTextSize(60);
		draw_color4f(1,1,1,1);
		setTextAlign(TEXT_CENTER);
		font_drawText(0, 0, "GET READY!");
		break;
	case LEVEL_RUNNING: case LEVEL_TIMESUP:
		draw_color4f(1,1,1,1);
		int time_remaining, min, sec;
		time_remaining = (currentlvl->timelimit - game_time + 0.5f);
		//if (time_remaining < 0) time_remaining = 0;
		min = time_remaining / 60;
		sec = time_remaining % 60;
		sprintf(time_temp,"%01d:%02d",min,sec);
		int extra_size = (time_remaining < 10 ? 10 - time_remaining : 0) * 30;
		if (time_remaining < 10) {
			if (time_remaining % 2 == 0) {
				draw_color4f(1,0,0,1);
			} else {
				draw_color4f(1.0,1.0,1.0,1);
			}
		}
		setTextAlign(TEXT_CENTER);
		setTextSize(40 + extra_size);
		font_drawText(0, GAME_HEIGHT/2 - 29 - extra_size*1.5, time_temp);
		break;
	case LEVEL_CLEARED:
		setTextSize(60);
		setTextAlign(TEXT_CENTER);
		font_drawText(0, 0, "LEVEL CLEARED!");
		break;
	case LEVEL_TRANSITION:
		setTextSize(60);
		draw_color4f(0.8f,0.8f,0.8f,1);
		setTextAlign(TEXT_CENTER);
		//font_drawText(0, 0, "LOADING LEVEL...");
		break;
	case LEVEL_PLAYER_DEAD:
		setTextSize(60);
		draw_color4f(1,0,0,1);
		setTextAlign(TEXT_CENTER);

#if ARCADE_MODE
		setTextSize(80);
		font_drawText(0, 0, "GAME OVER");
		draw_color4f(0.1,0.9,0.1,1);

		static float button_timer = 0;
		static int button_down;
		button_timer+=dt;
		if(button_timer > 0.5){
			button_down = !button_down;
			button_timer = 0;
		}
		if(button_down){
			cpVect t = cpv(0,0-GAME_HEIGHT/4);
			//draw_texture(TEX_BUTTON_DOWN,&t,TEX_MAP_FULL,300,300,0);
		}else{
			cpVect t = cpv(0,-5.5-GAME_HEIGHT/4);
			//draw_texture(TEX_BUTTON,&t,TEX_MAP_FULL,300,300,0);
		}
#else
		font_drawText(0, 0, "GAME OVER-PRESS ENTER");
#endif
		setTextSize(120);
		break;
	case LEVEL_STATE_COUNT:
		/* invalid value */
		break;
	}
}

static void render_objects(object_data *obj)
{
	if((obj->body->p.x > cam_left - 200 && obj->body->p.x < cam_right + 200)) {
		obj->preset->render(obj);
		return;
	}

	if(cam_center_x < cam_left_limit){
		float old_cam_x = cam_center_x;
		float new_cam_center_x = cam_center_x + currentlvl->width;
		float new_cam_left = new_cam_center_x - camera_width;
		float new_cam_right = new_cam_center_x + camera_width;
		if((obj->body->p.x > new_cam_left - 200 && obj->body->p.x < new_cam_right + 200)){
			cam_center_x = new_cam_center_x;
			draw_push_matrix();
			draw_translate(-(currentlvl->right + abs(currentlvl->left)),0,0);
			obj->preset->render(obj);
			draw_pop_matrix();
			cam_center_x = old_cam_x;
		}
	}else if(cam_center_x > cam_right_limit){
		float old_cam_x = cam_center_x;
		float new_cam_center_x = cam_center_x - (currentlvl->right + abs(currentlvl->left));
		float new_cam_left = new_cam_center_x - camera_width;
		float new_cam_right = new_cam_center_x + camera_width;
		if((obj->body->p.x > new_cam_left - 200 && obj->body->p.x < new_cam_right + 200)){
			cam_center_x = new_cam_center_x;
			draw_push_matrix();
			draw_translate((currentlvl->right + abs(currentlvl->left)),0,0);
			obj->preset->render(obj);
			draw_pop_matrix();
			cam_center_x = old_cam_x;
		}
	}
}


#if GLES1
#define star_count 100
#else
#define star_count 100
#endif

static int stars_x[star_count];
static int stars_y[star_count];
static float stars_size[star_count];
#define SW 8000
static void stars_init()
{
	//init stars
	srand(122531);
	int i;
	for (i=0; i<star_count; i++) {
		stars_x[i] = rand()%(GAME_WIDTH*2*2) - GAME_WIDTH*2; // make sure that radius is greater than WIDTH * sqrt(2)
		stars_y[i] = rand()%(GAME_WIDTH*2*2) - GAME_WIDTH*2;
		stars_size[i] = 2 + 5*(rand() % 1000) / 1000.0f;
	}
}
void drawStars()
{
	static int tick2death = 1;

	if (tick2death > 0) {
		tick2death--;
		return;
	}

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	draw_push_matrix();

	draw_scale(0.5f * cam_zoom,0.5f * cam_zoom, 1);

	static float spaceship_angle;
	spaceship_angle += 0.01f * 360*dt;
	float cam_angle;
	if (currentlvl)
		cam_angle = (cam_center_x - currentlvl->left)  / (currentlvl->right - currentlvl->left) * 360 + spaceship_angle;
	else
		cam_angle = spaceship_angle;

	draw_rotate(-cam_angle,0,0,1);

	int i;
	draw_color4f(1,1,1,1);
	for (i = 0; i < star_count; i++) {
		float size = stars_size[i];
		float star_x = (stars_x[i]);
		float star_y = (stars_y[i]);
		draw_box_append(star_x,star_y,size,size,0,1);
	}
	draw_flush_simple();
	draw_pop_matrix();
}

void space_init_level(int space_station, int deck)
{
	joystick_release(joy_left);
	joystick_release(joy_right);

	static object_group_player *player;

	if(player==NULL){
		player = (object_group_player*)object_create_player();
	} else {
		player->disable = 1;
		player->hp_bar.value = player->param->max_hp;
		if (space_station == 1 && deck == 1) { // reset player score if level 1 is initializing
			player->data.preset->init((object_data*) player);
		}else{
			player->aim_speed += 0.3;
			player->rotation_speed += 0.3;
			if(deck == 6){
				player->gun_level = 3;
			}
		}
	}

	//TODO manage persistent objects(like player) in a better way, instead of removing and then re-adding

	objects_clear();

	objects_add((object_data*)player);

	if (currentlvl != NULL) {
		level_unload(currentlvl);
	}

	currentlvl = level_load(space_station,deck);

	if (currentlvl == NULL) {
		SDL_Log( "space_level_init failed!\n");
		exit(-1);
	}


	float offset = currentlvl->tiles->tile_height;
	/* SETS the gamestate */
	change_state(LEVEL_START);

	player->data.body->p.x = currentlvl->left + offset + 50;
	player->data.body->p.y = currentlvl->height - offset - 50;
	cpBodySetAngle(player->data.body,3*(M_PI/2));
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

	currentlvl->floor = offset;
	currentlvl->ceiling = currentlvl->height - offset/2;
	floor = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left-100,0), cpv(currentlvl->right+100,0), offset)); // ground level at 0
	cpShapeSetFriction(floor, 1);
	cpShapeSetCollisionType(floor, ID_GROUND);
	cpShapeSetElasticity(floor, 0.7f);

	ceiling = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(currentlvl->left-100,currentlvl->height), cpv(currentlvl->right+100,currentlvl->height), offset));
	cpShapeSetFriction(ceiling, 1);
	cpShapeSetCollisionType(ceiling, ID_GROUND);
	cpShapeSetElasticity(ceiling, 0.7f);

	/*
	 * puts all shapes in correct position
	 */
	update_all();

	particles_clear();
}

static void on_enter()
{
	game_paused = 0;
}

static void game_over()
{
	lvl_cleared=0;
	statesystem_set_state(state_gameover);
	gameover_setstate(enter_name);
	//change_state(LEVEL_TRANSITION);
}

static void pause_game()
{
	menu_change_current_menu(MENU_INGAME);
	statesystem_push_state(state_menu);
	game_paused = 1;
}

static void sdl_event(SDL_Event *event)
{
	SDL_Scancode key;
	switch(event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;

		/* Opens the pause menu */
		if (key == KEY_ESCAPE && gamestate == LEVEL_RUNNING) {
			pause_game();
		}

		switch (gamestate) {
		case LEVEL_PLAYER_DEAD:
			if(key == KEY_RETURN_2 || key == KEY_RETURN_1){//state_timer > 3){
				game_over();
			}
			break;
		default:
			break;
		}
		break;
		case SDL_FINGERDOWN:
			if (gamestate == LEVEL_RUNNING) {
				if (button_finger_down(btn_pause, &event->tfinger))
					return;
			} else if (gamestate == LEVEL_PLAYER_DEAD) {
				button_finger_down(btn_fullscreen, &event->tfinger);
			}

			if (joystick_finger_down(joy_left, &event->tfinger))
				return;
			if (joystick_finger_down(joy_right, &event->tfinger))
				return;

			break;
		case SDL_FINGERMOTION:
			if (button_finger_move(btn_pause, &event->tfinger)) return;
			if (joystick_finger_move(joy_left, &event->tfinger)) return;
			if (joystick_finger_move(joy_right, &event->tfinger)) return;
			break;
		case SDL_FINGERUP:
			if (gamestate == LEVEL_RUNNING) {
				if (button_finger_up(btn_pause, &event->tfinger)) {
					pause_game();
				}

			} else if (gamestate == LEVEL_PLAYER_DEAD) {
				if (button_finger_up(btn_fullscreen, &event->tfinger)) {
					game_over();
				}
			}
			joystick_finger_up(joy_left, &event->tfinger);
			joystick_finger_up(joy_right, &event->tfinger);
			break;
	}
}

static void on_leave()
{

}

static void destroy()
{
    objects_destroy();
    cpSpaceDestroy(space);
	joystick_free(joy_left);
	joystick_free(joy_right);
}

void space_init()
{
	state_funcs space_funcs = {on_enter,pre_update,post_update,draw,sdl_event,on_leave,destroy};
	statesystem_register(state_space,LEVEL_STATE_COUNT);
    statesystem_add_inner_state(state_space,LEVEL_START,level_start,NULL);
    statesystem_add_inner_state(state_space,LEVEL_RUNNING,level_running,NULL);
    statesystem_add_inner_state(state_space,LEVEL_TIMESUP,level_timesup,NULL);
    statesystem_add_inner_state(state_space,LEVEL_PLAYER_DEAD,level_player_dead,NULL);
    statesystem_add_inner_state(state_space,LEVEL_CLEARED,level_cleared,NULL);
    statesystem_add_inner_state(state_space,LEVEL_TRANSITION,level_transition,NULL);

	objects_init();

	state_timer = 10;
	change_state(LEVEL_START);

	cpVect gravity = cpv(0, -600);
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	cpSpaceSetDamping(space, 0.99);

    collisioncallbacks_init();

    stars_init();

    btn_pause = button_create(GAME_WIDTH/2-70, GAME_HEIGHT/2-70, 80, 80, SPRITE_BUTTON_PAUSE, BTN_IMAGE_SIZED);
    joy_left = joystick_create(0, 80, 10, -GAME_WIDTH/2, -GAME_HEIGHT/2, GAME_WIDTH/2, GAME_HEIGHT);
    joy_right = joystick_create(0, 80, 10, 0, -GAME_HEIGHT/2, GAME_WIDTH/2, GAME_HEIGHT);
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

void input()
{
#if !GOT_TOUCH
	/* update joystick positions */
	int axis_x = keys[KEY_RIGHT_1] - keys[KEY_LEFT_1];
	int axis_y = keys[KEY_UP_1] - keys[KEY_DOWN_1];
	joystick_axis(joy_left, axis_x, axis_y);

	axis_x = keys[KEY_RIGHT_2] - keys[KEY_LEFT_2];
	axis_y = keys[KEY_UP_2] - keys[KEY_DOWN_2];
	joystick_axis(joy_right, axis_x, axis_y);

	/*
	 * Camera modes + F11 = timeout + F8 = reload particles (broken)
	 */
	if (keys[SDL_SCANCODE_F1]) {
		cam_mode = 1;
	} else if (keys[SDL_SCANCODE_F2]) {
		cam_mode = 2;
	} else if (keys[SDL_SCANCODE_F3]) {
		cam_mode = 3;
	} else if (keys[SDL_SCANCODE_F4]) {
		cam_mode = 4;
	} else if (keys[SDL_SCANCODE_F5]) {
		cam_mode = 5;
	} else if (keys[SDL_SCANCODE_F6]) {
		cam_mode = 6;
	} else if (keys[SDL_SCANCODE_F11]) {
		game_time = currentlvl->timelimit;
		return;
	}

	/* DEBUG KEYS*/

#if !ARCADE_MODE
	if(keys[SDL_SCANCODE_G]){
		keys[SDL_SCANCODE_G] = 0;
		cpVect gravity = cpv(0, -2);
		cpSpaceSetGravity(space, gravity);
	}
#endif
#endif
}
