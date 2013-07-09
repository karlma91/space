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


//extern SPRITE_ID start
SPRITE_ID SPRITE_PLAYER;
SPRITE_ID SPRITE_PLAYER_GUN;
SPRITE_ID SPRITE_GLOW_DOT;
SPRITE_ID SPRITE_BUTTON;
SPRITE_ID SPRITE_TANK_BODY;
SPRITE_ID SPRITE_TANK_WHEEL;
SPRITE_ID SPRITE_TANK_TURRET;
SPRITE_ID SPRITE_BUTTON_PAUSE;
//extern SPRITE_ID end
//extern particles
int EMITTER_FLAME;
int EMITTER_ROCKET_FLAME;
int EMITTER_EXPLOSION;
int EMITTER_SPARKS;
int EMITTER_SMOKE;
int EMITTER_SCORE;
int EMITTER_FRAGMENTS;
int EMITTER_COUNT;
//extern particles end

/* GLOBAL SOUND CHUNKS */
Mix_Chunk *SND_LASER_1;
Mix_Chunk *SND_LASER_2;
Mix_Chunk *SND_EXPLOSION;

/* GLOBAL MUSIC TRACKS */
Mix_Music *MUSIC_MENU;
Mix_Music *MUSIC_LEVEL;
Mix_Music *MUSIC_GAMEOVER;


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

void game_sprites()
{
	SPRITE_PLAYER = 			sprite_link("player");
	SPRITE_PLAYER_GUN =			sprite_link("player_gun");
	SPRITE_GLOW_DOT =			sprite_link("glow_dot");
	SPRITE_BUTTON  =			sprite_link("button");
	SPRITE_BUTTON_PAUSE =		sprite_link("button_pause");
	SPRITE_TANK_BODY =			sprite_link("tank_body");
	SPRITE_TANK_WHEEL =			sprite_link("tank_wheel");
	SPRITE_TANK_TURRET =		sprite_link("tank_turret");
}

void game_particles()
{
#define PARTICLE_STRESS 1
#if PARTICLE_STRESS
    EMITTER_FLAME =         read_emitter_from_file("flame_stress.xml");
#else
    EMITTER_FLAME =         read_emitter_from_file("flame_3.xml");
#endif
    EMITTER_ROCKET_FLAME =  read_emitter_from_file("rocket_flame.xml");
    EMITTER_EXPLOSION =     read_emitter_from_file("explosion_ground.xml");
    EMITTER_SPARKS =        read_emitter_from_file("sparks.xml");
    EMITTER_SMOKE =         read_emitter_from_file("smoke.xml");
    EMITTER_SCORE =         read_emitter_from_file("score.xml");
    EMITTER_FRAGMENTS =     read_emitter_from_file("fragments.xml");
}

void game_audio()
{
	SND_LASER_1 = sound_load_chunk("laser_01.ogg");
	SND_LASER_2 = sound_load_chunk("laser_02.ogg");
	SND_EXPLOSION = sound_load_chunk("explosion.ogg");

	MUSIC_MENU = sound_load_music("Broken_Logic.ogg");
	MUSIC_LEVEL = sound_load_music("Blocked Bus.ogg");
	MUSIC_GAMEOVER = sound_load_music("Idling.ogg");
}

void game_init()
{
	//TODO generalize particles.c and ?level.c
	game_sprites();
	game_audio();
	game_particles();
	mainmenu_init();
	level_init();
    menu_init();
    space_init();
    gameover_init();
    levelselect_init();

    //statesystem_set_state(state_menu);
    statesystem_set_state(state_mainmenu);
}
