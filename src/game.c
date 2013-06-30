
#include "menu.h"
#include "space.h"
#include "gameover.h"
#include "levelselect.h"

//TODO move game specific code from main.c into this file

void game_init()
{
    menu_init();
    space_init();
    gameover_init();
    levelselect_init();
}
