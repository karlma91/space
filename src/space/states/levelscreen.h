/*
 * levelscreen.h
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#ifndef LEVELSCREEN_H_
#define LEVELSCREEN_H_
#include "../solarsystem.h"
#include "../level/spacelvl.h"

void levelscreen_init(void);
void levelscreen_change_to(station * ship);
spacelvl *get_current_lvl_template(void);

#endif /* LEVELSCREEN_H_ */
