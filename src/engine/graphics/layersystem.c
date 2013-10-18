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
		ls->li_layers[i].tex_list = al_new();
		llist_set_remove_callback(ls->li_layers[i].li_spr, free);
	}

	return ls;
}

void layersystem_add_sprite(layer_system *ls, int layer, SPRITE_ID id, float w, float h, cpVect p, float a)
{
	if(check_bounds(ls, layer)){
		return;
	}
	sprite *s = calloc(1, sizeof *s);
	sprite_create(s, id, w, h, 0);
	s->a = a;
	s->pos = p;
	llist_add(ls->li_layers[layer].li_spr, s);
}

void layersystem_register_sprite(layer_system *ls, int layer, sprite * spr)
{
	if(check_bounds(ls, layer)){
		return;
	}

	arraylist *al = ls->li_layers[layer].tex_list;

	sprite_data *data = (sprite_data*)spr->id;
	int tex_id = 0;
	if (data) {
		tex_id = data->tex_id;
	}

	LList l = al_get(al,tex_id);
	if(l == NULL) {
		l = llist_create();
		al_set(al,tex_id,l);
	}
	llist_add(l, spr);

}

void layersystem_render(layer_system *ls, cpVect p)
{
	if(ls == NULL) {
		SDL_Log("LAYERSYSTEM: rendering NULL");
		return;
	}

	int i=ls->num_layers - 1;
	while (i--) {
		layer *lay = &(ls->li_layers[i]);

		llist_begin_loop(lay->li_spr);
		draw_push_matrix();
		//draw_translatev(cpvmult(cpvadd(lay->offset,p), lay->parallax_factor));
		while(llist_hasnext(lay->li_spr)) {
			sprite *s = llist_next(lay->li_spr);
			sprite_final_render(s);
		}
		draw_pop_matrix();
		llist_end_loop(lay->li_spr);
		draw_flush();

		int j;
		int size = al_size(lay->tex_list);
		for (j = 0; j < size; j++) {
			LList l = al_get(lay->tex_list, j);
			if (l) {
				llist_begin_loop(l);
				draw_push_matrix();
				//camera *cam = current_camera;

				//draw_translatev(cpvmult(cpvadd(lay->offset,p), lay->parallax_factor));
				while(llist_hasnext(l)) {
					sprite *s = llist_next(l);
					sprite_final_render(s);
				}
				llist_end_loop(l);
				draw_pop_matrix();
			}
		}
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
