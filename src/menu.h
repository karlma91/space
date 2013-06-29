#ifndef MAINMENU_H_
#define MAINMENU_H_

#include "main.h"
#include "statesystem.h"

enum MINU_INNER_STATES {
	MENU_MAIN,
	MENU_INGAME
};

extern STATE_ID STATE_MENU;

void menu_init();
void menu_change_current_menu(int);

#endif /* MAINMENU_H_ */
