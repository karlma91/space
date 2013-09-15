#include "game.h"
#include "../engine/engine.h"

#include "cJSON.h"

#include "states/space.h"
//#include "states/stations.h"
#include "states/leveldone.h"
#include "states/levelscreen.h"
#include "obj/object_types.h"
//#include "states/pause.h"
//#include "states/upgrades.h"
//#include "states/leveldone.h"
//#include "states/settings.h"

extern void pause_init(void);
extern void settings_init(void);
extern void upgrades_init(void);
extern void stations_init(void);


#include "states/arcade/menu.h"
#include "states/arcade/gameover.h"

#if !(TARGET_OS_IPHONE || __ANDROID__)
#include "ini.h"
#endif

/* Default keybindings for desktop version */
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


/* COMPONENTS DEFINITIONS */
int CMP_HPBAR;
int CMP_COINS;
//int CMP_SCORE;
int CMP_DAMAGE;
int CMP_BODIES;
int CMP_MINIMAP;
int CMP_CREATOR;
/* END OF COMPONENTS DEFINITIONS */

/* global sprite definitions */
SPRITE_ID SPRITE_PLAYER;
SPRITE_ID SPRITE_PLAYER_GUN;
SPRITE_ID SPRITE_GLOW_DOT;
SPRITE_ID SPRITE_BUTTON;
SPRITE_ID SPRITE_TANK_BODY;
SPRITE_ID SPRITE_TANK_WHEEL;
SPRITE_ID SPRITE_TANK_TURRET;
SPRITE_ID SPRITE_BUTTON_PAUSE;
SPRITE_ID SPRITE_STATION_01;
SPRITE_ID SPRITE_STAR;
SPRITE_ID SPRITE_GEAR;
SPRITE_ID SPRITE_RETRY;
SPRITE_ID SPRITE_NEXT;
SPRITE_ID SPRITE_HOME;
SPRITE_ID SPRITE_JOYSTICK;
SPRITE_ID SPRITE_JOYSTICK_BACK;
SPRITE_ID SPRITE_SPIKEBALL;
SPRITE_ID SPRITE_SAW;
SPRITE_ID SPRITE_COIN;
SPRITE_ID SPRITE_TURRET;
/* end of global sprite definitions*/

/* polyshapes */
POLYSHAPE_ID POLYSHAPE_RAMP;
POLYSHAPE_ID POLYSHAPE_TURRET;
POLYSHAPE_ID POLYSHAPE_TANK;

/* end*

/* global emitter */
int EMITTER_FLAME;
int EMITTER_ROCKET_FLAME;
int EMITTER_EXPLOSION;
int EMITTER_SPARKS;
int EMITTER_SMOKE;
int EMITTER_SCORE;
int EMITTER_FRAGMENTS;
int EMITTER_COUNT;
/* extern particles end */

//extern fonts
bm_font * FONT_NORMAL;
bm_font * FONT_BIG;

/* GLOBAL SOUND CHUNKS */
Mix_Chunk *SND_LASER_1;
Mix_Chunk *SND_LASER_2;
Mix_Chunk *SND_LASER_MISS;
Mix_Chunk *SND_HIT_1;
Mix_Chunk *SND_HIT_2;
Mix_Chunk *SND_TANK_EXPLODE;
Mix_Chunk *SND_FACTORY_EXPLODE;
Mix_Chunk *SND_TURRET_EXPLODE;
Mix_Chunk *SND_COIN;
Mix_Chunk *SND_EXPLOSION;

/* GLOBAL MUSIC TRACKS */
Mix_Music *MUSIC_MENU;
Mix_Music *MUSIC_LEVEL;
Mix_Music *MUSIC_GAMEOVER;

/* GLOBAL TOUCHABLES */
button btn_settings;


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

void game_config(void)
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

void game_components(void)
{
	REGISTER_CMP(HPBAR, 1);
	REGISTER_CMP(COINS, 1);
	//REGISTER_CMP(SCORE, 1);
	REGISTER_CMP(DAMAGE, 1);
	REGISTER_CMP(BODIES, SPACE_BODIES_MAX);
	REGISTER_CMP(MINIMAP, 1);
	REGISTER_CMP(CREATOR, 1);
}

void game_sprites(void)
{
	REGISTER_SPRITE( PLAYER );
	REGISTER_SPRITE( PLAYER_GUN );
	REGISTER_SPRITE( GLOW_DOT );
	REGISTER_SPRITE( BUTTON );
	REGISTER_SPRITE( BUTTON_PAUSE );
	REGISTER_SPRITE( TANK_BODY );
	REGISTER_SPRITE( TANK_WHEEL );
	REGISTER_SPRITE( TANK_TURRET );
	REGISTER_SPRITE( STATION_01 );
	REGISTER_SPRITE( STAR );
	REGISTER_SPRITE( GEAR );
	REGISTER_SPRITE( HOME );
	REGISTER_SPRITE( RETRY );
	REGISTER_SPRITE( NEXT );
	REGISTER_SPRITE( JOYSTICK );
	REGISTER_SPRITE( JOYSTICK_BACK );
	REGISTER_SPRITE( SPIKEBALL );
	REGISTER_SPRITE( SAW );
	REGISTER_SPRITE( COIN );
	REGISTER_SPRITE( TURRET );
}

void game_polyshapes(void)
{
	POLYSHAPE_RAMP = shape_read("ramp.shape");
	POLYSHAPE_TURRET = shape_read("turret.shape");
	POLYSHAPE_TANK = shape_read("tank.shape");
}

void game_particles(void)
{
#define PARTICLE_STRESS 0
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

void game_font(void)
{
   FONT_NORMAL = bmfont_read_font("Arial.fnt");
   FONT_BIG = bmfont_read_font("bigariel.fnt");
}

void game_audio(void)
{
	SND_LASER_1 = sound_load_chunk("laser_03.ogg");
	SND_LASER_2 = sound_load_chunk("laser_02.ogg");
	SND_LASER_MISS = sound_load_chunk("laser_melt.ogg");
	SND_HIT_1 = sound_load_chunk("hit_01.ogg");
	SND_HIT_2 = sound_load_chunk("hit_02.ogg");
	SND_TANK_EXPLODE = sound_load_chunk("hit_03.ogg");
	SND_FACTORY_EXPLODE = sound_load_chunk("factory_explode.ogg");
	SND_TURRET_EXPLODE = sound_load_chunk("turret_explode.ogg");
	SND_COIN = sound_load_chunk("coin_pickup.ogg");
	SND_EXPLOSION = sound_load_chunk("explosion.ogg");

	MUSIC_MENU = sound_load_music("Broken_Logic.ogg");
	MUSIC_LEVEL = sound_load_music("Blocked Bus.ogg");
	MUSIC_GAMEOVER = sound_load_music("Idling.ogg");
}


/* general button callbacks */
static void open_settings(void)
{
	statesystem_push_state(state_settings);
}

/* all global touchables goes in here. NB! Cannot contain direct reference to any state-id as they are uninitialized! */
void game_touchables(void)
{
	btn_settings = button_create(SPRITE_GEAR, 0, "", GAME_WIDTH/2 - 100, GAME_HEIGHT/2 - 100, 125, 125);
	button_set_callback(btn_settings, open_settings, 0);
	button_set_enlargement(btn_settings, 1.5);
	button_set_hotkeys(btn_settings, SDL_SCANCODE_F1, 0);
}

void game_init(void)
{
	//TODO generalize particles.c and sprites.c, (and level.c?)
	game_sprites();
	game_polyshapes();
	game_audio();
	game_particles();
	game_font();

	game_components();
	object_types_init();
	level_init();

    game_touchables();

	/* init all states (warning: make sure that no init method depends on uninitialized state_id!) */
    settings_init();
    stations_init();
    menu_init();
    space_init();
    pause_init();
    upgrades_init();
    gameover_init();
    levelscreen_init();
    leveldone_init();

#if ARCADE_MODE
    statesystem_set_state(state_menu);
#else
    statesystem_set_state(state_stations);
#endif

	sound_music(MUSIC_LEVEL);
}

/* Component functions */
minimap cmp_new_minimap(float size, Color c)
{
	minimap m = {size, c};
	return m;
}

void game_destroy(void)
{

	level_destroy();
}
