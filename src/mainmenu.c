#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"


static void mainmenu_draw(float dt);
static void mainmenu_update(float dt);
static void optionmenu_func();
static void mainmenu_func();
static void levelmenu_func();


state mainMenuState = {
	mainmenu_draw,
	mainmenu_update,
	NULL
};

#define MAX_MENU_ITEMS 5

struct menu {
	int num_items;
	int escape_item;
	void (*func)();
	char texts[MAX_MENU_ITEMS][15];
};

struct menu mainMenuTest = {
		4,
		3,
		mainmenu_func,
		{"START GAME","LEVELS","CREDITS","EXIT"}
};
struct menu ingameMenu = {
		2,
		0,
		optionmenu_func,
		{"RESUME","MAINMENU"}
};
struct menu levelSelect = {
		5,
		0,
		levelmenu_func,
		{"LEVEL 1","LEVEL 2","LEVEL 3","LEVEL 4","LEVEL 5"}
};

static struct menu *curMenu; //current active menu

static int select_id = 0;
static int escapePress = 0;
static int i;

static const Color col_item   = {1,0,0,1};
static const Color col_select = {0,0,1,1};


void mainmenu_init()
{
	curMenu = &mainMenuTest;
}

static void mainmenu_update(float dt)
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

static void mainmenu_func()
{
	switch (select_id) {
	case 0:
		currentState = &spaceState;
		curMenu = &ingameMenu;
		break;
	case 1:
		curMenu = &levelSelect;
		break;
	case 3:
		main_stop();
		break;
	default:
		break;
	}
}
static void optionmenu_func()
{
	switch (select_id) {
	case 0:
		currentState = &spaceState;
		break;
	case 1:
		mainMenuState.parentState = 0;
		curMenu = &mainMenuTest;
		currentState = &mainMenuState;
		break;
	default:
		break;
	}
}

static void levelmenu_func()
{
	switch (select_id) {
	case 0:
		currentState = &spaceState;
		curMenu = &ingameMenu;
		break;
	case 3:
		main_stop();
		break;
	default:
		break;
	}
}

static void mainmenu_draw(float dt)
{
	setTextAlign(TEXT_CENTER);
	setTextSize(40);

	for (i = 0; i < curMenu->num_items; i++) {
		glColor_from_color((select_id == i) ? col_select : col_item);
		font_drawText(0,100 - 60 * i, curMenu->texts[i]);
	}
}

void mainmenu_destroy()
{

}
