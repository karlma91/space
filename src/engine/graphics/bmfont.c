#include <stdio.h>
#include <stdlib.h>

#include "draw.h"

#include "mxml.h"
#include "../engine.h"
#include "../io/xmlh.h"
#include "../data/llist.h"
#include "../io/waffle_utils.h"

#include "../graphics/draw.h"
#include "../graphics/texture.h"

LList font_list;

#define FILE_BUFFER_SIZE 16384
static char buffer[FILE_BUFFER_SIZE];

#define is_utf8_0xCX(chr) ((((chr) & 0xF0) ^ 0xC0) == 0)

static void flush_line(bm_font *font, float text_width, float x, float y);

void bmfont_init(void)
{
    font_list = llist_create();
    llist_set_remove_callback(font_list, free);
}

bm_font * bmfont_read_font(char *filename)
{
#if !ARCADE_MODE
    SDL_Log("Loading font: %s",filename);
#endif

    char fileread[50];
    sprintf(fileread,"bmfont/%s",filename);

    bm_font *f = (bm_font*)calloc(1,sizeof *f);

    int filesize = waffle_read_file(fileread, &buffer[0], FILE_BUFFER_SIZE);

    mxml_node_t * tree = NULL;
    mxml_node_t * node  = NULL;

    if (filesize) {
        tree = mxmlLoadString(NULL, buffer, MXML_OPAQUE_CALLBACK);
    }else {
        SDL_Log("Could Not Open the File Provided");
        exit(1);
    }
    if(tree == NULL){
    	SDL_Log("BMFONT: file %s is empty \n", fileread);
    	exit(1);
    }
    for (node = mxmlFindElement(tree, tree,NULL,NULL, NULL,MXML_DESCEND);
    		node != NULL;
    		node=mxmlWalkNext (node, NULL, MXML_DESCEND)
    ){
    	if(TESTNAME("common")){
    		parse_int(node,"lineHeight",&(f->line_height));
    		parse_int(node,"base",&(f->base));
    		parse_int(node,"scaleW",&(f->tex_w));
    		parse_int(node,"scaleH",&(f->tex_h));
    	}else if(TESTNAME("page")){
    		char *texture[1];
    		parse_string(node,"file", texture);
    		char *suffix = strchr(texture[0], '.');
    		if (suffix) *suffix = '\0'; /* removes suffix */
    		f->spr_id = sprite_link(texture[0]);
    	}else if(TESTNAME("chars")){
    		int count;
    		parse_int(node,"count",&(count));
    	}else if(TESTNAME("char")){
    		int id = 0;
    		parse_int(node,"id",&(id));
    		if(id <= 0xFFFF){
    			bm_char *c = &(f->chars[id]);
    			c->id = id;
    			parse_int(node,"x",&(c->x));
    			parse_int(node,"y",&(c->y));
    			parse_int(node,"width",&(c->w));
    			parse_int(node,"height",&(c->h));
    			parse_int(node,"xoffset",&(c->x_offset));
    			parse_int(node,"yoffset",&(c->y_offset));
    			parse_int(node,"xadvance",&(c->x_advance));
            }

    	}
    }
    mxmlDelete(tree);
    return f;
}


static int get_line_width(bm_font *font, unsigned const char * text) {
	int width = 0;
	while(*text != '\n' && *text != '\0') {
		if(*text == ' ') {
			width += 16;
		} else if (is_utf8_0xCX(*text)) { // utf-8 two byte character
			int chr = *text << 8;
			chr |= *++text;
			bm_char *c = &(font->chars[chr]);
			width += (c->x_advance);
		} else {
			bm_char * c = &(font->chars[*text]); //TODO support æøåÆØÅ!
			width += (c->x_advance);
		}
		text++;
	}
	return width;
}

static void draw_char(bm_font *f, bm_char *c)
{
	//sprite_subimg subimg = sprite_get_subimg(f->spr_id);
	float quad[8] = {0, 0, c->w, 0, 0, c->h, c->w, c->h};

	sprite_subimg subimg = {1.0f*c->x / f->tex_w,          1.0f*c->y / f->tex_h,
		                    1.0f*(c->x + c->w) / f->tex_w, 1.0f*(c->y+ c->h) / f->tex_h};

    GLfloat tex_map[8];
	sprite_get_subimg_mapped(f->spr_id, subimg, tex_map);

    cpVect pos = cpv(c->x_offset,(f->line_height - c->h)-c->y_offset);
    draw_push_matrix();
    draw_translatev(pos);
    draw_quad_new(0, quad, tex_map);
    draw_pop_matrix();
    draw_translate(c->x_advance,0);
}

void bmfont_left(bm_font *font, cpVect pos, float scale, const char *format, ...)
{
    if (!format || format[0] == '\0') return;
    char text[BMFONT_MAXLEN];
    va_list ap; va_start(ap, format); vsnprintf(text, BMFONT_MAXLEN, format, ap); va_end(ap);
    bmfont_render(font, BMFONT_LEFT,pos.x,pos.y,scale,(unsigned char *)text);
}

void bmfont_center(bm_font *font, cpVect pos, float scale, const char *format, ...)
{
    if (!format || format[0] == '\0') return;
    char text[BMFONT_MAXLEN];
    va_list ap; va_start(ap, format); vsnprintf(text, BMFONT_MAXLEN, format, ap); va_end(ap);
    bmfont_render(font, BMFONT_CENTER,pos.x,pos.y,scale,(unsigned char *)text);
}

void bmfont_right(bm_font *font, cpVect pos, float scale, const char *format, ...)
{
    if (!format || format[0] == '\0') return;
    char text[BMFONT_MAXLEN];
    va_list ap; va_start(ap, format); vsnprintf(text, BMFONT_MAXLEN, format, ap); va_end(ap);
    bmfont_render(font, BMFONT_RIGHT,pos.x,pos.y,scale,(unsigned char *)text);
}

void bmfont_render(bm_font *font, int align, float x, float y, float scale, unsigned const char *text)
{
    int line_width = get_line_width(font, text);

    draw_push_matrix();
    draw_translate(x,y);
    draw_scale(scale, scale);
    draw_translate(0,font->base-font->line_height);

    texture_bind_virt(sprite_get_texture(font->spr_id));

	if (align == BMFONT_CENTER) {
		draw_translate(-(line_width/2.0f), 0);
	}else if(align == BMFONT_RIGHT) {
		draw_translate(-(line_width), 0);
	}
    int i = 0;
    while(1){
    	if(text[i] == '\n'){
    		switch(align) {
    		case BMFONT_LEFT:
    			draw_translate(-(line_width), 0);
    			line_width = get_line_width(font, &text[i+1]);
    			break;
    		case BMFONT_CENTER:
    			draw_translate(-(line_width/2), 0);
    			line_width = get_line_width(font, &text[i+1]);
    			draw_translate(-(line_width/2), 0);
    			break;
    		case BMFONT_RIGHT:
    			line_width = get_line_width(font, &text[i+1]);
    			draw_translate(-(line_width), 0);
    			break;
    		}
            draw_translate(0, -font->line_height);
        } else if (text[i] == '\0') {
        	break;
        } else if (is_utf8_0xCX(text[i])) { // utf-8 two byte character
        	int chr = text[i] << 8 | text[i+1];
            bm_char * c = &(font->chars[chr]);
            draw_char(font, c);
            i++;
        } else if (text[i] != ' ') {
            bm_char * c = &(font->chars[(int)text[i]]);
            draw_char(font, c);
        } else {
            draw_translate(16,0);
        }
        i++;
    }
    draw_pop_matrix();
}

