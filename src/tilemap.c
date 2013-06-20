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
#include "main.h"
#include "spaceengine.h"

#include "waffle_utils.h"
#include "SDL_log.h"

#define TILEMAP_READ_BUFFER_SIZE 128000

static int parse_data(tilemap *map, char *data);
static void draw_subimage(GLfloat x, GLfloat y, GLfloat tx, GLfloat ty, GLfloat w, GLfloat h, GLfloat tile_width, GLfloat tile_height);

void tilemap_render(tilemap *map)
{
    draw_color4f(1,1,1,1);
    texture_bind(map->texture_id);

    GLfloat w = ((GLfloat)map->tile_width)/map->image_width;
    GLfloat h = ((GLfloat)map->tile_height)/map->image_height;

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
    map_height +=4;
#endif

    /** draws from top and down **/
    for(i = 0; i < map_height; i++){
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

#if EXPERIMENTAL_GRAPHICS
    			/* tmp test displacement modifiers */
    			//float p = 1.0f * (j-j_start) / (j_end - j_start);

    			//float r_1 = (currentlvl->right - currentlvl->left)/(2*M_PI);//2100; // inner space station radius



    			//float r_1_tmp = r_1;
    			//r_1 += MOUSE_X*4 - MOUSE_Y*16;

    			//float next_theta = - theta_max * (cam_center_x - (lvl_x - map->tile_width)) / ((cam_right - cam_left)/2);
    			//float next_x = o_x + (r_1 + ry) * sin(next_theta);

    			//float computed_size = new_x - next_x + 0.01f;

    			{
    				GLfloat tx = (x*w), ty = (y*h);
    				cpVect p = cpv(lvl_x,lvl_y);
    				float angle = se_rect2arch(&p);

    				texture_map sub_map = {{tx,ty+h, tx+w,ty+h, tx,ty, tx+w,ty}};

    				cpVect
    					p1 = {lvl_x, lvl_y},
    					p2 = {lvl_x+map->tile_width, lvl_y},
    					p3 = {lvl_x, lvl_y+map->tile_height},
    					p4 = {lvl_x+map->tile_width, lvl_y+map->tile_height};

    				 se_rect2arch(&p1);
    				 se_rect2arch(&p2);
    				 se_rect2arch(&p3);
    				 se_rect2arch(&p4);

    				GLfloat texture_quad[8] = {p1.x, p1.y,
    										p2.x,  p2.y,
    										p3.x,  p3.y,
    										p4.x,  p4.y};

    				draw_current_texture_basic(&sub_map,texture_quad,4);
    			}

    			//r_1 = r_1_tmp;

#else
    			draw_subimage(lvl_x, lvl_y, (x*w), (y*h), w, h, map->tile_width, map->tile_height);
#endif
    		}
    	}
    }
}

static void draw_subimage(GLfloat x, GLfloat y, GLfloat tx, GLfloat ty, GLfloat w, GLfloat h, GLfloat tile_width, GLfloat tile_height)
{
	cpVect p = cpv(x,y);
	texture_map sub_map = {{tx,ty+h, tx+w,ty+h, tx,ty, tx+w,ty}};

	draw_current_texture_all(&p, &sub_map,tile_width,tile_height,0,corner_quad);
}



/**
 * parsing
 */
int tilemap_create (tilemap *map, char *filename)
{
	mxml_node_t * tree = NULL;
	mxml_node_t * node  = NULL;

	char tilemap_name[200];
	sprintf(tilemap_name,"tilemaps/%s", filename);

	ZZIP_FILE *fp = waffle_open(tilemap_name);

	if (fp ){
		char buffer[TILEMAP_READ_BUFFER_SIZE];
		int filesize = zzip_file_read(fp, buffer, TILEMAP_READ_BUFFER_SIZE);
		SDL_Log("filesize: %d", filesize);
		zzip_file_close(fp);
		buffer[filesize] = '\0';

		tree = mxmlLoadString (NULL , buffer , MXML_OPAQUE_CALLBACK);
	}else {
		SDL_Log("tilemap.c: file: %s could not be loaded\n",tilemap_name);
		return 1;
	}
	if(tree == NULL){
		SDL_Log("tilemap.c file: %s is empty \n",tilemap_name);
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

	// post-edit height (so we later draw to tiles below ground)
	map->height -= 4;

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
	//SDL_Log("HELLO %d TEXTUREMAP.c \n",__LINE__);
	return 0;
}
