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
static void arcade_update();
static void arcade_draw();
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
static int current_menu;

static const Color col_item   = {1,0,0,1};
static const Color col_select = {0,0,1,1};


void menu_init()
{
    curMenu = &mainMenuTest;


    if(config.arcade){
    	statesystem_init_state(STATESYSTEM_MENU, 0,
    	            on_enter,
    	            arcade_update,
    	            NULL,
    	            arcade_draw,
    	            on_leave,
    	            destroy);
    }else{
    	statesystem_init_state(STATESYSTEM_MENU, 0,
    			on_enter,
    			update,
    			NULL,
    			draw,
    			on_leave,
    			destroy);
    }

}

static void on_enter()
{

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

static void update()
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


static void arcade_update()
{


	if (keys[SDLK_SPACE] || keys[SDLK_RETURN]) {
		switch(current_menu){
			case MENU_MAIN:
				space_init_level(1,1);
				statesystem_set_state(STATESYSTEM_SPACE);
				menu_change_current_menu(MENU_INGAME);
				break;
			case MENU_INGAME:
				statesystem_set_state(STATESYSTEM_SPACE);
				break;
			}
		keys[SDLK_SPACE] = 0, keys[SDLK_RETURN] = 0;
	}

	if(keys[SDLK_ESCAPE]){
		menu_change_current_menu(MENU_MAIN);
		statesystem_set_state(STATESYSTEM_MENU);
		keys[SDLK_ESCAPE] = 0;
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
		font_drawText(0,0.5f*HEIGHT/2, "PAUSE");
		break;
	case MENU_MAIN:
		glColor_from_color(draw_col_rainbow((int)(timer*1000)));
		font_drawText(0,0.8f*HEIGHT/2, "SPACE");

		setTextAlign(TEXT_CENTER);
		setTextSize(40);

		font_drawText(0,0.5f*HEIGHT/2, "PRESS GREEN");
		break;
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
