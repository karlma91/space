#include "game.h"
#include "../engine/engine.h"

#include "cJSON.h"

#include "states/space.h"
//#include "states/stations.h"
#include "states/leveldone.h"
#include "states/levelscreen.h"
#include "states/editor.h"
#include "obj/object_types.h"
//#include "states/pause.h"
//#include "states/upgrades.h"
//#include "states/leveldone.h"
//#include "states/settings.h"

extern void pause_init(void);
extern void msgbox_init(void);
extern void textinput_init(void);
extern void settings_init(void);
extern void upgrades_init(void);
extern void stations_init(void);
extern void log_init(void);


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
SPRITE_ID SPRITE_PLAYERBODY001;
SPRITE_ID SPRITE_PLAYERBODY002;
SPRITE_ID SPRITE_PLAYERGUN001;
SPRITE_ID SPRITE_PLAYERGUN002;
SPRITE_ID SPRITE_BTN_PUSH;
SPRITE_ID SPRITE_TANKBODY001;
SPRITE_ID SPRITE_TANKWHEEL001;
SPRITE_ID SPRITE_TANKGUN001;
SPRITE_ID SPRITE_BTN_PAUSE;
SPRITE_ID SPRITE_STATION001;
SPRITE_ID SPRITE_WRENCH;
SPRITE_ID SPRITE_STAR;
SPRITE_ID SPRITE_GEAR;
SPRITE_ID SPRITE_BTN_RETRY;
SPRITE_ID SPRITE_BTN_NEXT;
SPRITE_ID SPRITE_BTN_HOME;
SPRITE_ID SPRITE_JOYSTICK;
SPRITE_ID SPRITE_JOYSTICK_BACK;
SPRITE_ID SPRITE_SPIKEBALL;
SPRITE_ID SPRITE_SAW;
SPRITE_ID SPRITE_COIN;
SPRITE_ID SPRITE_METAL01;
SPRITE_ID SPRITE_TURRETBODY001;
SPRITE_ID SPRITE_TURRETGUN001;

SPRITE_ID SPRITE_BTN1;
SPRITE_ID SPRITE_BTN_EDIT;
SPRITE_ID SPRITE_DIALOG1;
SPRITE_ID SPRITE_DIALOG2;
/* end of global sprite definitions*/

/* polyshapes */
POLYSHAPE_ID POLYSHAPE_RAMP;
POLYSHAPE_ID POLYSHAPE_TURRET;
POLYSHAPE_ID POLYSHAPE_TANK;
POLYSHAPE_ID POLYSHAPE_POLY;

/* end */

/* global emitter */
EMITTER_ID EM_FLAME;
EMITTER_ID EM_ROCKETFLAME;
EMITTER_ID EM_EXPLOSION;
EMITTER_ID EM_EXPLOSIONBIG;
EMITTER_ID EM_SPARKS;
EMITTER_ID EM_SMOKE;
EMITTER_ID EM_SCORE;
EMITTER_ID EM_FRAGMENTS;
EMITTER_ID EM_COUNT;
/* extern particles end */

//extern fonts
bm_font * FONT_NORMAL;
bm_font * FONT_BIG;
bm_font * FONT_SANS;
bm_font * FONT_SANS_PLAIN;
bm_font * FONT_COURIER;

/* GLOBAL SOUND CHUNKS */
Mix_Chunk *SND_LASER_1;
Mix_Chunk *SND_LASER_2;
Mix_Chunk *SND_LASER_MISS;
Mix_Chunk *SND_HIT_1;
Mix_Chunk *SND_HIT_2;
Mix_Chunk *SND_UNIT_EXPLODE;
Mix_Chunk *SND_BUILDING_EXPLODE;
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

void game_spritepacks(void)
{
	sprite_packload("spacetex.pack");
}

void game_sprites(void)
{
	//TODO make sure engine doesn't depend on these directly
	SPRITE_ERROR = sprite_link("error");
	SPRITE_GLOWDOT = sprite_link("glowdot");
	SPRITE_DOT = sprite_link("dot");
	SPRITE_GLOW = sprite_link("glow");
	SPRITE_WHITE = sprite_link("pixel");
	SPRITE_BAR = sprite_link("bar");

	REGISTER_SPRITE( PLAYERBODY001 );
	REGISTER_SPRITE( PLAYERBODY002 );
	REGISTER_SPRITE( PLAYERGUN001 );
	REGISTER_SPRITE( PLAYERGUN002 );
	REGISTER_SPRITE( BTN_PUSH );
	REGISTER_SPRITE( TANKBODY001 );
	REGISTER_SPRITE( TANKWHEEL001 );
	REGISTER_SPRITE( TANKGUN001 );
	REGISTER_SPRITE( BTN_PAUSE );
	REGISTER_SPRITE( STATION001 );
	REGISTER_SPRITE( WRENCH );
	REGISTER_SPRITE( STAR );
	REGISTER_SPRITE( GEAR );
	REGISTER_SPRITE( BTN_RETRY );
	REGISTER_SPRITE( BTN_NEXT );
	REGISTER_SPRITE( BTN_HOME );
	REGISTER_SPRITE( JOYSTICK );
	REGISTER_SPRITE( JOYSTICK_BACK );
	REGISTER_SPRITE( SPIKEBALL );
	REGISTER_SPRITE( SAW );
	REGISTER_SPRITE( COIN );
	REGISTER_SPRITE( METAL01 );
	REGISTER_SPRITE( TURRETBODY001 );
	REGISTER_SPRITE( TURRETGUN001 );

	REGISTER_SPRITE( BTN1 );
	REGISTER_SPRITE( BTN_EDIT );
	REGISTER_SPRITE( DIALOG1 );
	REGISTER_SPRITE( DIALOG2 );
}

void game_polyshapes(void)
{
	POLYSHAPE_RAMP = shape_read("ramp.shape");
	POLYSHAPE_TURRET = shape_read("turret.shape");
	POLYSHAPE_TANK = shape_read("tank.shape");
	POLYSHAPE_POLY = shape_read("tank.shape");
}

void game_particles(void)
{
#define PARTICLE_STRESS 0
#if PARTICLE_STRESS
    EM_FLAME =         read_emitter_from_file("flame_stress.xml");
#else
    EM_FLAME =         read_emitter_from_file("flame_3.xml");
#endif
    EM_ROCKETFLAME =  read_emitter_from_file("rocket_flame.xml");
    EM_EXPLOSION =     read_emitter_from_file("explosion_ground.xml");
    EM_EXPLOSIONBIG = read_emitter_from_file("explosion_building.xml");
    EM_SPARKS =        read_emitter_from_file("sparks.xml");
    EM_SMOKE =         read_emitter_from_file("smoke.xml");
    EM_SCORE =         read_emitter_from_file("score.xml");
    EM_FRAGMENTS =     read_emitter_from_file("fragments.xml");
}

void game_font(void)
{
   FONT_NORMAL = bmfont_read_font("Arial.fnt");
   FONT_BIG = bmfont_read_font("bigariel.fnt");
   FONT_SANS = bmfont_read_font("sans.fnt");
   FONT_SANS_PLAIN = bmfont_read_font("sans_plain.fnt");
   FONT_COURIER = bmfont_read_font("courier.fnt");
}

void game_audio(void)
{
	SND_LASER_1 = sound_loadchunk("laser_03.ogg");
	SND_LASER_2 = sound_loadchunk("laser_02.ogg");
	SND_LASER_MISS = sound_loadchunk("laser_melt.ogg");
	SND_HIT_1 = sound_loadchunk("hit_01.ogg");
	SND_HIT_2 = sound_loadchunk("hit_02.ogg");
	SND_UNIT_EXPLODE = sound_loadchunk("hit_03.ogg");
	SND_BUILDING_EXPLODE = sound_loadchunk("factory_explode.ogg");
	SND_TURRET_EXPLODE = sound_loadchunk("turret_explode.ogg");
	SND_COIN = sound_loadchunk("coin_pickup.ogg");
	SND_EXPLOSION = sound_loadchunk("explosion.ogg");

	MUSIC_MENU = sound_loadmusic("Broken_Logic.ogg");
	MUSIC_LEVEL = sound_loadmusic("Blocked Bus.ogg");
	MUSIC_GAMEOVER = sound_loadmusic("Idling.ogg");
}


/* general button callbacks */
static void open_settings(void *unused)
{
	statesystem_push_state(state_settings);
}

/* all global touchables goes in here. NB! Cannot contain direct reference to any state-id as they are uninitialized! */
void game_touchables(void)
{
	btn_settings = button_create(SPRITE_GEAR, 0, "", GAME_WIDTH/2 - 130, GAME_HEIGHT/2 - 130, 150, 150);
	button_set_click_callback(btn_settings, open_settings, 0);
	button_set_enlargement(btn_settings, 1.5);
	button_set_hotkeys(btn_settings, SDL_SCANCODE_F1, 0);
}

#ifdef GOT_SDL_NET
static IPaddress serverIP;
static TCPsocket tcpsock = NULL;

int db_connected = 0;
#define SERVER_HOST "78.46.42.4"
#define SERVER_PORT 3306

void connect_thread(void *unused)
{
	while (1) {
		SDL_Delay(5000);
		if (!db_connected) {
			db_connected = 0;
			SDLNet_ResolveHost(&serverIP, SERVER_HOST, SERVER_PORT);
			if (serverIP.host == INADDR_NONE) {
				fprintf(stderr, "SERVER: Could not resolve host: %s\n", SDLNet_GetError());
			} else {
				tcpsock = SDLNet_TCP_Open(&serverIP);
				if (tcpsock == NULL) {
					fprintf(stderr,"SERVER: Connection failed: %s\n", SDLNet_GetError());
				} else {
					db_connected = 1;
					fprintf(stderr,"SERVER: Connected!\n");
				}
			}
		}
	}
}

void connect_init(void)
{
	SDL_CreateThread(connect_thread, "network", NULL);
}
#else
void connect_init(void){}
#endif

void game_init(void)
{
	//TODO generalize particles.c and sprites.c, (and level.c?)
	game_spritepacks();
	game_sprites();
	game_polyshapes();
	game_audio();
	game_particles();
	game_font();

	game_components();
	object_types_init();
	tween_init();

    game_touchables();

	/* init all states (warning: make sure that no init method depends on uninitialized state_id!) */
    msgbox_init();
    textinput_init();
    log_init();
    settings_init();
    stations_init();
    menu_init();
    space_init();
    pause_init();
    upgrades_init();
    gameover_init();
    editor_init();
    levelscreen_init();
    leveldone_init();
#if ARCADE_MODE
    statesystem_set_state(state_menu);
#else
    statesystem_set_state(state_stations);
#endif

	sound_music(MUSIC_LEVEL);
	connect_init();
}

/* Component functions */
minimap cmp_new_minimap(float size, Color c)
{
	minimap m = {size, c};
	return m;
}

void game_destroy(void)
{
}
