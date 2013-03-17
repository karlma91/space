/*
 * tilemap.c
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 */

#include "stdio.h"
#include "tilemap.h"
#include "mxml.h"


#define TESTNAME(s) strcmp(node->value.element.name, s) == 0

int create_tilemap (tilemap *map, char *filename)
    {

    FILE *fp  = NULL;

    mxml_node_t * tree = NULL;
    mxml_node_t * node  = NULL;

    fp = fopen (filename, "r");
    if (fp ){
	tree = mxmlLoadFile (NULL , fp , MXML_OPAQUE_CALLBACK);
    }else {
	fprintf(stderr,"Could Not Open the File Provided");
	return 1;
    }
    if(tree == NULL){
	fprintf(stderr,"tilemap.c file: %s is empty \n",filename);
	return 1;
    }
    fprintf(stderr,"tilemap.c parsing %s \n",filename);

    for (node = mxmlFindElement(tree, tree,NULL,NULL, NULL,MXML_DESCEND);
	    node != NULL;
	    node=mxmlWalkNext (node, NULL, MXML_DESCEND)
    ){
	if (node->type  == MXML_ELEMENT) {
	    if(TESTNAME("map")){

	    }
	    }else if(TESTNAME("image")){

	    }else if(TESTNAME("data")){

	    }else{

	    }
	}
    }
