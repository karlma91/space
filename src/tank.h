#ifndef TANK_H_
#define TANK_H_

#include "objects.h"
#include "tankfactory.h"

object *tank_init( struct tank_factory *fac, float max_hp);

extern struct obj_type type_tank;


#endif /* TANK_H_ */
