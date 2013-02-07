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

/* Drawing */
#include "draw.h"
#include "font.h"
#include "level.h"

/* static prototypes */
static void init();
static void update();
static void render();
static void destroy();


/* extern */
state state_levelselect = {
		init,
		update,
		render,
		destroy,
		NULL
};

static void render_ship(struct level_ship *ship);

#define SHIP_COUNT 4

static int decs;
struct level_ship *ships;

static int overview = 1;
static int sel = 0;
static int max_sel = 3;
static float spaceing = 10;

static float camera_x = 0;
static float camera_y = 0;
static float camera_speedx = 0;
static float camera_speedy = 0;

static float camera_zoom = 0;


static void init()
{
	level_get_ships(&ships, &decs);
}

static void update()
{

	int i;
	for(i = 0; i < 4; i++){
		ships[i].rotation += 360*ships[i].rotation_speed*dt;
	}

	if (keys[SDLK_ESCAPE]){
		currentState = &state_menu;
		keys[SDLK_ESCAPE] = 0;
		}

	if(overview){
		camera_zoom = 0.1;
		float speed = 400;
		if (keys[SDLK_UP]){
			//camera_y += speed*dt;
			sel+=1;
			keys[SDLK_UP] = 0;
		}
		if (keys[SDLK_DOWN]){
			//camera_y -= speed*dt;
			sel-=1;
			keys[SDLK_DOWN] = 0;
		}
		if (keys[SDLK_LEFT]){
			camera_x += speed*dt;
		}
		if (keys[SDLK_RIGHT]){
			camera_x -= speed*dt;
		}

		sel = (sel < 0) ? decs - 1 : (sel >= decs ? 0 : sel);;


		camera_speedx = (ships[sel].x - camera_x)*5;
		camera_speedy = (ships[sel].y - camera_y)*5;

		camera_x += camera_speedx*dt;
		camera_y += camera_speedy*dt;


//		camera_x = ships[sel].x;
//		camera_y = ships[sel].y;
	}else{
		if (keys[SDLK_w] || keys[SDLK_UP]){
			sel--;
			sel = (sel <= 0) ? max_sel : sel;
			keys[SDLK_w] = 0, keys[SDLK_UP] = 0;
		}
		if (keys[SDLK_s] || keys[SDLK_DOWN]){
			sel++;
			sel = sel > max_sel ? 1 : sel;
			keys[SDLK_s] = 0, keys[SDLK_DOWN] = 0;
		}
		if(keys[SDLK_ESCAPE]){
			currentState = &state_menu;
			keys[SDLK_ESCAPE]=0;
		}
		if (keys[SDLK_SPACE] || keys[SDLK_RETURN]) {
			currentState = &state_space;
			change_current_menu(INGAME_MENU_ID);
			keys[SDLK_SPACE] = 0, keys[SDLK_RETURN] = 0;
		}
		camera_speedy = -(camera_y - sel*100 - 100*2 - spaceing)/20;
		camera_y += camera_speedy;
		//camera_zoom = ((1.0f * HEIGHT)/(camera_y*2));
	}
}

static void render()
{
	if(overview){
		glPushMatrix();
		glScalef(camera_zoom,camera_zoom,1);
		glTranslatef(-camera_x,-camera_y,0);
		int i;
		for(i = 0; i < 4; i++){
			if(i<4-1){
				draw_line(ships[i].x,ships[i].y,ships[i+1].x,ships[i+1].y, 512);
			}
			render_ship(&ships[i]);
		}
		glPopMatrix();
	}else{
		static char string[9];
		glPushMatrix();
		glScalef(cam_zoom,cam_zoom,1);
		//glTranslatef(0,ypos,0);

		int i = 0;
		glColor3f(0.1,0.1,0.1);
		draw_circle(0,0,100);

		for(i = 1; i < max_sel+1; i++){
			if(sel == i){
				glColor3f(1.0f,0,0);
			}else{
				glColor3f(0,0,1.0f);
			}
			draw_donut(0,0,i*100 + spaceing,i*100 + 100);
			glColor3f(1,1,1);
			sprintf(string,"LEVEL %d",i);
			int textpos = -i*100-100/2;
			if(i == 0) textpos = 0;
			setTextSize(30);
			setTextAlign(TEXT_CENTER);
			font_drawText(0,textpos,string);
		}
		glPopMatrix();
		glLoadIdentity();
		sprintf(string,"LEVEL %d",sel);
		setTextAlign(TEXT_LEFT);
		font_drawText(-WIDTH/2 + 10,HEIGHT/2 - 25,string);
	}
}

static void render_ship(struct level_ship *ship)
{
		glPushMatrix();

		float gravity = (4*M_PI*M_PI * ship->radius) * (ship->rotation_speed * ship->rotation_speed);
		char te[30];
		sprintf(te,"%.1f",gravity );
		glColor3f(1,1,1);
		setTextSize(100);
		font_drawText(ship->x,ship->y + ship->radius + 200,te);

		glTranslatef(ship->x,ship->y,0);
		glRotatef(ship->rotation,0,0,1);

		glColor3f(0.1,0.1,0.1);
		draw_circle(0,0,(ship->radius - (ship->count-1)*100));

		glColor3f(0.8,0.1,0.1);
		draw_simple_box(0,-10,(ship->radius - (ship->count-1)*100),20);

		int i;
		for(i = 1; i < ship->count; i++){

			glColor3f(1.0f,0,0);
			draw_donut(0,0,(ship->radius - i*100) + 20,(ship->radius - i*100) + 100);

		}
		glPopMatrix();
}

static void destroy()
{

}
