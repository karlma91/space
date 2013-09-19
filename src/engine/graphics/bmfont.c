#include <stdio.h>
#include <stdlib.h>

#include "draw.h"

#include "mxml.h"
#include "../engine.h"
#include "../io/xmlh.h"
#include "../data/llist.h"


LList font_list;

#define FILE_BUFFER_SIZE 16384
static char buffer[FILE_BUFFER_SIZE];

static void flush_line(bm_font *font, float text_width, float x, float y);

void bmfont_init(void)
{
    font_list = llist_create();
    llist_set_remove_callback(font_list, free);
}

bm_font * bmfont_read_font(char *filename)
{
    SDL_Log("Loading font: %s",filename);

    char fileread[50];
    sprintf(fileread,"bmfont/%s",filename);

    bm_font *f = (bm_font*)calloc(1,sizeof *f);
    f->scale = 1;

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
                char *(texture[1]);
                parse_string(node,"file",texture);
                f->tex_id = texture_load(*texture);
            }else if(TESTNAME("chars")){
                int count;
                parse_int(node,"count",&(count));
            }else if(TESTNAME("char")){
                int id = 0;
                bm_char *c;
                parse_int(node,"id",&(id));
                c = &(f->chars[id]);
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
    return f;
}


static void draw_char(bm_font *f, bm_char *c)
{

    GLfloat quad[8] = {c->x_offset, c->y_offset,
            c->x_offset+c->w,   c->y_offset,
            c->x_offset,  c->y_offset+c->h,
            c->x_offset+c->w,  c->y_offset+c->h};
    float ty = 1.0f*c->y / f->tex_h;
    float ty2 = 1.0f*(c->y+ c->h) / f->tex_h;


    float tx = 1.0f*c->x / f->tex_w;
    float tx2 = 1.0f*(c->x + c->w) / f->tex_w;
    GLfloat tex_map[8] = {tx,                  ty,
                         tx2,          ty,
                          tx,                 ty2,
                         tx2,         ty2};

    //SDL_Log("ID: %d X: %d, y: %d, w: %d, h: %d, texh: %d, texw: %d",c->id, c->x, c->y, c->w, c->h, f->tex_h, f->tex_w);

    draw_vertex_pointer(2,GL_FLOAT,0,quad);
    draw_tex_pointer(2,GL_FLOAT,0,tex_map);
   // cpVect pos = cpv(c->x_offset,c->y_offset);
    //cpVect pos = cpv(0,0);
    draw_append_quad();
    //draw_current_texture(&pos,tex_map,c->w,c->h,0);
    //draw_current_texture_all(&pos,tex_map,1,1,0,quad);
    draw_translate(c->x_advance - c->x_offset,0);
}

void bmfont_render(bm_font *font, int align, float x, float y, char *format, ...)
{
    va_list     ap;
    char text[100];
    if (!strlen(format)) return;

    va_start(ap, format);
    vsprintf(text, format, ap);
    va_end(ap);
    texture_bind(font->tex_id);
    draw_push_matrix();
    draw_load_identity();
    int i = 0;
    int text_width = 0;;
    while(1){
        if(text[i] == '\n' || text[i] == '\0'){
            if(align == 0){
                flush_line(font, text_width, x, y);
                text_width = 0;
            }else{
                flush_line(font, 0, x, y);
            }
            draw_load_identity();
            if(text[i] == '\0'){
                break;
            }else{
                draw_translate(0, font->line_height);
            }
        }else if(text[i] != ' '){
            bm_char * c = &(font->chars[text[i]]);
            draw_char(font, c);
            text_width+= (c->x_advance - c->x_offset);
        }else{
            draw_translate(16,0);
            text_width+=16;
        }
        i++;
    }
    draw_pop_matrix();
}

static void flush_line(bm_font *font, float text_width, float x, float y)
{
    draw_push_matrix();
    draw_load_identity();
    draw_translate(x-(text_width/2.0f)*font->scale, y);
    text_width=0;
    draw_scale(font->scale,-font->scale);
    draw_flush_and_multiply();
    draw_pop_matrix();
}

