/*
 * monitor.h
 *
 *  Created on: Feb 14, 2014
 *      Author: karl
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#include "chipmunk.h"
#include "../graphics/camera.h"

void monitor_enable(int e);
void monitor_float(char *name, float  var);
void monitor_int(char *name, int  var);
void monitor_string(char *name, char * var);
void monitor_cpvect(char *name, cpVect var);
void monitor_render(view *cam);


#endif /* MONITOR_H_ */
