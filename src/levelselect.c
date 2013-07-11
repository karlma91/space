/* header */
#include "levelselect.h"

/* standard c-libraries */
#include <stdio.h>

/* Chipmunk physics library */
#include "chipmunk.h"

#include "game.h"
/* Game state */
#include "main.h"
#include "space.h"
#include "state.h"

/* Drawing */
#include "draw.h"
#include "font.h"
#include "level.h"

#include "waffle_utils.h"

STATE_ID state_levelselect;

#define deck_height 800

static void render_ship(struct level_ship *ship, int selected);

#define SHIP_COUNT 4

static int decks;
struct level_ship *ships;

static int overview = 1;
static int sel = 0;
static int level_select = 0;

static float camera_x = 0;
static float camera_y = 0;
static float camera_speedx = 0;
static float camera_speedy = 0;

static float camera_zoom = 0.1; // start zoom

static float zoomed_temp_y = 0;
static float zoomed_cam_y = 0;

static void sdl_event(SDL_Event *event)
{

	SDL_Scancode key;
	switch (event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;

		if (overview) {
			if (key == KEY_UP_2) {
				sel += 1;
				sel = (sel >= decks) ? 0 : sel;

			} else if (key == KEY_DOWN_2) {
				sel -= 1;
				sel = (sel < 0) ? decks - 1 : sel;

			} else if (key == KEY_RETURN_2 || key == KEY_RETURN_1) {
				overview = 0;
				zoomed_temp_y = (1.0f * GAME_HEIGHT) / (camera_zoom * 2);

			} else if (key == KEY_ESCAPE) {
				statesystem_set_state(state_menu);

			}
		} else {
			if (key == KEY_UP_1 || key == KEY_UP_2) {
				level_select--;
				level_select = (level_select < 0) ? ships[sel].count - 1 : level_select;

			} else if (key == KEY_DOWN_1 || key == KEY_DOWN_2) {
				level_select++;
				level_select = (level_select >= ships[sel].count) ? 0 : level_select;

			} else if (key == KEY_RETURN_2 || key == KEY_RETURN_1) {
				space_start_demo();
				space_init_level(sel + 1, level_select + 1); /* load correct level */

			} else if (key == KEY_ESCAPE || key == SDL_SCANCODE_BACKSPACE) {
				overview = 1;
				level_select = 0;

			}
		}
		break;
	}
}

void levelselect_init()
{
	statesystem_register(state_levelselect, 0);
	level_get_ships(&ships, &decks);
}

static void on_enter()
{

}
static void on_leave()
{

}

static void pre_update()
{
	int i;
	for(i = 0; i < decks; i++){
		ships[i].rotation += 360*ships[i].rotation_speed*dt;
	}

	if (overview) {
		camera_zoom -= (camera_zoom - 0.1) * 5 * dt;
	} else {
		camera_zoom = (1.0f * GAME_HEIGHT) / (zoomed_temp_y * 2);
		zoomed_cam_y = (ships[sel].radius + (level_select) * deck_height + 200 - zoomed_temp_y) * 5;
		zoomed_temp_y += zoomed_cam_y * dt;
	}

	camera_speedx = (ships[sel].x - camera_x)*5;
	camera_speedy = (ships[sel].y - camera_y)*5;

	camera_x += camera_speedx*dt;
	camera_y += camera_speedy*dt;

}

static void post_update() {}

static void draw()
{
#if GLES1


#else
	//if(overview){
		draw_push_matrix();
		draw_scale(camera_zoom,camera_zoom,1);
		draw_translate(-camera_x,-camera_y,0);
		int i;
		for(i = 0; i < decks; i++){
			if(i<decks-1){
				//draw_line(ships[i].x,ships[i].y,ships[i+1].x,ships[i+1].y, 512);
			}
			render_ship(&ships[i], sel == i);
		}
		draw_pop_matrix();
//	}else{

	//}
#endif
}

static void render_ship(struct level_ship *ship, int selected)
{
#if GLES1


#else
		draw_push_matrix();

//		float gravity = (4*M_PI*M_PI * ship->radius) * (ship->rotation_speed * ship->rotation_speed);
		float gravity = 300;
		ship->rotation_speed = sqrtf((gravity)/(4*M_PI*M_PI*ship->radius));

		char te[30];
		sprintf(te,"GRAV: %.3f ROT: %.3f",gravity, ship->rotation_speed );
		draw_color4f(1,1,1,1);
		setTextSize(100);
		font_drawText(ship->x,ship->y + ship->radius + 200,te);

		draw_translate(ship->x,ship->y,0);
		draw_rotate(ship->rotation,0,0,1);

		draw_color4f(0.1,0.1,0.1,1);
		draw_circle(0,0,(ship->radius) + 120);

		draw_color4f(0.8,0.1,0.1,1);
		draw_box(-5,0,(ship->radius - (ship->count-1)*deck_height),30,0,0);

		int i;
		for(i = 0; i < ship->count; i++){

			if(selected && i == ship->count-level_select - 1){
				draw_color4f(0,0,1,1);
			}else{
				draw_color4f(1.0f,0,0,1);
			}
			draw_donut(0,0,(ship->radius - i*deck_height) + 100,(ship->radius - i*deck_height) + deck_height);

		}
		draw_pop_matrix();
#endif
}

static void destroy()
{
}
