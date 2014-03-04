
#include "../game.h"
#include "../obj/object_types.h"
#include "../upgrades.h"
#include "we_defstate.h"
#include "space.h"
#include "msgbox.h"
#include <ctype.h>

#define MAX_MSG_LENGTH 64
#define MIN_MSG_LENGTH 1

STATE_ID state_msgbox;

static char title[MAX_MSG_LENGTH];
static char msg[MAX_MSG_LENGTH];
static msg_call_t msg_call;
static button btn_ok, btn_cancel, btn_c_full;

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void msg_done(void *done)
{
	statesystem_pop_state(NULL);
	msg_call(done);
}

static void on_enter(STATE_ID state_prev)
{

}

static void pre_update(void)
{
}

static void post_update(void)
{
}
static int sdl_event(SDL_Event *event)
{
	return 0;
}
static void draw(void)
{
	draw_load_identity();
	draw_color_rgbmulta4b(0,0,0,150);
	draw_box(RLAY_BACK_BACK, cpv(0, 0),cpv(GAME_WIDTH, GAME_HEIGHT),0,1);
	draw_color(COL_WHITE);
	draw_quad_patch_center(RLAY_BACK_BACK,SPRITE_COIN,cpvzero,cpv(800,500),100,0);
	bmfont_center(FONT_COURIER, cpv(0,150), 200, title);
	bmfont_center(FONT_COURIER, cpv(0,90), 60, msg);
}

static void on_pause(void)
{
}

static void on_leave(STATE_ID state_next)
{
}

static void destroy(void)
{

}

void msgbox_show(const char * ftitle, const char * message, msg_call_t callback)
{
	if(!message){return;}
	msg_call = callback;
	strncpy(title, ftitle, MAX_MSG_LENGTH);
	strncpy(msg, message, MAX_MSG_LENGTH);
	statesystem_push_state(state_msgbox);
}

void msgbox_init(void)
{
	statesystem_register(state_msgbox, 0);
	btn_ok = button_create(SPRITE_SAW, 1, "OK", -200, -150, 200, 200);
	btn_cancel = button_create(SPRITE_SAW, 1, "CANCEL", 200, -150, 300, 200);
	btn_c_full = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);
	button_set_click_callback(btn_ok, msg_done,MSGBOX_OK);
	button_set_click_callback(btn_cancel, msg_done,MSGBOX_CANCEL);
	button_set_click_callback(btn_c_full, msg_done,MSGBOX_CANCEL);
	button_set_hotkeys(btn_ok, KEY_RETURN_1, 0);
	button_set_hotkeys(btn_cancel, KEY_ESCAPE, 0);
	button_set_enlargement(btn_ok,1.5f);
	button_set_enlargement(btn_cancel,1.5f);
	state_register_touchable(state_msgbox, btn_ok);
	state_register_touchable(state_msgbox, btn_cancel);
	state_register_touchable(state_msgbox, btn_c_full);
}

