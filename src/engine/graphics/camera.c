
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
	return c;
}

void view_set_port(view *cam, cpVect port_pos, cpVect port_size, int orientation)
{
	cam->port_pos = port_pos;
	cam->port_size = port_size;
	cam->port_orientation = orientation;
	cam->priv_port_angle = (cam->port_orientation & 0x3) * M_PI_2;

	float pw = cam->port_size.x;
	float ph = cam->port_size.y;
	float gw = GAME_WIDTH;
	float gh = GAME_HEIGHT;


	cam->ratio = (gw * ph ) / (pw * gh);
}

void view_update(view *cam, cpVect pos, float rot)
{
    /* dynamic camera pos */
    cam->p = pos;
    cam->rotation = rot + cam->priv_port_angle;

    /* camera constraints */
    cam->width = cam->port_size.x / (2 * cam->zoom);
#warning outdated constraints!
    cam->left = cam->p.x - cam->width;
    cam->right = cam->p.x + cam->width;
}

void view_clip(view *cam)
{
	//TODO create viewport stack in draw?
	glViewport(cam->port_pos.x, cam->port_pos.y, cam->port_size.x, cam->port_size.y);
	glScissor(cam->port_pos.x, cam->port_pos.y, cam->port_size.x, cam->port_size.y);
}

void view_transform2view(view *cam)
{
	current_view = cam;

	draw_load_identity();
	draw_scale(cam->zoom * cam->ratio, cam->zoom);
	draw_rotate(cam->rotation);
	draw_translatev(cpvneg(cam->p));
}

void view_transform2port(view *cam)
{
	current_view = cam;

	draw_load_identity();
	draw_rotate(cam->priv_port_angle); // supported angles 0, 90, 180, 270
	//draw_scale(cam->zoom, cam->zoom);
	draw_translatev(cpvneg(cam->port_pos));
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

            scrlvl = 1.0f * GAME_HEIGHT/level_height;

            zoomlvl = cam->mode == 1 ? 4 : 12;
            if (py < 0) {
                /* undefined zoom! Reset/fix player position? */
            } else if ( py < 0.2) {
                cam->zoom = 2 / zoomlvl + scrlvl;
                cam->p.y = GAME_HEIGHT / (2*cam->zoom);
            } else if (py < 0.4) {
                cam->zoom = (1 + cos(5*M_PI * (py + 1))) / zoomlvl + scrlvl;
                cam->p.y = GAME_HEIGHT / (2*cam->zoom);
            } else if (py < 0.6) {
                cam->zoom = scrlvl;
                cam->p.y = level_height / (2);
            } else if (py < 0.8) {
                cam->zoom = (1 - cos(5*M_PI * (py - 0.4 + 1))) / zoomlvl + scrlvl;
                cam->p.y= level_height - GAME_HEIGHT / (2*(cam->zoom));
            } else if (py <= 1.0) {
                cam->zoom = 2 / zoomlvl + scrlvl;
                cam->p.y = level_height - GAME_HEIGHT / (2*cam->zoom);
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
            if(cam->p.y > level_height - GAME_HEIGHT/(2*cam->zoom)){
            	cam->p.y = level_height - GAME_HEIGHT/(2*cam->zoom);
            }else if(cam->p.y <  GAME_HEIGHT/(2*cam->zoom)){
            	cam->p.y = GAME_HEIGHT/(2*cam->zoom);
            }
            break;
        case 5:
            cam->zoom = 1.0f*GAME_HEIGHT/level_height;
            cam->p = cpvadd(cam->p, pos);
            break;
        case 6:
        	cam->zoom = 0.3f*GAME_HEIGHT/(level_height);
        	cam->p = cpvzero;
        	break;
        default:
            cam->zoom = 1.0f*GAME_HEIGHT/level_height;
            cam->p.y = 1.0f*level_height/2;
            break;
        }
}

void view_free(view *c)
{
	free(c);
}

