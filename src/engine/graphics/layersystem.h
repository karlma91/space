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
#include "../graphics/camera.h"

typedef struct {
	float parallax_factor;
	float parallax_zoom;
	cpVect offset;
	LList ll_spr;
	LList ll_blend_modes;
}layer_ins;

typedef struct {
	int num_layers;
	arraylist *layers;
}layer_system;

typedef struct {
	int id;
	union {
		sprite *spr;
	};
} render_object;


void layersystem_init(void);
layer_system * layersystem_new(void);
int layersystem_add_layer(layer_system *ls);
void layersystem_free(layer_system *ls);
void layersystem_destroy(void);
#endif /* LAYERSYSTEM_H_ */
