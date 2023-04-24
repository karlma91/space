
#include "stdio.h"
#include "xmlh.h"

#include "waffle_utils.h"

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
int parse_float(mxml_node_t *node, char *name, float *v)
{
	char *val = mxmlElementGetAttr(node, name);
	if (val != NULL) {
		*v = strtod(val,NULL);
		return 0;
	}else{
		SDL_Log("Error parsing atribute: %s in node %s \n", name, mxmlGetElement(node));
		return -1;
	}
}

/**
 * parse a int from a mxml_node atribute
 */
int parse_int(mxml_node_t *node, char *name, int *v)
{
	char *val = mxmlElementGetAttr(node, name);
	if (val != NULL) {
		*v =(int)strtol(val,(char **)NULL,10);
		return 0;
	} else {
		SDL_Log("Error parsing atribute: %s in node %s \n", name, mxmlGetElement(node));
		return -1;
	}
}

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
int parse_bool(mxml_node_t *node, char *name, int *v)
{
	char *val = mxmlElementGetAttr(node, name);
	if (val != NULL) {
		if(strcmp(val, "true")){
			*v = 0;
		}else{
			*v = 1;
		}
    	return 0;
    }else{
    	SDL_Log("Error parsing boolean in node %s \n", mxmlGetElement(node));
    	return -1;
    }
}

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
int parse_string(mxml_node_t *node, char *name, char **c)
{
    char *val = mxmlElementGetAttr(node, name);
	if (val != NULL) {
		*c = (val);
    }
    return 0;
}

