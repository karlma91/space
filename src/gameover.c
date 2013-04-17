/* header */
#include "gameover.h"


/* standard c-libraries */
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Game state */
#include "main.h"
#include "menu.h"
#include "space.h"
#include "levelselect.h"
#include "statesystem.h"

/* Drawing */
#include "draw.h"
#include "font.h"

/* Game components */
#include "highscorelist.h"

/* static prototypes */
static void on_enter();
static void on_leave();
static void update();
static void draw();
static void destroy();
static void draw_highscore();

#define MAX_NAME_LENGTH 3

/* static variables */
static char input[MAX_NAME_LENGTH+1] = "   ";
static int valid_index[MAX_NAME_LENGTH];
static const char valid_char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
static const int char_count = 37; /* valid_char length*/
static scorelist * list;

void gameover_init()
{
	list = malloc(sizeof(scorelist));
	highscorelist_create(list);
	highscorelist_readfile(list,"bin/data/highscores");

	statesystem_init_state(STATESYSTEM_GAMEOVER,0, on_enter,update,NULL,draw, on_leave, destroy);

}


static void on_enter()
{
	if(config.arcade){
		printf("exit %d\n", getPlayerScore());
		statesystem_set_state(STATESYSTEM_MENU);
		main_stop();
		return;
	}
}
static void on_leave()
{

}
static enum gameover_state gameover_state = enter_name;

static int cursor = 0;
static int win = 0; //TMP solution for win screens

static void update() {
/*{ //DEBUG CODE
	if (keys[SDLK_RETURN]) {
		gameover_state = (1+gameover_state)%3;
		keys[SDLK_RETURN] = 0;
		return;
	}
*/

	static float key_dt = 0;
	static float key_ddt = 0.25;
	static const float key_ddt_min = 0.12f;

	switch(gameover_state) {
	case GAMEOVER_WIN:
		win = 1;
		/* no break */
	case enter_name:
		if (keys[SDLK_DOWN]) {
			if (key_dt<=0) {
				key_dt = key_ddt;
				key_ddt = key_ddt_min;

				if (++valid_index[cursor] >= char_count) valid_index[cursor] -= char_count;
			}
		} else if (keys[SDLK_UP]) {
			if (key_dt<=0) {
				key_dt = key_ddt;
				key_ddt = key_ddt_min;

				if (--valid_index[cursor] < 0) valid_index[cursor] += char_count;
			}
		} else {
			key_dt = 0;
			key_ddt = 0.25;
		}
		key_dt -= dt;

		if (keys[SDLK_RIGHT] || keys[SDLK_RETURN]) {
			if (++cursor >= MAX_NAME_LENGTH) {
				cursor = 0;
				gameover_state = confirm_name;
				win = 0;
			}
			keys[SDLK_RIGHT] = 0;
			keys[SDLK_RETURN] = 0;
		} else if (keys[SDLK_LEFT] || keys[SDLK_ESCAPE]) {
			if (cursor > 0) --cursor;
			keys[SDLK_LEFT] = 0;
			keys[SDLK_ESCAPE] = 0;
		}

		input[cursor] = valid_char[valid_index[cursor]];
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
		if (keys[SDLK_ESCAPE] || keys[SDLK_RETURN]) {
		    menu_change_current_menu(MENU_MAIN);
		    statesystem_set_state(STATESYSTEM_MENU);
			keys[SDLK_ESCAPE] = 0;
			keys[SDLK_RETURN] = 0;
		}
		break;
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

	if (gameover_state != show_highscore) {
		if (win)
			font_drawText(0,0.4f*HEIGHT, "YOU WON");
		else
			font_drawText(0,0.4f*HEIGHT, "GAME OVER");
	}
	setTextSize(60);

	switch(gameover_state) {
	case GAMEOVER_WIN:
		/*no break*/
	case enter_name:
		font_drawText(0,0, &input[0]);
		font_drawText(1.5f*60*(cursor - 1),-60/4, "_");

		char tmp[2];
		tmp[1] = '\0';
		int draw_offset = 3;
		int from = ((valid_index[cursor] - draw_offset));
		int to = (valid_index[cursor] + draw_offset);
		int k = -draw_offset;
		glColor3f(0.3,0.3,0.3);
		int j;
		for(j=from; j<=to; j++,k++){
			tmp[0] = valid_char[j < 0 ? j + char_count : j % char_count];
			if(k){
				font_drawText(1.5f*60*(cursor - 1),-k*100,tmp);
			}
		}
		break;
	case confirm_name:
		font_drawText(0,0, &input[0]);
		setTextSize(25);
		if (timer<1) {
			font_drawText(200,-100,"RIGHT TO CONFIRM");
			font_drawText(-200,-150,"LEFT TO RENAME");
		} else if(timer>=2) timer=0;
		break;
	case show_highscore:
		draw_highscore();
		setTextSize(80);
		setTextAlign(TEXT_CENTER);
		font_drawText(0,0.4f*HEIGHT, "HIGHSCORES");
		break;
	}
}

static void destroy()
{
	highscorelist_writefile(list,"bin/data/highscores");
	highscorelist_destroy(list);
	free(list);
}

char * covertToUpper(char *str)
    {
        int i = 0;
        int len = 0;

        len = strlen(str);

        for(i = 0; str[i]; i++)
        {
           str[i] = toupper(str[i]);
        }
        //terminate string
        str[i]= '\0';
        return str;

    }

static void draw_highscore()
{
	scoreelement score = {"----",0,0,0};
	char temp[100];
	int i;
	setTextAlign(TEXT_LEFT);
	setTextSize(20);
	for(i=0;i<10;i++){
		if(highscorelist_getscore(list,i+1,&score) != 0) {

		}
		time_t tim = (time_t)(score.time);
		 struct tm *tmm = gmtime(&tim);

		sprintf(temp,"%2d %-5s %10d         %02d-%02d-%02d", (i+1), score.name, score.score,tmm->tm_mday,tmm->tm_mon+1,tmm->tm_year%100);
		font_drawText(-10*40*1.5f, 300 - i*50*1.5f, temp);
	}
}

int gameover_setstate(enum gameover_state state)
{
	gameover_state = state;
	return 0;
}
