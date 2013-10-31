#ifndef MENU_H_
#define MENU_H_

#include "we_graphics.h"

sprite spr_startbtn;

enum MINU_INNER_STATES {
	MENU_MAIN,
	MENU_INGAME
};

void menu_init(void);
void menu_change_current_menu(int);

#endif /* MENU_H_ */
