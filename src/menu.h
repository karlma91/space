#ifndef MAINMENU_H_
#define MAINMENU_H_

#include "main.h"

extern state state_menu;
void change_current_menu(int menu);

enum MENU_ID{
	MAIN_MENU_ID,
	INGAME_MENU_ID
};

#endif /* MAINMENU_H_ */
