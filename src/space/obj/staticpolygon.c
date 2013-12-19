#include "object_types.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define OBJ_NAME staticpolygon
#include "we_defobj.h"


static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	staticpolygon->outline = staticpolygon->param.outline;
	staticpolygon->texture_scale = staticpolygon->param.texture_scale;
	staticpolygon->scale = staticpolygon->param.scale;
	staticpolygon->tex_id = texture_load(staticpolygon->param.tex_name);
	staticpolygon->shape_id = staticpolygon->param.shape_id;

	staticpolygon->body = cpBodyNewStatic();
	cpBodySetPos(staticpolygon->body, staticpolygon->data.p_start);

	shape_add_shapes(current_space, staticpolygon->shape_id, staticpolygon->body, staticpolygon->scale, cpvzero, 1, 0.7, staticpolygon, ID_GROUND, CP_ALL_LAYERS, 0);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float size = staticpolygon->scale;
	polyshape ps = staticpolygon->shape_id;
	LList p = ps->pshape;
	llist_begin_loop(p);
	while (llist_hasnext(p)) {
		polygon_ins *pi = (polygon_ins*) llist_next(p);

		llist_begin_loop(pi->shape);
		while (llist_hasnext(pi->shape)) {
			vertex_array *data = (vertex_array*) llist_next(pi->shape);
			int tex_id = staticpolygon->tex_id;
			draw_polygon_textured(3, data->num, data->vertices, staticpolygon->body->p, staticpolygon->body->a, size, staticpolygon->texture_scale, tex_id);
		}
		llist_end_loop(pi->shape);

		draw_color(COL_WHITE);
		if(staticpolygon->outline){
			llist_begin_loop(pi->outlines);
			while (llist_hasnext(pi->outlines)) {
				vertex_array *data = (vertex_array*) llist_next(pi->outlines);
				draw_polygon_outline(data->num,data->vertices, staticpolygon->body->p, staticpolygon->body->a, staticpolygon->scale);
			}
			llist_end_loop(pi->outlines);
		}
	}
	llist_end_loop(p);
}


static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove_static(current_space, &staticpolygon->body);
}
