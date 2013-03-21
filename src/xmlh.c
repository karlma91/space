
#include "stdio.h"
#include "xmlh.h"

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
int parse_float(mxml_node_t *node, char *name, float *v)
{
	int ok = 0;
	int k;
	for (k = 0; k < node->value.element.num_attrs; k++){
		if(strcmp(node->value.element.attrs[k].name, name) == 0){
			*v = strtod(node->value.element.attrs[k].value,NULL);
			ok++;
		}
	}
	if(ok == 1){
		return 0;
	}else{
		fprintf(stderr,"Error parsing atribute: %s in node %s \n", name, node->value.element.name);
		return -1;
	}
}

/**
 * parse a int from a mxml_node atribute
 */
int parse_int(mxml_node_t *node, char *name, int *v)
{
	int ok = 0;
	int k;
	for (k = 0; k < node->value.element.num_attrs; k++){
		if(strcmp(node->value.element.attrs[k].name, name) == 0){
			*v =(int)strtol(node->value.element.attrs[k].value,(char **)NULL,10);
			ok++;
		}
	}
	if(ok == 1){
		return 0;
	}else{
		fprintf(stderr,"Error parsing atribute: %s in node %s \n", name, node->value.element.name);
		return -1;
	}
}

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
int parse_bool(mxml_node_t *node, char *name, int *v)
{
    int ok = 0;
    int k;
    for (k = 0; k < node->value.element.num_attrs; k++){
    	if(strcmp(node->value.element.attrs[k].name, name) == 0){
    		if(strcmp(node->value.element.attrs[k].value,"true")){
    			*v = 0;
    		}else{
    			*v = 1;
    		}
    		ok++;
    	}
    }
    if(ok == 1){
    	return 0;
    }else{
    	fprintf(stderr,"Error parsing boolean in node %s \n", node->value.element.name);
    	return -1;
    }
}

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
int parse_string(mxml_node_t *node, char *name, char **c)
{
    int k;
    for (k = 0; k < node->value.element.num_attrs; k++){
    	if(strcmp(node->value.element.attrs[k].name, name) == 0){
    		*c = (node->value.element.attrs[k].value);
    	}
    }
    return 0;
}

