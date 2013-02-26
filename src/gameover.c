/* header */
#include "gameover.h"

/* standard c-libraries */
#include <stdio.h>
#include <string.h>

/* Game state */
#include "main.h"
#include "menu.h"
#include "space.h"
#include "levelselect.h"

/* Drawing */
#include "draw.h"
#include "font.h"

/* Game components */
#include "highscorelist.h"

/* static prototypes */
static void gameover_init();
static void gameover_update();
static void gameover_draw();
static void gameover_destroy();
static void draw_highscore();

state state_gameover = {
		gameover_init,
		gameover_draw,
		gameover_update,
		gameover_destroy,
		NULL
};

#define MAX_NAME_LENGTH 4

/* static variables */
static char input[MAX_NAME_LENGTH+1] = "    ";
static int valid_index[MAX_NAME_LENGTH];
static const char valid_char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
static const int char_count = 37; /* valid_char length*/
static scorelist * list;

static void gameover_init()
{
	list = malloc(sizeof(scorelist));
	highscorelist_create(list);
	highscorelist_readfile(list,"bin/data/highscores");
}

enum gameover_state_ {
	enter_name,
	confirm_name,
	show_highscore
};

static int gameover_state = enter_name;

static int i = 0;
static void gameover_update()
{
	if (keys[SDLK_RETURN]) {
		gameover_state = (++gameover_state)%3;
		keys[SDLK_RETURN] = 0;
		return;
	}

	static float key_dt = 0;
	static float key_ddt = 0.25;
	static const float key_ddt_min = 0.12f;

	switch(gameover_state) {
	case enter_name:

		if (keys[SDLK_UP]) {
			if (key_dt<=0) {
				key_dt = key_ddt;
				key_ddt = key_ddt_min;

				if (++valid_index[i] >= char_count) valid_index[i] -= char_count;
			}
		} else if (keys[SDLK_DOWN]) {
			if (key_dt<=0) {
				key_dt = key_ddt;
				key_ddt = key_ddt_min;

				if (--valid_index[i] < 0) valid_index[i] += char_count;
			}
		} else {
			key_dt = 0;
			key_ddt = 0.25;
		}
		key_dt -= dt;

		if (keys[SDLK_RIGHT]) {
			if (++i >= MAX_NAME_LENGTH) i -= MAX_NAME_LENGTH;
			keys[SDLK_RIGHT] = 0;
		} else if (keys[SDLK_LEFT]) {
			if (--i < 0) i += MAX_NAME_LENGTH;
			keys[SDLK_LEFT] = 0;
		}

		input[i] = valid_char[valid_index[i]];
		break;
	case confirm_name:
		if (keys[SDLK_LEFT]) {
			gameover_state = enter_name;
			keys[SDLK_LEFT] = 0;
		} else if (keys[SDLK_RIGHT]) {
			/* add score */
			gameover_state = show_highscore;
			highscorelist_addscore(list,&input[0],getPlayerScore());
		}
		break;
	case show_highscore:

		break;
	}
}

static void gameover_draw()
{
	static float timer;
	timer +=dt;

	setTextAngle(0);
	setTextSize(80);
	setTextAlign(TEXT_CENTER);
	glColor_from_color(draw_col_rainbow((int)(timer*1000)));
	font_drawText(0,0.4f*HEIGHT, "GAME OVER");
	setTextSize(60);

	switch(gameover_state) {
	case enter_name:
		font_drawText(0,0, &input[0]);
		font_drawText(1.5f*60*(i-2+0.5f),-60/4, "_");
		break;
	case confirm_name:
		font_drawText(0,0, &input[0]);
		setTextSize(30);
		if (timer<1) font_drawText(0,-100,"PRESS RIGHT TO CONFIRM"); else if(timer>=2) timer=0;
		break;
	case show_highscore:
		draw_highscore();
		break;
	}
}

static void gameover_destroy()
{
	highscorelist_writefile(list,"bin/data/highscores");
	highscorelist_destroy(list);
	free(list);
}

static void draw_highscore()
{
	char name[5];
	char temp[50];
	int score;
	int i;
	setTextAlign(TEXT_LEFT);
	setTextSize(40);
	for(i=0;i<10;i++){
		if(highscorelist_getscore(list,i+1,name,&score) == 0){
			sprintf(temp,"%-9s %10d",name, score);
			font_drawText(-10*40*1.5f, 300 - i*50, temp);
		}else{

		}
	}
}
