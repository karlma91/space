/*
 * statesystem.h
 *
 *  Created on: Mar 22, 2013
 *      Author: karlmka
 */

#ifndef STATESYSTEM_H_
#define STATESYSTEM_H_

enum STATESYSTEM_STATES {
		STATESYSTEM_MAIN,
		STATESYSTEM_OPTION,
		STATESYSTEM_SPACE,
		STATESYSTEM_INGAME_MENU,
		STATESYSTEM_GAMEOVER
};

/**
 * standar functions
 */
void statesyste_init();
void statesyste_update();
void statesyste_draw();
void statesyste_destroy();

/**
 * statesystem functions
 */

int statesystem_init_state(int state, int inner_states, void (*on_enter)(), void (*update)(), void (*draw)(), void (*on_leave)());

int statesystem_add_inner_state(int state, int inner_state, void (*update)(), void (*draw)());

int statesystem_set_inner_state(int state, int inner_state);

int statesystem_push_state(int state);
int statesystem_pop_state(int state);
int statesystem_set_state(int state);


#endif /* STATESYSTEM_H_ */
