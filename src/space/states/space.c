/* header */
#include "space.h"

/* standard c-libraries */
#include <stdio.h>

#include "../game.h"
#include "we_defstate.h"

/* Game state */
#include "arcade/menu.h"
#include "arcade/gameover.h"
#include "leveldone.h"
#include "../upgrades.h"

#include "../tilemap.h"
#include "../spaceengine.h"
#include "../level.h"

static float position_start = 50;
static float position_end = 400;
static float position_now = 0;
static float position_time = 0;
static float position_dir = 1;

static float accumulator = 0;

//TODO: remove in final game <-- ?

STATE_ID state_space;

/**
 * The global space state
*/

static void SPACE_draw(void);
static void update_instances(instance *, void *);
static void render_instances(instance *, void *);
static void update_camera_zoom(int mode);
static void update_camera_position(void);

static void sticks_init(void);
static void sticks_hide(void);

static void radar_draw(float x, float y);

int space_rendering_map = 0;

particle_system *parti;

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

/* level boundaries */
static LList *ll_floor;
static cpShape *ceiling;

/* level data */
level *currentlvl;

static camera space_cam;

static void input(void);

/*
 * The ingame states for level transition and delays
 */
enum game_state{
	LEVEL_START,
	LEVEL_RUNNING,
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
	"LEVEL_PLAYER_DEAD",
	"LEVEL_CLEARED",
	"LEVEL_TRANSITION"
};

int multiplayer;

/*
 * the current state of the game
 */
enum game_state gamestate = LEVEL_START;

static void level_start(void);
static void level_running(void);
static void level_player_dead(void);
static void level_cleared(void);
static void level_transition(void);
static void change_state(enum game_state state);
static void update_all(void);

static void draw_gui(void);

/* The state timer */
static float state_timer = 0;
/**
 * Inner state functions
 */

#define QUICK_ENTER 1

static void level_start(void)
{
	game_time = 0;
#if QUICK_ENTER
	state_timer = 1.6;
#endif

	int start;
	int p1_ready = joy_p1_left->pressed && joy_p1_right->pressed;
	int p2l = joy_p2_left->pressed;
	int p2r = joy_p2_right->pressed;

	if (p1_ready && p2l && p2r) {
		multiplayer = 1;
		start = 1;
	} else {
		multiplayer = 0;
		start = (p1_ready && (!(p2l || p2r))) || (!GOT_TOUCH && (state_timer > 1.5));
	}

	if (start) {
		obj_player *player = (obj_player*)instance_first(obj_id_player);
		player->disable = 0;
		sticks_init();
		change_state(LEVEL_RUNNING);
	}
}
static void level_running(void)
{
	/* update game time */
	game_time += dt;

	input();

	update_all();
	obj_player *player = (obj_player*)instance_first(obj_id_player);
	if(player && player->hp_bar.value <= 0){
		player->disable = 1;
		change_state(LEVEL_PLAYER_DEAD);
	}

	if ((instance_count(obj_id_factory) + instance_count(obj_id_tank) + instance_count(obj_id_turret)) == 0) {
		change_state(LEVEL_CLEARED);
	}
}

int lvl_cleared = 0; //TODO tmp lvl cleared;
static void level_player_dead(void)
{
	obj_player *player = (obj_player *)instance_first(obj_id_player);
	update_all();

	static int tmp_atom = 0;
	if (state_timer > 1 && !tmp_atom) {
		tmp_atom = 1;
		lvl_cleared=0;
		sticks_hide();

		leveldone_status(0, player->coins, game_time);
		statesystem_push_state(state_leveldone);
	} else {
		tmp_atom = 0;
	}
}
static void level_cleared(void)
{
	obj_player *player = (obj_player *)instance_first(obj_id_player);
	update_all();

	static int tmp_atom = 0;
	if (state_timer > 2 && !tmp_atom) {
		float p = (player->hp_bar.value / player->hp_bar.max_hp);
		player->coins += (int)(p*p * 4)*4 * 25;

		lvl_cleared=1;
		sticks_hide();

		//TODO set star rating based on missions
		leveldone_status(1 + player->coins / (4000.0 + 1000 * currentlvl->deck), player->coins, game_time);

		statesystem_push_state(state_leveldone);
		tmp_atom = 1;
	} else {
		tmp_atom = 0;
	}
}
static void level_transition(void)
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
static void pre_update(void)
{
	input();

	/* runs the current state */
	state_timer+=dt; /* updates the timer */
	//state_functions[gamestate]();
}

static void post_update(void)
{
	update_camera_zoom(current_camera->mode);
	update_camera_position();
}


int nan_check(cpBody *body)
{
	return (body->p.x != body->p.x || body->p.y != body->p.y) << 0 |
			(body->v.x != body->v.x || body->v.y != body->v.y) << 1 |
			(body->w != body->w) << 2 |
			(body->a != body->a) << 3;
}
void nan_check_instance(instance *ins, void *msg)
{
	int nan = nan_check(ins->body);
	if (nan) {
		SDL_Log("ERROR: %s, NaN for %s[%d]'s body (NaN var: %d)", (char *)msg, ins->TYPE->NAME, ins->instance_id, nan);
		main_stop();
	}
}

void nan_check_all(char *msg)
{
	int nan = nan_check(space->staticBody);
	if (nan) {
		SDL_Log("ERROR: %s, NaN for static space body (NaN var: %d)", msg, nan);
		main_stop();
	}

	instance_iterate(nan_check_instance, msg);
}

/**
 * Updates all the objects in objects and in chipmunk
 */
static void update_all(void)
{
	/* chipmunk timestep counter */
	accumulator += dt;

	nan_check_all("pre_iterate");
	instance_iterate(update_instances, NULL);
	nan_check_all("post_iterate");
	particles_update(parti);

	/* update all chipmunk objects 60 times per second */
	while(accumulator >= phys_step)
	{
		cpSpaceStep(space, phys_step);
		accumulator -= phys_step;
	}

	nan_check_all("post_phys");
}

static void update_instances(instance *obj, void *data)
{
	if(obj->alive){
		instance_update(obj);
	}
}

static void render_instances(instance *obj, void *data)
{
	instance_render(obj);
}


static void draw(void)
{
	SPACE_draw();
#if LIGHT_SYSTEM
	draw_light_map();
#endif
}

static void update_camera_zoom(int mode) //TODO remove method
{
	obj_player *player = ((obj_player*)instance_first(obj_id_player));
	//camera_update_zoom(current_camera, player->data.body->p, currentlvl->height);
}

static void update_camera_position(void)
{
    static int follow_player = 1;
    if(keys[SDL_SCANCODE_F]){
        keys[SDL_SCANCODE_F] = 0;
        follow_player = !follow_player;
    }

    obj_player *player = ((obj_player*)instance_first(obj_id_player));
    obj_tank *tank = ((obj_tank*)instance_first(obj_id_tank));

    if(tank != NULL && !follow_player){
        cpVect vel = tank->data.body->v;
        vel = cpvnormalize(vel);
        camera_update(current_camera, tank->data.body->p, cpv(0,1));
    } else if (player) {
    	cpVect c_pos = player->data.body->p;
    	cpVect c_rot = player->data.body->rot;

    	camera_update_zoom(current_camera, player->data.body->p, currentlvl->height);
    	camera_update(current_camera, c_pos, c_rot);
    }

    cam_left_limit = currentlvl->left + current_camera->width;
    cam_right_limit = currentlvl->right - current_camera->width;
}

static void draw_deck()
{
	/* draw ceiling */
	draw_color4f(0.1,0.3,0.7,0.9);
	{
		glScalef(current_camera->zoom,current_camera->zoom,1); //TODO REMOVE
		glTranslatef(-current_camera->x, -current_camera->y, 0.0f); //TODO REMOVE
	}
	draw_circle(0,0,currentlvl->inner_radius);
	{
		glLoadIdentity(); //TODO REMOVE
	}

	/* draw floor */
	draw_donut(0,0,currentlvl->outer_radius, currentlvl->outer_radius + 3000);

	// DEBUG SEGMENTS
	llist_begin_loop(ll_floor);
	while(llist_hasnext(ll_floor)) {
		cpShape *seg = (cpShape *)llist_next(ll_floor);
		cpVect a = cpSegmentShapeGetA(seg);
		cpVect b = cpSegmentShapeGetB(seg);
		cpFloat r = cpSegmentShapeGetRadius(seg);
		draw_line(TEX_GLOW_DOT, a.x, a.y, b.x, b.y, r);
	}
	llist_end_loop(ll_floor);
}

static void SPACE_draw(void)
{
	space_rendering_map = 1;
	/* draw background */
	drawStars();

	position_time += dt * position_dir * 0.1;
	if(position_time > 1 || position_time < 0){
		position_dir *= -1;
	}
	position_now = tween_move_f(position_start, position_end, position_time, ExponentialEaseInOut);

	draw_box(0,position_now, 100, 100, 0, 1);

	/* translate view */
	draw_load_identity();
	draw_scale(current_camera->zoom,current_camera->zoom,1);
	draw_translate(-current_camera->x, -current_camera->y, 0.0f);

	/* draw tilemap */
	tilemap_render(currentlvl->tiles);

	setTextAngle(0);
	/* draw all objects */
	instance_iterate(render_instances, NULL);
	draw_deck();

	/* draw particle effects */
	particles_draw(parti);

	space_rendering_map = 0;
	draw_gui();
}

static float radar_x; //TODO cleanup this non-object oriented code
static float radar_y;
static void plot_on_radar(instance *obj, int *x_ref)
{
	minimap *m = obj->components[CMP_MINIMAP];
	if(m != NULL){
		float r = 63;
		float x = (obj->body->p.x - *x_ref + currentlvl->left)*2*M_PI/currentlvl->width + M_PI / 2;
		float y = 1-(obj->body->p.y / currentlvl->height);
		r = 50+r*y;
		float px = cos(x)*r;
		float py = sin(x)*r;
		draw_color4f(m->c.r, m->c.g, m->c.b, 0.5);
		draw_box(radar_x + px, radar_y + py, m->size, m->size, x*(180/M_PI), 1);
	}
}

static void radar_draw(float x, float y)
{
	int offset = se_distance_to_player(0);
	radar_x = x;
	radar_y = y;
	draw_color4f(0.3, 0.5, 0.7, 0.6);
	draw_donut(radar_x, radar_y, 50, 110);
	instance_iterate_comp(CMP_MINIMAP, (void (*)(instance *, void *))plot_on_radar, &offset);
}

void draw_gui(void)
{
	/* reset transform matrix */
	draw_load_identity();
	draw_color4f(1,1,1,1);

	obj_player *player = ((obj_player*)instance_first(obj_id_player));

	setTextAngle(0); // TODO don't use global variables for setting font properties
	setTextAlign(TEXT_LEFT);
	setTextSize(35);

	if (gamestate != LEVEL_START) {
		radar_draw(-GAME_WIDTH/2 + 120, GAME_HEIGHT/2 - 175);

		if (player) {

		/* simple score animation */
		char score_temp[20];
		static int score_anim = 0;
		static int score_adder = 1;
		if (score_anim + score_adder < player->coins) {
			score_anim += score_adder;
			score_adder += 11;
		} else {
			score_anim = player->coins;
			score_adder = 1;
		}

		sprintf(score_temp,"%d",score_anim);
		draw_color4f(0,0,0,1);
		font_drawText(-GAME_WIDTH/2+20+4,GAME_HEIGHT/2 - 26-4,score_temp);
		draw_color4f(1,1,1,1);
		font_drawText(-GAME_WIDTH/2+20,GAME_HEIGHT/2 - 26,score_temp);
		}

		draw_color4f(0.5,0.1,0.1,1);
		setTextSize(20);
		char goals_left[100];
		sprintf(goals_left, "%d",
				instance_count(obj_id_factory)+
				instance_count(obj_id_turret)+
				instance_count(obj_id_tank));
		font_drawText(-GAME_WIDTH/2+20,-GAME_HEIGHT/2 + 100,goals_left);

		//draw_color4f(1,1,1,1);
		//setTextSize(20);
		//char particles_temp[20];
		//char particles2_temp[20];
		//char particles3_temp[20];
		//sprintf(particles_temp,"%d",particles_active);
		//sprintf(particles2_temp,"%d",available_particle_counter);
		//sprintf(particles3_temp,"%d",(available_particle_counter + particles_active));
		//font_drawText(-WIDTH/2+20,HEIGHT/2 - 100,particles_temp);
		//font_drawText(-WIDTH/2+20,HEIGHT/2 - 140,particles2_temp);
		//font_drawText(-WIDTH/2+20,HEIGHT/2 - 180,particles3_temp);

		//char pos_temp[20];
		//sprintf(pos_temp,"X: %4.0f Y: %4.0f",player->data.body->p.x,player->data.body->p.y);
		//font_drawText(-WIDTH/2+15,-HEIGHT/2+12,pos_temp);

		//setTextAlign(TEXT_RIGHT);
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
		font_time2str(time_temp, game_time);

		setTextSize(40);
		setTextAlign(TEXT_CENTER);

		draw_color4f(0,0,0,1);
		font_drawText(4, GAME_HEIGHT/2 - 29 - 4, time_temp);
		draw_color4f(1,1,1,1);
		font_drawText(0, GAME_HEIGHT/2 - 29, time_temp);
	}

	switch(gamestate) {
	case LEVEL_START:
		draw_color4f(1,1,1,1);
		setTextAlign(TEXT_CENTER);

#if GOT_TOUCH
		setTextSize(40);
		if (((int)state_timer)%2) {
			font_drawText(0, 0, "TOUCH JOYSTICKS");
		}
		setTextSize(25);
		setTextAngle(90);
		font_drawText(GAME_WIDTH/2-100, 0, "CO-OP PLAYER 2");
		setTextAngle(270);
		font_drawText(-GAME_WIDTH/2+100, 0, "CO-OP PLAYER 1");

#define STR_SPACE_START "STEER - SINGLE PLAY - SHOOT"
		setTextAngle(0);
		setTextSize(25);
		draw_color4f(0,0,0,1);
		font_drawText(4, -GAME_HEIGHT/2+110-4, STR_SPACE_START);
		draw_color4f(1,1,1,1);
		font_drawText(0, -GAME_HEIGHT/2+110, STR_SPACE_START);
#else
		setTextSize(70);
		font_drawText(0, 0, "GET READY!");
#endif
		/* no break */
	case LEVEL_RUNNING:
		break;
	case LEVEL_CLEARED:
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
		//TODO draw button texture for arcade mode
		if(button_down){
			cpVect t = cpv(0,0-GAME_HEIGHT/4);
			//draw_texture(TEX_BUTTON_DOWN,&t,TEX_MAP_FULL,300,300,0);
		}else{
			cpVect t = cpv(0,-5.5-GAME_HEIGHT/4);
			//draw_texture(TEX_BUTTON,&t,TEX_MAP_FULL,300,300,0);
		}
#endif
		setTextSize(120);
		break;
	case LEVEL_STATE_COUNT:
		/* invalid value */
		break;
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
static void stars_init(void)
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
void drawStars(void)
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


static void sticks_init(void) {
	joystick_release(joy_p1_left);
	joystick_release(joy_p1_right);
	joystick_release(joy_p2_left);
	joystick_release(joy_p2_right);

	((touchable *)joy_p1_left)->visible = 1;
	((touchable *)joy_p1_right)->visible = 1;

#if GOT_TOUCH
	if (multiplayer) {
		((touchable *)joy_p2_left)->visible = 1;
		((touchable *)joy_p2_right)->visible = 1;
	} else {
#else
	multiplayer = 0;
	{
#endif
		((touchable *)joy_p2_left)->visible = 0;
		((touchable *)joy_p2_right)->visible = 0;
	}
}

static void sticks_hide(void) {
	((touchable *)joy_p1_left)->visible = 0;
	((touchable *)joy_p1_right)->visible = 0;
	((touchable *)joy_p2_left)->visible = 0;
	((touchable *)joy_p2_right)->visible = 0;
}

void space_init_level(int space_station, int deck)
{
	static obj_player *player;

	multiplayer = -1;
	sticks_init();

	if(player==NULL){
		obj_param_player default_player = {
				.max_hp = 200,
				.gun_cooldown = 0.2f,
				.cash_radius = 250
		};
		player = (obj_player *)instance_create(obj_id_player, &default_player, 0,0,0,0);
	} else {
		player->disable = 1;
		//player->hp_bar.value = player->param->max_hp;

		player->data.TYPE->call.init((instance*) player);
		//player->aim_speed += 0.3*deck;
		//player->rotation_speed += 0.3*deck;
		if(deck >= 6){
			player->gun_level = 3;
		}
	}
	//TODO manage persistent objects(like player) in a better way, instead of removing and then re-adding
	instance_clear();
	instance_add((instance*)player);

	/* set player specs based on selected upgrades */
	player->force = engines[engine_index].force;
	cpBodySetVelLimit(player->data.body, engines[engine_index].max_speed);
	cpBodySetMass(player->data.body, upg_total_mass);
	player->param.gun_cooldown = 1 / weapons[weapon_index].lvls[weapons[weapon_index].level].firerate;
	player->bullet_dmg = weapons[weapon_index].lvls[weapons[weapon_index].level].damage;
	player->bullet_type = object_by_name(weapons[weapon_index].obj_name);
	player->hp_bar.max_hp = armors[armor_index].max_hp;
	player->hp_bar.value = player->hp_bar.max_hp;

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
	player->data.body->v.x = 0;
	player->data.body->v.y = -10;

	/* static ground */
	cpBody *staticBody = space->staticBody;

	/* remove floor and ceiling */
	if(llist_size(ll_floor) > 0 && ceiling != NULL){
		cpSpaceRemoveStaticShape(space,ceiling);
		llist_begin_loop(ll_floor);
		while(llist_hasnext(ll_floor)) {
			cpShape *seg = (cpShape *)llist_next(ll_floor);
			cpSpaceRemoveStaticShape(space, seg);
		}
		llist_end_loop(ll_floor);
	}

	static const int segments = 100;
	static const float seg_radius = 10;
	static const float seg_length = 5000;
	int i;
	for (i = 0; i < segments; ++i) {
		cpVect angle = cpvforangle(2 * M_PI * i / segments);
		cpVect n = cpvmult(cpvperp(angle), seg_length);
		cpVect p = cpvmult(angle, currentlvl->outer_radius + seg_radius);

		cpVect a = cpvadd(p,cpvneg(n));
		cpVect b = cpvadd(p,n);

		cpShape *seg = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, a, b, seg_radius)); // ground level at 0
		cpShapeSetFriction(seg, 0.9f);
		cpShapeSetCollisionType(seg, ID_GROUND);
		cpShapeSetElasticity(seg, 0.7f);

		llist_add(ll_floor, seg);
	}

	ceiling = cpSpaceAddShape(space, cpCircleShapeNew(staticBody, currentlvl->inner_radius,cpvzero));
	cpShapeSetFriction(ceiling, 0.9f);
	cpShapeSetCollisionType(ceiling, ID_GROUND);
	cpShapeSetElasticity(ceiling, 0.7f);

	/*
	 * puts all shapes in correct position
	 */
	update_all();

	particles_clear(parti);
}

static void on_enter(void)
{
	game_paused = 0;
}

static void game_over(void)
{
	lvl_cleared=0;
	statesystem_push_state(state_leveldone);
}

static void pause_game(void)
{
	statesystem_push_state(state_pause);
	game_paused = 1;
}

static void sdl_event(SDL_Event *event)
{
	SDL_Scancode key;
	switch(event->type) {
	case SDL_KEYDOWN:
		switch (gamestate) {
		case LEVEL_PLAYER_DEAD:
			//TODO implement button for this:
			if(key == KEY_RETURN_2 || key == KEY_RETURN_1){//state_timer > 3){
				game_over();
			}
			break;
		default:
			break;
		}
		break;
	}
}

static void on_pause(void)
{
	if (gamestate == LEVEL_RUNNING) {
		pause_game();
	}
}

static void on_leave(void)
{

}

static void destroy(void)
{
	particles_destroy_system(parti);
	cpSpaceDestroy(space);
	joystick_free(joy_p1_left);
	joystick_free(joy_p1_right);
	llist_destroy(ll_floor);
}

void space_init(void)
{
	parti = particles_create_system();
	statesystem_register(state_space,LEVEL_STATE_COUNT);
    statesystem_add_inner_state(state_space,LEVEL_START,level_start,NULL);
    statesystem_add_inner_state(state_space,LEVEL_RUNNING,level_running,NULL);
    statesystem_add_inner_state(state_space,LEVEL_PLAYER_DEAD,level_player_dead,NULL);
    statesystem_add_inner_state(state_space,LEVEL_CLEARED,level_cleared,NULL);
    statesystem_add_inner_state(state_space,LEVEL_TRANSITION,level_transition,NULL);

    btn_pause = button_create(SPRITE_BUTTON_PAUSE, 0, "", GAME_WIDTH/2-85, GAME_HEIGHT/2-77, 80, 80);
    button_set_callback(btn_pause, statesystem_pause, 0);
    button_set_enlargement(btn_pause, 2.0f);
    button_set_hotkeys(btn_pause, KEY_ESCAPE, SDL_SCANCODE_PAUSE);
    statesystem_register_touchable(this, btn_pause);

    ll_floor = llist_create();
    ceiling = NULL;

	cpVect gravity = cpv(0, -600);
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	cpSpaceSetDamping(space, 0.99);

	extern void collisioncallbacks_init(void);
    collisioncallbacks_init();

    stars_init();

    float h = GAME_HEIGHT*0.5;

    joy_p1_left = joystick_create(0, 120, 2, -GAME_WIDTH/2 + 170, -0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p1_right = joystick_create(0, 120, 2, GAME_WIDTH/2 - 170, -0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p2_left = joystick_create(0, 120, 2, -GAME_WIDTH/2 + 170, +0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p2_right = joystick_create(0, 120, 2, GAME_WIDTH/2 - 170, +0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);

    joystick_set_hotkeys(joy_p1_left, KEY_LEFT_1,KEY_UP_1,KEY_RIGHT_1,KEY_DOWN_1);
    joystick_set_hotkeys(joy_p1_right, KEY_LEFT_2,KEY_UP_2,KEY_RIGHT_2,KEY_DOWN_2);

    statesystem_register_touchable(this, joy_p1_left);
    statesystem_register_touchable(this, joy_p1_right);
    statesystem_register_touchable(this, joy_p2_left);
    statesystem_register_touchable(this, joy_p2_right);

    state_timer = 10;
	change_state(LEVEL_START);
}


float getGameTime(void)
{
	return game_time;
}

int getPlayerScore(void)
{
	obj_player *player = ((obj_player*)instance_first(obj_id_player));
	if (player != NULL)
		return player->coins;
	else
		return -1;
}

void input(void)
{
#if !GOT_TOUCH
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

void space_start_demo(void) {
	//TODO set and reset all per-game variables
	multiplayer = 0;

	current_camera = &space_cam;

	space_init_level(1,1);
	statesystem_set_state(state_space);
}

void space_start_multiplayer(void) {
	//TODO set and reset all per-game variables
	multiplayer = 1;

	space_init_level(1,1);
	statesystem_set_state(state_space);
}

void space_restart_level(void)
{
	statesystem_set_state(state_space);
	space_init_level(currentlvl->station, currentlvl->deck);
}

void space_next_level(void)
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
