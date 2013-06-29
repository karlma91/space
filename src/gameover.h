#ifndef GAMEOVER_H_
#define GAMEOVER_H_

#include "main.h"
#include "statesystem.h"

extern STATE_ID STATE_GAMEOVER;

//TODO: change to more specific names referring to gameover
enum gameover_state {
	GAMEOVER_WIN,
	enter_name,
	confirm_name,
	show_highscore
};

extern int gameover_setstate(enum gameover_state state);
void gameover_init();

#endif /* GAMEOVER_H_ */
