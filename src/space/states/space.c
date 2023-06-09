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
#include "levelscreen.h"

#include "../tilemap.h"
#include "../spaceengine.h"

STATE_ID state_space;
#define STICK_RADIUS 120

/**
 * The global space state
*/

static void update_camera_position(void);

static void sticks_init(void);
static void sticks_hide(void);

obj_player * space_create_player(int id);

view *view_p1, *view_p2;

static button btn_pause;
static button btn_continue;

static float game_time;

/* level data */
static spacelvl *lvl_tmpl;
spacelvl *currentlvl;

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
#define DAMPING 0.99f

#define ARCADE_LVL_COUNT 7
#define ARCADE_SCORE_LVL (60 * 5 * 10) // in decisec
static int arcade_lvl_score = ARCADE_LVL_COUNT * ARCADE_SCORE_LVL;

static int continue_touched = 0, waiting_continue = 0;
#if !GOT_TOUCH
static void wait_continue(void){}
static void continue_func(void){}
#else
static void wait_continue(void)
{
	continue_touched = 0;
	waiting_continue = 1;
	btn_continue->enabled = 1;
}

static void continue_func(void)
{
	continue_touched = 1;
	waiting_continue = 0;
	btn_continue->enabled = 0;
}
#endif

/* The state timer */
static float state_timer = 0;

/**
 * Inner state functions
 */

#define QUICK_ENTER 1

void setup_singleplay(void);
void setup_multiplay(void);

static void level_start(void)
{
	state_enable_objects(state_space, 0);
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
		start = (p1_ready && (!(p2l || p2r))) || (!GOT_TOUCH && (state_timer > 0.8));
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

	state_enable_objects(state_space, 1);
	if(player1 && player1->hp_bar.value <= 0 && (player2 == NULL || player2->hp_bar.value <=0 )){
		//player->disable = 1;
		wait_continue();
		change_state(LEVEL_PLAYER_DEAD);
	}

	if ((instance_count(obj_id_factory) + instance_count(obj_id_tank) + instance_count(obj_id_turret) + instance_count(obj_id_robotarm)) == 0) {
		wait_continue();
		change_state(LEVEL_CLEARED);
	}
}

static void level_player_dead(void)
{
	obj_player *player = (obj_player *)instance_first(obj_id_player);
	//update_all();

	static int tmp_atom = 0;
#if GOT_TOUCH
	if (continue_touched && !tmp_atom) {
#else
	if (state_timer > 1 && !tmp_atom) {
#endif
		tmp_atom = 1;
		sticks_hide();

#if !ARCADE_MODE
		leveldone_status(0, player->coins, game_time);
		statesystem_push_state(state_leveldone);
#endif
	} else {
		tmp_atom = 0;
	}
}

static void level_cleared(void)
{
	obj_player *player = (obj_player *)instance_first(obj_id_player);
	//update_all();
//TODO split player1 and player2 coin amount?
	static int tmp_atom = 0;
#if GOT_TOUCH
	if (continue_touched && !tmp_atom) {
#else
	if (state_timer > 2 && !tmp_atom) {
#endif
		//float p = (player->hp_bar.value / player->hp_bar.max_hp);
		//player->coins += (int)(p*p * 4)*4 * 25;

		sticks_hide();

#if !ARCADE_MODE
		//TODO set star rating based on missions
		//leveldone_status(1 + player->coins / (4000.0 + 1000 * currentlvl->deck), player->coins, game_time);
		statesystem_push_state(state_leveldone);
#else
#endif
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
	if (keys[SDL_SCANCODE_F9]) {
	} else if (keys[SDL_SCANCODE_F8]) {
		if (player1) instance_destroy((instance *)player1);
		if (player2) instance_destroy((instance *)player2);
		keys[SDL_SCANCODE_F8] = 0;
	}
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

static void view_update_zoom(view *cam, cpVect pos)
{
	cam->p = cpvadd(cam->p, pos);
}
static void view_mode_update(view *v, int mode, instance *ins)
{
	float rot = 0;
	float zoom = 1;
	cpVect pos = ins->body->p;
	if (mode == 1) {
		rot = -se_tangent(pos);
		zoom = 1.0f * v->view_height / 2000;
	} else if (mode == 2) {
		zoom = 1.0f * v->view_height / 2000;
	} else if (mode == 3) {
		zoom = 1.0f * v->view_height / 2000;
		float boxsize = 20;
		float delta_box = 200;
		cpFloat dist = cpvdist(v->p, pos);
		if (dist > boxsize) {
			cpFloat ddi = dist - boxsize;
			pos = cpvadd(v->p,
					cpvmult(cpvsub(pos, v->p),
							((ddi * (ddi / delta_box)) / dist)));
		} else {
			pos = v->p;
		}
	} else if (mode == 4) {
		rot = -se_tangent(pos);
		zoom = 1.0f * v->view_height / (lvl_tmpl->outer_radius * 2);
		pos = cpvzero;
	} else if (mode == 5) {
		zoom = 1.0f * v->view_height / (lvl_tmpl->outer_radius * 2);
		pos = cpvzero;
	}
	v->zoom = zoom;
	view_update_zoom(v, pos);
	view_update(v, pos, rot);
}

static void update_camera_position(void)
{
    if (player1) {
    	instance *tank = instance_first(obj_id_tank);
    	if (tank && keys[SDL_SCANCODE_LCTRL]) {
    		view_mode_update(view_p1, player_camera_mode, tank);
    	} else {
			view_mode_update(view_p1, player_camera_mode, (instance *)player1);
    	}
    }

    if (player2 && multiplayer) {
    	view_mode_update(view_p2, player_camera_mode, (instance *)player2);
    }
}

void space_draw_deck(spacelvl *lvl)
{
	/* draw ceiling and floor */
	//draw_color4f(0.3,0.3,0.3,0.6);
	//draw_color4f(0.3,0.6,0.6,0.6);
	//Color col_back = {20,80,20,160};
	Color col_back = {50,150,50,255};
	draw_color(col_back);
	draw_circle(RLAY_BACK_BACK, cpvzero, lvl->inner_radius+5); //TMP margin to cover holes between tilemap and floor and ceiling
	//draw_donut(RLAY_BACK_BACK, cpvzero, lvl->outer_radius-5, lvl->outer_radius + 300);//0);
}

static void draw(void)
{
	draw_color4f(1,1,1,1);

	//drawStars();

	/* draw tilemap */
	//tilemap_render(RLAY_BACK_BACK, currentlvl->tiles);
	space_draw_deck(lvl_tmpl);
	tilemap_render(&currentlvl->tm);
	//draw_light_map();
}


#define RADAR_SIZE 120.0f
static void plot_on_radar(instance *obj, void *unused)
{
	minimap *m = obj->components[CMP_MINIMAP];
	if(m != NULL){
		cpVect p = cpvmult(obj->body->p, RADAR_SIZE / currentlvl->outer_radius);
		draw_color(m->c);
		draw_box(0, p, cpv(m->size, m->size), cpvtoangle(p), 1);
	}
}

static void radar_draw(cpVect pos)
{
	obj_player *player = current_view == view_p1 ? player1 : player2;
	draw_push_matrix();
	draw_translatev(pos);
	if (player) {
		draw_rotate(-se_tangent(player->data.body->p));
	}
	draw_color4b(50,130,210,150);
	draw_donut(0, cpvzero, currentlvl->inner_radius / currentlvl->outer_radius * RADAR_SIZE, RADAR_SIZE);
	instance_iterate_comp(CMP_MINIMAP, (void (*)(instance *, void *))plot_on_radar, NULL);
	draw_pop_matrix();
}
#if ARCADE_MODE
#include "arcade/menu.h"
#endif

static void draw_gui(view *cam)
{
	setTextAngle(0);
	//todo use current view's port size instead of using cam->game_width and cam->game_height! (?or not)
	draw_color(COL_WHITE);

	setTextAngle(0); // TODO don't use global variables for setting font properties
	setTextAlign(TEXT_LEFT);
	setTextSize(35);
	float vleft = -cam->view_width/2;
	float vtop = cam->view_height/2;
	float vright = cam->view_width/2;
	float vbot = -cam->view_height/2;
	float vcenter = 0;

	if (gamestate != LEVEL_START) {
		radar_draw(cpv(vleft+130,vtop-190));

		draw_color(COL_WHITE);
		int score = 0;
		obj_player *player = current_view == view_p1 ? player1 : player2;
		if (player) score = player->coins;
		sprite_render_by_id(RLAY_GUI_FRONT, SPRITE_COIN, cpv(vleft+40,vtop-35),cpv(60,60),0);
		bmfont_left(FONT_SANS, cpv(vleft+70,vtop-60), 60, "%d", score);
		//bmfont_center(FONT_SANS, cpv(vcenter, vbot+24),0.7,"Level %d.%d", currentlvl->station,currentlvl->deck);

		//DEBUG INSTANCE COUNT
		draw_color4f(0.5,0.1,0.1,1);
		bmfont_left(FONT_SANS, cpv(vleft+70,vbot+100), 30, "%d",
				instance_count(obj_id_factory)+
				instance_count(obj_id_turret)+
				instance_count(obj_id_tank));

#if !ARCADE_MODE
		draw_color4f(1.0,0.4,0.4,1);
		bmfont_right(FONT_SANS, cpv(vright-15, vtop-40), 30, fps_buf);
#endif
		if (!multiplayer) {
			char str_time[20];
			font_time2str(str_time, game_time);
			draw_color(COL_WHITE);
			bmfont_left(FONT_SANS, cpv(vcenter-150, vtop-60),60,"%s",str_time);
		}
	}

	switch(gamestate) {
	case LEVEL_START:
		draw_color(COL_WHITE);
		setTextAlign(TEXT_CENTER);

#if GOT_TOUCH
		if (((int)state_timer)%2) {
			bmfont_center(FONT_SANS, cpv(vcenter, vcenter),1.5,"TOUCH JOYSTICKS");
		}
		setTextSize(25);
		setTextAngle(WE_PI_2);
		font_drawText(RLAY_GUI_FRONT,cam->view_width/2-100, 0, "CO-OP PLAYER 2");
		setTextAngle(WE_3PI_2);
		font_drawText(0,-cam->view_width/2+100, 0, "CO-OP PLAYER 1");
		setTextAngle(0);

		bmfont_center(FONT_SANS, cpv(vcenter, vbot+60), 60, "- Move - Single player - Fire -");
#else
		bmfont_center(FONT_SANS, cpv(vcenter, vcenter), 120, "GET READY!");
#endif
		/* no break */
	case LEVEL_RUNNING:
		break;
	case LEVEL_CLEARED:
#if ARCADE_MODE
		bmfont_center(FONT_SANS, cpv(vcenter, vcenter), 120, "LEVEL CLEARED!");
#endif
		break;
	case LEVEL_TRANSITION:
		break;
	case LEVEL_PLAYER_DEAD:

#if ARCADE_MODE
		draw_color(COL_RED);
		bmfont_center(FONT_SANS, cpv(vcenter, vcenter), 120, "GAME OVER!");
		draw_color4f(0.1,0.9,0.1,1);
		sprite_update(&spr_startbtn);
		sprite_render(RLAY_GUI_MID, &spr_startbtn, cpv(0,-0.5f*GAME_HEIGHT/2), 0);
#endif
		break;
	case LEVEL_STATE_COUNT:
		/* invalid value */
		break;
	}

	if (waiting_continue) {
		draw_color4f(1,1,1,1);
		bmfont_center(FONT_SANS, cpvzero, 60, "Tap for results!");
	}

}


#define star_count 50

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

static void sticks_init(void) {
	joystick_release(joy_p1_left);
	joystick_release(joy_p1_right);
	joystick_release(joy_p2_left);
	joystick_release(joy_p2_right);
#if !ARCADE_MODE
	((touchable *)joy_p1_left)->visible = 1;
	((touchable *)joy_p1_right)->visible = 1;
#else
	((touchable *)joy_p1_left)->visible = 0;
	((touchable *)joy_p1_right)->visible = 0;
#endif
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

/*
void space_init_level(char *name)
{
	if(currentlvl == NULL || strcmp(currentlvl->name, name) != 0 ) {
		currentlvl = spacelvl_parse(WAFFLE_DOCUMENTS, name);
	}
	space_init_level_from_level(currentlvl);
}
*/

void space_init_game(void)
{
	multiplayer = -1;
	sticks_init();

	player1 = NULL;
	player2 = NULL;

	player1 = (obj_player *)instance_first(obj_id_player);
	setup_singleplay();

	change_state(LEVEL_START);
}

void space_init_level_from_level(spacelvl * lvl)
{

}


static void game_over(void)
{
#if ARCADE_MODE
	arcade_lvl_score -= player1->coins/10;
	gameover_setstate(enter_name);
	statesystem_set_state(state_gameover);
#else
	statesystem_push_state(state_leveldone);
#endif
}

static void pause_game(void)
{
	statesystem_push_state(state_pause);
}

static int sdl_event(SDL_Event *event)
{
	SDL_Scancode key;
	EMITTER_ID em_id = NULL;
	switch(event->type) {
	case SDL_KEYDOWN:
        key = event->key.keysym.scancode;
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
		switch(key) {
		case SDL_SCANCODE_1: em_id = EM_EXPLOSION; break;
		case SDL_SCANCODE_2: em_id = EM_EXPLOSIONBIG; break;
		case SDL_SCANCODE_3: em_id = EM_FRAGMENTS; break;
		case SDL_SCANCODE_4: em_id = EM_SMOKE; break;
		case SDL_SCANCODE_5: em_id = EM_SPARKS; break;
		case SDL_SCANCODE_6: em_id = EM_ROCKETFLAME; break;
		particles_get_emitter_at(RLAY_GUI_FRONT, em_id, player1->data.body->p); break;
		default: break;
		}
		break;
	}
	return 0;
}

static void on_pause(void)
{
	if (gamestate == LEVEL_RUNNING) {
		pause_game();
	}
}

static we_bool from_editor = WE_FALSE;
static void on_enter(STATE_ID state_prev)
{
	if (state_prev == state_levelscreen || state_prev == state_editor || state_prev == state_leveldone || state_prev == state_space) {
		from_editor = (state_prev == state_editor) ? WE_TRUE : ((state_prev == state_levelscreen) ? WE_FALSE : from_editor);
		fprintf(stderr,"TODO: entering space state from %s\n", state_prev);
		lvl_tmpl = get_current_lvl_template();
		lvl_tmpl = spacelvl_copy(lvl_tmpl);
		currentlvl = lvl_tmpl; //TODO remove currentlvl variable completely
		spacelvl_load2state(lvl_tmpl);
		space_init_game();
		//TODO copy lvl from editor to space, or just keep dynamic variables in separate struct?
	} else if (state_prev == state_pause) {
	} else {
		statesystem_set_state(state_log);
	}
}

static void on_leave(STATE_ID state_next)
{
	//TODO unload lvl with spacelvl_unload2state();
	if (state_next == state_leveldone) {
	} else if (state_next == state_editor || state_next == state_space || state_next == state_stations) {
		spacelvl_unload2state(lvl_tmpl);
		spacelvl_freecopy(&lvl_tmpl);
	} else if (state_next == state_pause) {
	} else if (state_next == state_log) {
	} else {
		SDL_Log("ERROR: illegal state transition from SPACE to %s", state_next);
		statesystem_set_state(state_log);
	}
}

static void destroy(void)
{
	spacelvl_unload2state(lvl_tmpl);
	spacelvl_freecopy(&lvl_tmpl);
	joystick_free(joy_p1_left);
	joystick_free(joy_p1_right);
}


static cpVect space_particle_g_func(cpVect pos)
{
    return cpvnormalize_safe(pos);
}

void space_init(void)
{
	statesystem_register(state_space,LEVEL_STATE_COUNT);
    state_add_inner_state(state_space,LEVEL_START,level_start,NULL);
    state_add_inner_state(state_space,LEVEL_RUNNING,level_running,NULL);
    state_add_inner_state(state_space,LEVEL_PLAYER_DEAD,level_player_dead,NULL);
    state_add_inner_state(state_space,LEVEL_CLEARED,level_cleared,NULL);
    state_add_inner_state(state_space,LEVEL_TRANSITION,level_transition,NULL);

	int i;
    state_add_layers(state_space, 21);
	int layers = state_layer_count(state_space);
	for(i = 11; i<layers; i++){
		//float depth =  2 + 10*tan((1.0f*i/la_sys->num_layers)*WE_PI_2);
		float f = (layers - i * 0.99f) / (layers);
		state_set_layer_parallax(state_space, i, f, f);
	}
	SPRITE_ID spr = sprite_link("starcross01");
	for(i = 0; i<400; i++){
        Color col1 = {255,255,255,0};
        Color col2 = {0,0,0,0};
		int layer =  11 + roundf((1-we_randf*we_randf)*(layers-1-11));
		float size = 50 + we_randf*300 - layer*4;
		//byte l = 255 - 200 * layer / layers;
		//col = {l,l,l,255};
		float rand_x = we_randf;
		float rand_y = we_randf;
		cpVect pos = cpvmult(cpv(rand_x-0.5,rand_y-0.5),40000);
		float f = minf(rand_x*0.6 + 0.4*rand_y*(0.9+0.1*rand_x), 1);
		//col2.g = 255*(f);
		//col2.b = 255*(1-f);
        col2 = draw_col_rainbow((f+0.5)*1536);
		if (we_randf < 0.1) {
			col2.a = 1;
			state_add_sprite(state_space, layer, SPRITE_SPIKEBALL, size,size, pos, 0, col2);
		} else {
			col2.r = (255 + col2.r)/2;
			col2.g = (255 + col2.g)/2;
			col2.b = (255 + col2.b)/2;
			col2.a = 0;
			state_add_dualsprite(state_space, layer, spr, pos, cpv(size,size), col1, col2);
		}
	}

    view_p1 = state_view_get(state_space, 0);
    view_p2 = state_view_add(state_space);
    view_p2->enabled = 0;

    view_p1->GUI = draw_gui;
    view_p2->GUI = draw_gui;

    btn_pause = button_create(SPRITE_BTN_PAUSE, 0, "", GAME_WIDTH/2-100, GAME_HEIGHT/2-100, 120, 120);
	btn_continue = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);
	button_set_click_callback(btn_pause, (btn_click_callback) statesystem_pause, 0);
	button_set_click_callback(btn_continue, (btn_click_callback) continue_func, 0);
    button_set_enlargement(btn_pause, 1.5f);
    button_set_hotkeys(btn_pause, KEY_ESCAPE, SDL_SCANCODE_PAUSE);
	button_set_hotkeys(btn_continue, KEY_ESCAPE, KEY_RETURN_2);
	btn_continue->enabled = 0;
#if !ARCADE_MODE
    state_register_touchable_view(view_p1, btn_pause);
#endif

    state_enable_objects(state_space, 1);
    state_enable_particles(state_space, 1);
	particle_set_gravity_func(current_particles, space_particle_g_func );

	cpSpaceSetGravity(current_space, cpv(GRAVITY,0));
	cpSpaceSetDamping(current_space, DAMPING);

	extern void collisioncallbacks_init(void);
    collisioncallbacks_init();

    stars_init();

    float h = GAME_HEIGHT*0.5;

    joy_p1_left = joystick_create(0, STICK_RADIUS, 2, -GAME_WIDTH/2 + 170, -0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p1_right = joystick_create(0, STICK_RADIUS, 2, GAME_WIDTH/2 - 170, -0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p2_left = joystick_create(0, STICK_RADIUS, 2, -GAME_WIDTH/2 + 170, +0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);
    joy_p2_right = joystick_create(0, STICK_RADIUS, 2, GAME_WIDTH/2 - 170, +0.25*GAME_HEIGHT, 340, h, SPRITE_JOYSTICK_BACK, SPRITE_JOYSTICK);

    state_register_touchable_view(view_p1, (touchable *)joy_p1_left);
    state_register_touchable_view(view_p1, (touchable *)joy_p1_right);
    state_register_touchable_view(view_p2, (touchable *)joy_p2_left);
    state_register_touchable_view(view_p2, (touchable *)joy_p2_right);
    state_register_touchable_view(view_p1, btn_continue);
    state_timer = 10;
	change_state(LEVEL_START);
}


float getGameTime(void)
{
	return game_time;
}

int getArcadeScore(void)
{
	return ARCADE_LVL_COUNT * (ARCADE_SCORE_LVL + 1000) - arcade_lvl_score;
}

void input(void)
{
#if !GOT_TOUCH
	/*
	 * Camera modes + F11 = timeout + F8 = reload particles (broken)
	 */
	if (keys[SDL_SCANCODE_F1]) {
		player_camera_mode = 1;
	} else if (keys[SDL_SCANCODE_F2]) {
		player_camera_mode = 2;
	} else if (keys[SDL_SCANCODE_F3]) {
		player_camera_mode = 3;
	} else if (keys[SDL_SCANCODE_F4]) {
		player_camera_mode = 4;
	} else if (keys[SDL_SCANCODE_F5]) {
		player_camera_mode = 5;
	} else if (keys[SDL_SCANCODE_F6]) {
		player_camera_mode = 6;
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

    touch_place((touchable *)btn_pause, view_p1->view_width/2-80, view_p1->view_height/2-70);
    joystick_reposition(joy_p1_left, STICK_RADIUS, 2, -view_p1->view_width/2 + 170, -0.25*view_p1->view_height, w, h);
	joystick_reposition(joy_p1_right, STICK_RADIUS, 2, view_p1->view_width/2 - 170, -0.25*view_p1->view_height, w, h);
    joystick_reposition(joy_p2_left, STICK_RADIUS, 2, -view_p1->view_width/2 + 170, 0.25*view_p1->view_height, w, h);
    joystick_reposition(joy_p2_right, STICK_RADIUS, 2, view_p1->view_width/2 - 170, 0.25*view_p1->view_height, w, h);

    LList ll_touchies = view_p1->touch_objects;
    if (!llist_contains(ll_touchies, joy_p2_left)) {
    	llist_add(ll_touchies, joy_p2_left);
    	llist_add(ll_touchies, joy_p2_right);
    }

#if !ARCADE_MODE
    joy_p2_left->touch_data.visible = 1;
    joy_p2_right->touch_data.visible = 1;
#endif
}

obj_player * space_create_player(int id)
{
	obj_param_player default_player = {
			.player_id = id
	};
	return (obj_player *)instance_create(obj_id_player, &default_player, cpv(0,-(currentlvl->height/2+currentlvl->inner_radius)), cpvzero);
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

    touch_place((touchable *)btn_pause, view_p1->view_width/2-80, view_p1->view_height/2-70);
	joystick_place(joy_p1_left, -view_p1->port_width/3, -view_p1->port_height/3);
	joystick_place(joy_p1_right, +view_p1->port_width/3, -view_p1->port_height/3);
	joystick_place(joy_p2_left, -view_p1->port_width/3, -view_p1->port_height/3);
	joystick_place(joy_p2_right, +view_p1->port_width/3, -view_p1->port_height/3);

    float w = 340;
    float h1 = view_p1->view_height*0.5;
    float h2 = view_p2->view_height*0.5;
	joystick_reposition(joy_p1_left, STICK_RADIUS, 2, -view_p1->view_width/2 + 170, -0.25*view_p1->view_height, w, h1);
	joystick_reposition(joy_p1_right, STICK_RADIUS, 2, view_p1->view_width/2 - 170, -0.25*view_p1->view_height, w, h1);
    joystick_reposition(joy_p2_left, STICK_RADIUS, 2, -view_p2->view_width/2 + 170, -0.25*view_p2->view_height, w, h2);
    joystick_reposition(joy_p2_right, STICK_RADIUS, 2, view_p2->view_width/2 - 170, -0.25*view_p2->view_height, w, h2);

    #if !ARCADE_MODE
    joy_p2_left->touch_data.visible = 1;
    joy_p2_right->touch_data.visible = 1;
#endif

    LList ll_touchies = view_p1->touch_objects;
    llist_remove(ll_touchies, joy_p2_left);
    llist_remove(ll_touchies, joy_p2_right);

    if (player2 == NULL) {
		player2 = space_create_player(2);
	}
}


//TODO init multiplayer to 0 when starting lvl


void space_return(void *unused)
{
	statesystem_set_state(from_editor ? state_editor : state_stations);
}

void space_restart_level(void *unused)
{
	statesystem_set_state(state_space);

	//statesystem_set_state(state_space); //restart?
}
