#include "layersystem.h"
#include "we_graphics.h"
#include "../data/array.h"
#include "../state/statesystem.h"

#define MAX_ELEM_COUNT_BATCH 100000 // Memory for 1000 elements for each combination of texture, blend, layer and state!


static int check_bounds(layer_system *ls, int layer)
{
	if(ls == NULL){
		SDL_Log("LAYERSYSTEM: Trying access a NULL system");
		return 1;
	}
	if(layer < 0 && layer >= ls->num_layers) {
		SDL_Log("LAYERSYSTEM: Trying to access layer %d outside range %d", layer, ls->num_layers);
		return 1;
	}
	return 0;
}

static arraylist *render_tree;

layer_system * state_get_layersystem(STATE_ID state_id);

typedef enum ATOM_ID {
	ATOM_ID_SPRITE,
	ATOM_ID_POLLYGON,
	ATOM_ID_QUADLINE,
	ATOM_ID_TEXT
} ATOM_ID;

typedef struct blend_tree {
	//int active;
	Blend type;
	arraylist *al_tex;
} blend_tree;

typedef struct vertex_elem { // 1 independent quad = 120 bytes
	float x, y; //TODO use indices to save memory!
	float tx, ty; //TODO use short here instead of float!
	Color col;
} vertex_elem;

typedef struct render_batch {
	array *elems;
	int count;
} render_batch;


void layersystem_init(void)
{
	render_tree = alist_new();
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
	layer->parallax_factor = 0;
	layer->parallax_zoom = 1;
	lsys->num_layers += 1;

	LList *blend_modes = alist_get(render_tree, lsys->num_layers - 1);
	if(blend_modes == NULL) {
		blend_modes = llist_create();
		alist_set_safe(render_tree, lsys->num_layers - 1, blend_modes);
	}

	return alist_add(lsys->layers, layer);
}

void state_add_sprite(STATE_ID state_id, int layer, SPRITE_ID id, float w, float h, cpVect p, float a)
{
	//FIXME: CURRENTLY NOT SUPPORTED
/*
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
*/
}

int TMP_DRAW_CALLS = 0;

void layersystem_render(STATE_ID state_id, view *cam)
{
	layer_system *ls = state_get_layersystem(state_id);
	cpVect p = cam->p; //TODO use for parallax effect (zoom and offset)
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
			//layersystem_register_sprite(0, s);
		}
		llist_end_loop(lay->ll_spr);

		LList *layer_blends = alist_get(render_tree, layer_index);
		/* iterate blend modes */
		llist_begin_loop(layer_blends);

		while(llist_hasnext(layer_blends)) {
			blend_tree *blends = llist_next(layer_blends);
			glBlendFunc(blends->type.src_factor, blends->type.dst_factor);

			/* iterate textures */
			int tex_index;
			int tex_count = alist_size(blends->al_tex);
			for (tex_index = 0; tex_index < tex_count; tex_index++) {
				render_batch *batch = alist_get(blends->al_tex, tex_index);
				if (batch && batch->count) {
					vertex_elem *elems = (vertex_elem*) array_get(batch->elems, 0);

					//GLES2
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof *elems, &elems[0].x);
					glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof *elems, &elems[0].tx);
					glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof *elems, &(elems[0].col.r));

					glBindTexture(GL_TEXTURE_2D, alist_get(textures, tex_index));
					glDrawArrays(GL_TRIANGLE_STRIP, 0, batch->count);
					++TMP_DRAW_CALLS;
					batch->count = 0;
				}
			}
		}
		llist_end_loop(layer_blends);
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


//TODO call this after an actual state change right before appending an element?
//TODO detect change inside current_batch() method? by storing last values inside layersystem
static render_batch *current_batch(int layer)
{
	STATE_ID state_id = statesystem_get_render_state();

	layer_system *ls =state_get_layersystem(state_id);
	if(check_bounds(ls, layer)){
		return NULL;
	}

	/* current layer */
	LList ll_blend = alist_get(render_tree, layer);
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
		SDL_Log("Did not find current blend in tree, creating new blend...");
	}

	GLint tex_id = texture_get_current();
	arraylist *al = blend_texs->al_tex;
	render_batch *batch = alist_get(al, tex_id);

	if (batch == NULL) {
		batch = calloc(1, sizeof *batch);
		batch->elems = array_new(sizeof(vertex_elem));
        if (batch == NULL) {
            SDL_Log("ERROR: NOT ENOUGH MEMORY!");
        } else {
        	SDL_Log("New batch renderer created (state_id=%p, blend=%x, layer=%d, tex_id=%d): p=%p", state_id, current_blend.src_factor, layer, tex_id, batch);
        	alist_set_safe(al, tex_id, batch);
        }
	}

	return batch;
}

int ELEMENT_APPEND_COUNT = 0;
int ELEMENT_APPEND_ACTUAL_COUNT = 0;

static inline void render_append_elem(render_batch *batch, vertex_elem *elem)
{
	++ELEMENT_APPEND_COUNT;
    if (batch) {
        int index = batch->count;
        if (index >= MAX_ELEM_COUNT_BATCH) {
            static int notified = 0;
            if (!notified)
                SDL_Log("WARNING: render batch index out of bounds!");
            notified = 1;
            return;
        }
        ++ELEMENT_APPEND_ACTUAL_COUNT;
        array_set_safe(batch->elems, index, elem);
        batch->count = index + 1;
    }
}

static inline void render_append_vertex(render_batch *batch, float x, float y, float tx, float ty)
{
	vertex_elem elem = {x, y, tx, ty, draw_get_current_color()};
	matrix2d_multp(&elem.x); // assumes that x and y is of type float and are consecutive
	render_append_elem(batch, &elem);
}

static inline void render_append_vertex2fv(render_batch *batch, float **ver_point, float **tex_point)
{
	float x  = *(*ver_point)++, y  = *(*ver_point)++;
	float tx = *(*tex_point)++, ty = *(*tex_point)++;
	Color col = draw_get_current_color();
	vertex_elem elem = {x, y, tx, ty, col};
	matrix2d_multp(&elem.x);
	render_append_elem(batch, &elem);
}

static inline void render_append_repeat(render_batch *batch)
{
    if (batch) {
        int index = batch->count;
        if (index == 0) {
            return; //no need to repeat
        }
        render_append_elem(batch, (vertex_elem*) array_get(batch->elems, index - 1));
    }
}


void draw_quad_new(int layer, float ver_quad[8], const float tex_quad[8]) /* appends an independent quad to the current render state */
{
	float **tex = (float **) &tex_quad;
	render_batch *batch = current_batch(layer); //TODO remove out of this call?
	render_append_repeat(batch); // repeat previous point if any
	render_append_vertex2fv(batch, &ver_quad, tex); // point A
	render_append_repeat(batch); // repeat point A
	render_append_vertex2fv(batch, &ver_quad, tex); // point B
	render_append_vertex2fv(batch, &ver_quad, tex); // point C
	render_append_vertex2fv(batch, &ver_quad, tex); // point D
}

void draw_quad_continue(int layer, float ver_edge[4], const float tex_edge[4]) /* extends a previous quad by adding 2 vertices specified by edge */
{
	float **tex = (float **) &tex_edge;
	render_batch *batch = current_batch(layer);
	render_append_vertex2fv(batch, &ver_edge, tex); // point A
	render_append_vertex2fv(batch, &ver_edge, tex); // point B
}

void draw_triangle_strip(int layer, float *ver_list, const float *tex_list, int count) // appends a triangle strip to current render state
{
	if (count <= 0) return;
	float **tex = (float **) &tex_list;
	render_batch *batch = current_batch(layer); //TODO move out of this method?

	render_append_repeat(batch); // repeat previous point if any
	render_append_vertex(batch, ver_list[0],ver_list[1],tex_list[0],tex_list[1]); // repeat previous point if any
	do render_append_vertex2fv(batch, &ver_list, tex); while (--count);
}

void draw_triangle_fan(int layer, float *ver_fan, const float *tex_fan, int count) /* appends a triangle fan to current render state */
{
	if (count <= 0) return;
	float **tex = (float **) &tex_fan;
	render_batch *batch = current_batch(layer);
    if (!batch) return;
	vertex_elem *fan_origin;

	render_append_repeat(batch); // repeat previous point
	render_append_vertex2fv(batch, &ver_fan, tex); // fan origin
	fan_origin = array_get(batch->elems,batch->count-1);
	render_append_elem(batch, fan_origin); // repeat p0

	for (;;) {
		if (--count > 0) render_append_vertex2fv(batch, &ver_fan, tex); else break;
		if (--count > 0) render_append_vertex2fv(batch, &ver_fan, tex); else break;
		render_append_repeat(batch); // repeat previous point
		render_append_elem(batch, fan_origin); // repeat p0
	}
}


void layersystem_free(layer_system *ls)
{
	int i;
	for(i = 0; i < ls->num_layers; i++) {
		layer_ins *lay = alist_get(ls->layers, i);
		llist_destroy(lay->ll_spr);
		free(lay);
	}
	free(ls);
}

void layersystem_destroy()
{
	int i;
	int num_layers = alist_size(render_tree);
	for(i = 0; i < num_layers; i++) {
		LList *layer_blends = alist_get(render_tree, i);
		/* iterate blend modes */
		llist_begin_loop(layer_blends);
		while(llist_hasnext(layer_blends)) {
			blend_tree *blends = llist_next(layer_blends);
			/* iterate textures */
			int tex_index;
			int tex_count = alist_size(blends->al_tex);
			for (tex_index = 0; tex_index < tex_count; tex_index++) {
				render_batch *batch = alist_get(blends->al_tex, tex_index);
				if (batch) {
					array_destroy(batch->elems);
					free(batch);
				}
			}
			alist_destroy(blends->al_tex);
			free(blends);

		}
		llist_end_loop(layer_blends);
		llist_destroy(layer_blends);
	}
	alist_destroy(render_tree);
}
