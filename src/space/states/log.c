/*
 * log.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */
#include "../game.h"
#include "we_defstate.h"

STATE_ID state_log;

#define LINE_WIDTH 200
#define LINE_COUNT 1000
static char debug_log[LINE_COUNT][LINE_WIDTH];
static int x=0, y=0;

static scroll_p scr_view;
static button btn_space;
/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
	cpVect pos = cpv(10-GAME_WIDTH/2, -GAME_HEIGHT/2 + 30 + y * 30);
	scroll_set_offset(scr_view, pos);
    scroll_set_zoom(scr_view, 1);
}

static void pre_update(void)
{
}

static void post_update(void)
{
}

static void draw(void)
{
	int i;
	draw_color4f(1,1,1,1);
    cpVect pos = scroll_get_offset(scr_view);


    float zoom = scroll_get_zoom(scr_view);
    draw_scale(zoom,zoom);
	for (i=0; i<LINE_COUNT; i++) {
		pos.y -= 30; //TODO add scroll offset
		bmfont_left(FONT_COURIER, pos, 0.5, "%s", debug_log[i]);
	}
}

static int sdl_event(SDL_Event *event)
{
	return 0;
}

static void on_pause(void)
{
}

static void on_leave(void)
{
}

static void destroy(void)
{
}

void log_init(void)
{
	statesystem_register(state_log, 0);

	btn_space = button_create(SPRITE_JOYSTICK, 0, "", GAME_WIDTH/2 - 100, GAME_HEIGHT/2 - 100, 130, 130);
	button_set_callback(btn_space, statesystem_pop_state, NULL);
	button_set_enlargement(btn_space, 1.5);
	button_set_hotkeys(btn_space, KEY_ESCAPE, 0);
	state_register_touchable(this, btn_space);

	scr_view = scroll_create(0,0,GAME_WIDTH, GAME_HEIGHT, 0.9, 800, 0, 1, 0);
	state_register_touchable(this, scr_view);
}

void logprintln(char *format, ...)
{
	char buf[LINE_WIDTH];
	va_list arglist;

	va_start( arglist, format );
	vsnprintf(buf, LINE_WIDTH, format, arglist);
	va_end( arglist );

	int i = 0;
	while(buf[i]) {
		char c = buf[i++];
		if (c == '\n') {
            debug_log[y][x] = '\0';
			y++;
			y = y < LINE_COUNT ? y : 0;
			x = 0;
		} else {
			debug_log[y][x++] = c;
			x = x < LINE_WIDTH ? x : 0;
		}
	}
    debug_log[y][x] = '\0';
    y++;
    y = y < LINE_COUNT ? y : 0;
    x = 0;
}

void logprint(char *format, ...)
{
	char buf[LINE_WIDTH];
	va_list arglist;

	va_start( arglist, format );
	vsnprintf(buf, LINE_WIDTH, format, arglist);
	va_end( arglist );

	int i = 0;
	while(buf[i]) {
		char c = buf[i++];
		if (c == '\n') {
			y++;
			y = y < LINE_COUNT ? y : 0;
			x = 0;
		} else {
			debug_log[y][x++] = c;
			x = x < LINE_WIDTH ? x : 0;
		}
	}
}

