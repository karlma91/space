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
static int select = 1;

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
		select--;
		select = select < 1 ? 4 : select;
		keys[SDLK_w] = 0;
	}
	if(keys[SDLK_s]){
		select++;
		select = select > 4 ? 1 : select;
		keys[SDLK_s] = 0;
	}

	if((keys[SDLK_SPACE] || keys[SDLK_RETURN]) && select == 1) {

		currentState = &spaceState;
	}
}

static void mainmenu_draw(float dt)
{
	setTextAlign(TEXT_CENTER);
	setTextSize(40);

	glColor3f(1,0,0);
	if(select == 1){
		glColor3f(0,0,1);
	}
	font_drawText(0,100,"START GAME");
	glColor3f(1,0,0);
	if(select == 2){
			glColor3f(0,0,1);
		}
	font_drawText(0,50,"OPTIONS");
	glColor3f(1,0,0);
	if(select == 3){
			glColor3f(0,0,1);
		}
	font_drawText(0,0,"CREDITS");
	glColor3f(1,0,0);
	if(select == 4){
			glColor3f(0,0,1);
		}
	font_drawText(0,-50,"EXIT");

}

void mainmenu_destroy()
{

}
