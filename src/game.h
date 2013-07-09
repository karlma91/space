/*
 * game.h
 *
 *  Created on: 29. juni 2013
 *      Author: Mathias
 */

#ifndef GAME_H_
#define GAME_H_

#include "statesystem.h"
#include "sprite.h"
#include "particles.h"

extern STATE_ID state_mainmenu;
extern STATE_ID state_menu;
extern STATE_ID state_space;
extern STATE_ID state_gameover;
extern STATE_ID state_levelselect;

extern SPRITE_ID SPRITE_PLAYER;
extern SPRITE_ID SPRITE_PLAYER_GUN;
extern SPRITE_ID SPRITE_GLOW_DOT;
extern SPRITE_ID SPRITE_BUTTON;
extern SPRITE_ID SPRITE_BUTTON_PAUSE;
extern SPRITE_ID SPRITE_TANK_BODY;
extern SPRITE_ID SPRITE_TANK_WHEEL;
extern SPRITE_ID SPRITE_TANK_TURRET;

extern int EMITTER_FLAME;
extern int EMITTER_ROCKET_FLAME;
extern int EMITTER_EXPLOSION;
extern int EMITTER_SPARKS;
extern int EMITTER_SMOKE;
extern int EMITTER_SCORE;
extern int EMITTER_FRAGMENTS;
extern int EMITTER_COUNT;


#ifndef ARCADE_MODE // definer evt. ARCADE_MODE under symbols i project settings for � overstyre
#define ARCADE_MODE 1
#endif

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

void game_config();
void game_init();
void game_destroy();


#endif /* GAME_H_ */
