#ifndef CAMERA_H_
#define CAMERA_H_

#include "chipmunk.h"

typedef struct {

	float port_x;
	float port_y;
	float port_width;
	float port_height;

    int mode;
    float left;
    float right;
    float width;
    float height;
    float x;
    float y;
    float zoom;
    float rotation;

} camera;


void camera_init(camera *cam, float port_x, float port_y, float port_w, float port_h);
void camera_update(camera *cam, cpVect pos, cpVect rot);
void camera_update_zoom(camera *cam, cpVect pos, float level_height);
void camera_move(camera * cam, float x, float y);
void camera_movex(camera * cam, float x);
void camera_movetox(camera * cam, float x);
void camera_translate(camera *cam);
void camera_rotate(camera *cam, float a);

#endif
