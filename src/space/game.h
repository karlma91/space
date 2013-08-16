/*
 * game.h
 *
 *  Created on: 29. juni 2013
 *      Author: Mathias
 */

#ifndef GAME_H_
#define GAME_H_

#include "../engine/state/statesystem.h"
#include "../engine/audio/sound.h"

#include "../engine/graphics/particles.h"
#include "../engine/graphics/customparticles.h"

#include "../engine/input/button.h"

#include "obj/object_types.h"

#define SPACE_BODIES_MAX 5

/* COMPONENTS */
extern int CMP_HPBAR;
extern int CMP_SCORE;
extern int CMP_DAMAGE;
extern int CMP_BODIES;

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

/* SPRITES */
extern SPRITE_ID SPRITE_PLAYER;
extern SPRITE_ID SPRITE_PLAYER_GUN;
extern SPRITE_ID SPRITE_GLOW_DOT;
extern SPRITE_ID SPRITE_BUTTON;
extern SPRITE_ID SPRITE_BUTTON_PAUSE;
extern SPRITE_ID SPRITE_TANK_BODY;
extern SPRITE_ID SPRITE_TANK_WHEEL;
extern SPRITE_ID SPRITE_TANK_TURRET;
extern SPRITE_ID SPRITE_STATION_01;
extern SPRITE_ID SPRITE_STAR;
extern SPRITE_ID SPRITE_GEAR;
extern SPRITE_ID SPRITE_RETRY;
extern SPRITE_ID SPRITE_NEXT;
extern SPRITE_ID SPRITE_HOME;
extern SPRITE_ID SPRITE_JOYSTICK;
extern SPRITE_ID SPRITE_JOYSTICK_BACK;
extern SPRITE_ID SPRITE_SPIKEBALL;
extern SPRITE_ID SPRITE_SAW;

/* PARTICLES */
extern int EMITTER_FLAME;
extern int EMITTER_ROCKET_FLAME;
extern int EMITTER_EXPLOSION;
extern int EMITTER_SPARKS;
extern int EMITTER_SMOKE;
extern int EMITTER_SCORE;
extern int EMITTER_FRAGMENTS;
extern int EMITTER_COUNT;

extern bm_font * FONT_NORMAL;
extern bm_font * FONT_BIG;

/* GLOBAL SOUND CHUNKS */
extern Mix_Chunk *SND_LASER_1;
extern Mix_Chunk *SND_LASER_2;
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

void game_config();
void game_init();
void game_destroy();

extern float upg_total_mass;

#define ID_GROUND 0xABA198 //TMP define with random number

#endif /* GAME_H_ */
