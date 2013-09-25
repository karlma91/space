/*
 * layersystem.h
 *
 *  Created on: Sep 21, 2013
 *      Author: karlmka
 */

#ifndef LAYERSYSTEM_H_
#define LAYERSYSTEM_H_

#include "sprite.h"
#include "we_data.h"

typedef struct {
	cpVect pos;
	float a;
	sprite spr;
} sprite_ext;

typedef struct {
	float parallax_depth;
	float parallax_zoom;
	cpVect offset;
	LList li_spr;
}layer;

typedef struct {
	int num_layers;
	layer *li_layers;
}layer_system;


void layersystem_init(void);
layer_system * layersystem_new(int num_layers);
void layersystem_add_sprite(layer_system *ls, int layer, SPRITE_ID id, float w, float h, cpVect p, float a);
void layersystem_render(layer_system *ls, cpVect cam_position);
void layersystem_set_layer_offset(layer_system *ls, int layer, cpVect offset);
void layersystem_set_layer_parallax(layer_system *ls, int layer, float depth, float zoom);
void layersystem_destroy(layer_system *ls);
#endif /* LAYERSYSTEM_H_ */
