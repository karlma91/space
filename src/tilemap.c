/*
 * tilemap.c
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 */

#include "stdio.h"
#include "tilemap.h"
#include "texture.h"
#include "xmlh.h"
#include "constants.h"
#include "draw.h"
#include "space.h"


#include "waffle_utils.h"

static int parse_data(tilemap *map, char *data);
static void draw_subimage(GLfloat x, GLfloat y, GLfloat tx, GLfloat ty, GLfloat w, GLfloat h, GLfloat tile_width, GLfloat tile_height);

void tilemap_render(tilemap *map)
{
	//TODO move all gl dependent code out of this file!
#if GLES2


#else
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glColor3f(1,1,1);
    texture_bind(map->texture_id);

    GLfloat w = ((GLfloat)map->tile_width)/map->image_width;
    GLfloat h = ((GLfloat)map->tile_height)/map->image_height;

    int i,j;
    int x,y;

    //TODO: check if cam_left and cam_right is correct
    int j_start, j_end;
    j_start = (cam_left - map->tile_width) / map->tile_width + map->width / 2; //OLD: 0
    j_end = (cam_right + map->tile_width) / map->tile_width + map->width / 2; //OLD: map->width

    //TODO: fix special case, when supposed to render two sides, add another loop
    //if (j_start < 0) j_start = 0; //special case
    //if (j_end > map->width) j_end = map->width; //special case

    /** draws from top and down **/
    for(i = 0; i < map->height; i++){
	int lvl_y = i*map->tile_height;
	lvl_y = map->tile_height * (map->height-1) - lvl_y;
	for(j = j_start; j < j_end; j++){

	    /** Circular indexing **/
	    int k = 0;
	    if(j < 0){
		/* + because (j % map->width) is negative */
		k = map->width + (j % map->width);
	    }else if(j >= map->width){
		k = j % map->width;
	    }else{
		k = j;
	    }

	    x = map->data[k + i * map->width] % (map->image_width / map->tile_width) - 1;
	    y = map->data[k + i * map->width] / (map->image_height / map->tile_height);

	    if(map->data[k + i*map->width] > 0){
		int lvl_x = j*map->tile_width - (map->width*map->tile_width)/2;
		draw_subimage(lvl_x, lvl_y, (x*w), (y*h), w, h, map->tile_width, map->tile_height);
	    }
	}
    }
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
#endif
}

static void draw_subimage(GLfloat x, GLfloat y, GLfloat tx, GLfloat ty, GLfloat w, GLfloat h, GLfloat tile_width, GLfloat tile_height)
{
#if GLES2


#else
	glPushMatrix();
	glTranslatef(x,y,0);
	glBegin(GL_QUADS);
	glTexCoord2d(tx, ty+h); glVertex2d(0, 0);
	glTexCoord2d(tx, (ty)); glVertex2d(0, tile_height);
	glTexCoord2d(tx+w, ty); glVertex2d(tile_width, tile_height);
	glTexCoord2d(tx+w, (ty+h)); glVertex2d(tile_width, 0);
	glEnd();
	glPopMatrix();
#endif
}



/**
 * parsing
 */
int tilemap_create (tilemap *map, char *filename)
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
		fclose(fp);
		return 1;
	}
	//fprintf(stderr,"tilemap.c parsing %s \n",filename);

	for (node = mxmlFindElement(tree, tree,NULL,NULL, NULL,MXML_DESCEND);
			node != NULL;
			node=mxmlWalkNext (node, NULL, MXML_DESCEND)
	){
		if (node->type  == MXML_ELEMENT) {
			if(TESTNAME("map")){
				parse_int(node,"width",&(map->width));
				parse_int(node,"height",&(map->height));
				parse_int(node,"tilewidth",&(map->tile_width));
				parse_int(node,"tileheight",&(map->tile_height));
				map->data = (int*) calloc(sizeof(int)*(map->width)*(map->height),sizeof(int));
			}else if(TESTNAME("image")){
				char *(temp[1]);
				char name[40];
				parse_string(node,"source",temp);
				sscanf(*temp,"../textures/%s", name);
				map->texture_id = texture_load(name);

				parse_int(node,"width",&(map->image_width));
				parse_int(node,"height",&(map->image_height));

			}else if(TESTNAME("data")){
				parse_data(map, node->child->value.opaque);
			}else{

			}
		}
	}
	mxmlDelete(tree);
	fclose(fp);
	return 0;
}

void tilemap_destroy(tilemap *map)
{
	free(map->data);
}

static int parse_data(tilemap *map, char *data)
{
	int x,y;
	for(x = 0; x < map->height; x++){
		for(y=0;y < map->width; y++){
			map->data[y+x*map->width] = (int) strtol(data, &(data), 10);
			data++;
		}
	}
	//fprintf(stderr,"HELLO %d TEXTUREMAP.c \n",__LINE__);
	return 0;
}
