#ifndef GAMEOVER_H_
#define GAMEOVER_H_

//TODO: change to more specific names referring to gameover
enum gameover_state {
	GAMEOVER_WIN,
	enter_name,
	confirm_name,
	show_highscore
};

extern int gameover_setstate(enum gameover_state state);
void gameover_init(void);
void gameover_showhighscores(void);

#endif /* GAMEOVER_H_ */
