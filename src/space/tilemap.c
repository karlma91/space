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
#include "game.h"

#include "spaceengine.h"

#define TILEMAP_READ_BUFFER_SIZE 128000

int tile_render_layers[TILEMAP_LAYERS] = {
		RLAY_GAME_FRONT,
		RLAY_BACK_FRONT,
		RLAY_BACK_MID
};

void tilemap2_render(tilemap2 *tm)
{
	draw_color(COL_WHITE);
	//texture_bind_virt(SPRITE_WHITE);

	//TODO use current view to determine which tiles to render


	byte data;
	int l, x, y;
	float verts[8], tex[8];
	sprite_get_subimg_by_index(SPRITE_WHITE, 0, tex);
	for (l=TLAY_COUNT-1; l>=0; l--) {
		//draw_color_rgbmulta4f(l==0, l==1, l==2, 0.4);

		if (l==1) {
			//Color col = {50,150,50,255}; //TMP TODO REMOVE
			draw_color4f(0.2,0.6,0.2,1);
		} else {
			draw_color4f(l==2 ? 0 : 1, l==2 ? 0.3 : 0.2, l==2 ? 0.6 : 0.2, l==2 ? 1 : 0.4);
		}
		for (y = tm->grid->inner_i; y < tm->grid->outer_i; y++) {
			for (x=0; x<tm->grid->cols; x++) {
				Color col = {50,150,50,255}; //TMP TODO REMOVE
				data = tilemap_gettype(tm, l, x, y);
				if (data) {
					grid_getquad8f(tm->grid, verts, x, y);
					switch (data) {
					case TILE_TYPE_DIAG_SWL:
						verts[2] = (verts[6] + verts[2]) / 2;
						verts[3] = (verts[7] + verts[3]) / 2;
						break;
					case TILE_TYPE_DIAG_SWR:
						verts[0] = (verts[4] + verts[0]) / 2;
						verts[1] = (verts[5] + verts[1]) / 2;
						verts[2] = verts[0];
						verts[3] = verts[1];
						break;
					case TILE_TYPE_DIAG_SW:
						verts[2] = verts[0];
						verts[3] = verts[1];
						break;
					case TILE_TYPE_DIAG_SEL:
						verts[2] = (verts[6] + verts[2]) / 2;
						verts[3] = (verts[7] + verts[3]) / 2;
						verts[0] = verts[2];
						verts[1] = verts[3];
						break;
					case TILE_TYPE_DIAG_SER:
						verts[0] = (verts[4] + verts[0]) / 2;
						verts[1] = (verts[5] + verts[1]) / 2;
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
					if (tilemap_isdestroyable(tm, l, x, y)) {
						col.r = col.g;
						col.g /= 2;
						col.b = 0;
					}

					draw_color(col);
					draw_quad_new(tile_render_layers[l], verts, tex);
				}
			}
		}
	}


	GLfloat tex2[8] = {0, 0,
			1,  0,
			0,    0.5,
			1,  0.5};

	texture_bind_virt(texture_load("metal01.png"));
	for (y = tm->grid->inner_i; y < tm->grid->outer_i; y++) {
		for (x=0; x<tm->grid->cols; x++) {
			data = tilemap_gettype(tm, 1, x, y);
			if (data && tilemap_isground(tm, 1, x, y)) {
				Color col = {200,250,200,200}; //TMP TODO REMOVE?
				grid_getquad8f(tm->grid, verts, x, y);

				switch (data) {
				case TILE_TYPE_DIAG_SW:
					verts[2] = verts[6];
					verts[3] = verts[7];
					break;
				case TILE_TYPE_DIAG_SE:
					verts[0] = verts[4];
					verts[1] = verts[5];
					break;
				case TILE_TYPE_DIAG_SWL:
					verts[2] = (verts[6] + verts[2]) / 2;
					verts[3] = (verts[7] + verts[3]) / 2;
					break;
				case TILE_TYPE_DIAG_SWR:
					verts[0] = (verts[4] + verts[0]) / 2;
					verts[1] = (verts[5] + verts[1]) / 2;
					verts[2] = verts[6];
					verts[3] = verts[7];
					break;
				case TILE_TYPE_DIAG_SEL:
					verts[2] = (verts[6] + verts[2]) / 2;
					verts[3] = (verts[7] + verts[3]) / 2;
					verts[0] = verts[4];
					verts[1] = verts[5];
					break;
				case TILE_TYPE_DIAG_SER:
					verts[0] = (verts[4] + verts[0]) / 2;
					verts[1] = (verts[5] + verts[1]) / 2;
					break;
				}
				if (tilemap_isdestroyable(tm, 1, x, y)) {
					col.r = col.g;
					col.g /= 2;
					col.b = 0;
				}
				cpVect a = cpv(verts[0],verts[1]);
				cpVect b = cpv(verts[2],verts[3]);

				float m_top = 0.015;
				float m_bot = 0.015;
				cpVect v0 = cpvadd(a, cpvmult(a, -m_top));
				cpVect v1 = cpvadd(b, cpvmult(b, -m_top));
				cpVect v2 = cpvadd(a, cpvmult(a, m_bot));
				cpVect v3 = cpvadd(b, cpvmult(b, m_bot));

				verts[0] = v0.x; verts[1] = v0.y;
				verts[2] = v1.x; verts[3] = v1.y;
				verts[4] = v2.x; verts[5] = v2.y;
				verts[6] = v3.x; verts[7] = v3.y;

				draw_color(col);
				draw_quad_new(tile_render_layers[TLAY_SOLID], verts, tex2);
			}
		}
	}
}

void tilemap_updatetile(tilemap2 *tm, int layer, int x, int y)
{
	grid_wrap_index(tm->grid, &x, &y);
	byte data = tilemap_gettype(tm, layer, x, y);

	if (data) {
		we_bool is_destroyable = tilemap_isdestroyable(tm, layer, x, y);
		byte N, E, S, W, NW, NE, SE, SW;
		N = tilemap_gettype(tm, layer, x,   y-1);
		E = tilemap_gettype(tm, layer, x+1, y);
		S = tilemap_gettype(tm, layer, x,   y+1);
		W = tilemap_gettype(tm, layer, x-1, y);

		NW = tilemap_gettype(tm, layer, x-1, y-1);
		NE = tilemap_gettype(tm, layer, x+1, y-1);
		SE = tilemap_gettype(tm, layer, x+1, y+1);
		SW = tilemap_gettype(tm, layer, x-1, y+1);

		byte NW2, W2, SW2;
		NW2 = tilemap_gettype(tm, layer, x-2, y-1);
		W2  = tilemap_gettype(tm, layer, x-2, y);
		SW2 = tilemap_gettype(tm, layer, x-2, y+1);
		byte NE2, E2, SE2;
		NE2 = tilemap_gettype(tm, layer, x+2, y-1);
		E2  = tilemap_gettype(tm, layer, x+2, y);
		SE2 = tilemap_gettype(tm, layer, x+2, y+1);


		we_bool D_SW, D_SE, D_NE, D_NW;
		D_SW = !(N | E) && ((S && SE) || (W && NW)); /* |\ */
		D_SE = !(N | W) && ((S && SW) || (E && NE)); /* /| */
		D_NE = !(S | W) && ((N && NW) || (E && SE)); /* \| */
		D_NW = !(S | E) && ((N && NE) || (W && SW)); /* |/ */

		we_bool D_SWL, D_SWR, D_SEL, D_SER;
		D_SEL = !(W | N | NE) && ((SW && S && SE) || (NE2 && E2)) && E;
		D_SER = !(W2 | NW | N) && ((SW2 && SW && S) || (NE && E)) && W;
		D_SWL = !(E2 | NE | N) && ((SE2 && SE && S) || (NW && W)) && E;
		D_SWR = !(E | N | NW) && ((SE && S && SW) || (NW2 && W2)) && W;


		tm->data[layer][y][x] =
		D_SEL && !(D_SWL | D_SWR) ?
			TILE_TYPE_DIAG_SEL :
		D_SER && !(D_SWL | D_SWR) ?
			TILE_TYPE_DIAG_SER :
		D_SWL && !(D_SEL | D_SER) ?
			TILE_TYPE_DIAG_SWL :
		D_SWR && !(D_SEL | D_SER) ?
			TILE_TYPE_DIAG_SWR :
		D_SW && !(D_SE || D_NW) ?
			TILE_TYPE_DIAG_SW :
		D_SE && !(D_SW || D_NE) ?
			TILE_TYPE_DIAG_SE :
		D_NE && !(D_NW || D_SE) ?
			TILE_TYPE_DIAG_NE :
		D_NW && !(D_NE || D_SW) ?
			TILE_TYPE_DIAG_NW :
		!N ? TILE_TYPE_GROUND : TILE_TYPE_FULL;

		tm->data[layer][y][x] |= is_destroyable ? TILE_DESTROYABLE_BIT : 0;
	}
}

void tilemap_clear(tilemap2 *tm)
{
	int l, x, y;
	for (l=TLAY_COUNT-1; l>=0; l--)
		for (y = tm->grid->inner_i; y < tm->grid->outer_i - 1; y++)
			for (x=0; x<tm->grid->cols; x++) {
				tm->data[l][y][x] = TILE_TYPE_NONE;
			}
	for (y = tm->grid->inner_i; y < tm->grid->outer_i - 1; y++)
		for (x=0; x<tm->grid->cols; x++) {
			cpShape *shape = tm->metadata[y][x].block;
			tm->metadata[y][x].block = NULL;
			if (shape) {
				cpSpaceRemoveShape(current_space, shape);
				cpShapeFree(shape);
			}
		}
}
