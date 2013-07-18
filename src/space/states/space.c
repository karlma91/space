/* header */
#include "space.h"

/* standard c-libraries */
#include <stdio.h>


#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"

/* Game state */
#include "arcade/menu.h"
#include "arcade/gameover.h"

/* Drawing */
#include "../../engine/graphics/draw.h"
#include "../../engine/graphics/font.h"
#include "../../engine/graphics/particles.h"
#include "../tilemap.h"

#include "../../engine/input/button.h"
//#include "../../engine/io/waffle_utils.h"
#include "../spaceengine.h"

#include "../level.h"
//#include "../collisioncallbacks.h"

/* Game components */
#include "../obj/objects.h"
#include "../obj/player.h"
#include "../obj/tankfactory.h"
#include "../obj/robotarm.h"
#include "../obj/tank.h"

static float accumulator = 0;

//TODO: remove in final game

STATE_ID state_space;

/**
 * The global space state
 */

static void SPACE_draw();
static void update_objects(object_data *obj);
static void render_objects(object_data *obj);
static void update_camera_zoom(int mode);
static void update_camera_position();

int space_rendering_map = 0;

static button btn_pause;
static int game_paused = 0;

static float game_time;

// Chipmunk
static cpFloat phys_step = 1/60.0f;
/* extern */
cpSpace *space;

/* camera settings */
static float cam_left_limit;
static float cam_right_limit;

/* level data */
level *currentlvl;

static camera space_cam;

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

joystick *joy_p1_left, *joy_p1_right;
joystick *joy_p2_left, *joy_p2_right;

char *game_state_names[] = {
	"LEVEL_START",
	"LEVEL_RUNNING",
	"LEVEL_TIMESUP",
	"LEVEL_PLAYER_DEAD",
	"LEVEL_CLEARED",
	"LEVEL_TRANSITION"
};


int multiplayer;

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
static void change_state(enum game_state state);
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
#if ARCADE_MODE
	//if(state_timer > 1){
		//space_init_level(1,1);
		if (lvl_cleared==1) {
			statesystem_set_state(state_leveldone);
			//TODO remove tmp next level
			int next_lvl = currentlvl->deck + 1;
			if (next_lvl > level_get_level_count(currentlvl->station)) {
				gameover_setstate(GAMEOVER_WIN);
			} else {
				space_init_level(1,next_lvl); //TODO TMP
			}
		} else {
			//space_init_level(1,1); //TODO TMP
		}

		/* update objects to move shapes to same position as body */
		change_state(LEVEL_START);
	//}
#else
	statesystem_push_state(state_leveldone);
#endif
}

/**
 * Resets timer and changes state
 */
static void change_state(enum game_state state)
{
	state_timer = 0;
	statesystem_set_inner_state(state_space,state);
	gamestate = state;

	btn_pause->enabled = (state == LEVEL_RUNNING);
	btn_pause->visible = btn_pause->enabled;

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
	update_camera_zoom(current_camera->mode);
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

	objects_iterate(update_objects);

	particles_update(dt);

	/* update all chipmunk objects 60 times per second */
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
#if LIGHT_SYSTEM
	draw_light_map();
#endif
}

static void update_camera_zoom(int mode)
{
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

	camera_update_zoom(current_camera, player->data.body->p, currentlvl->height);
}

static void update_camera_position()
{

    static int follow_player = 1;
    if(keys[SDL_SCANCODE_F]){
        keys[SDL_SCANCODE_F] = 0;
        follow_player = !follow_player;
    }

    object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));
    object_group_tank *tank = ((object_group_tank*)objects_first(ID_TANK));



    if(tank != NULL && !follow_player){
        cpVect vel = tank->data.body->v;
        vel = cpvnormalize(vel);
        camera_update(current_camera, tank->data.body->p, cpv(0,1));
    }else{
    	cpVect c_pos = player->data.body->p;
    	cpVect c_rot = player->data.body->rot;

    	if (accelerometer) {
#define SINT16_MAX ((float)(0x7FFF))
    	    /* get joystick (accelerometer) axis values and normalize them */
    	    float ax = (float) SDL_JoystickGetAxis(accelerometer, 0) / SINT16_MAX * 5;
    	    float ay = -(float) SDL_JoystickGetAxis(accelerometer, 1) / SINT16_MAX * 5;
    	    float az = (float) SDL_JoystickGetAxis(accelerometer, 2) / SINT16_MAX * 5;

    	    static int teller = 0;
    	    if (++teller > 30) {
    	    SDL_Log("\t%f\t%f\t%f",ax,ay,az);
    	    teller = 0;
    	    }

    	    float roll = atan2(ax, ay) * 180.0f / M_PI;
    	    //float pitch = atan2(ax, hypotf(ay, az)) * 1.0f / M_PI;


			if (ay > 1) ay = 1;
			if (ay < -1) ay = -1;

    	    static float last = 0;
    	    last = last*0.7 + ay*0.3;

    		c_pos.x += last * 300;
    	}

    	camera_update(current_camera, c_pos, c_rot);
    }

    cam_left_limit = currentlvl->left + current_camera->width;
    cam_right_limit = currentlvl->right - current_camera->width;
}

static void SPACE_draw()
{
	space_rendering_map = 1;
	/* draw background */
	drawStars();

	/* translate view */
	draw_load_identity();
	draw_scale(current_camera->zoom,current_camera->zoom,1);
	draw_translate(-current_camera->x, -current_camera->y, 0.0f);

	/* draw tilemap */
	tilemap_render(currentlvl->tiles);

	setTextAngle(0);
	/* draw all objects */
	objects_iterate(render_objects);

	/* draw particle effects */
	particles_draw(dt);

	space_rendering_map = 0;
	draw_gui();
}

void draw_gui()
{
	/* reset transform matrix */
	draw_load_identity();
	draw_color4f(1,1,1,1);

#if GOT_TOUCH
	draw_color4f(1,1,1,1);
	joystick_render(joy_p1_left);
	joystick_render(joy_p1_right);
#endif

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
		setTextSize(120);
		break;
	case LEVEL_STATE_COUNT:
		/* invalid value */
		break;
	}
}

static void render_objects(object_data *obj)
{
	if((obj->body->p.x > current_camera->left - 200) && (obj->body->p.x < current_camera->right + 200)) {
		obj->preset->render(obj);
		return;
	}

	if(current_camera->x < cam_left_limit){
		float old_cam_x = current_camera->x;
		float new_cam_center_x = current_camera->x + currentlvl->width;
		float new_cam_left = new_cam_center_x - current_camera->width;
		float new_cam_right = new_cam_center_x + current_camera->width;
		if((obj->body->p.x > new_cam_left - 200 && obj->body->p.x < new_cam_right + 200)){
		    current_camera->x = new_cam_center_x;
			draw_push_matrix();
			draw_translate(-(currentlvl->right + abs(currentlvl->left)),0,0);
			obj->preset->render(obj);
			draw_pop_matrix();
			current_camera->x = old_cam_x;
		}
	}else if(current_camera->x > cam_right_limit){
		float old_cam_x = current_camera->x;
		float new_cam_center_x = current_camera->x - currentlvl->width;
		float new_cam_left = new_cam_center_x - current_camera->width;
		float new_cam_right = new_cam_center_x + current_camera->width;
		if((obj->body->p.x > new_cam_left - 200 && obj->body->p.x < new_cam_right + 200)){
		    current_camera->x = new_cam_center_x;
			draw_push_matrix();
			draw_translate((currentlvl->right + abs(currentlvl->left)),0,0);
			obj->preset->render(obj);
			draw_pop_matrix();
			current_camera->x = old_cam_x;
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

	draw_scale(0.5f * current_camera->zoom,0.5f * current_camera->zoom, 1);

	static float spaceship_angle;
	spaceship_angle += 0.01f * 360*dt;
	float cam_angle;
	if (currentlvl)
		cam_angle = (current_camera->x - currentlvl->left)  / (currentlvl->right - currentlvl->left) * 360 + spaceship_angle;
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
	joystick_release(joy_p1_left);
	joystick_release(joy_p1_right);

	static object_group_player *player;

	if(player==NULL){
		player = (object_group_player*)object_create_player();
	} else {
		player->disable = 1;
		player->hp_bar.value = player->param->max_hp;

		player->data.preset->init((object_data*) player);
		player->aim_speed += 0.3*deck;
		player->rotation_speed += 0.3*deck;
		if(deck >= 6){
			player->gun_level = 3;
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
	statesystem_push_state(state_leveldone);
}

static void pause_game()
{
	statesystem_push_state(state_pause);
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
			if (joystick_finger_down(joy_p1_left, &event->tfinger))
				return;
			if (joystick_finger_down(joy_p1_right, &event->tfinger))
				return;
			break;
		case SDL_FINGERMOTION:
			if (joystick_finger_move(joy_p1_left, &event->tfinger)) return;
			if (joystick_finger_move(joy_p1_right, &event->tfinger)) return;
			break;
		case SDL_FINGERUP:
			if (gamestate == LEVEL_RUNNING) {
			} else if (gamestate == LEVEL_PLAYER_DEAD) {
				//if (button_finger_up(btn_fullscreen, &event->tfinger)) {
					game_over();
				//}
			}
			joystick_finger_up(joy_p1_left, &event->tfinger);
			joystick_finger_up(joy_p1_right, &event->tfinger);
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
	joystick_free(joy_p1_left);
	joystick_free(joy_p1_right);
}

void space_init()
{
	statesystem_register(state_space,LEVEL_STATE_COUNT);
    statesystem_add_inner_state(state_space,LEVEL_START,level_start,NULL);
    statesystem_add_inner_state(state_space,LEVEL_RUNNING,level_running,NULL);
    statesystem_add_inner_state(state_space,LEVEL_TIMESUP,level_timesup,NULL);
    statesystem_add_inner_state(state_space,LEVEL_PLAYER_DEAD,level_player_dead,NULL);
    statesystem_add_inner_state(state_space,LEVEL_CLEARED,level_cleared,NULL);
    statesystem_add_inner_state(state_space,LEVEL_TRANSITION,level_transition,NULL);

	objects_init();

    btn_pause = button_create(SPRITE_BUTTON_PAUSE, 0, "", GAME_WIDTH/2-85, GAME_HEIGHT/2-77, 80, 80);
    button_set_callback(btn_pause, pause_game, 0);
    button_set_enlargement(btn_pause, 2.0f);
    statesystem_register_touchable(this, btn_pause);

	cpVect gravity = cpv(0, -600);
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	cpSpaceSetDamping(space, 0.99);

	extern void collisioncallbacks_init();
    collisioncallbacks_init();

    stars_init();

    joy_p1_left = joystick_create(0, 80, 10, -GAME_WIDTH/2, -GAME_HEIGHT/2, GAME_WIDTH/2, GAME_HEIGHT);
    joy_p1_right = joystick_create(0, 80, 10, 0, -GAME_HEIGHT/2, GAME_WIDTH/2, GAME_HEIGHT);

    state_timer = 10;
	change_state(LEVEL_START);
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
	joystick_axis(joy_p1_left, axis_x, axis_y);

	axis_x = keys[KEY_RIGHT_2] - keys[KEY_LEFT_2];
	axis_y = keys[KEY_UP_2] - keys[KEY_DOWN_2];
	joystick_axis(joy_p1_right, axis_x, axis_y);

	/*
	 * Camera modes + F11 = timeout + F8 = reload particles (broken)
	 */
	if (keys[SDL_SCANCODE_F1]) {
		current_camera->mode = 1;
	} else if (keys[SDL_SCANCODE_F2]) {
		current_camera->mode = 2;
	} else if (keys[SDL_SCANCODE_F3]) {
		current_camera->mode = 3;
	} else if (keys[SDL_SCANCODE_F4]) {
		current_camera->mode = 4;
	} else if (keys[SDL_SCANCODE_F5]) {
		current_camera->mode = 5;
	} else if (keys[SDL_SCANCODE_F6]) {
		current_camera->mode = 6;
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

void space_start_demo() {
	//TODO set and reset all per-game variables
	multiplayer = 0;

	current_camera = &space_cam;

	space_init_level(1,1);
	statesystem_set_state(state_space);
}

void space_start_multiplayer() {
	//TODO set and reset all per-game variables
	multiplayer = 1;

	space_init_level(1,1);
	statesystem_set_state(state_space);
}

void space_restart_level()
{
	statesystem_set_state(state_space);
	space_init_level(currentlvl->station, currentlvl->deck);
}

void space_next_level()
{
	int station = currentlvl->station;
	int deck = currentlvl->deck + 1;

	if (deck <= level_get_level_count(station)) {
		statesystem_set_state(state_space);
		space_init_level(station, deck);
	} else {
		//TODO decide what happens when last level on current station is cleared
		statesystem_set_state(state_stations);
	}


}
