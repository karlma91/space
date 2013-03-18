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


static int parse_data(tilemap *map, char *data);

static void draw_subimage(GLfloat x, GLfloat y, GLfloat tx, GLfloat ty, GLfloat w, GLfloat h);



void tilemap_render(tilemap *map)
{
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
	for(i = 0; i < map->height; i++){
		for(j=0; j < map->width; j++){
			x = map->data[j + (map->height - i - 1)*map->width]%(map->image_width/map->tile_width) - 1;
			y = map->data[j + (map->height - i - 1)*map->width]/(map->image_height/map->tile_width);
			//fprintf(stderr,"DRAW: %d %d \n", x, y);
			if(map->data[j + i*map->width]>0){
				draw_subimage(j*map->tile_width - (map->width*map->tile_width)/2,i*map->tile_height, (x*w), (y*h), w, h);
			}
		}
	}
	//fprintf(stderr,"END");
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

}
static void draw_subimage(GLfloat x, GLfloat y, GLfloat tx, GLfloat ty, GLfloat w, GLfloat h)
{
	//fprintf(stderr,"DRAW: %.2f %.2f %.2f %.2f %.2f %.2f \n",x,y,tx,ty,w,h);
	glPushMatrix();
	glTranslatef(x,y,0);
	glBegin(GL_QUADS);
	glTexCoord2d(tx, ty+h); glVertex2d(0, 0);
	glTexCoord2d(tx, (ty)); glVertex2d(0, 128);
	glTexCoord2d(tx+w, ty); glVertex2d(128, 128);
	glTexCoord2d(tx+w, (ty+h)); glVertex2d(128, 0);
	glEnd();
	glPopMatrix();
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
	fprintf(stderr,"tilemap.c parsing %s \n",filename);

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
				map->data = (int*) malloc(sizeof(int)*(map->width)*(map->height));
			}else if(TESTNAME("image")){
				char *(temp[1]);
				char name[40];
				parse_string(node,"source",temp);
				//sprintf(*temp,"../textures/%s", name);
				map->texture_id = texture_load(*temp);

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
	fprintf(stderr,"HELLO %d TEXTUREMAP.c \n",__LINE__);
	return 0;
}

static int parse_data(tilemap *map, char *string)
{
	char temp[5000];
	strcpy(temp,string);
	char *data = temp;
//	fprintf(stderr,"data: %s\n", data);
	int x,y;
	for(x = 0; x < map->height; x++){
		for(y=0;y < map->width; y++){
			//fprintf(stderr,"char: A %c A\n",*data);
			map->data[y+x*map->width] = (int) strtol(data, &(data), 10);
			data++;
			//fprintf(stderr,"char: %c == %d E\n",*data,map->data[x+y*map->width]);
		}
	}
	fprintf(stderr,"HELLO %d TEXTUREMAP.c \n",__LINE__);
	return 0;
}




