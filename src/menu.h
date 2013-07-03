#ifndef MENU_H_
#define MENU_H_

#include "main.h"
#include "statesystem.h"

enum MINU_INNER_STATES {
	MENU_MAIN,
	MENU_INGAME
};

void menu_init();
void menu_change_current_menu(int);

#endif /* MENU_H_ */
