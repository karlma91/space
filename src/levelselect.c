/* header */
#include "levelselect.h"

/* standard c-libraries */
#include <stdio.h>

/* Chipmunk physics library */
#include "chipmunk.h"

/* Game state */
#include "main.h"
#include "menu.h"
#include "space.h"

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

static int sel = 1;
static int max_sel = 20;
static float ypos = 0;
static float yspeed = 0;
static float size = 300;
static float scale = 1;
static float spaceing = 10;
static void init()
{

}

static void update()
{

	if (keys[SDLK_w] || keys[SDLK_UP]){
		sel--;
		sel = (sel <= 0) ? max_sel : sel;
		keys[SDLK_w] = 0, keys[SDLK_UP] = 0;
	}
	if (keys[SDLK_s] || keys[SDLK_DOWN]){
		sel++;
		sel = sel > max_sel ? 1 : sel;
		keys[SDLK_s] = 0, keys[SDLK_DOWN] = 0;
	}
	if(keys[SDLK_ESCAPE]){
		currentState = &state_menu;
		keys[SDLK_ESCAPE]=0;
	}
	if (keys[SDLK_SPACE] || keys[SDLK_RETURN]) {
		currentState = &state_space;
		change_current_menu(INGAME_MENU_ID);
		keys[SDLK_SPACE] = 0, keys[SDLK_RETURN] = 0;
	}
	yspeed = -(ypos - sel*size - size*2 - spaceing)/20;
	ypos += yspeed;
	scale = ((1.0f * HEIGHT)/(ypos*2));
}

static void render()
{
	static char string[9];
	glPushMatrix();
	glScalef(scale,scale,1);
	//glTranslatef(0,ypos,0);

	int i = 0;
	glColor3f(0.1,0.1,0.1);
	draw_circle(0,0,size);
	for(i = 1; i < max_sel+1; i++){
		if(sel == i){
			glColor3f(1.0f,0,0);
		}else{
			glColor3f(0,0,1.0f);
		}
		draw_donut(0,0,i*size + spaceing,i*size + size);
		glColor3f(1,1,1);
		sprintf(string,"LEVEL %d",i);
		int textpos = -i*size-size/2;
		if(i == 0) textpos = 0;
		setTextSize(30);
		setTextAlign(TEXT_CENTER);
		font_drawText(0,textpos,string);
	}
	glPopMatrix();
	glLoadIdentity();
	sprintf(string,"LEVEL %d",sel);
	setTextAlign(TEXT_LEFT);
	font_drawText(-WIDTH/2 + 10,HEIGHT/2 - 25,string);
}

static void destroy()
{

}
