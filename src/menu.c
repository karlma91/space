/* header */
#include "menu.h"

/* standard c-libraries */
#include <stdio.h>

#include "game.h"

/* Game state */
#include "main.h"
#include "space.h"
#include "gameover.h"
#include "levelselect.h"
#include "statesystem.h"
#include "draw.h"

STATE_ID STATE_MENU;

/* Drawing */
#include "draw.h"
#include "font.h"

#define AUTO_ENTER 1

#define MAX_MENU_ITEMS 5

static void inner_main();
static void inner_ingame();

struct menu {
	int num_items;
	int escape_item;
	void (*func)();
	char texts[MAX_MENU_ITEMS][15];
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

static const Color col_item   = {1,0,0,1};
static const Color col_select = {0,0,1,1};

static void sdl_event(SDL_Event *event)
{
	SDL_Scancode key;
	switch (event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;

#if !ARCADE_MODE
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
		break;
	case SDL_FINGERUP:
		curMenu->func();
		break;
	}
}

static void on_enter()
{
	select_id = 0;
}
static void on_leave()
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

#if ARCADE_MODE
static void arcade_update()
{
	if (keys[KEY_RETURN_2] || keys[KEY_RETURN_1]) {
		switch(current_menu){
			case MENU_MAIN:
				space_init_level(1,1);
				statesystem_set_state(STATE_SPACE);
				menu_change_current_menu(MENU_INGAME);
				break;
			case MENU_INGAME:
				statesystem_set_state(STATE_SPACE);
				break;
			}
		keys[KEY_RETURN_2] = 0;
		keys[KEY_RETURN_1] = 0;
	}

	if (keys[KEY_ESCAPE]){
		menu_change_current_menu(MENU_MAIN);
		statesystem_set_state(STATE_MENU);
		keys[KEY_ESCAPE] = 0;
	}
}

static void arcade_draw()
{
	static float timer;
	timer +=dt;

	setTextAngle(0);
	setTextSize(80);
	setTextAlign(TEXT_CENTER);
	switch(current_menu){
	case MENU_INGAME:
		setTextAlign(TEXT_CENTER);
		setTextSize(40);
		//font_drawText(0,0.5f*HEIGHT/2, "PAUSE");
		break;
	case MENU_MAIN:
		drawStars();
		draw_color(draw_col_rainbow((int)(timer*1000)));
		font_drawText(0,0.8f*HEIGHT/2, "SPACE");

		draw_color4f(1,1,1,1);

		setTextSize(30);
		font_drawText(-WIDTH*0.4,-0.8f*HEIGHT/2, "STYR");
		font_drawText(-WIDTH*0.4,-0.8f*HEIGHT/2-50, "VVV");

		font_drawText(+WIDTH*0.4,-0.8f*HEIGHT/2, "SKYT");
		font_drawText(+WIDTH*0.4,-0.8f*HEIGHT/2-50, "VVV");

		setTextAlign(TEXT_CENTER);
		setTextSize(40);

		static float button_timer = 0;
		static int button_down;
		button_timer+=dt;
		if(button_timer > 0.5){
			button_down = !button_down;
			button_timer = 0;
		}
		font_drawText(0,-0.5f*HEIGHT/2, "START SPILLET");
		draw_color4f(0.1,0.9,0.1,1);
		if(button_down){
			cpVect t = cpv(0,0);
			draw_texture(TEX_BUTTON_DOWN,&t,TEX_MAP_FULL,300,300,0);
		}else{
			cpVect t = cpv(0,-5.5);
			draw_texture(TEX_BUTTON,&t,TEX_MAP_FULL,300,300,0);
		}
		break;
	}
}
#else
static void draw()
{
	draw_load_identity();

	static float timer;
	timer +=dt;

	setTextAngle(0);
	setTextSize(80);
	setTextAlign(TEXT_CENTER);
	draw_color(draw_col_rainbow((int)(timer*1000)));
	font_drawText(0,0.8f*HEIGHT/2, "SPACE");

	setTextAlign(TEXT_CENTER);
	setTextSize(40);

	for (i = 0; i < curMenu->num_items; i++) {
		draw_color((select_id == i) ? col_select : col_item);
		font_drawText(0,100 - 60 * i, curMenu->texts[i]);
	}
}
#endif

static void inner_main()
{
	switch (select_id) {
	case 0: //START GAME
		space_init_level(1,1);
		statesystem_set_state(STATE_SPACE);
		curMenu = &ingameMenu;
		break;
	case 1: //LEVEL SELECT
	    statesystem_set_state(STATE_LEVELSELECT);
		break;
	case 2: //HIGHSCORE
		gameover_setstate(show_highscore);
		statesystem_set_state(STATE_GAMEOVER);
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

static void inner_ingame()
{
	switch (select_id) {
	case 0: //RESUME GAME
		statesystem_set_state(STATE_SPACE);
		break;
	case 1: //RESTART GAME
	    statesystem_set_state(STATE_SPACE);
	    menu_change_current_menu(MENU_INGAME);
		space_init_level(1,1);
		break;
	case 2:
		curMenu = &mainMenuTest;
		statesystem_set_state(STATE_MENU);
		break;
	default:
		break;
	}
}

static void destroy()
{

}

void menu_init()
{
	curMenu = &mainMenuTest;

#if ARCADE_MODE
	STATE_MENU = statesystem_add_state(0, on_enter, arcade_update, NULL, arcade_draw, sdl_event, on_leave, destroy);
#else
	STATE_MENU = statesystem_add_state(0, on_enter, NULL, NULL, draw, sdl_event, on_leave, destroy);
#endif
}
