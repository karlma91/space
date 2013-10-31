#include "menu.h"
#include "../../game.h"
#include "../space.h"
#include "gameover.h"
#include "we_defstate.h"

STATE_ID state_menu;

sprite spr_startbtn;

#define MAX_MENU_ITEMS 5
#define MENU_LAYERS 4

static void inner_main(void);
static void inner_ingame(void);

struct menu {
	int num_items;
	int escape_item;
	void (*func)(void);
	char texts[MAX_MENU_ITEMS][20];
};


static struct menu mainMenuTest = {
		5,
		4,
		inner_main,
		{"START GAME","LEVELS","HIGHSCORES","CREDITS","EXIT"}
};

static struct menu ingameMenu = {
		3,
		0,
		inner_ingame,
		{"RESUME","RESTART","MAINMENU"}
};

static struct menu *curMenu; //current active menu
static int select_id = 0;
static int current_menu;

static const Color col_item   = {255,0,  0,255};
static const Color col_select = {  0,0,255,255};

static view *view_main;

static void sdl_event(SDL_Event *event)
{
	SDL_Scancode key;
	switch (event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;

#if !ARCADE_MODE // because there is no menus in arcade mode
		if (key == KEY_UP_1 || key == KEY_UP_2) {
			--select_id;
			select_id = (select_id < 0) ? curMenu->num_items-1 : select_id;
		} else

		if (key == KEY_DOWN_1 || key == KEY_DOWN_2) {
			select_id++;
			select_id = select_id >= curMenu->num_items ? 0 : select_id;
		}

		if (key == KEY_RETURN_2 || key == KEY_RETURN_1) {
			curMenu->func();
		} else

		if (key == KEY_ESCAPE){
			select_id = curMenu->escape_item;
			curMenu->func();
		}
#endif
		break;
	case SDL_FINGERDOWN:
		//button_finger_down(btn_start_tmp, &event->tfinger);
		break;
	case SDL_FINGERMOTION:
		//button_finger_move(btn_start_tmp, &event->tfinger);
		break;
	case SDL_FINGERUP:
		//if (button_finger_up(btn_start_tmp, &event->tfinger)) {
		//	curMenu->func();
		//}
		break;
	}
}

static void on_enter(void)
{
	select_id = 0;
	sound_music(MUSIC_MENU);
}

static void on_pause(void)
{
}

static void on_leave(void)
{
}

void menu_change_current_menu(int menu)
{
	current_menu = menu;
	switch(menu){
	case MENU_INGAME:
		curMenu = &ingameMenu;
		break;
	case MENU_MAIN:
		curMenu = &mainMenuTest;
		break;
	}
}

static cpVect p_cam = {0,0};

static void pre_update(void)
{
	if (keys[KEY_RETURN_2] || keys[KEY_RETURN_1]) {
		switch(current_menu){
			case MENU_MAIN:
				space_start_demo(1,1);
				break;
			case MENU_INGAME:
				statesystem_set_state(state_space);
				break;
			}
		keys[KEY_RETURN_2] = 0;
		keys[KEY_RETURN_1] = 0;
	}

	if (keys[KEY_ESCAPE]){
		menu_change_current_menu(MENU_MAIN);
		statesystem_set_state(state_menu);
		keys[KEY_ESCAPE] = 0;
	}

	float angle = engine_time*WE_2PI;
	p_cam = cpv(cosf(angle/16)*220, sinf(angle/17)*97);
	//p_cam = cpvadd(p_cam,cpvmult(cpvforangle(angle/31),200));
	view_update(view_main, p_cam, 0);
	view_main->zoom = 1 - cosf(angle/22)/3;
}

static void post_update(){}

static void draw(void)
{
	draw_load_identity();

	static float timer;
	timer +=dt;

	setTextAngle(0);
	setTextAlign(TEXT_CENTER);
	setTextSize(120);
	//drawStars();
	draw_color(draw_col_rainbow((int)(timer*1000)));
	font_drawText(RLAY_GUI_FRONT, 0,0.7f*GAME_HEIGHT/2, "SPACE");

	draw_color4f(1,1,1,1);

	setTextSize(30);
	font_drawText(RLAY_GUI_FRONT, -GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2, "STYR");
	font_drawText(RLAY_GUI_FRONT, -GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2-50, "VVV");

	font_drawText(RLAY_GUI_FRONT, +GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2, "SKYT");
	font_drawText(RLAY_GUI_FRONT, +GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2-50, "VVV");

	setTextAlign(TEXT_CENTER);
	setTextSize(40);

	font_drawText(RLAY_GUI_FRONT, 0,0, "START SPILLET");

	draw_color4f(0.1,0.9,0.1,1);
	sprite_update(&spr_startbtn);
	sprite_render(RLAY_GUI_MID, &spr_startbtn, cpv(0,-0.5f*GAME_HEIGHT/2), 0);
}

static void inner_main(void)
{
	switch (select_id) {
	case 0: //START GAME
		space_start_demo(1,1);
		break;
	case 1: //LEVEL SELECT

		break;
	case 2: //HIGHSCORE
		gameover_showhighscores();
		break;
	case 3: //CREDITS
		break;
	case 4: //EXIT
		main_stop();
		break;
	default:
		break;
	}
}

static void inner_ingame(void)
{
	switch (select_id) {
	case 0: //RESUME GAME
		statesystem_set_state(state_space);
		break;
	case 1: //RESTART GAME
		space_start_demo(1,1);
		break;
	case 2:
		curMenu = &mainMenuTest;
		statesystem_set_state(state_menu);
		break;
	default:
		break;
	}
}

static void destroy(void)
{

}

void menu_init(void)
{
	curMenu = &mainMenuTest;
	statesystem_register(state_menu, 0);
	sprite_create(&spr_startbtn, SPRITE_BUTTON, 300,250, 2);

	view_main = state_view_get(state_menu, 0);

	state_add_layers(state_menu, 100);
	int layers = state_layer_count(state_menu);

	int i;
	for(i = 2; i<layers; i++){
		//float depth =  2 + 10*tan((1.0f*i/la_sys->num_layers)*WE_PI_2);
		float f = (layers - i * 0.99f) / (layers);
		state_set_layer_parallax(state_menu, i, f, f);
	}
	for(i = 0; i<1000; i++){
		int layer =  2 + roundf(we_randf*(layers-1-2));
		float size = 150 + we_randf*90 - layer*4;
		cpVect pos = cpvmult(cpv(we_randf-0.5,we_randf-0.5),6600);
		SPRITE_ID spr;
		int s = rand() & 7;
		switch(s) {
        default: spr = SPRITE_SPIKEBALL; break;
		case 1: spr = SPRITE_GEAR; break;
		case 2: spr = SPRITE_STATION_01; break;
		case 3: spr = SPRITE_TANK_WHEEL; break;
		}

		state_add_sprite(state_menu, layer, spr, size, size, pos, we_randf*WE_2PI * (spr != SPRITE_STATION_01));
	}
}
