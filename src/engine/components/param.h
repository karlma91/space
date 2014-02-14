/*
 * param.h
 *
 *  Created on: Feb 13, 2014
 *      Author: karlmka
 */

#ifndef PARAM_H_
#define PARAM_H_

#include "../data/hashmap.h"

typedef struct param_defs {
	hashmap * hm_param;
} param_list;

void param_init(void);
void param_load(int dir_type, char *file);
void param_list_destroy(void);
void *param_get(const char *type, const char * name);

#endif /* PARAM_H_ */
