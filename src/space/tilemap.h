/*
 * tilemap.h
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 */

#ifndef TILEMAP_H_
#define TILEMAP_H_

#include "we_graphics.h"

#define MAX_SINGLE_TILE_COUNT 64


typedef struct tilemap {

    int total_width; /* total width in pixels */
    int total_height;

    int tile_width;
    int tile_height;

    int image_width;
    int image_height;
    SPRITE_ID spr_id;

    int width; /* number of tiles in level */
    int height;

    float x2tc[MAX_SINGLE_TILE_COUNT];
    float y2tc[MAX_SINGLE_TILE_COUNT];

    int *data; /* tilmap data array */

}tilemap;

#define TILEMAP_MAX_COLS 100
#define TILEMAP_MAX_ROWS 100
#define TILEMAP_LAYERS TLAY_COUNT
typedef enum tile_layers {
	TLAY_OVERLAY,
	TLAY_SOLID,
	TLAY_BACKGROUND,
	TLAY_COUNT
} tile_layers;

typedef struct tilemap2 {
	int layers;

	int cols;
	int rows;
	char *tileset_name;
	SPRITE_ID tileset;
	byte data[TILEMAP_LAYERS][TILEMAP_MAX_ROWS][TILEMAP_MAX_COLS];
}tilemap2;

void tilemap_fill(int layers, int rows, int cols, byte data[TILEMAP_LAYERS][TILEMAP_MAX_ROWS][TILEMAP_MAX_COLS], tilemap2 *tiles);

void tilemap_render(int layer, tilemap *map);
int tilemap_create (tilemap *map, char *filename);
void tilemap_destroy(tilemap *map);


#endif /* TILEMAP_H_ */
