
#include "../game.h"
#include "../obj/object_types.h"
#include "../upgrades.h"
#include "we_defstate.h"
#include "space.h"
#include "textinput.h"
static char *input_buffer;

STATE_ID from_state;

STATE_ID state_textinput;


/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void resume()
{
	SDL_StopTextInput();
	statesystem_set_state(from_state);
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

	draw_color_rgbmulta4b(100,100,100,156);
	draw_box(RLAY_BACK_BACK, cpv(-300, 0),cpv(400, 60),0,0);

	int keyboard_shown = SDL_IsScreenKeyboardShown(window);
	draw_color(COL_WHITE);

	bmfont_left(FONT_COURIER, cpv(-300,0), 1,"text:%s",input_buffer);

}


static int sdl_event(SDL_Event *event)
{
	switch(event->type) {
	case SDL_KEYDOWN:
		if(event->key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
			int len = strlen(input_buffer);
			if(len>0){
				input_buffer[len - 1] = 0;
			}
		}else if(event->key.keysym.scancode == SDL_SCANCODE_RETURN) {
			resume();
		}
		break;
	case SDL_TEXTINPUT:
        /* Add new text onto the end of our text */
        strcat(input_buffer, event->text.text);

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
	return 1;
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


void textinput_start(STATE_ID from)
{
	from_state = from;
	SDL_StartTextInput();
	memset(input_buffer,0,1000);
	SDL_Rect r = {0, 0-200, 400, 400};
	SDL_SetTextInputRect(&r);
}

char * textinput_get()
{
	return input_buffer;
}

void textinput_init(void)
{
	input_buffer = calloc(sizeof (char), 1000);
	statesystem_register(state_textinput, 0);
	textinput_start(this);
}

