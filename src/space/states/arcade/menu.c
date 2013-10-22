#include "menu.h"
#include "../../game.h"
#include "../space.h"
#include "gameover.h"
#include "we_defstate.h"

STATE_ID state_menu;

static button btn_start_tmp;

#define MAX_MENU_ITEMS 5

static void inner_main(void);
static void inner_ingame(void);

struct menu {
	int num_items;
	int escape_item;
	void (*func)(void);
	char texts[MAX_MENU_ITEMS][20];
};


static struct menu mainMenuTest = {
		5,
		4,
		inner_main,
		{"START GAME","LEVELS","HIGHSCORES","CREDITS","EXIT"}
};

static struct menu ingameMenu = {
		3,
		0,
		inner_ingame,
		{"RESUME","RESTART","MAINMENU"}
};

static struct menu *curMenu; //current active menu
static int select_id = 0;
static int current_menu;

static const Color col_item   = {255,0,  0,255};
static const Color col_select = {  0,0,255,255};

static void sdl_event(SDL_Event *event)
{
	SDL_Scancode key;
	switch (event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;

#if !ARCADE_MODE // because there is no menus in arcade mode
		if (key == KEY_UP_1 || key == KEY_UP_2) {
			--select_id;
			select_id = (select_id < 0) ? curMenu->num_items-1 : select_id;
		} else

		if (key == KEY_DOWN_1 || key == KEY_DOWN_2) {
			select_id++;
			select_id = select_id >= curMenu->num_items ? 0 : select_id;
		}

		if (key == KEY_RETURN_2 || key == KEY_RETURN_1) {
			curMenu->func();
		} else

		if (key == KEY_ESCAPE){
			select_id = curMenu->escape_item;
			curMenu->func();
		}
#endif
		break;
	case SDL_FINGERDOWN:
		//button_finger_down(btn_start_tmp, &event->tfinger);
		break;
	case SDL_FINGERMOTION:
		//button_finger_move(btn_start_tmp, &event->tfinger);
		break;
	case SDL_FINGERUP:
		//if (button_finger_up(btn_start_tmp, &event->tfinger)) {
		//	curMenu->func();
		//}
		break;
	}
}

static void on_enter(void)
{
	select_id = 0;
	sound_music(MUSIC_MENU);
}

static void on_pause(void)
{
}

static void on_leave(void)
{
}

void menu_change_current_menu(int menu)
{
	current_menu = menu;
	switch(menu){
	case MENU_INGAME:
		curMenu = &ingameMenu;
		break;
	case MENU_MAIN:
		curMenu = &mainMenuTest;
		break;
	}
}

static void pre_update(void)
{
	if (keys[KEY_RETURN_2] || keys[KEY_RETURN_1]) {
		switch(current_menu){
			case MENU_MAIN:
				space_start_demo(1,1);
				break;
			case MENU_INGAME:
				statesystem_set_state(state_space);
				break;
			}
		keys[KEY_RETURN_2] = 0;
		keys[KEY_RETURN_1] = 0;
	}

	if (keys[KEY_ESCAPE]){
		menu_change_current_menu(MENU_MAIN);
		statesystem_set_state(state_menu);
		keys[KEY_ESCAPE] = 0;
	}
}

static void post_update(){}

static void draw(void)
{
	draw_load_identity();

	draw_color4f(0,0,0,0.5f);
	draw_box(cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);

	static float timer;
	timer +=dt;

	setTextAngle(0);
	setTextAlign(TEXT_CENTER);
	switch(current_menu){
	case MENU_INGAME:
		draw_color4f(1,1,1,1);
		setTextSize(80);
		setTextAlign(TEXT_CENTER);
		font_drawText(0,0.6f*GAME_HEIGHT/2, "GAME PAUSED");
		break;
	case MENU_MAIN:
		setTextSize(120);
		//drawStars();
		draw_color(draw_col_rainbow((int)(timer*1000)));
		font_drawText(0,0.7f*GAME_HEIGHT/2, "SPACE");

		draw_color4f(1,1,1,1);

		setTextSize(30);
		font_drawText(-GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2, "STYR");
		font_drawText(-GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2-50, "VVV");

		font_drawText(+GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2, "SKYT");
		font_drawText(+GAME_WIDTH*0.4,-0.8f*GAME_HEIGHT/2-50, "VVV");

		setTextAlign(TEXT_CENTER);
		setTextSize(40);


		font_drawText(0,-0.5f*GAME_HEIGHT/2, "START SPILLET");

		draw_color4f(0.1,0.9,0.1,1);
		btn_start_tmp->calls->render(btn_start_tmp);

		break;
	}
}

static void inner_main(void)
{
	switch (select_id) {
	case 0: //START GAME
		space_start_demo(1,1);
		break;
	case 1: //LEVEL SELECT

		break;
	case 2: //HIGHSCORE
		gameover_showhighscores();
		break;
	case 3: //CREDITS
		break;
	case 4: //EXIT
		main_stop();
		break;
	default:
		break;
	}
}

static void inner_ingame(void)
{
	switch (select_id) {
	case 0: //RESUME GAME
		statesystem_set_state(state_space);
		break;
	case 1: //RESTART GAME
		space_start_demo(1,1);
		break;
	case 2:
		curMenu = &mainMenuTest;
		statesystem_set_state(state_menu);
		break;
	default:
		break;
	}
}

static void destroy(void)
{

}

void menu_init(void)
{
	curMenu = &mainMenuTest;
	statesystem_register(state_menu, 0);

	btn_start_tmp = button_create(SPRITE_BUTTON, 0, "TXT", 0, 0, 192*2, 128*2);
}
