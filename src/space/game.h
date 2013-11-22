/*
 * game.h
 *
 *  Created on: 29. juni 2013
 *      Author: Mathias
 */

#ifndef GAME_H_
#define GAME_H_

#include "we.h"

#define SPACE_BODIES_MAX 5

// RLAYS
#define RLAY_GUI_FRONT 0
#define RLAY_GUI_MID 1
#define RLAY_GUI_BACK 2
#define RLAY_GAME_FRONT 4
#define RLAY_GAME_MID 5
#define RLAY_GAME_BACK 6
#define RLAY_BACK_FRONT 8
#define RLAY_BACK_MID 9
#define RLAY_BACK_BACK 10

/* COMPONENTS */
extern int CMP_HPBAR;
extern int CMP_COINS;
//extern int CMP_SCORE;
extern int CMP_DAMAGE;
extern int CMP_BODIES;
extern int CMP_MINIMAP;
extern int CMP_CREATOR;

/* component structs? */
typedef struct {
	float size;
	Color c;
} minimap;
minimap cmp_new_minimap(float size, Color c);

#include "obj/object_types.h"
#include "autogen/paramsgen.h"


/* STATES */
extern STATE_ID state_menu; /* deprecated state! */
extern STATE_ID state_gameover; /* deprecated state! */
extern STATE_ID state_settings;
extern STATE_ID state_space;
extern STATE_ID state_pause;
extern STATE_ID state_store;
extern STATE_ID state_leveldone;
extern STATE_ID state_stations;
extern STATE_ID state_levelscreen;
extern STATE_ID state_editor;

/* SPRITES */
extern SPRITE_ID SPRITE_PLAYERBODY001;
extern SPRITE_ID SPRITE_PLAYERGUN001;
extern SPRITE_ID SPRITE_BTN_PUSH;
extern SPRITE_ID SPRITE_BTN_PAUSE;
extern SPRITE_ID SPRITE_TANKBODY001;
extern SPRITE_ID SPRITE_TANKWHEEL001;
extern SPRITE_ID SPRITE_TANKGUN001;
extern SPRITE_ID SPRITE_STATION001;
extern SPRITE_ID SPRITE_WRENCH;
extern SPRITE_ID SPRITE_STAR;
extern SPRITE_ID SPRITE_GEAR;
extern SPRITE_ID SPRITE_BTN_RETRY;
extern SPRITE_ID SPRITE_BTN_NEXT;
extern SPRITE_ID SPRITE_BTN_HOME;
extern SPRITE_ID SPRITE_JOYSTICK;
extern SPRITE_ID SPRITE_JOYSTICK_BACK;
extern SPRITE_ID SPRITE_SPIKEBALL;
extern SPRITE_ID SPRITE_SAW;
extern SPRITE_ID SPRITE_COIN;
extern SPRITE_ID SPRITE_METAL01;
extern SPRITE_ID SPRITE_TURRETBODY001;
extern SPRITE_ID SPRITE_TURRETGUN001;

/* POLYSHAPES */
extern POLYSHAPE_ID POLYSHAPE_RAMP;
extern POLYSHAPE_ID POLYSHAPE_TURRET;
extern POLYSHAPE_ID POLYSHAPE_TANK;
extern POLYSHAPE_ID POLYSHAPE_POLY;


/* PARTICLES */
extern int EMITTER_FLAME;
extern int EMITTER_ROCKET_FLAME;
extern int EMITTER_EXPLOSION;
extern int EMITTER_EXPLOSION_BIG;
extern int EMITTER_SPARKS;
extern int EMITTER_SMOKE;
extern int EMITTER_SCORE;
extern int EMITTER_FRAGMENTS;
extern int EMITTER_COUNT;

extern bm_font * FONT_NORMAL;
extern bm_font * FONT_BIG;
extern bm_font * FONT_SANS;

/* GLOBAL SOUND CHUNKS */
extern Mix_Chunk *SND_LASER_1;
extern Mix_Chunk *SND_LASER_2;
extern Mix_Chunk *SND_LASER_MISS;
extern Mix_Chunk *SND_HIT_1;
extern Mix_Chunk *SND_HIT_2;
extern Mix_Chunk *SND_TANK_EXPLODE;
extern Mix_Chunk *SND_FACTORY_EXPLODE;
extern Mix_Chunk *SND_TURRET_EXPLODE;
extern Mix_Chunk *SND_COIN;
extern Mix_Chunk *SND_EXPLOSION;

/* GLOBAL MUSIC TRACKS */
extern Mix_Music *MUSIC_MENU;
extern Mix_Music *MUSIC_LEVEL;
extern Mix_Music *MUSIC_GAMEOVER;

/* GLOBAL TOUCHABLES */
extern button btn_settings;

#ifndef ARCADE_MODE // definer evt. ARCADE_MODE under symbols i project settings for � overstyre
#define ARCADE_MODE 0
#endif

/* global settings */
extern int player_assisted_steering;
extern int player_cheat_invulnerable;


extern unsigned int KEY_UP_1;
extern unsigned int KEY_UP_2;
extern unsigned int KEY_LEFT_1;
extern unsigned int KEY_LEFT_2;
extern unsigned int KEY_RIGHT_1;
extern unsigned int KEY_RIGHT_2;
extern unsigned int KEY_DOWN_1;
extern unsigned int KEY_DOWN_2;

extern unsigned int KEY_RETURN_1;
extern unsigned int KEY_RETURN_2;
extern unsigned int KEY_ESCAPE;

extern int weapon_index;
extern int armor_index;
extern int engine_index;

void game_config(void);
void game_init(void);
void game_destroy(void);

extern float upg_total_mass;

#define ID_GROUND (void *)0x1017 //TMP define with random number

/* digits hotkeys */
extern SDL_Scancode digit2scancode[10];

#endif /* GAME_H_ */
