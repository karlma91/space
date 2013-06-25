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

#define TILEMAP_ARRAY_BUFFER_SIZE 1000

static float vertex_buffer[TILEMAP_ARRAY_BUFFER_SIZE];
static float uv_buffer[TILEMAP_ARRAY_BUFFER_SIZE];

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

	int map_height = map->height;

	static int tile_count_max = 0;
	int tile_count = 0;

	map_height +=4;

	// prepare vertex and uv buffers
	float *vertex_pointer = &vertex_buffer[0];
	float *uv_pointer = &uv_buffer[0];

	int flush_count = 0; //stats
	static int flush_count_max = 0;

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
				float tx_1 = map->x2tc[x];
				float tx_2 = map->x2tc[x+1];
				float ty_2 = map->y2tc[y];
				float ty_1 = map->y2tc[y+1];

				float sub_map[8] = {
						tx_1, ty_1,
						tx_2, ty_1,
						tx_1, ty_2,
						tx_2, ty_2,
				};

				cpVect
				p1 = {lvl_x, lvl_y},
				p2 = {lvl_x+map->tile_width, lvl_y},
				p3 = {lvl_x, lvl_y+map->tile_height},
				p4 = {lvl_x+map->tile_width, lvl_y+map->tile_height};

				se_rect2arch_from_data(&p1, &data1);
				se_rect2arch_from_data(&p2, &data2);
				se_rect2arch_from_data(&p3, &data1);
				se_rect2arch_from_data(&p4, &data2);

				GLfloat vertex_quad[8] = {p1.x, p1.y,
						p2.x,  p2.y,
						p3.x,  p3.y,
						p4.x,  p4.y};

				vertex_pointer = draw_append_quad(vertex_pointer, &vertex_quad[0]);
				uv_pointer = draw_append_quad(uv_pointer, &sub_map[0]);

				int size = (vertex_pointer - &vertex_buffer[0]);
				if (size >= TILEMAP_ARRAY_BUFFER_SIZE - 12) {
					++flush_count;
					draw_flush(vertex_buffer, uv_buffer, size);
					vertex_pointer = &vertex_buffer[0];
					uv_pointer = &uv_buffer[0];
				}
				++tile_count;
			}
		}
	}
	// flush buffers
	int size = (vertex_pointer - &vertex_buffer[0]);
	if (size > 0) {
		draw_flush(vertex_buffer, uv_buffer, size);
		++flush_count;
	}

	if (flush_count > flush_count_max) {
		flush_count_max = flush_count;
		SDL_Log("max tilemap flush count: %d",flush_count);
	}
	if (tile_count > tile_count_max) {
		tile_count_max = tile_count;
		SDL_Log("max tile count rendered: %d",tile_count);
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
