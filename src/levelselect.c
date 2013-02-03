/* header */
#include "levelselect.h"

/* standard c-libraries */
#include <stdio.h>

/* Chipmunk physics library */
#include "chipmunk.h"

/* Game state */
#include "main.h"
#include "menu.h"

/* Drawing */
#include "draw.h"
#include "font.h"

/* static prototypes */
static void init();
static void update();
static void render();
static void destroy();

/* extern */
state state_levelselect = {
		init,
		update,
		render,
		destroy,
		NULL
};

static int sel = 0;

static void init()
{

}

static void update()
{
	if (keys[SDLK_w] || keys[SDLK_UP]){

		sel++;
		sel = sel >= 5 ? 0 : sel;
		keys[SDLK_w] = 0, keys[SDLK_UP] = 0;
	}

	if (keys[SDLK_s] || keys[SDLK_DOWN]){
		sel--;
			sel = (sel < 0) ? 5 : sel;
		keys[SDLK_s] = 0, keys[SDLK_DOWN] = 0;
	}

}

static void render()
{
	int i = 0;
	for(i = 0; i < 6; i++){
		if(sel == i){
			draw_simple_box(10, 100*(i + 1), 60,60);
		}
		draw_simple_box(10, 100*(i + 1), 50,50);
	}
}

static void destroy()
{

}
