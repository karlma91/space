/*
 * tilemap.h
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 */

#ifndef TILEMAP_H_
#define TILEMAP_H_

#include "we_graphics.h"
#include "we_data.h"

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

typedef enum TILE_TYPE{
	TILE_TYPE_NONE,
	TILE_TYPE_UNDEF,
	TILE_TYPE_FULL,
	TILE_TYPE_DIAG_SW, // TILE: |\ SW
	TILE_TYPE_DIAG_SE, // TILE: /| SE
	TILE_TYPE_DIAG_NE, // TILE: \| NE
	TILE_TYPE_DIAG_NW, // TILE: |/ NW
	TILE_TYPE_COUNT
} TILE_TYPE;

typedef struct tilemap2 {
	int layers;
	polgrid *grid;

	//int cols;
	//int rows;

	char *tileset_name;
	SPRITE_ID tileset;
	byte data[TILEMAP_LAYERS][TILEMAP_MAX_ROWS][TILEMAP_MAX_COLS];
	cpShape *blocks[TILEMAP_MAX_ROWS][TILEMAP_MAX_COLS];
	int render_layers[TILEMAP_LAYERS];
}tilemap2;

static __inline__ byte tilemap_getdata(tilemap2 *tm, int layer, int x, int y)
{
	int cols = tm->grid->cols, rows = tm->grid->rows;
	return tm->data[layer]
	        [(y >= rows-1) ? (rows-2 > 0 ? rows-2 : 0) : (y < 0) ? 0 : y]
	        [(x >= cols) ? x - cols : (x < 0) ? x + cols : x];
}

static __inline__ void tilemap_updatetile(tilemap2 *tm, int layer, int x, int y)
{
	int cols = tm->grid->cols, rows = tm->grid->rows;
	x = (x >= cols) ? x - cols : (x < 0) ? x + cols : x;
	y = (y >= rows-1) ? (rows-2 > 0 ? rows-2 : 0) : (y < 0) ? 0 : y;

	byte data = tilemap_getdata(tm, layer, x, y);

	if (data) {
		byte N, E, S, W;
		N = tilemap_getdata(tm, layer, x,   y-1);
		E = tilemap_getdata(tm, layer, x+1, y);
		S = tilemap_getdata(tm, layer, x,   y+1);
		W = tilemap_getdata(tm, layer, x-1, y);

		data = TILE_TYPE_FULL;
		if (!N && !E && S && W) {         /* |\ */
			data = TILE_TYPE_DIAG_SW;
		} else if (!N && E && S && !W) {  /* /| */
			data = TILE_TYPE_DIAG_SE;
		} else if (N && E && !S && !W) {  /* \| */
			data = TILE_TYPE_DIAG_NE;
		} else if (N && !E && !S && W) {  /* |/ */
			data = TILE_TYPE_DIAG_NW;
		}
		tm->data[layer][y][x] = data;
	}
}

static __inline__ void tilemap_settile(tilemap2 *tm, int layer, int x, int y, we_bool set)
{
	int cols = tm->grid->cols, rows = tm->grid->rows;
	x = (x >= cols) ? x - cols : (x < 0) ? x + cols : x;
	y = (y >= rows-1) ? (rows-2 > 0 ? rows-2 : 0) : (y < 0) ? 0 : y;

	tm->data[layer][y][x] = set ? TILE_TYPE_UNDEF : TILE_TYPE_NONE;
	tilemap_updatetile(tm, layer, x, y);
	tilemap_updatetile(tm, layer, x+1, y);
	tilemap_updatetile(tm, layer, x-1, y);
	tilemap_updatetile(tm, layer, x, y+1);
	tilemap_updatetile(tm, layer, x, y-1);
}

void tilemap2_render(tilemap2 *tm);
void tilemap_fill(void *unused, int layers, void *unused2, tilemap2 *tiles);
void tilemap_render(int layer, tilemap *map);
int tilemap_create (tilemap *map, char *filename);
void tilemap_destroy(tilemap *map);

#endif /* TILEMAP_H_ */
