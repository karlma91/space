/* header */
#include "menu.h"

/* standard c-libraries */
#include <stdio.h>

/* Game state */
#include "main.h"
#include "space.h"
#include "gameover.h"
#include "levelselect.h"

/* Drawing */
#include "draw.h"
#include "font.h"

/* static prototypes */
static void menu_init();
static void menu_update();
static void menu_draw();
static void menu_destroy();

static void optionmenu_func();
static void mainmenu_func();

state state_menu = {
	menu_init,
	menu_draw,
	menu_update,
	menu_destroy,
	NULL
};

#define MAX_MENU_ITEMS 5

struct menu {
	int num_items;
	int escape_item;
	void (*func)();
	char texts[MAX_MENU_ITEMS][15];
};

static struct menu mainMenuTest = {
		5,
		4,
		mainmenu_func,
		{"START GAME","LEVELS","HIGHSCORES","CREDITS","EXIT"}
};

static struct menu ingameMenu = {
		3,
		0,
		optionmenu_func,
		{"RESUME","RESTART","MAINMENU"}
};

static struct menu *curMenu; //current active menu
static int select_id = 0;
static int i;

static const Color col_item   = {1,0,0,1};
static const Color col_select = {0,0,1,1};


void change_current_menu(int menu)
{
	switch(menu){
	case INGAME_MENU_ID:
		curMenu = &ingameMenu;
		break;
	case MAIN_MENU_ID:
		curMenu = &mainMenuTest;
		break;
	}
}

static void menu_init()
{
	curMenu = &mainMenuTest;
}

static void menu_update()
{
	if (keys[SDLK_w] || keys[SDLK_UP]){
		select_id--;
		select_id = (select_id < 0) ? curMenu->num_items-1 : select_id;
		keys[SDLK_w] = 0, keys[SDLK_UP] = 0;
	}

	if (keys[SDLK_s] || keys[SDLK_DOWN]){
		select_id++;
		select_id = select_id >= curMenu->num_items ? 0 : select_id;
		keys[SDLK_s] = 0, keys[SDLK_DOWN] = 0;
	}

	if (keys[SDLK_SPACE] || keys[SDLK_RETURN]) {
		curMenu->func();
		keys[SDLK_SPACE] = 0, keys[SDLK_RETURN] = 0;
	}

	if(keys[SDLK_ESCAPE]){
		select_id = curMenu->escape_item;
		curMenu->func();
		keys[SDLK_ESCAPE] = 0;
	}
}

static void menu_draw()
{
	static float timer;
	timer +=dt;

	setTextAngle(0);
	setTextSize(80);
	setTextAlign(TEXT_CENTER);
	glColor_from_color(draw_col_rainbow((int)(timer*1000)));
	font_drawText(0,0.8f*HEIGHT/2, "SPACE");

	setTextAlign(TEXT_CENTER);
	setTextSize(40);

	for (i = 0; i < curMenu->num_items; i++) {
		glColor_from_color((select_id == i) ? col_select : col_item);
		font_drawText(0,100 - 60 * i, curMenu->texts[i]);
	}
}

static void mainmenu_func()
{
	switch (select_id) {
	case 0: //START GAME
		space_init_level(1,1);
		currentState = &state_space;
		curMenu = &ingameMenu;
		break;
	case 1: //LEVEL SELECT
		currentState = &state_levelselect;
		break;
	case 2: //HIGHSCORE
		currentState = &state_gameover;
		gameover_setstate(show_highscore);
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

static void optionmenu_func()
{
	switch (select_id) {
	case 0: //RESUME GAME
		currentState = &state_space;
		break;
	case 1: //RESTART GAME
		currentState = &state_space;
		space_init_level(1,1);
		break;
	case 2:
		state_menu.parentState = 0;
		curMenu = &mainMenuTest;
		currentState = &state_menu;
		break;
	default:
		break;
	}
}

static void menu_destroy()
{

}
