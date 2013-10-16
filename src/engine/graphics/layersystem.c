#include "layersystem.h"
#include "we_graphics.h"
static int check_bounds(layer_system *ls, int layer)
{
	if(ls == NULL){
		SDL_Log("LAYERSYSTEM: Trying access a NULL system");
		return 1;
	}
	if(layer < 0 && layer > ls->num_layers) {
		SDL_Log("LAYERSYSTEM: Trying to access layer %d outside range %d", layer, ls->num_layers);
		return 1;
	}
	return 0;
}

void layersystem_init(void)
{

}

layer_system * layersystem_new(int num_layers)
{
	layer_system * ls = calloc(1, sizeof *ls);

	if(num_layers > 0) {
		ls->num_layers = num_layers;
		ls->li_layers = calloc(num_layers, sizeof(layer));
	}else{
		return NULL;
	}

	int i;
	for(i=0; i < num_layers; i++) {
		ls->li_layers[i].li_spr = llist_create();
		ls->li_layers[i].parallax_factor = 1;
		ls->li_layers[i].parallax_zoom = 1;
		llist_set_remove_callback(ls->li_layers[i].li_spr, free);
	}

	return ls;
}

void layersystem_add_sprite(layer_system *ls, int layer, SPRITE_ID id, float w, float h, cpVect p, float a)
{
	if(check_bounds(ls, layer)){
		return;
	}
	sprite_ext *se = calloc(1, sizeof *se);
	se->a = a;
	se->pos = p;
	sprite_create(&(se->spr), id, w, h, 0);
	llist_add(ls->li_layers[layer].li_spr, se);
}

void layersystem_render(layer_system *ls, cpVect p)
{
	if(ls == NULL) {
		SDL_Log("LAYERSYSTEM: rendering NULL");
		return;
	}

	int i=ls->num_layers;
	while (--i) {
		layer *lay = &(ls->li_layers[i]);
		llist_begin_loop(lay->li_spr);
		draw_push_matrix();
		//camera *cam = current_camera;

		draw_translatev(cpvmult(cpvadd(lay->offset,p), lay->parallax_factor));
		while(llist_hasnext(ls->li_layers[i].li_spr)) {
			sprite_ext *se = llist_next(lay->li_spr);
			sprite_render(&(se->spr), se->pos, se->a);
		}
		llist_end_loop(lay->li_spr);
		draw_pop_matrix();
	}
}


void layersystem_set_layer_offset(layer_system *ls, int layer, cpVect offset)
{
	if(check_bounds(ls, layer)){
		return;
	}
	ls->li_layers[layer].offset = offset;
}

void layersystem_set_layer_parallax(layer_system *ls, int layer, float factor, float zoom_factor)
{
	if(check_bounds(ls, layer)){
		return;
	}
	ls->li_layers[layer].parallax_factor = factor;
	ls->li_layers[layer].parallax_zoom = zoom_factor;
}


void layersystem_destroy(layer_system *ls)
{
	int i;
	for(i = 0; i < ls->num_layers; i++) {
		llist_destroy(ls->li_layers[i].li_spr);
	}
	free(ls->li_layers);
	free(ls);
}
