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
static int selector = 1;

state mainMenuState = {
	mainmenu_draw,
	mainmenu_update,
	NULL
};

void mainmenu_init()
{

}

static void mainmenu_update(float dt)
{
	if(keys[SDLK_w]){
		selector--;
		selector = selector < 1 ? 4 : selector;
		keys[SDLK_w] = 0;
	}
	if(keys[SDLK_s]){
		selector++;
		selector = selector > 4 ? 1 : selector;
		keys[SDLK_s] = 0;
	}

	if((keys[SDLK_SPACE] || keys[SDLK_RETURN]) && selector == 1) {

		currentState = &spaceState;
	}
}

static void mainmenu_draw(float dt)
{
	setTextAlign(TEXT_CENTER);
	setTextSize(40);

	glColor3f(1,0,0);
	if(selector == 1){
		glColor3f(0,0,1);
	}
	font_drawText(0,100,"START GAME");
	glColor3f(1,0,0);
	if(selector == 2){
			glColor3f(0,0,1);
		}
	font_drawText(0,50,"OPTIONS");
	glColor3f(1,0,0);
	if(selector == 3){
			glColor3f(0,0,1);
		}
	font_drawText(0,0,"CREDITS");
	glColor3f(1,0,0);
	if(selector == 4){
			glColor3f(0,0,1);
		}
	font_drawText(0,-50,"EXIT");

}

void mainmenu_destroy()
{

}
