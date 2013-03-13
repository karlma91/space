#ifndef GAMEOVER_H_
#define GAMEOVER_H_

#include "main.h"

//TODO: change to more specific names refering to gameover
enum gameover_state {
	GAMEOVER_WIN,
	enter_name,
	confirm_name,
	show_highscore
};

extern state state_gameover;
extern int gameover_setstate(enum gameover_state state);

#endif /* GAMEOVER_H_ */
