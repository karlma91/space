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

typedef enum ATOM_ID {
	ATOM_ID_SPRITE,
	ATOM_ID_POLLYGON,
	ATOM_ID_OUTLINE,
	ATOM_ID_TEXT
} ATOM_ID;

typedef struct blend_tree {
	//int active;
	Blend type;
	arraylist *al_tex;
} blend_tree;

typedef struct atom_draw {
	ATOM_ID type;
	Color col;
	//cpVect pos;
	//float scale;
	void *data;
} atom_draw;

pool *pool_atom;


void layersystem_init(void)
{
	pool_atom = pool_create(sizeof (atom_draw));
}

layer_system * layersystem_new(void)
{
	layer_system * ls = calloc(1, sizeof *ls);
	ls->num_layers = 0;
	ls->layers = alist_new();
	layersystem_add_layer(ls);
	return ls;
}

int layersystem_add_layer(layer_system *lsys)
{
	layer_ins *layer = calloc(1,sizeof(layer_ins));
	layer->ll_spr = llist_create();
	llist_set_remove_callback(layer->ll_spr, free);
	layer->parallax_factor = 1;
	layer->parallax_zoom = 1;
	layer->ll_blend_modes = llist_create();
	lsys->num_layers += 1;
	return alist_add(lsys->layers, layer);
}

void state_add_sprite(STATE_ID state_id, int layer, SPRITE_ID id, float w, float h, cpVect p, float a)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}

	layer_ins *lay = alist_get(ls->layers, layer);

	sprite *s = calloc(1, sizeof *s);
	sprite_create(s, id, w, h, 0);
	s->a = a;
	s->pos = p;
	llist_add(lay->ll_spr, s);
}

static void llrmcall(atom_draw *atom)
{
	pool_release(pool_atom, atom);
}

void layersystem_register_sprite(STATE_ID state_id, int layer, sprite * spr)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}

	/* current layer */
	layer_ins *lay = alist_get(ls->layers, layer);
	LList ll_blend = lay->ll_blend_modes;
	Blend current_blend = draw_get_current_blend();

	blend_tree *blend_texs = NULL;

	/* search for existing blend mode */
	llist_begin_loop(ll_blend);
	while(llist_hasnext(ll_blend)) {
		blend_tree *blend_mode = llist_next(ll_blend);
		if (blend_mode->type.dst_factor == current_blend.dst_factor &&
				blend_mode->type.src_factor == current_blend.src_factor) {
			blend_texs = blend_mode;
			break;
		}
	}
	llist_end_loop(ll_blend);

	/* add blend if not found */
	if (!blend_texs) {
		blend_texs = calloc(1, sizeof *blend_texs);
		blend_texs->type = current_blend;
		blend_texs->al_tex = alist_new();
		llist_add(ll_blend, blend_texs);
	}


	int tex_id = sprite_get_texture(spr);
	arraylist *al = blend_texs->al_tex;

	LList ll_atoms = alist_get(al,tex_id);
	if(ll_atoms == NULL) {
		ll_atoms = llist_create();
		llist_set_remove_callback(ll_atoms, (ll_rm_callback) llrmcall);
		alist_set(al,tex_id,ll_atoms);
	}

	atom_draw *atom = pool_instance(pool_atom);
	atom->type = ATOM_ID_SPRITE;
	atom->data = spr;
	draw_get_current_color((byte *)&atom->col);
	llist_add(ll_atoms, atom);
}

void layersystem_render(STATE_ID state_id, view *cam)
{
	layer_system *ls = state_get_layersystem(state_id);
//	cpVect p = cam->p; //TODO use for parallax effect (zoom and offset)
	if(ls == NULL) {
		SDL_Log("LAYERSYSTEM: rendering NULL");
		return;
	}

	int layer_index = ls->num_layers;
	while (layer_index--) {
		/* register layersystem sprites */
		layer_ins *lay = alist_get(ls->layers, layer_index);
		llist_begin_loop(lay->ll_spr);
		while(llist_hasnext(lay->ll_spr)) {
			sprite *s = llist_next(lay->ll_spr);
			layersystem_register_sprite(state_id, 0, s);
		}
		llist_end_loop(lay->ll_spr);

		/* iterate blend modes */
		llist_begin_loop(lay->ll_blend_modes);
		while(llist_hasnext(lay->ll_blend_modes)) {
			blend_tree *blends = llist_next(lay->ll_blend_modes);
			glBlendFunc(blends->type.src_factor, blends->type.dst_factor);

			/* iterate textures */
			int tex_index;
			int tex_count = alist_size(blends->al_tex);
			for (tex_index = 0; tex_index < tex_count; tex_index++) {
				LList ll_atoms = alist_get(blends->al_tex, tex_index);
				if (ll_atoms) {
					draw_push_matrix();
					//draw_translatev(cpvmult(cpvadd(lay->offset,p), lay->parallax_factor));

					/* iterate atomic draw calls */
					llist_begin_loop(ll_atoms);
					while(llist_hasnext(ll_atoms)) {
						atom_draw *atom = llist_next(ll_atoms);
						draw_color(atom->col);
						switch (atom->type) {
						case ATOM_ID_SPRITE:
							sprite_final_render((sprite *)(atom->data));
							break;
						case ATOM_ID_POLLYGON:
							break;
						case ATOM_ID_OUTLINE:
							break;
						case ATOM_ID_TEXT:
							break;
						}
						llist_remove(ll_atoms, atom);
					}
					llist_end_loop(ll_atoms);

					draw_flush();
					draw_pop_matrix();
				}
			}
		}
		llist_end_loop(lay->ll_blend_modes);
	}
}


void state_set_layer_offset(STATE_ID state_id, int layer, cpVect offset)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}
	layer_ins *lay = alist_get(ls->layers, layer);
	lay->offset = offset;
}

void state_set_layer_parallax(STATE_ID state_id, int layer, float factor, float zoom_factor)
{
	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return;
	}
	layer_ins *lay = alist_get(ls->layers, layer);
	lay->parallax_factor = factor;
	lay->parallax_zoom = zoom_factor;
}


void layersystem_free(layer_system *ls)
{
	int i;
	for(i = 0; i < ls->num_layers; i++) {
		layer_ins *lay = alist_get(ls->layers, i);
		llist_destroy(lay->ll_spr);
		llist_destroy(lay->ll_blend_modes); //TODO add remove callback to free arraylists and linked lists inside ll_blend!
#warning SOME ARRAYLISTS AND LINKED LISTS ARE NOT BEEING FREED!
	}
	alist_destroy(ls->layers);
	free(ls);
}

void layersystem_destroy()
{
	pool_destroy(pool_atom);
}
