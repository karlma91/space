
#include "chipmunk_private.h"
#include "chipmunk.h"
#include "debugdraw.h"
#include "we_graphics.h"

static void circle_line(cpVect p, float r);

int debug_draw = 0;

static void DrawShape(cpShape *shape, void * unused)
{
    cpBody *body = shape->body;

    draw_color(COL_RED);
    switch(shape->klass->type){
        case CP_CIRCLE_SHAPE: {
            cpCircleShape *circle = (cpCircleShape *)shape;
            circle_line(circle->tc, circle->r);
            break;
        }
        case CP_SEGMENT_SHAPE: {
            cpSegmentShape *seg = (cpSegmentShape *)shape;
            draw_quad_line(0, seg->ta, seg->tb, seg->r*2);
            //draw_flush_simple();
            break;
        }
        case CP_POLY_SHAPE: {
            cpPolyShape *poly = (cpPolyShape *)shape;
            draw_push_matrix();
            draw_translate(body->p.x, body->p.y);
            draw_rotate(body->a);

            int i = 0;
            for(i=0; i< poly->numVerts; i++) {
            	if(i < poly->numVerts - 1) {
            		draw_quad_line(0, poly->verts[i], poly->verts[i + 1], 2);
            	} else {
            		draw_quad_line(0, poly->verts[i], poly->verts[0], 2);
            	}
            }
            //draw_flush_simple();
        	draw_pop_matrix();

            break;
        }
        default: break;
    }
}

void debugdraw_space(cpSpace *space)
{
    if (space && debug_draw) {
        cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)DrawShape, NULL);
    }
}

static void circle_line(cpVect p, float r)
{
	//static GLfloat circle[128];
	//int i=0;
	//for(i = 0; i < 128; i += 2) {
	//	circle[i] = p.x + sinf( 2*M_PI*i / (128-2)) * r;
	//	circle[i+1] = p.y + cosf( 2*M_PI*i / (128-2)) * r;
	//}
	//texture_bind(TEX_WHITE);
	//draw_vertex_pointer(2, GL_FLOAT, 0, circle);
	//draw_draw_arrays(GL_LINE_STRIP, 0, 64);
//#warning GL_LINE_STRIP not supported by renderer/layer_system!
	draw_donut(0, p, r-5,r+5);
}
