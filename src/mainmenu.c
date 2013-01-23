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

state mainMenuState = {
	mainmenu_draw,
	mainmenu_update,
	NULL
};

enum menu {
	START_GAME,
	OPTIONS,
	CREDITS,
	EXIT
};

static enum menu select_id = START_GAME;
static char menu_item[4][15] = {"START GAME","OPTIONS","CREDITS","EXIT"};

static int i;

static const Color col_item   = {1,0,0,1};
static const Color col_select = {0,0,1,1};


void mainmenu_init()
{

}

static void mainmenu_update(float dt)
{
	if (keys[SDLK_w] || keys[SDLK_UP]){
		select_id--;
		select_id = (select_id < START_GAME || select_id > EXIT) ? EXIT : select_id;
		keys[SDLK_w] = 0, keys[SDLK_UP] = 0;
	}
	if (keys[SDLK_s] || keys[SDLK_DOWN]){
		select_id++;
		select_id = select_id > EXIT ? START_GAME : select_id;
		keys[SDLK_s] = 0, keys[SDLK_DOWN] = 0;
	}

	if (keys[SDLK_SPACE] || keys[SDLK_RETURN]) {
		switch (select_id) {
			case START_GAME:
				currentState = &spaceState;
				break;
			case EXIT:
				main_stop();
				break;
			default:
				break;
		}
		keys[SDLK_SPACE] = 0, keys[SDLK_RETURN] = 0;
	}
}

static void mainmenu_draw(float dt)
{
	setTextAlign(TEXT_CENTER);
	setTextSize(40);

	for (i = START_GAME; i <= EXIT; i++) {
		glColor_from_color((select_id == i) ? col_select : col_item);
		font_drawText(0,100 - 60 * i,menu_item[i]);
	}
}

void mainmenu_destroy()
{

}
