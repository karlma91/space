/* header */
#include "gameover.h"

/* standard c-libraries */
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "game.h"

/* Game state */
#include "main.h"
#include "menu.h"
#include "space.h"
#include "levelselect.h"
#include "state.h"

/* Drawing */
#include "draw.h"
#include "font.h"

/* Game components */
#include "highscorelist.h"

STATE_ID state_gameover;

/* static prototypes */
static void draw_highscore();

#define MAX_NAME_LENGTH 3

/* static variables */
static char input[MAX_NAME_LENGTH+1] = "A  ";
static int valid_index[MAX_NAME_LENGTH];
static const char valid_char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
static const int char_count = 37; /* valid_char length*/
static scorelist * list;

static int score_position;
static int score_value;
static int score_newly_added;

static enum gameover_state gameover_state = enter_name;

static int cursor = 0;
static int win = 0; //TMP solution for win screens

static int score_index = 0;
static int score_page = 0;

static void sdl_event(SDL_Event *event)
{
	SDL_Scancode key;
	switch (event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;
		switch(gameover_state) {
		case enter_name:
			if (key == KEY_RIGHT_2 || key == KEY_RIGHT_1 || key == KEY_RETURN_2 || key == KEY_RETURN_1) {
				if (++cursor >= MAX_NAME_LENGTH) {
					cursor = 0;
					gameover_state = confirm_name;
				}
			} else if (key == KEY_LEFT_2 ||key == KEY_LEFT_1 || key == KEY_ESCAPE) {
				if (cursor > 0) --cursor;
			}
			break;
		case confirm_name:
			if (key == KEY_LEFT_2 || key == KEY_LEFT_1 || key == KEY_ESCAPE) {
				gameover_state = enter_name;
			} else if (key == KEY_RIGHT_2 || key == KEY_RIGHT_1) {
				/* add score */
				gameover_state = show_highscore;
				win = 0;
				score_value = getPlayerScore();
				score_position = highscorelist_addscore(list,&input[0], score_value);
				score_newly_added = 1;
			}
			break;
		case show_highscore:
			if (key == KEY_DOWN_1 || key == KEY_DOWN_2) {
					++score_page;
			} else if (key == KEY_UP_1 || key == KEY_UP_2) {
				if (score_page > 0) {
					--score_page;
				}
			} else if (key == KEY_RETURN_2 || key == KEY_RETURN_1 || key == KEY_ESCAPE) {
				score_newly_added = 0;
				score_index = 0;
				score_page = 0;
#if ARCADE_MODE
				printf("exit %d\n", getPlayerScore());
				main_stop();
				return;
#else
				menu_change_current_menu(MENU_MAIN);
				statesystem_set_state(state_menu);
				win = 0;
#endif
			}
			break;
		default:
			break;
		}
		break;
		case SDL_FINGERDOWN:
			button_finger_down(btn_fullscreen, &event->tfinger);
			break;
		case SDL_FINGERUP:
			if (button_finger_up(btn_fullscreen, &event->tfinger)) {
				switch(gameover_state) {
				case enter_name:
					if (++cursor >= MAX_NAME_LENGTH) {
						cursor = 0;
						gameover_state = confirm_name;
					}
					break;
				case confirm_name:
					/* add score */
					gameover_state = show_highscore;
					win = 0;
					score_value = getPlayerScore();
					score_position = highscorelist_addscore(list,&input[0], score_value);
					score_newly_added = 1;
					break;
				case show_highscore:
					if (++score_page > 2) {
						score_newly_added = 0;
						score_index = 0;
						score_page = 0;
#if ARCADE_MODE
						printf("exit %d\n", getPlayerScore());
						main_stop();
						return;
#else
						menu_change_current_menu(MENU_MAIN);
						statesystem_set_state(state_menu);
						win = 0;
#endif
					}
					break;
				default:
					break;
				}
			}
			break;
	}
}

void gameover_init()
{
	list = malloc(sizeof(scorelist));
	highscorelist_create(list);
	highscorelist_readfile(list,"highscores"); // NB! moved from bin/data/highscores

	statesystem_register(state_gameover,0);
}


static void on_enter()
{

}
static void on_leave()
{

}

static void pre_update() {
	static float key_dt = 0;
	static float key_ddt = 0.25;
	static const float key_ddt_min = 0.12f;

	switch(gameover_state) {
	case GAMEOVER_WIN:
		win = 1;
		gameover_state = enter_name;
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

		input[cursor] = valid_char[valid_index[cursor]];
		break;
	case confirm_name:
		break;
	case show_highscore:
		// highscore scrolling
		if (score_index < score_page*10) {
			++score_index;
		} else if (score_index > score_page*10) {
			--score_index;
		}

		break;
	}
}

Color color;
static void draw()
{
	static float timer;
	timer +=dt;

	setTextAngle(0);
	setTextSize(80);
	setTextAlign(TEXT_CENTER);
	color = draw_col_rainbow((int)(timer*1000));
	draw_color(color);

	if (gameover_state != show_highscore) {
		if (win)
			font_drawText(0,0.4f*GAME_HEIGHT, "YOU WON");
		else
			font_drawText(0,0.4f*GAME_HEIGHT, "GAME OVER");
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
		draw_color4f(0.3,0.3,0.3,1);
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
		font_drawText(0,0.4f*GAME_HEIGHT, "HIGHSCORES");

		draw_highscore();

		if (score_newly_added) {
			char current_score_buffer[100];
			setTextSize(30);
			setTextAlign(TEXT_CENTER);
			draw_color(color);
			sprintf(&current_score_buffer[0], "%s FIKK %d. PLASS MED %d POENG!", &input[0], score_position, score_value);
			draw_color4f(1,1,1,1);
			font_drawText(0,-0.4f*GAME_HEIGHT, &current_score_buffer[0]);
		}
		break;
	}
}

static void destroy()
{
	highscorelist_writefile(list);
	highscorelist_destroy(list);
	free(list);
}

static void draw_highscore(int start_index)
{
	scoreelement score = {"---",0,0,0};
	char temp[100];
	int i;
	setTextAlign(TEXT_LEFT);
	setTextSize(20);
	for(i=0;i<10;i++){
		int position = i+1+score_index;
		if(highscorelist_getscore(list,position,&score) != 0) {

		}
		time_t tim = (time_t)(score.time);
		 struct tm *tmm = gmtime(&tim);

		sprintf(temp,"%9d %-5s %10d      %02d-%02d-%02d", position, score.name, score.score,tmm->tm_mday,tmm->tm_mon+1,tmm->tm_year%100);

		if (score_position == position) {
			draw_color(color);
		} else {
			draw_color4f(1,1,1,1);
		}
		font_drawText(-10*45*1.5f, 300 - i*50*1.5f, temp);
	}
}

int gameover_setstate(enum gameover_state state)
{
	gameover_state = state;
	return 0;
}

static void post_update() {}
