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

static int score_position;
static int score_value;
static int score_newly_added;

void gameover_init()
{
	list = malloc(sizeof(scorelist));
	highscorelist_create(list);
	highscorelist_readfile(list,"bin/data/highscores");

	statesystem_init_state(STATESYSTEM_GAMEOVER,0, on_enter,update,NULL,draw, on_leave, destroy);

}


static void on_enter()
{

}
static void on_leave()
{

}
static enum gameover_state gameover_state = enter_name;

static int cursor = 0;
static int win = 0; //TMP solution for win screens

static void update() {
	static float key_dt = 0;
	static float key_ddt = 0.25;
	static const float key_ddt_min = 0.12f;

	switch(gameover_state) {
	case GAMEOVER_WIN:
		win = 1;
		/* no break */
	case enter_name:
		if (keys[KEY_DOWN_2] || keys[KEY_DOWN_1]) {
			if (key_dt<=0) {
				key_dt = key_ddt;
				key_ddt = key_ddt_min;

				if (++valid_index[cursor] >= char_count) valid_index[cursor] -= char_count;
			}
		} else if (keys[KEY_UP_2] || keys[KEY_UP_1]) {
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

		if (keys[KEY_RIGHT_2] || keys[KEY_RIGHT_1] || keys[KEY_RETURN_2] || keys[KEY_RETURN_1]) {
			if (++cursor >= MAX_NAME_LENGTH) {
				cursor = 0;
				gameover_state = confirm_name;
			}
			keys[KEY_RIGHT_2] = 0,keys[KEY_RIGHT_1] = 0, keys[KEY_RETURN_1] = 0, keys[KEY_RETURN_2] = 0;
		} else if (keys[KEY_LEFT_2] ||keys[KEY_LEFT_1] || keys[KEY_ESCAPE]) {
			if (cursor > 0) --cursor;
			keys[KEY_LEFT_2] = 0,keys[KEY_LEFT_1] = 0, keys[KEY_ESCAPE] = 0;
		}

		input[cursor] = valid_char[valid_index[cursor]];
		break;
	case confirm_name:
		if (keys[KEY_LEFT_2] || keys[KEY_LEFT_1]) {
			gameover_state = enter_name;
			keys[KEY_LEFT_2] = 0;
			keys[KEY_LEFT_1] = 0;
		} else if (keys[KEY_RIGHT_2] || keys[KEY_RIGHT_1]) {
			/* add score */
			gameover_state = show_highscore;
			win = 0;
			score_value = getPlayerScore();
			score_position = highscorelist_addscore(list,&input[0], score_value);
			score_newly_added = 1;
		}
		break;
	case show_highscore:
		if (keys[KEY_ESCAPE] || keys[KEY_RETURN_2] || keys[KEY_RETURN_1]) {
			score_newly_added = 0;
			if(config.arcade){
				printf("exit %d\n", getPlayerScore());
				main_stop();
				return;
			}
		    menu_change_current_menu(MENU_MAIN);
		    statesystem_set_state(STATESYSTEM_MENU);
			keys[KEY_ESCAPE] = 0;
			keys[KEY_RETURN_1] = 0;
			keys[KEY_RETURN_2] = 0;
			win = 0;
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
			font_drawText(200,-100,"MOVE RIGHT TO CONFIRM");
			font_drawText(-200,-150,"MOVE LEFT TO RENAME");
		} else if(timer>=2) timer=0;
		break;
	case show_highscore:
		setTextSize(80);
		setTextAlign(TEXT_CENTER);
		font_drawText(0,0.4f*HEIGHT, "HIGHSCORES");

		draw_highscore();

		if (score_newly_added) {
			char current_score_buffer[100];
			setTextSize(35);
			setTextAlign(TEXT_CENTER);
			sprintf(&current_score_buffer[0], "%s FIKK %d. PLASS MED %d POENG!", &input[0], score_position, score_value);
			glColor3f(1,1,1);
			font_drawText(0,-0.4f*HEIGHT, &current_score_buffer[0]);
		}
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
	scoreelement score = {"---",0,0,0};
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
