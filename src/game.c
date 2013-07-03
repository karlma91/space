#include "game.h"
#include "main.h"
#include "menu.h"
#include "space.h"
#include "gameover.h"
#include "mainmenu.h"
#include "levelselect.h"

#if !(TARGET_OS_IPHONE || __ANDROID__)
#include "ini.h"
#endif

//TODO move game specific code from main.c into this file
unsigned int KEY_UP_1 = SDL_SCANCODE_W;
unsigned int KEY_UP_2 = SDL_SCANCODE_UP;
unsigned int KEY_LEFT_1 = SDL_SCANCODE_A;
unsigned int KEY_LEFT_2 = SDL_SCANCODE_LEFT;
unsigned int KEY_RIGHT_1 = SDL_SCANCODE_D;
unsigned int KEY_RIGHT_2 = SDL_SCANCODE_RIGHT;
unsigned int KEY_DOWN_1 = SDL_SCANCODE_S;
unsigned int KEY_DOWN_2 = SDL_SCANCODE_DOWN;

unsigned int KEY_RETURN_1 = SDL_SCANCODE_SPACE;
unsigned int KEY_RETURN_2 = SDL_SCANCODE_RETURN;
unsigned int KEY_ESCAPE = SDL_SCANCODE_ESCAPE;

#if !(TARGET_OS_IPHONE || __ANDROID__)
static int handler(void* config, const char* section, const char* name,
		const char* value) {
	configuration* pconfig = (configuration*) config;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("video", "fullscreen")) {
		pconfig->fullscreen = atoi(value);
	}else if (MATCH("video", "arcade")) {
		pconfig->arcade = atoi(value);
	} else if (MATCH("video", "arcade_keys")) {
		pconfig->arcade_keys = atoi(value);

	} else if (MATCH("video", "width")) {
		pconfig->width = atoi(value);
	} else if (MATCH("video", "height")) {
		pconfig->height = atoi(value);
	} else if (MATCH("keyboard", "key_left")) {
		pconfig->key_left = atoi(value);
	} else if (MATCH("keyboard", "key_up")) {
		pconfig->key_up = atoi(value);
	} else if (MATCH("keyboard", "key_right")) {
		pconfig->key_right = atoi(value);
	} else if (MATCH("keyboard", "key_down")) {
		pconfig->key_down = atoi(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}
#endif

void game_config()
{
#if !(TARGET_OS_IPHONE || __ANDROID__)
	if (ini_parse("bin/config.ini", handler, &config) < 0) {
		SDL_Log("Could not load 'bin/config.ini'\n");
		exit(-1);
	}
#else

#endif

#if ARCADE_MODE
		KEY_UP_2 = SDL_SCANCODE_UP;
		KEY_UP_1 = SDL_SCANCODE_W;
		KEY_LEFT_2 = SDL_SCANCODE_LEFT;
		KEY_LEFT_1 = SDL_SCANCODE_A;
		KEY_RIGHT_2 = SDL_SCANCODE_RIGHT;
		KEY_RIGHT_1 = SDL_SCANCODE_D;
		KEY_DOWN_2 = SDL_SCANCODE_DOWN;
		KEY_DOWN_1 = SDL_SCANCODE_S;

		KEY_RETURN_1 = SDL_SCANCODE_K;
		KEY_RETURN_2 = SDL_SCANCODE_G;
		KEY_ESCAPE = SDL_SCANCODE_ESCAPE;
#endif
}

void game_init()
{
	//TODO generalize particles.c and ?level.c
	mainmenu_init();
	level_init();
    menu_init();
    space_init();
    gameover_init();
    levelselect_init();

    statesystem_set_state(state_menu);
    //statesystem_set_state(state_mainmenu);
}
