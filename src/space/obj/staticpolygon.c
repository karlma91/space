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
	staticpolygon->param.shape_id = POLYSHAPE_RAMP;
	staticpolygon->param.sprite_id = SPRITE_BUTTON;
	sprite_create(&staticpolygon->data.spr, staticpolygon->param.sprite_id, 400, 400, 30);
	shape_add_shapes(current_space, staticpolygon->param.shape_id, current_space->staticBody, 2000, cpv(700,100), 1, 0.7, staticpolygon, ID_GROUND, CP_ALL_LAYERS, 1);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float size = 2000;
	int texture = TEX_STARS; //sprite_get_texture(&staticpolygon->data.spr);
	polyshape p = POLYSHAPE_RAMP;
	llist_begin_loop(p);
	while (llist_hasnext(p)) {
		LList rb = (LList) llist_next(p);
			llist_begin_loop(rb);
			while (llist_hasnext(rb)) {
				shape_instance *data = (shape_instance*) llist_next(rb);
				cpVect d[data->num];
				float test[data->num * 2];
				float testt[data->num * 2];
				int i, j = 0;
				for (i = 0; i < data->num; i++) {
					d[i] = data->shape[i];
					d[i] = cpvadd(cpvmult(d[i], size), cpv(700,100));
					test[j] = d[i].x;
					testt[j] = data->shape[i].x * 1;
					j++;
					test[j] = d[i].y;
					testt[j] = data->shape[i].y * 1;
					j++;
				}

				texture_bind(texture);
				draw_push_matrix();
				//draw_load_identity();
				draw_color4f(1,1,1,1);
				draw_vertex_pointer(2, sizeof(cpFloat), 0, test);
				draw_tex_pointer(2, GL_FLOAT, 0, testt);
				glEnable(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				draw_draw_arrays(GL_TRIANGLE_FAN, 0, data->num);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glDisable(GL_TEXTURE_2D);
				draw_pop_matrix();
			}
			llist_end_loop(rb);
		}
	llist_end_loop(p);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
}
