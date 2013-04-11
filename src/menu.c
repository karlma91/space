/* header */
#include "menu.h"

/* standard c-libraries */
#include <stdio.h>

/* Game state */
#include "main.h"
#include "space.h"
#include "gameover.h"
#include "levelselect.h"
#include "statesystem.h"


/* Drawing */
#include "draw.h"
#include "font.h"

/* static prototypes */
static void on_enter();
static void update();
static void draw();
static void on_leave();
static void destroy();

static void inner_ingame();
static void inner_main();

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
static int i;

static const Color col_item   = {1,0,0,1};
static const Color col_select = {0,0,1,1};


void menu_init()
{
    curMenu = &mainMenuTest;

    statesystem_init_state(STATESYSTEM_MENU, 0,
            on_enter,
            update,
            NULL,
            draw,
            on_leave,
            destroy);

}

static void on_enter()
{

}
static void on_leave()
{

}

void menu_change_current_menu(int menu)
{
	switch(menu){
	case MENU_INGAME:
		curMenu = &ingameMenu;
		break;
	case MENU_MAIN:
		curMenu = &mainMenuTest;
		break;
	}
}

static void update()
{
	if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]){
		select_id--;
		select_id = (select_id < 0) ? curMenu->num_items-1 : select_id;
		keys[SDL_SCANCODE_W] = 0, keys[SDL_SCANCODE_UP] = 0;
	}

	if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]){
		select_id++;
		select_id = select_id >= curMenu->num_items ? 0 : select_id;
		keys[SDL_SCANCODE_S] = 0, keys[SDL_SCANCODE_DOWN] = 0;
	}

	if (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_RETURN]) {
		curMenu->func();
		keys[SDL_SCANCODE_SPACE] = 0, keys[SDL_SCANCODE_RETURN] = 0;
	}

	if(keys[SDL_SCANCODE_ESCAPE]){
		select_id = curMenu->escape_item;
		curMenu->func();
		keys[SDL_SCANCODE_ESCAPE] = 0;
	}
}

static void draw()
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

static void inner_main()
{
	switch (select_id) {
	case 0: //START GAME
		space_init_level(1,1);
		statesystem_set_state(STATESYSTEM_SPACE);
		curMenu = &ingameMenu;
		break;
	case 1: //LEVEL SELECT
	    statesystem_set_state(STATESYSTEM_LEVELSELECT);
		break;
	case 2: //HIGHSCORE
		gameover_setstate(show_highscore);
		statesystem_set_state(STATESYSTEM_GAMEOVER);
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
		statesystem_set_state(STATESYSTEM_SPACE);
		break;
	case 1: //RESTART GAME
	    statesystem_set_state(STATESYSTEM_SPACE);
	    menu_change_current_menu(MENU_INGAME);
		space_init_level(1,1);
		break;
	case 2:
		curMenu = &mainMenuTest;
		statesystem_set_state(STATESYSTEM_MENU);
		break;
	default:
		break;
	}
}

static void destroy()
{

}
