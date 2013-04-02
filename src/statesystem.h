/*
 * statesystem.h
 *
 *  Created on: Mar 22, 2013
 *      Author: karlmka
 */

#ifndef STATESYSTEM_H_
#define STATESYSTEM_H_

enum STATESYSTEM_STATES {
    STATESYSTEM_MENU,
    STATESYSTEM_LEVELSELECT,
    STATESYSTEM_SPACE,
    STATESYSTEM_INGAME_MENU,
    STATESYSTEM_GAMEOVER,
    STATESYSTEM_COUNT
};

/**
 * standar functions
 */
void statesystem_init();
void statesystem_update();
void statesystem_draw();
void statesystem_destroy();

/**
 * statesystem functions
 */

void statesystem_init_state(int state, int inner_states,
        void (*on_enter)(),
        void (*pre_update)(),
        void (*post_update)(),
        void (*draw)(),
        void (*on_leave)(),
        void (*destroy)());

void statesystem_add_inner_state(int state, int inner_state, void (*update)(), void (*draw)());

void statesystem_set_inner_state(int state, int inner_state);

void statesystem_push_state(int state);
void statesystem_pop_state(int state);
void statesystem_set_state(int state);


#endif /* STATESYSTEM_H_ */
