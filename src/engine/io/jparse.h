/*
 * jparse.h
 *
 *  Created on: 13. feb. 2014
 *      Author: mathiahw
 */

#ifndef JPARSE_H_
#define JPARSE_H_
#include "we_utils.h"
#include "we_io.h"

typedef we_bool (*parse_function)(cJSON *, void *, void *);

void jparse_init(void);
/* all types in Waffle Engine default */
void jparse_register(char *type, parse_function pf);
we_bool jparse_parse(cJSON *param, char *field_name, char *type, void *field, void *def);

void jparse_close(cJSON* j);
cJSON* jparse_open(int dir_type, char *filepath);
cJSON* jparse_new();
void jparse_save(cJSON *c, int dir_type, char *filepath);

#endif /* JPARSE_H_ */
