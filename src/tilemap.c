/*
 * tilemap.c
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 *      Author: Mathias
 */

#include "stdio.h"
#include "tilemap.h"
#include "texture.h"
#include "xmlh.h"
#include "constants.h"
#include "draw.h"
#include "space.h"
#include "main.h"
#include "spaceengine.h"

#include "waffle_utils.h"

#define TILEMAP_READ_BUFFER_SIZE 128000


static int parse_data(tilemap *map, char *data);

void tilemap_render(tilemap *map)
{
    glEnable(GL_TEXTURE_2D);
    draw_color4f(1,1,1,1);
    texture_bind(map->texture_id);

    int i,j;
    int x,y;

    //TODO: check if cam_left and cam_right is correct
    int j_start, j_end;
    j_start = (cam_left - map->tile_width) / map->tile_width + map->width / 2 - 2; //OLD: 0
    j_end = (cam_right + map->tile_width) / map->tile_width + map->width / 2 + 2; //OLD: map->width

    //DEBUG
    static int DEBUG_TILEMAP = 0;
    if (keys[SDL_SCANCODE_Y]) {
    	keys[SDL_SCANCODE_Y] = 0;
    	DEBUG_TILEMAP ^=1;
    }
    if (DEBUG_TILEMAP) {
    	j_start = 0;
    	j_end = map->width;
    }

    //TODO: fix special case, when supposed to render two sides, add another loop
    //if (j_start < 0) j_start = 0; //special case
    //if (j_end > map->width) j_end = map->width; //special case

    int map_height = map->height;
#if EXPERIMENTAL_GRAPHICS
    //map_height +=4;
#endif

    /** draws from top and down **/
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


        int lvl_x = j*map->tile_width - (map->width*map->tile_width)/2;

        float data1[3];
        float data2[3];

        se_rect2arch_column(lvl_x,data1);
        se_rect2arch_column(lvl_x + map->tile_width,data2);

        for(i = 0; i < map_height; i++){
            int lvl_y = i*map->tile_height;
            lvl_y = map->tile_height * (map->height-1) - lvl_y;

            x = map->data[k + i * map->width] % (map->image_width / map->tile_width) - 1;
            y = map->data[k + i * map->width] / (map->image_height / map->tile_height);


            float tx_1 = map->x2tc[x];
            float tx_2 = map->x2tc[x+1];
            float ty_2 = map->y2tc[y];
            float ty_1 = map->y2tc[y+1];

            cpVect
            p1 = {lvl_x, lvl_y},
            p2 = {lvl_x+map->tile_width, lvl_y},
            p3 = {lvl_x, lvl_y+map->tile_height},
            p4 = {lvl_x+map->tile_width, lvl_y+map->tile_height};

            if(map->data[k + i*map->width] <= 0){
                p1.x = 0;
                p1.y = 0;
                p2.x = 0;
                p2.y = 0;
                p3.x = 0;
                p3.y = 0;
                p4.x = 0;
                p4.y = 0;
            }else{
                se_rect2arch_from_data(&p1,data1);
                se_rect2arch_from_data(&p2,data2);
                se_rect2arch_from_data(&p3,data1);
                se_rect2arch_from_data(&p4,data2);
            }

            map->vertex[k][i*12    ] = p1.x;
            map->vertex[k][i*12 + 1] = p1.y;
            map->vertex[k][i*12 + 2] = p2.x;
            map->vertex[k][i*12 + 3] = p2.y;
            map->vertex[k][i*12 + 4] = p3.x;
            map->vertex[k][i*12 + 5] = p3.y;

            map->vertex[k][i*12 + 6] = p2.x;
            map->vertex[k][i*12 + 7] = p2.y;
            map->vertex[k][i*12 + 8] = p3.x;
            map->vertex[k][i*12 + 9] = p3.y;
            map->vertex[k][i*12 + 10] = p4.x;
            map->vertex[k][i*12 + 11] = p4.y;

            map->textco[k][i*12    ] = tx_1;
            map->textco[k][i*12 + 1] = ty_1;
            map->textco[k][i*12 + 2] = tx_2;
            map->textco[k][i*12 + 3] = ty_1;
            map->textco[k][i*12 + 4] = tx_1;
            map->textco[k][i*12 + 5] = ty_2;

            map->textco[k][i*12 + 6] = tx_2;
            map->textco[k][i*12 + 7] = ty_1;
            map->textco[k][i*12 + 8] = tx_1;
            map->textco[k][i*12 + 9] = ty_2;
            map->textco[k][i*12 + 10] = tx_2;
            map->textco[k][i*12 + 11] = ty_2;

        }

        glVertexPointer(2, GL_FLOAT, 0, map->vertex[k]);
        glTexCoordPointer( 2, GL_FLOAT, 0, map->textco[k]);
        glDrawArrays(GL_TRIANGLES,0, map->height*6);

    }
    glDisable(GL_TEXTURE_2D);
}



/**
 * parsing
 */
int tilemap_create (tilemap *map, char *filename)
{

	mxml_node_t * tree = NULL;
	mxml_node_t * node  = NULL;

	char tilemap_path[200];
	sprintf(tilemap_path,"tilemaps/%s", filename);

	char buffer[TILEMAP_READ_BUFFER_SIZE];
	int filesize = waffle_read_file(tilemap_path, buffer, TILEMAP_READ_BUFFER_SIZE);

	if (filesize){
		tree = mxmlLoadString (NULL , buffer , MXML_OPAQUE_CALLBACK);
	}else {
		SDL_Log("tilemap.c: file: %s could not be loaded\n",tilemap_path);
		return 1;
	}
	if(tree == NULL){
		SDL_Log("tilemap.c file: %s is empty \n",tilemap_path);
		return 1;
	}

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

	// post-edit height (to be able to draw tiles below ground)
	map->height -= 4;

	int width = map->image_width / map->tile_width;
	int height = map->image_height / map->tile_height;

	map->vertex = (GLfloat**)calloc(map->width, sizeof(GLfloat*));
	map->textco = (GLfloat**)calloc(map->width, sizeof(GLfloat*));

	int i;
	for (i = 0; i < map->width; i++){
	    map->vertex[i] = (GLfloat*)calloc(map->height * 6 * 2 , sizeof(GLfloat));
	    map->textco[i] = (GLfloat*)calloc(map->height * 6 * 2 , sizeof(GLfloat));
	}
	for (i = 0;  i <= width; i++) {
	    map->x2tc[i] = 1.0f * i / width;
	}
	for (i = 0;  i <= height; i++) {
	    map->y2tc[i] = 1.0f * (i) / height;
	}

	return 0;
}

void tilemap_destroy(tilemap *map)
{
	free(map->data);
	int i;
    for (i = 0; i < map->width; i++){
        free(map->vertex[i]);
        free(map->textco[i]);
    }
    free(map->vertex);
    free(map->textco);
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
	//SDL_Log("HELLO %d TEXTUREMAP.c \n",__LINE__);
	return 0;
}
