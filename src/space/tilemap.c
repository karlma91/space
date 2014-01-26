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

void tilemap2_render(tilemap2 *tm)
{
	draw_color(COL_WHITE);
	//texture_bind_virt(SPRITE_WHITE);

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

		we_bool D_SW, D_SE, D_NE, D_NW;

		D_SW = !(N | E) && ((S && SE) || (W && NW)); /* |\ */
		D_SE = !(N | W) && ((S && SW) || (E && NE)); /* /| */
		D_NE = !(S | W) && ((N && NW) || (E && SE)); /* \| */
		D_NW = !(S | E) && ((N && NE) || (W && SW)); /* |/ */

		tm->data[layer][y][x] =
		D_SW && !(D_SE || D_NW) ?
			TILE_TYPE_DIAG_SW :
		D_SE && !(D_SW || D_NE) ?
			TILE_TYPE_DIAG_SE :
		D_NE && !(D_NW || D_SE) ?
			TILE_TYPE_DIAG_NE :
		D_NW && !(D_NE || D_SW) ?
			TILE_TYPE_DIAG_NW : TILE_TYPE_FULL;
	}
}

