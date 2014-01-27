
#include "../game.h"
#include "../obj/object_types.h"
#include "../upgrades.h"
#include "we_defstate.h"
#include "space.h"
#include "textinput.h"
#include <ctype.h>

static char input_buffer[1000];
static char *outer_string;
static char title[1000];
static int buffer_length = 0;
static int max_length = 20;
static int min_length = 0;

static button btn_resume;

STATE_ID from_state;

STATE_ID state_textinput;


/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static int valid_char(char c){
	return isalnum(c);
}

static void resume(void *data)
{
	SDL_StopTextInput();
		outer_string = NULL;
		statesystem_set_state(from_state);
}

static void resume_done(void *data)
{
	if(buffer_length >= min_length){
		strcpy(outer_string, input_buffer);
	}
	resume(0);
}
static void resume_cancel(void *data)
{
	resume(0);
}

static void on_enter(void)
{

}

static void pre_update(void)
{
}

static void post_update(void)
{
}

static void draw(void)
{
	draw_load_identity();


	draw_color_rgbmulta4b(50,50,50,150);
	draw_box(RLAY_BACK_BACK, cpv(0, 0),cpv(GAME_WIDTH, GAME_HEIGHT),0,1);

	//int keyboard_shown = SDL_IsScreenKeyboardShown(window);
	/*if(!keyboard_shown){
		resume(0);
	}*/
	int over_min = buffer_length >= min_length;
	draw_color3f(1, 1*over_min, 1*over_min);

	bmfont_left(FONT_COURIER, cpv(-300,300), 1.5, title);
	bmfont_left(FONT_COURIER, cpv(-300,200), 1, "%s", input_buffer);

}


static int sdl_event(SDL_Event *event)
{
	int add_len = 0;
	switch(event->type) {
	case SDL_KEYDOWN:
		if ((event->key.keysym.scancode == SDL_SCANCODE_BACKSPACE) || (event->key.keysym.scancode == SDL_SCANCODE_DELETE)) {
			if(buffer_length>0){
				buffer_length -= 1;
				input_buffer[buffer_length] = 0;
			}
		}else if(event->key.keysym.scancode == SDL_SCANCODE_RETURN) {
			resume_done(0);
		}
		return 1;
		break;
	case SDL_TEXTINPUT:
        /* Add new text onto the end of our text */
		if(buffer_length < max_length && valid_char(event->text.text[0])) {
			add_len = strlen(event->text.text);
			buffer_length += add_len;
			strcat(input_buffer, event->text.text);
		}
		return 1;
		break;
	case SDL_TEXTEDITING:
        /*
        Update the composition text.
        Update the cursor position.
        Update the selection length (if any).
        */
		SDL_Log("TEXTEDIT->entered: %s\n", event->text.text);
		/*
        char *composition = event->edit.text;
        int cursor = event->edit.start;
        int selection_len = event->edit.length;
		 */

		break;
	}
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

void textinput_start(STATE_ID from, char * string,char *ti,  int min_len, int max_len)
{
	if(!string){return;}
	strcpy(title, ti);
	outer_string = string;
	strcpy(input_buffer, string);
	buffer_length = strlen(string);
	min_length = min_len;
	max_length = max_len;
	from_state = from;
	SDL_StartTextInput();
	SDL_Rect r = {0, 0-200, 400, 400};
	SDL_SetTextInputRect(&r);
	statesystem_push_state(this);
}

char * textinput_get()
{
	return outer_string;
}

void textinput_init(void)
{
	statesystem_register(state_textinput, 0);

	btn_resume = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);
	button_set_click_callback(btn_resume, resume_cancel, 0);
	button_set_hotkeys(btn_resume, KEY_ESCAPE, 0);
	state_register_touchable(this, btn_resume);
}

