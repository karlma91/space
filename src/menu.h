#ifndef MAINMENU_H_
#define MAINMENU_H_

#include "main.h"

enum MINU_INNER_STATES {
	MENU_MAIN,
	MENU_INGAME
};

void menu_init();
void menu_change_current_menu(int);

#endif /* MAINMENU_H_ */
