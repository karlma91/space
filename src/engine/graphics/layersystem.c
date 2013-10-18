#include "layersystem.h"
#include "we_graphics.h"
#include "../state/statesystem.h"
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

layer_system * state_get_layersystem(STATE_ID state_id);

void layersystem_init(void)
{

}

layer_system * layersystem_new(void)
{
	layer_system * ls = calloc(1, sizeof *ls);
	ls->num_layers = 0;
	ls->layers = al_new();
	layersystem_add_layer(ls);
	return ls;
}

int layersystem_add_layer(layer_system *ls)
{
	layer_ins *lay = calloc(1,sizeof(layer_ins));
	lay->li_spr = llist_create();
	lay->parallax_factor = 1;
	lay->parallax_zoom = 1;
	lay->tex_list = al_new();
	llist_set_remove_callback(lay->li_spr, free);
	ls->num_layers += 1;
	return al_add(ls->layers, lay);
}

void state_add_sprite(STATE_ID state_id, int layer, SPRITE_ID id, float w, float h, cpVect p, float a)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}

	layer_ins *lay = al_get(ls->layers, layer);

	sprite *s = calloc(1, sizeof *s);
	sprite_create(s, id, w, h, 0);
	s->a = a;
	s->pos = p;
	llist_add(lay->li_spr, s);
}

void layersystem_register_sprite(STATE_ID state_id, int layer, sprite * spr)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}

	layer_ins *lay = al_get(ls->layers, layer);

	arraylist *al = lay->tex_list;

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

void layersystem_render(STATE_ID state_id, view *cam)
{
	layer_system *ls = state_get_layersystem(state_id);
	cpVect p = cam->p;
	if(ls == NULL) {
		SDL_Log("LAYERSYSTEM: rendering NULL");
		return;
	}

	int i = ls->num_layers;
	while (i--) {
		layer_ins *lay = al_get(ls->layers, i);
		llist_begin_loop(lay->li_spr);
		while(llist_hasnext(lay->li_spr)) {
			sprite *s = llist_next(lay->li_spr);
			layersystem_register_sprite(state_id, 0, s);
		}
		llist_end_loop(lay->li_spr);

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
					llist_remove(l, s);
				}
				draw_flush();
				llist_end_loop(l);
				draw_pop_matrix();
			}
		}
	}

}


void state_set_layer_offset(STATE_ID state_id, int layer, cpVect offset)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}
	layer_ins *lay = al_get(ls->layers, layer);
	lay->offset = offset;
}

void state_set_layer_parallax(STATE_ID state_id, int layer, float factor, float zoom_factor)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}
	layer_ins *lay = al_get(ls->layers, layer);
	lay->parallax_factor = factor;
	lay->parallax_zoom = zoom_factor;
}


void layersystem_destroy(layer_system *ls)
{
	int i;
	for(i = 0; i < ls->num_layers; i++) {
		layer_ins *lay = al_get(ls->layers, i);
		llist_destroy(lay->li_spr);
	}
	al_destroy(ls->layers);
	free(ls);
}
