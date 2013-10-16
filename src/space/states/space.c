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

STATE_ID state_space;

/**
 * The global space state
*/

static void update_camera_position(void);

static void sticks_init(void);
static void sticks_hide(void);

static void radar_draw(float x, float y);

static obj_player * space_create_player(int id);

view *view_p1, *view_p2;

static button btn_pause;
static int game_paused = 0;

static float game_time;


/* level boundaries */
static LList *ll_floor_segs;
static cpShape *ceiling;

/* level data */
level *currentlvl;

static void input(void);

static layer_system *layersystem;

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
static obj_player *player1;
static obj_player *player2;

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

static void draw_gui(view *cam);

#define GRAVITY 600.0f
#define DAMPING 0.8f


/* The state timer */
static float state_timer = 0;

/**
 * Inner state functions
 */

#define QUICK_ENTER 0

void setup_singleplay(void);
void setup_multiplay(void);

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

	view_p2->enabled = 0;
	if (p1_ready && p2l && p2r) {
		start = 1;
		multiplayer = 1;
	} else {
		multiplayer = 0;
		start = (p1_ready && (!(p2l || p2r))) || (!GOT_TOUCH && (state_timer > 1.5));
	}

	if (start) {
		if (multiplayer) {
			setup_multiplay();
		}

		sticks_init();

		if (player1) {
			player1->disable = 0;
		}

		if (player2) {
			player2->disable = 0;
		}

		change_state(LEVEL_RUNNING);

		//TMP test
	}
}
static void level_running(void)
{
	/* update game time */
	game_time += dt;

	input();

	//update_all();
	if(player1 && player1->hp_bar.value <= 0 && (player2 == NULL || player2->hp_bar.value <=0 )){
		//player->disable = 1;
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
	//update_all();

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
	//update_all();

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
	state_set_inner_state(state_space,state);
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

static void update_camera_position(void)
{
    cpVect v_pos;
    float v_rot;

    if (player1) {
    	v_pos = player1->data.body->p;
    	view_update_zoom(view_p1, player1->data.body->p, currentlvl->height);
    	v_rot = -se_tangent(v_pos);
    	view_update(view_p1, v_pos, v_rot);
    }

    if (player2 && multiplayer) {
    	v_pos = player2->data.body->p;
    	view_update_zoom(view_p2, player2->data.body->p, currentlvl->height);
    	v_rot = -se_tangent(v_pos);
    	view_update(view_p2, v_pos, v_rot);
    }
}

static void draw_deck()
{
	/* draw ceiling and floor */
	draw_color4f(0,0,0,0.6);
	draw_circle(cpvzero, currentlvl->inner_radius);
	draw_donut(cpvzero, currentlvl->outer_radius, currentlvl->outer_radius + 3000);
}

static void draw(void)
{
	draw_color4f(1,1,1,1);
	//TODO move out to statesystem!
	layersystem_render(layersystem, current_view->p);

	//drawStars();

	/* draw tilemap */
	tilemap_render(currentlvl->tiles);
	draw_deck();

	//draw_light_map();
}


#define RADAR_SIZE 120.0f
static void plot_on_radar(instance *obj, void *unused)
{
	minimap *m = obj->components[CMP_MINIMAP];
	if(m != NULL){
		cpVect p = cpvmult(obj->body->p, 1 / (currentlvl->outer_radius - 5 * 64) * RADAR_SIZE);
		draw_color4f(m->c.r, m->c.g, m->c.b, 0.5);
		draw_box(p, cpv(m->size, m->size), cpvtoangle(p), 1);
	}
}

static void radar_draw(float x, float y)
{
	obj_player *player = current_view == view_p1 ? player1 : player2;
	draw_push_matrix();
	if (player) {
		draw_translate(x, y);
		draw_rotate(-se_tangent(player->data.body->p));
	}
	draw_color4f(0.7, 0.3, 0.3, 0.6);
	draw_donut(cpvzero, (currentlvl->inner_radius + 2 * 64) / currentlvl->outer_radius * RADAR_SIZE, RADAR_SIZE);
	instance_iterate_comp(CMP_MINIMAP, (void (*)(instance *, void *))plot_on_radar, NULL);
	draw_pop_matrix();
}

void draw_gui(view *cam)
{
	setTextAngle(0);
	//todo use current view's port size instead of using cam->game_width and cam->game_height! (?or not)
	draw_color4f(1,1,1,1);

	setTextAngle(0); // TODO don't use global variables for setting font properties
	setTextAlign(TEXT_LEFT);
	setTextSize(35);

	if (gamestate != LEVEL_START) {
		/*
		obj_player *player = current_view == view_p1 ? player1 : player2;
		if (player) {

		// simple score animation
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
		*/

		radar_draw(-cam->view_width/2 + 120, cam->view_height/2 - 175);

		int score_anim = 0;
		obj_player *player = current_view == view_p1 ? player1 : player2;
		if (player) score_anim = player->coins;
		char score_temp[20];
		sprintf(score_temp,"%d",score_anim); //score_anim
		draw_color4f(0,0,0,1);
		font_drawText(-cam->view_width/2+20+4,cam->view_height/2 - 26-4,score_temp);
		draw_color4f(1,1,1,1);
		font_drawText(-cam->view_width/2+20,cam->view_height/2 - 26,score_temp);
		//}

		draw_color4f(0.5,0.1,0.1,1);
		setTextSize(20);
		char goals_left[100];
		sprintf(goals_left, "%d",
				instance_count(obj_id_factory)+
				instance_count(obj_id_turret)+
				instance_count(obj_id_tank));
		font_drawText(-cam->view_width/2+20,-cam->view_height/2 + 100,goals_left);

		draw_color4f(1,1,1,1);
		setTextSize(15);
		char level_temp[20];
		setTextAlign(TEXT_CENTER);
		//		sprintf(level_temp,"STATION: %d DECK: %d", currentlvl->station, currentlvl->deck);
		sprintf(level_temp,"LEVEL %d", currentlvl->deck);
		font_drawText(0, -cam->view_height/2+24, level_temp);

		setTextAlign(TEXT_RIGHT);

#if !ARCADE_MODE
		font_drawText(cam->view_width/2 - 15, cam->view_height/2 - 20, fps_buf);
#endif
		if (!multiplayer) {
			char time_temp[20];
			font_time2str(time_temp, game_time);

			setTextSize(40);
			setTextAlign(TEXT_CENTER);

			draw_color4f(0,0,0,1);
			font_drawText(4, cam->view_height/2 - 29 - 4, time_temp);
			draw_color4f(1,1,1,1);
			font_drawText(0, cam->view_height/2 - 29, time_temp);
		}
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
		setTextAngle(WE_PI_2);
		font_drawText(cam->view_width/2-100, 0, "CO-OP PLAYER 2");
		setTextAngle(WE_3PI_2);
		font_drawText(-cam->view_width/2+100, 0, "CO-OP PLAYER 1");

#define STR_SPACE_START "STEER - SINGLE PLAY - SHOOT"
		setTextAngle(0);
		setTextSize(25);
		draw_color4f(0,0,0,1);
		font_drawText(4, -cam->view_height/2+110-4, STR_SPACE_START);
		draw_color4f(1,1,1,1);
		font_drawText(0, -cam->view_height/2+110, STR_SPACE_START);
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
			cpVect t = cpv(0,0-cam->view_height/4);
			//draw_texture(TEX_BUTTON_DOWN,&t,TEX_MAP_FULL,300,300,0);
		}else{
			cpVect t = cpv(0,-5.5-cam->view_height/4);
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
#define star_count 50
#else
#define star_count 50
#endif

static int stars_x[star_count];
static int stars_y[star_count];
static float stars_size[star_count];
static void stars_init(void)
{
	//init stars
	int i;
	for (i=0; i<star_count; i++) {
		stars_x[i] = rand()%(GAME_WIDTH*2) - GAME_WIDTH; // make sure that radius is greater than WIDTH * sqrt(2)
		stars_y[i] = rand()%(GAME_WIDTH*2) - GAME_WIDTH;
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
	draw_push_matrix();

	static float spaceship_angle;
	spaceship_angle -= 0.01f * WE_2PI*dt;
	float cam_angle = spaceship_angle + view_p1->rotation;
	draw_rotate(cam_angle);

	int i;
	draw_color4f(1,1,1,1);
	for (i = 0; i < star_count; i++) {
		cpVect p = {stars_x[i], stars_y[i]};
		cpVect s = {stars_size[i],stars_size[i]};
		draw_box_append(p,s,0,1);
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

// TODO: REMOVE
static obj_param_staticpolygon polytest = {
		.texture = 0,
		.outline = 1,
		.scale = 700,
		.texture_scale = 1,
};

void space_init_level(int space_station, int deck)
{

	multiplayer = -1;
	sticks_init();

	objectsystem_clear();

	player1 = NULL;
	player2 = NULL;


	//TODO manage persistent objects(like player) in a better way, instead of removing and then re-adding


	if (currentlvl != NULL) {
		level_unload(currentlvl);
	}

	currentlvl = level_load(space_station,deck);

	if (currentlvl == NULL) {
		SDL_Log( "space_level_init failed!\n");
		exit(-1);
	}

	player1 = space_create_player(1);
	setup_singleplay();

	change_state(LEVEL_START);

	/* static ground */
	cpBody *staticBody = current_space->staticBody;

	/* remove floor and ceiling */
	if(llist_size(ll_floor_segs) > 0 && ceiling != NULL){
		cpSpaceRemoveStaticShape(current_space,ceiling);
		llist_clear(ll_floor_segs);
	}

	float r_in = currentlvl->inner_radius;
	float r_out = currentlvl->outer_radius;
	float r_ceil = r_in + 64 * (r_out - r_in) / currentlvl->height;
	float r_floor = r_out - 5 * 64 * (r_out - r_in) / currentlvl->height;

	static const int segments = 100;
	static const float seg_radius = 50;
	static const float seg_length = 300;
	int i;
	for (i = 0; i < segments; ++i) {
		cpVect angle = cpvforangle(2 * M_PI * i / segments);
		cpVect n = cpvmult(cpvperp(angle), seg_length);
		cpVect p = cpvmult(angle, r_floor + seg_radius);

		cpVect a = cpvadd(p,cpvneg(n));
		cpVect b = cpvadd(p,n);

		cpShape *seg = cpSpaceAddShape(current_space, cpSegmentShapeNew(staticBody, a, b, seg_radius)); // ground level at 0
		cpShapeSetFriction(seg, 0.9f);
		cpShapeSetCollisionType(seg, ID_GROUND);
		cpShapeSetElasticity(seg, 0.7f);

		llist_add(ll_floor_segs, seg);
	}
	ceiling = cpSpaceAddShape(current_space, cpCircleShapeNew(staticBody, r_ceil, cpvzero));
	cpShapeSetFriction(ceiling, 0.9f);
	cpShapeSetCollisionType(ceiling, ID_GROUND);
	cpShapeSetElasticity(ceiling, 0.7f);

	/*
	 * puts all shapes in correct position
	 */
	//update_all();

	particles_clear(current_particles);

	polytest.texture = TEX_METAL;
	polytest.shape_id = POLYSHAPE_TURRET;
	polytest.texture_scale = 0.4;
	instance_create(obj_id_staticpolygon, &polytest, cpv(0,800), cpvzero);
	polytest.shape_id = POLYSHAPE_RAMP;
	polytest.texture_scale = 0.6;
	instance_create(obj_id_staticpolygon, &polytest, cpv(0,-800), cpvzero);
	polytest.shape_id = POLYSHAPE_TANK;
	polytest.texture_scale = 1;
	polytest.outline = 0;
	instance_create(obj_id_staticpolygon, &polytest, cpv(800,0), cpvzero);
	polytest.shape_id = POLYSHAPE_TURRET;
	polytest.texture_scale = 4;
	polytest.scale = 1000;
	obj_staticpolygon *sp = instance_create(obj_id_staticpolygon, &polytest, cpv(-800,0), cpvzero);
	cpBodySetAngle(sp->body, 312);
	cpSpaceReindexShapesForBody(current_space, sp->body);
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
	llist_destroy(ll_floor_segs);
	joystick_free(joy_p1_left);
	joystick_free(joy_p1_right);
}

static void remove_static(cpShape *shape)
{
	cpSpaceRemoveStaticShape(current_space, shape);
}


static cpVect space_particle_g_func(cpVect pos)
{
    return cpvnormalize_safe(pos);
}

void space_init(void)
{
	current_particles = particlesystem_new();
	particle_set_gravity_func(current_particles, space_particle_g_func );

	layersystem = layersystem_new(40);
	int i;
	for(i = 0; i < layersystem->num_layers; i++){
		//float depth =  1 + 10*tan((1.0f*i/layersystem->num_layers)*WE_PI_2);
		float f = 0.1 + 0.9 * i / (layersystem->num_layers); // -> 0 = nearest, 1 = furthest
		layersystem_set_layer_parallax(layersystem, i, f, 1);
	}
	for(i = 0; i<150; i++){
		int layer = roundf(we_randf*(layersystem->num_layers-1));
		float size = 25 + (400+70) -(we_randf*70 + layer*10);
		layersystem_add_sprite(layersystem, layer, SPRITE_SPIKEBALL, size, size, cpvmult(cpv(we_randf-0.5,we_randf-0.5),2600), we_randf*WE_2PI);
	}

	statesystem_register(state_space,LEVEL_STATE_COUNT);
    state_add_inner_state(state_space,LEVEL_START,level_start,NULL);
    state_add_inner_state(state_space,LEVEL_RUNNING,level_running,NULL);
    state_add_inner_state(state_space,LEVEL_PLAYER_DEAD,level_player_dead,NULL);
    state_add_inner_state(state_space,LEVEL_CLEARED,level_cleared,NULL);
    state_add_inner_state(state_space,LEVEL_TRANSITION,level_transition,NULL);

    view_p1 = state_view_get(state_space, 0);
    view_p2 = state_view_add(state_space);
    view_p2->enabled = 0;

    view_p1->GUI = draw_gui;
    view_p2->GUI = draw_gui;

    /*
    {
    	view *v = state_view_add(state_space); //TODO REMOVE TEST view
    	cpVect size = cpv(WINDOW_WIDTH/2,WINDOW_HEIGHT/2);
    	view_set_port(v,cpvadd(size,cpvmult(size,-0.5)),size,1);
    	//v->GUI = draw_gui;
    	v->zoom = 0.2;
    }
    */

    btn_pause = button_create(SPRITE_BUTTON_PAUSE, 0, "", GAME_WIDTH/2-85, GAME_HEIGHT/2-77, 80, 80);
    button_set_callback(btn_pause, (btn_callback) statesystem_pause, 0);
    button_set_enlargement(btn_pause, 2.0f);
    button_set_hotkeys(btn_pause, KEY_ESCAPE, SDL_SCANCODE_PAUSE);
    state_register_touchable_view(view_p1, btn_pause);

    ll_floor_segs = llist_create();
    llist_set_remove_callback(ll_floor_segs, (ll_remove_callback) remove_static);
    ceiling = NULL;

    state_enable_objects(state_space, 1);
    state_enable_particles(state_space, 1);

	cpSpaceSetGravity(current_space, cpv(GRAVITY,0));
	cpSpaceSetDamping(current_space, DAMPING);

	extern void collisioncallbacks_init(void);
    collisioncallbacks_init();

    stars_init();

    float h = GAME_HEIGHT*0.5;

    joy_p1_left = joystick_create(0, 120, 2, -GAME_WIDTH/2 + 170, -0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p1_right = joystick_create(0, 120, 2, GAME_WIDTH/2 - 170, -0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p2_left = joystick_create(0, 120, 2, -GAME_WIDTH/2 + 170, +0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p2_right = joystick_create(0, 120, 2, GAME_WIDTH/2 - 170, +0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);

    state_register_touchable_view(view_p1, joy_p1_left);
    state_register_touchable_view(view_p1, joy_p1_right);
    state_register_touchable_view(view_p2, joy_p2_left);
    state_register_touchable_view(view_p2, joy_p2_right);

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
		view_p1->mode = 1;
	} else if (keys[SDL_SCANCODE_F2]) {
		view_p1->mode = 2;
	} else if (keys[SDL_SCANCODE_F3]) {
		view_p1->mode = 3;
	} else if (keys[SDL_SCANCODE_F4]) {
		view_p1->mode = 4;
	} else if (keys[SDL_SCANCODE_F5]) {
		view_p1->mode = 5;
	} else if (keys[SDL_SCANCODE_F6]) {
		view_p1->mode = 6;
	} else if (keys[SDL_SCANCODE_F11]) {
		game_time = currentlvl->timelimit;
		return;
	}

	if (keys[SDL_SCANCODE_M]) {
		keys[SDL_SCANCODE_M] = 0;
		multiplayer = !multiplayer;
		if (multiplayer) {
			setup_multiplay();
		} else {
			setup_singleplay();
		}
	}

	if(view_p2){
		view_p2->mode = view_p1->mode;
	}

	/* DEBUG KEYS*/
#if !ARCADE_MODE
	if(keys[SDL_SCANCODE_G]){
		keys[SDL_SCANCODE_G] = 0;
		cpVect gravity = cpv(0, -2);
		cpSpaceSetGravity(current_space, gravity);
	}
#endif
#endif
}

void setup_singleplay(void)
{
	multiplayer = 0;
	//TODO create two players
	cpVect size = cpv(WINDOW_WIDTH,WINDOW_HEIGHT);
	view_set_port(view_p1, cpvzero, size, 0);
	view_p2->enabled = 0;

	if(player2){
		instance_remove((instance*)player2);
		player2 = NULL;
	}

    joystick_set_hotkeys(joy_p1_left, KEY_LEFT_1,KEY_UP_1,KEY_RIGHT_1,KEY_DOWN_1);
    joystick_set_hotkeys(joy_p1_right, KEY_LEFT_2,KEY_UP_2,KEY_RIGHT_2,KEY_DOWN_2);

    float w = 340;
    float h = view_p1->view_height*0.5;

    touch_place((touchable *)btn_pause, view_p1->view_width/2-85, view_p1->view_height/2-77);
    joystick_reposition(joy_p1_left, 120, 2, -view_p1->view_width/2 + 170, -0.25*view_p1->view_height, w, h);
	joystick_reposition(joy_p1_right, 120, 2, view_p1->view_width/2 - 170, -0.25*view_p1->view_height, w, h);
    joystick_reposition(joy_p2_left, 120, 2, -view_p1->view_width/2 + 170, 0.25*view_p1->view_height, w, h);
    joystick_reposition(joy_p2_right, 120, 2, view_p1->view_width/2 - 170, 0.25*view_p1->view_height, w, h);

    LList ll_touchies = view_p1->touch_objects;
    if (!llist_contains(ll_touchies, joy_p2_left)) {
    	llist_add(ll_touchies, joy_p2_left);
    	llist_add(ll_touchies, joy_p2_right);
    }

    joy_p2_left->touch_data.visible = 1;
    joy_p2_right->touch_data.visible = 1;
}

static obj_player * space_create_player(int id)
{
	obj_param_player default_player = {
			.max_hp = 200,
			.gun_cooldown = 0.2f,
			.cash_radius = 250,
			.player_id = id
	};
	return (obj_player *)instance_create(obj_id_player, &default_player, cpvzero, cpvzero);
}

void setup_multiplay(void)
{
	multiplayer = 1;
	//TODO create and init two players
	cpVect size = cpv(WINDOW_WIDTH/2,WINDOW_HEIGHT);
	view_set_port(view_p1, cpvzero, size, 3);
	view_set_port(view_p2, cpv(WINDOW_WIDTH/2,0), size, 1);
	view_p2->enabled = 1;
    joystick_set_hotkeys(joy_p1_left, KEY_LEFT_1,KEY_UP_1,KEY_RIGHT_1,KEY_DOWN_1);
    joystick_set_hotkeys(joy_p1_right, SDL_SCANCODE_J,SDL_SCANCODE_I,SDL_SCANCODE_L,SDL_SCANCODE_K);
    joystick_set_hotkeys(joy_p2_left, KEY_LEFT_2,KEY_UP_2,KEY_RIGHT_2,KEY_DOWN_2);
    joystick_set_hotkeys(joy_p2_right, SDL_SCANCODE_KP_4,SDL_SCANCODE_KP_8,SDL_SCANCODE_KP_6,SDL_SCANCODE_KP_5);

    touch_place((touchable *)btn_pause, view_p1->view_width/2-85, view_p1->view_height/2-77);
	joystick_place(joy_p1_left, -view_p1->port_width/3, -view_p1->port_height/3);
	joystick_place(joy_p1_right, +view_p1->port_width/3, -view_p1->port_height/3);
	joystick_place(joy_p2_left, -view_p1->port_width/3, -view_p1->port_height/3);
	joystick_place(joy_p2_right, +view_p1->port_width/3, -view_p1->port_height/3);

    float w = 340;
    float h1 = view_p1->view_height*0.5;
    float h2 = view_p2->view_height*0.5;
	joystick_reposition(joy_p1_left, 80, 2, -view_p1->view_width/2 + 170, -0.25*view_p1->view_height, w, h1);
	joystick_reposition(joy_p1_right, 80, 2, view_p1->view_width/2 - 170, -0.25*view_p1->view_height, w, h1);
    joystick_reposition(joy_p2_left, 80, 2, -view_p2->view_width/2 + 170, -0.25*view_p2->view_height, w, h2);
    joystick_reposition(joy_p2_right, 80, 2, view_p2->view_width/2 - 170, -0.25*view_p2->view_height, w, h2);

    joy_p2_left->touch_data.visible = 1;
    joy_p2_right->touch_data.visible = 1;

    LList ll_touchies = view_p1->touch_objects;
    llist_remove(ll_touchies, joy_p2_left);
    llist_remove(ll_touchies, joy_p2_right);

    if (player2 == NULL) {
		player2 = space_create_player(2);
	}
}

void space_start_demo(int station, int deck)
{
	statesystem_set_state(state_space);
	//TODO set and reset all per-game variables
	multiplayer = 0;

	//view_p2->enabled = 0;

	space_init_level(station,deck);
}

/*
void space_start_multiplayer(int station, int deck) {
	statesystem_set_state(state_space);
	//TODO set and reset all per-game variables
	multiplayer = 1;

	space_init_level(station, deck);
}
*/

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
