/*
 * paramloader.h
 *
 *  Created on: Jan 14, 2014
 *      Author: karlmka
 */

#ifndef PARAMLOADER_H_
#define PARAMLOADER_H_
#include "we_utils.h"
#include "object.h"
#include "shape.h"
#include "we_graphics.h"
#include "../audio/sound.h"
#include "cJSON.h"

typedef we_bool (*parse_function)(cJSON *, void *, void *);

void pl_init(void);
/* all types in Waffle Engine default */
void pl_register_type(char *type, parse_function pf);
we_bool pl_parse(cJSON *param, char *field_name, char *type, void *field, void *def);

#endif /* PARAMLOADER_H_ */
