#ifndef CAMERA_H_
#define CAMERA_H_

#include "chipmunk.h"
#include "../data/llist.h"
#include "../graphics/matrix2d.h"

typedef struct view view;
typedef void (*view_render) (struct view *);

struct view {
	cpVect port_pos;
	float port_width;
	float port_height;
	cpBB priv_port_box;
    matrix2d priv_port_invtransform;

	int port_orientation;
	float priv_port_angle;
	float ratio;

	float view_width;
	float view_height;

    cpVect p;
    float zoom;
    float rotation;
    matrix2d priv_view_transform;
    matrix2d priv_view_invtransform;

    view_render GUI;
    LList touch_objects; //overlay

    int enabled;
};


view * view_new();

void view_set_port(view *cam, cpVect port_pos, cpVect port_size, int orientation);
void view_set_orientation(view *cam, int orientation);
void view_update(view *cam, cpVect pos, float rot);
void view_free(view *c);

cpVect view_touch2view(view *cam, cpVect p);
cpVect view_touch2world(view *cam, cpVect p);
cpVect view_view2world(view* cam, cpVect p);
cpVect view_world2view(view* cam, cpVect p);

#endif
