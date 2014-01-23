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


void tilemap2_render(tilemap2 *tm)
{
	byte data;
	int l, x, y;
	float verts[8], tex[8];
	sprite_get_subimg_by_index(SPRITE_WHITE, 0, tex);
	for (l=TLAY_COUNT-1; l>=0; l--) {
		//draw_color_rgbmulta4f(l==0, l==1, l==2, 0.4);
		//draw_color4f(l==0, l==1, l==2, 0);

		if (l==1) {
			draw_color4f(0.2,0.6,0.2,0.2);
		} else {
			draw_color_rgbmulta4f(l==0, l==1, l==2, 0.4);
		}
		for (y = tm->grid->inner_i; y < tm->grid->outer_i - 1; y++) {
			for (x=0; x<tm->grid->cols; x++) {
				data = tilemap_getdata(tm, l, x, y);
				if (data) {
					grid_getquad8f(tm->grid, verts, x, y);
					switch (data) {
					case TILE_TYPE_DIAG_SW:
						verts[2] = verts[0];
						verts[3] = verts[1];
						break;
					case TILE_TYPE_DIAG_SE:
						verts[0] = verts[2];
						verts[1] = verts[3];
						break;
					case TILE_TYPE_DIAG_NE:
						verts[4] = verts[6];
						verts[5] = verts[7];
						break;
					case TILE_TYPE_DIAG_NW:
						verts[6] = verts[4];
						verts[7] = verts[5];
						break;
					}
					draw_quad_new(tm->render_layers[l], verts, tex);
				}
			}
		}
	}
}


void tilemap_fill(void *unused, int layers, void *unused2, tilemap2 *tiles)
{
	tiles->layers = layers;
	//tiles->rows = tiles->grid->rows;
	//tiles->cols = tiles->grid->cols;
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


void tilemap_updatetile(tilemap2 *tm, int layer, int x, int y)
{
	grid_wrap_index(tm->grid, &x, &y);
	byte data = tilemap_getdata(tm, layer, x, y);

	if (data) {
		byte N, E, S, W, NW, NE, SE, SW;
		N = tilemap_getdata(tm, layer, x,   y-1);
		E = tilemap_getdata(tm, layer, x+1, y);
		S = tilemap_getdata(tm, layer, x,   y+1);
		W = tilemap_getdata(tm, layer, x-1, y);

		NW = tilemap_getdata(tm, layer, x-1, y-1);
		NE = tilemap_getdata(tm, layer, x+1, y-1);
		SE = tilemap_getdata(tm, layer, x+1, y+1);
		SW = tilemap_getdata(tm, layer, x-1, y+1);

		if (N && E && S && W) {
			data = TILE_TYPE_FULL;
		} else if (!N && !E && (S || W) && (SE || NW)) {  /* |\ */
			data = TILE_TYPE_DIAG_SW;
		} else if (!N && !W && (S || E) && (SW || NE)) {  /* /| */
			data = TILE_TYPE_DIAG_SE;
		} else if (!S && !W && (N || E) && (NW || SE)) {  /* \| */
			data = TILE_TYPE_DIAG_NE;
		} else if (!S && !E && (N || W) && (NE || SW)) {  /* |/ */
			data = TILE_TYPE_DIAG_NW;
		} else {
			data = TILE_TYPE_FULL;
		}
		tm->data[layer][y][x] = data;
	}
}

