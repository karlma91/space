
#include "camera.h"
#include "../engine.h"
#include "../include/we_state.h"


view * view_new()
{
	view * c = calloc(1, sizeof *c);
	view_set_port(c, cpvzero, cpv(WINDOW_WIDTH, WINDOW_HEIGHT), 0);
	c->rotation = 0;
	c->enabled = 1;
	c->zoom = 1;
	c->p = cpvzero;

    c->touch_objects = llist_create();

	return c;
}

void view_set_port(view *cam, cpVect port_pos, cpVect port_size, int orientation)
{
	cam->port_pos = port_pos;
	cam->port_width = port_size.x;
	cam->port_height = port_size.y;
	view_set_orientation(cam, orientation);
}

void view_set_orientation(view *cam, int orientation)
{
	int landscape = !(orientation & 1);
	cam->port_orientation = orientation & 0x3;
	cam->priv_port_angle = cam->port_orientation * M_PI_2;

	float pw = cam->port_width;
	float ph = cam->port_height;
	float gw = GAME_WIDTH;
	float gh = GAME_HEIGHT;

	cam->ratio = (gw * ph ) / (pw * gh);

	float w = gw * cam->port_width / WINDOW_WIDTH;
	float h = gh * cam->port_height / WINDOW_HEIGHT;

	cam->view_width = landscape ? w : h;
	cam->view_height = landscape ? h : w;

	cam->priv_port_box = cpBBNew(cam->port_pos.x, cam->port_pos.y, cam->port_pos.x+cam->port_width, cam->port_pos.y+cam->port_height);
}

void view_update(view *cam, cpVect pos, float rot)
{
    /* dynamic camera pos */
    cam->p = pos;
    cam->rotation = rot + cam->priv_port_angle;

    /* camera constraints */
    //cam->width = cam->port_width / (2 * cam->zoom);
}

void view_clip(view *cam)
{
	//TODO create viewport stack in draw?
	glViewport(cam->port_pos.x, cam->port_pos.y, cam->port_width, cam->port_height);
	glScissor(cam->port_pos.x, cam->port_pos.y, cam->port_width, cam->port_height);
}

void view_transform2view(view *cam)
{
	current_view = cam;

	// get inverse transformation matrix
	draw_load_identity();
	draw_translatev(cam->p);
	draw_rotate(-cam->rotation);
	draw_scale(1/(cam->zoom * cam->ratio), 1/(cam->zoom));
	cam->priv_view_invtransform = matrix2d_get();

	draw_load_identity();
	draw_scale(cam->zoom * cam->ratio, cam->zoom);
	draw_rotate(cam->rotation);
	draw_translatev(cpvneg(cam->p));
	cam->priv_view_transform = matrix2d_get();
}

void view_transform2port(view *cam)
{
	current_view = cam;

	// get inverse transformation matrix
	draw_load_identity();
	draw_rotate(-cam->priv_port_angle);
	draw_scale(1/cam->ratio, 1);
	cam->priv_port_invtransform = matrix2d_get();

	// transform view to viewport
	draw_load_identity();
	draw_scale(cam->ratio, 1);
	draw_rotate(cam->priv_port_angle); // supported angles 0, 90, 180, 270
}

//TODO move out from camera/view
void view_update_zoom(view *cam, cpVect pos, float level_height)
{
    /* dynamic camera zoom */
        float py = pos.y / level_height;
        float scrlvl, zoomlvl;
        switch (cam->mode) {
        case 1:
        case 2: /* fanzy zoom camera */

            scrlvl = 1.0f * cam->view_height/level_height;

            zoomlvl = cam->mode == 1 ? 4 : 12;
            if (py < 0) {
                /* undefined zoom! Reset/fix player position? */
            } else if ( py < 0.2) {
                cam->zoom = 2 / zoomlvl + scrlvl;
                cam->p.y = cam->view_height / (2*cam->zoom);
            } else if (py < 0.4) {
                cam->zoom = (1 + cos(5*M_PI * (py + 1))) / zoomlvl + scrlvl;
                cam->p.y = cam->view_height / (2*cam->zoom);
            } else if (py < 0.6) {
                cam->zoom = scrlvl;
                cam->p.y = level_height / (2);
            } else if (py < 0.8) {
                cam->zoom = (1 - cos(5*M_PI * (py - 0.4 + 1))) / zoomlvl + scrlvl;
                cam->p.y= level_height - cam->view_height / (2*(cam->zoom));
            } else if (py <= 1.0) {
                cam->zoom = 2 / zoomlvl + scrlvl;
                cam->p.y = level_height - cam->view_height / (2*cam->zoom);
            } else {
                /* undefined zoom! Reset/fix player position? */
            }
            break;

        case 3:
        case 4:/* simple zoomed camera */
            if(cam->mode == 3){
                cam->zoom = 2;
            }else{
                cam->zoom = 1.3;
            }
            cam->p.y = pos.y;
            if(cam->p.y > level_height - cam->view_height/(2*cam->zoom)){
            	cam->p.y = level_height - cam->view_height/(2*cam->zoom);
            }else if(cam->p.y <  cam->view_height/(2*cam->zoom)){
            	cam->p.y = cam->view_height/(2*cam->zoom);
            }
            break;
        case 5:
            cam->zoom = 1.0f*cam->view_height/level_height;
            cam->p = cpvadd(cam->p, pos);
            break;
        case 6:
        	cam->zoom = 0.3f*cam->view_height/(level_height);
        	cam->p = cpvzero;
        	break;
        default:
            cam->zoom = 1.0f*cam->view_height/level_height;
            cam->p.y = 1.0f*level_height/2;
            break;
        }
}

void view_free(view *c)
{
	llist_destroy(c->touch_objects);
	free(c);
}

cpVect view_touch2view(view *cam, cpVect p)
{
	if (cam)
		p = cpv((p.x-0.5)*cam->view_width, (0.5-p.y)*cam->view_height);
	return p;
}
cpVect view_touch2world(view *cam, cpVect p)
{
	if (cam)
		p = view_view2world(cam, view_touch2view(cam, p));
	return p;
}

cpVect view_view2world(view* cam, cpVect p)
{
	if (cam)
		p = matrix2d_multcpv(cam->priv_view_invtransform, p);
	return p;
}
cpVect view_world2view(view* cam, cpVect p)
{
	if (cam)
		p = matrix2d_multcpv(cam->priv_view_transform, p);
	return p;
}

