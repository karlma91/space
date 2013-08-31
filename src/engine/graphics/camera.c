
#include "camera.h"
#include "../engine.h"

void camera_update(camera *cam, cpVect pos, cpVect rot)
{
    /* dynamic camera pos */
    static const float pos_delay = 0.99f;  // 1.0 = centered, 0.0 = no delay, <0 = oscillerende, >1 = undefined, default = 0.9
    static const float pos_rel_x = 0.2f; // 0.0 = centered, 0.5 = screen edge, -0.5 = opposite screen edge, default = 0.2
    static const float pos_rel_offset_x = 0; // >0 = offset up, <0 offset down, default = 0
    static float cam_dx;
    cam_dx = cam_dx * pos_delay + ((rot.x * pos_rel_x - pos_rel_offset_x) * GAME_WIDTH) * (1 - pos_delay) / cam->zoom;

    cam->x = pos.x + cam_dx;

    /* camera constraints */
    cam->width = GAME_WIDTH / (2 * cam->zoom);

    cam->left = cam->x - cam->width;
    cam->right = cam->x + cam->width;
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
                cam->y = GAME_HEIGHT / (2*cam->zoom);
            } else if (py < 0.4) {
                cam->zoom = (1 + cos(5*M_PI * (py + 1))) / zoomlvl + scrlvl;
                cam->y = GAME_HEIGHT / (2*cam->zoom);
            } else if (py < 0.6) {
                cam->zoom = scrlvl;
                cam->y = level_height / (2);
            } else if (py < 0.8) {
                cam->zoom = (1 - cos(5*M_PI * (py - 0.4 + 1))) / zoomlvl + scrlvl;
                cam->y = level_height - GAME_HEIGHT / (2*(cam->zoom));
            } else if (py <= 1.0) {
                cam->zoom = 2 / zoomlvl + scrlvl;
                cam->y = level_height - GAME_HEIGHT / (2*cam->zoom);
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
            cam->y = pos.y;
            if(cam->y > level_height - GAME_HEIGHT/(2*cam->zoom)){
                cam->y = level_height - GAME_HEIGHT/(2*cam->zoom);
            }else if(cam->y <  GAME_HEIGHT/(2*cam->zoom)){
                cam->y = GAME_HEIGHT/(2*cam->zoom);
            }
            break;
        case 5:
            cam->zoom = 1.0f*GAME_HEIGHT/level_height;
            cam->y = 1.0f*level_height/2;
            break;
        case 6:
            scrlvl = 1.0f * GAME_HEIGHT/level_height;
            /* parameters to change */
            zoomlvl = 4; /* amount of zoom less is more zoom */
            float startlvl = 0.8;
            float endlvl = 0.2;

            float freq = startlvl-endlvl;
            if (py < 0) {
                /* undefined zoom! Reset/fix player position? */
            } else if ( py < endlvl) {
                cam->zoom = 2 / zoomlvl + scrlvl;
                cam->y = GAME_HEIGHT / (2*cam->zoom);
            } else if (py < startlvl) {
                cam->zoom = (1 - cos( (1/freq)*M_PI*(py + (freq-endlvl) ))) / zoomlvl + scrlvl;
                cam->y = GAME_HEIGHT / (2*cam->zoom);
            } else if (py < 1) {
                cam->zoom = scrlvl;
                cam->y = level_height / (2);
            }else{
                /* undefined zoom! Reset/fix player position? */
            }
            break;
        default:
            cam->zoom = 1.0f*GAME_HEIGHT/level_height;
            cam->y = 1.0f*level_height/2;
            break;
        }
}

