/*
 * tilemap.c
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 *      Author: Mathias
 */

#include <stdio.h>

#include "tilemap.h"

#include "../engine/engine.h"
#include "../engine/io/xmlh.h"
#include "../engine/io/waffle_utils.h"
#include "../engine/graphics/draw.h"
#include "../engine/graphics/texture.h"
#include "we_state.h"

#include "spaceengine.h"


#define TILEMAP_READ_BUFFER_SIZE 128000

static int parse_data(tilemap *map, char *data);

void tilemap_render(int layer, tilemap *map)
{
	draw_color(COL_WHITE);
	texture_bind_virt(sprite_get_texture(map->spr_id));

	int i,j;
	int x,y;
	int j_start, j_end;

	j_start = 0;
	j_end = map->width;

	int map_height = map->height;

	static int tile_count_max = 0;
	int tile_count = 0;

	// prepare vertex and uv buffers
	cpVect data2;
	se_rect2arch_column(j_start*map->tile_width - (map->width*map->tile_width)/2, &data2);

	/** draws from top and down **/
	for(j = j_start; j < j_end; j++){
		/** Circular indexing **/
		int k = 0;
		if (j < 0) {
			/* + because (j % map->width) is negative */
			k = map->width + (j % map->width);
		} else if (j >= map->width) {
			k = j % map->width;
		} else {
			k = j;
		}

		int lvl_x = j*map->tile_width - (map->width*map->tile_width)/2;

		cpVect data1 = data2;
		se_rect2arch_column(lvl_x + map->tile_width,&data2);

		for(i = 0; i < map_height; i++){
			int lvl_y = map->tile_height * (map->height - 1 - i);

			x = map->data[k + i * map->width] % (map->image_width / map->tile_width) - 1;
			y = map->data[k + i * map->width] / (map->image_height / map->tile_height);

			if(map->data[k + i*map->width] > 0){
				float sub_map[8];
				sprite_subimg subimg = {map->x2tc[x], map->y2tc[y], map->x2tc[x+1], map->y2tc[y+1]};
				sprite_get_subimg_mapped(map->spr_id,subimg,sub_map);

				cpVect
				p1 = {lvl_x, lvl_y},
				p2 = {lvl_x+map->tile_width, lvl_y},
				p3 = {lvl_x, lvl_y+map->tile_height},
				p4 = {lvl_x+map->tile_width, lvl_y+map->tile_height};

#if EXPERIMENTAL_GRAPHICS
				se_rect2arch_from_data(&p1, &data1);
				se_rect2arch_from_data(&p2, &data2);
				se_rect2arch_from_data(&p3, &data1);
				se_rect2arch_from_data(&p4, &data2);
#endif
				GLfloat vertex_quad[8] = {p1.x, p1.y, p2.x,  p2.y, p3.x,  p3.y, p4.x,  p4.y};
				draw_quad_new(10, vertex_quad, sub_map);
				++tile_count;
			}
		}
	}

	if (tile_count > tile_count_max) {
		tile_count_max = tile_count;
#if !ARCADE_MODE
		SDL_Log("max tile count rendered: %d",tile_count);
#endif
	}
}


void tilemap_fill(polgrid *grid, int layers, byte data[TILEMAP_LAYERS][TILEMAP_MAX_ROWS][TILEMAP_MAX_COLS], tilemap2 *tiles)
{
	tiles->layers = layers;
	tiles->grid = grid;
	tiles->rows = grid->rows;
	tiles->cols = grid->cols;
	int i,j, k;
	for (i=0; i < tiles->layers; i++) {
		for (j= 0; j < tiles->rows; j++) {
			for (k = 0; k < tiles->cols; k++) {
				tiles->data[i][j][k] = data[i][j][k];
			}
		}
	}
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
	int filesize = waffle_read_file_zip(tilemap_path, buffer, TILEMAP_READ_BUFFER_SIZE);

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
	    		char *suffix = strchr(name, '.');
	    		if (suffix) *suffix = '\0'; /* removes suffix */
				map->spr_id = sprite_link(name);

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
	//map->height -= 4;

	int width = map->image_width / map->tile_width;
	int height = map->image_height / map->tile_height;

	int i;
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
