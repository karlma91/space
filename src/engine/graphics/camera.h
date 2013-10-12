#ifndef CAMERA_H_
#define CAMERA_H_

#include "chipmunk.h"

typedef struct view view;
typedef void (*view_render) (struct view *);

struct view {
	cpVect port_pos;
	cpVect port_size;
	int port_orientation;
	float priv_port_angle;
	float ratio;

    int mode;
    float left;
    float right;
    float width;
    float height;
    cpVect p;
    float zoom;
    float rotation;

    view_render GUI;

    int enabled;
};


view * view_new();

void view_set_port(view *cam, cpVect port_pos, cpVect port_size, int orientation);
void view_update(view *cam, cpVect pos, float rot);
void view_update_zoom(view *cam, cpVect pos, float level_height);
void view_free(view *c);

#endif
