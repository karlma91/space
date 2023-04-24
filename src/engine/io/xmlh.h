/*
 * xmlh.h
 *
 *  Created on: 18. mars 2013
 *      Author: Karl
 */

#ifndef XMLH_H_
#define XMLH_H_

#include "mxml.h"

/** used to test a string */
#define TESTNAME(s) mxmlGetElement(node) == 0 ? 0 : strcmp(mxmlGetElement(node), s) == 0

int parse_float(mxml_node_t *node,char *name, float *v);
int parse_int(mxml_node_t *node, char *name, int *v);
int parse_bool(mxml_node_t *node, char *name, int *v);
int parse_string(mxml_node_t *node, char *name, char **c);


#endif /* XMLH_H_ */
