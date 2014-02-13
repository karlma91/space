/*
 * tilemap.h
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 */

//TODO MOVE TO ENGINE

#ifndef TILEMAP_H_
#define TILEMAP_H_

#include "we_graphics.h"
#include "we_data.h"

#define MAX_SINGLE_TILE_COUNT 64

#define TILEMAP_LAYERS TLAY_COUNT
typedef enum tile_layers {
	TLAY_OVERLAY,
	TLAY_SOLID,
	TLAY_BACKGROUND,
	TLAY_COUNT
} tile_layers;

typedef enum TILE_SHAPE{
	TILE_TYPE_NONE,
	TILE_TYPE_UNDEF,
	TILE_TYPE_FULL,
	TILE_TYPE_DIAG_SW, // TILE: |\ SW
	TILE_TYPE_DIAG_SE, // TILE: /| SE
	TILE_TYPE_DIAG_NE, // TILE: \| NE
	TILE_TYPE_DIAG_NW, // TILE: |/ NW
	TILE_TYPE_DIAG_SER, // TILE:    -'| SE RIGHT HALF
	TILE_TYPE_DIAG_SEL, // TILE:  .-    SE LEFT HALF
	TILE_TYPE_DIAG_SWR, // TILE:  |'-   SW RIGHT HALF
	TILE_TYPE_DIAG_SWL, // TILE:     -. SW LEFT HALF
	TILE_TYPE_GROUND,
	/* NB! ALWAYS ADD NEW TILES RIGHT ABOVE THIS LINE */

	TILE_TYPE_MASK = 0x1F,
	TILE_DESTROYABLE_BIT = 0x20,

	TILE_TYPE_OUTSIDE = 0xFF ^ TILE_DESTROYABLE_BIT
} TILE_TYPE;

extern int tile_render_layers[TILEMAP_LAYERS];

typedef struct tile {
	we_bool destroyable, visible;
	int x_col, y_row;
	cpShape *block;
	float hp;
} meta_tile;

typedef struct tilemap2 {
	int layers;
	polgrid *grid;

	//int cols;
	//int rows;

	char *tileset_name;
	SPRITE_ID tileset;
	byte data[TILEMAP_LAYERS][GRID_MAXROW][GRID_MAXCOL];
	meta_tile metadata[GRID_MAXROW][GRID_MAXCOL];
}tilemap2;

static __inline__ byte tilemap_gettype(tilemap2 *tm, int layer, int x, int y)
{
	int outside = grid_wrap_index(tm->grid, &x, &y);
	return (outside ? TILE_TYPE_OUTSIDE : tm->data[layer][y][x]) & TILE_TYPE_MASK;
}

static __inline we_bool tilemap_isground(tilemap2 *tm, int layer, int x, int y)
{
	TILE_TYPE type = tilemap_gettype(tm, layer, x, y);
	TILE_TYPE type_N = tilemap_gettype(tm, layer, x, y-1);

	return ((type == TILE_TYPE_GROUND) |
			(type == TILE_TYPE_DIAG_SW) |
			(type == TILE_TYPE_DIAG_SE) |
			(type == TILE_TYPE_DIAG_SER) |
			(type == TILE_TYPE_DIAG_SEL) |
			(type == TILE_TYPE_DIAG_SWR) |
			(type == TILE_TYPE_DIAG_SWL)) ||
			(type_N == TILE_TYPE_NONE);
}

static __inline__ we_bool tilemap_isdestroyable(tilemap2 *tm, int layer, int x, int y)
{
	int outside = grid_wrap_index(tm->grid, &x, &y);
	return outside ? WE_FALSE : (tm->data[layer][y][x] & TILE_DESTROYABLE_BIT ? WE_TRUE : WE_FALSE);
}

/*static __inline__ */
void tilemap_updatetile(tilemap2 *tm, int layer, int x, int y);
static __inline__ void tilemap_settile(tilemap2 *tm, int layer, int x, int y, we_bool activate, we_bool destroyable)
{
	grid_wrap_index(tm->grid, &x, &y);
	tm->data[layer][y][x] = activate ? TILE_TYPE_UNDEF : TILE_TYPE_NONE;
	tilemap_updatetile(tm, layer, x, y);
	tilemap_updatetile(tm, layer, x+1, y);
	tilemap_updatetile(tm, layer, x-1, y);
	tilemap_updatetile(tm, layer, x, y+1);
	tilemap_updatetile(tm, layer, x, y-1);
	tilemap_updatetile(tm, layer, x+1, y+1);
	tilemap_updatetile(tm, layer, x-1, y+1);
	tilemap_updatetile(tm, layer, x+1, y-1);
	tilemap_updatetile(tm, layer, x-1, y-1);

	tilemap_updatetile(tm, layer, x-2, y-1);
	tilemap_updatetile(tm, layer, x-2, y);
	tilemap_updatetile(tm, layer, x-2, y+1);
	tilemap_updatetile(tm, layer, x+2, y-1);
	tilemap_updatetile(tm, layer, x+2, y);
	tilemap_updatetile(tm, layer, x+2, y+1);

	tm->data[layer][y][x] |= destroyable ? TILE_DESTROYABLE_BIT : 0;
}

static __inline__ void tilemap_updaterow(tilemap2 *tm, int y)
{
	int layer, x, cols = tm->grid->cols;

	for (layer = 0; layer < TLAY_COUNT; layer++) {
		for (x = 0; x < cols; x++) {
			tilemap_updatetile(tm, layer, x, y);
		}
	}
}

void tilemap_clear(tilemap2 *tm);
void tilemap2_render(tilemap2 *tm);
void tilemap_fill(void *unused, int layers, void *unused2, tilemap2 *tiles);

#endif /* TILEMAP_H_ */
