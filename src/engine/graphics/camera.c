
#include "camera.h"
#include "../engine.h"


camera * camera_new()
{
	camera * c = calloc(1, sizeof *c);
	c->port_height = WINDOW_HEIGHT;
	c->port_width = WINDOW_WIDTH;
	c->port_pos = cpvzero;
	c->p = cpvzero;
	c->rotation = 0;
	c->zoom = 1;
	return c;
}

void camera_set_port(camera *cam, float port_x, float port_y, float port_w, float port_h)
{
	// TODO: this
}


void camera_update(camera *cam, cpVect pos, cpVect rot)
{
    /* dynamic camera pos */
    static const float pos_delay = 0.99f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
    static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
    static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
    static float cam_dx;
    cam_dx = cam_dx * pos_delay + ((rot.x * pos_rel_x - pos_rel_offset_x) * GAME_WIDTH) * (1 - pos_delay) / cam->zoom;

    //cam->x = pos.x + cam_dx;
    cam->p = pos;

    /* camera constraints */
    cam->width = GAME_WIDTH / (2 * cam->zoom);

    cam->left = cam->p.x - cam->width;
    cam->right = cam->p.x + cam->width;
}

void camera_translate(camera *cam)
{
	draw_load_identity();
	draw_rotate(cam->rotation);
	draw_scale(cam->zoom, cam->zoom);
	draw_translatev(cpvneg(cam->p));
}

void camera_update_zoom(camera *cam, cpVect pos, float level_height)
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

void camera_free(camera *c)
{
	free(c);
}

