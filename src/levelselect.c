/* header */
#include "levelselect.h"

/* standard c-libraries */
#include <stdio.h>

/* Chipmunk physics library */
#include "chipmunk.h"

/* Game state */
#include "main.h"
#include "menu.h"
#include "space.h"
#include "statesystem.h"

/* Drawing */
#include "draw.h"
#include "font.h"
#include "level.h"

#include "waffle_utils.h"

/* static prototypes */
static void update();
static void render();
static void destroy();
static void on_enter();
static void on_leave();

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

void levelselect_init()
{
    statesystem_init_state(STATESYSTEM_LEVELSELECT, 0, on_enter, update, NULL, render, on_leave, destroy);

	level_get_ships(&ships, &decks);
	//SDL_Log( "decks: %d \n", decks);
	//int i;
	//for(i=0; i<decks; i++){
	//	SDL_Log( "x: %f y: %f radius: %f \n", ships[i].x,ships[i].y,ships[i].radius);
	//}
}

static void on_enter()
{

}
static void on_leave()
{

}


static void update()
{

	int i;
	for(i = 0; i < decks; i++){
		ships[i].rotation += 360*ships[i].rotation_speed*dt;
	}

	if (keys[KEY_ESCAPE]){
	    statesystem_set_state(STATESYSTEM_MENU);
		keys[KEY_ESCAPE] = 0;
		}

	if(overview){
		float speed = 400;
		if (keys[KEY_UP_2]){
			//camera_y += speed*dt;
			sel+=1;
			keys[KEY_UP_2] = 0;
		}
		if (keys[KEY_DOWN_2]){
			//camera_y -= speed*dt;
			sel-=1;
			keys[KEY_DOWN_2] = 0;
		}
		if (keys[KEY_LEFT_2]){
			camera_x += speed*dt;
		}
		if (keys[KEY_RIGHT_2]){
			camera_x -= speed*dt;
		}

		sel = (sel < 0) ? decks - 1 : (sel >= decks ? 0 : sel);;

		float temp_z =  (camera_zoom - 0.1)*5;
		camera_zoom -= temp_z*dt;

		if (keys[KEY_RETURN_2] || keys[KEY_RETURN_1]){
			overview = 0;
			zoomed_temp_y = ((1.0f * HEIGHT)/(camera_zoom*2));
			keys[KEY_RETURN_2] = 0, keys[KEY_RETURN_1] = 0;
		}

	}else{
		if (keys[KEY_UP_1] || keys[KEY_UP_2]){
			level_select--;
			keys[KEY_UP_1] = 0, keys[KEY_UP_2] = 0;
		}
		if (keys[KEY_DOWN_1] || keys[KEY_DOWN_2]){
			level_select++;
			keys[KEY_DOWN_1] = 0, keys[KEY_DOWN_2] = 0;
		}

		level_select = (level_select < 0) ? ships[sel].count - 1 : (level_select >= ships[sel].count ? 0 : level_select);;

		if (keys[KEY_RETURN_2] || keys[KEY_RETURN_1]) {
		    statesystem_set_state(STATESYSTEM_SPACE);
			/* load correct level */
			space_init_level(sel+1,level_select+1);
			keys[KEY_RETURN_2] = 0, keys[KEY_RETURN_1] = 0;
		}
		if(keys[SDL_SCANCODE_BACKSPACE]){
			overview = 1;
			level_select = 0;
			keys[SDL_SCANCODE_BACKSPACE]=0;
		}

		camera_zoom = ((1.0f * HEIGHT)/(zoomed_temp_y*2));
		zoomed_cam_y =  (ships[sel].radius + (level_select )*100 + 200 - zoomed_temp_y)*5;
		zoomed_temp_y += zoomed_cam_y*dt;
	}

	camera_speedx = (ships[sel].x - camera_x)*5;
	camera_speedy = (ships[sel].y - camera_y)*5;

	camera_x += camera_speedx*dt;
	camera_y += camera_speedy*dt;

}

static void render()
{
#if GLES1


#else
	//if(overview){
		glPushMatrix();
		glScalef(camera_zoom,camera_zoom,1);
		glTranslatef(-camera_x,-camera_y,0);
		int i;
		for(i = 0; i < decks; i++){
			if(i<decks-1){
				draw_line(ships[i].x,ships[i].y,ships[i+1].x,ships[i+1].y, 512);
			}
			render_ship(&ships[i], sel == i);
		}
		glPopMatrix();
//	}else{

	//}
#endif
}

static void render_ship(struct level_ship *ship, int selected)
{
#if GLES1


#else
		glPushMatrix();

//		float gravity = (4*M_PI*M_PI * ship->radius) * (ship->rotation_speed * ship->rotation_speed);
		float gravity = 300;
		ship->rotation_speed = sqrt((gravity)/(4*M_PI*M_PI*ship->radius));

		char te[30];
		sprintf(te,"GRAV: %.3f ROT: %.3f",gravity, ship->rotation_speed );
		glColor3f(1,1,1);
		setTextSize(100);
		font_drawText(ship->x,ship->y + ship->radius + 200,te);

		glTranslatef(ship->x,ship->y,0);
		glRotatef(ship->rotation,0,0,1);

		glColor3f(0.1,0.1,0.1);
		draw_circle(0,0,(ship->radius) + 120);

		glColor3f(0.8,0.1,0.1);
		draw_box(-5,0,(ship->radius - (ship->count-1)*100),30,0,0);

		int i;
		for(i = 0; i < ship->count; i++){

			if(selected && i == ship->count-level_select - 1){
				glColor3f(0,0,1);
			}else{
				glColor3f(1.0f,0,0);
			}
			draw_donut(0,0,(ship->radius - i*100) + 20,(ship->radius - i*100) + 100);

		}
		glPopMatrix();
#endif
}

static void destroy()
{

}
