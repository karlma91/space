#ifndef CAMERA_H_
#define CAMERA_H_

#include "chipmunk.h"

typedef struct {
	cpVect port_pos;
	float port_width;
	float port_height;

    int mode;
    float left;
    float right;
    float width;
    float height;
    cpVect p;
    float zoom;
    float rotation;

    int enabled;
} view;

view * view_new();

void view_port(view *cam, float port_x, float port_y, float port_w, float port_h);
void view_update(view *cam, cpVect pos, float rot);
void view_update_zoom(view *cam, cpVect pos, float level_height);
void view_free(view *c);

#endif
