#ifndef BMFONT_H_
#define BMFONT_H_

#define BMFONT_LEFT 0
#define BMFONT_CENTER 1
#define BMFONT_RIGHT 2
#define BMFONT_MAXLEN 256

typedef struct _bm_char{
    int id;
    int x;
    int y;
    int w;
    int h;
    int x_offset;
    int y_offset;
    int x_advance;
    int page;
    int chl;
} bm_char;


typedef struct _bm_font{
    int         base;
    int         line_height;
    int         tex_w;
    int         tex_h;
    int         tex_id;
    sprite s;
    bm_char     chars[256];
    float       mmat[16];
} bm_font;

bm_font * bmfont_read_font(char *filename);
void bmfont_init(void);

void bmfont_left(bm_font *font, cpVect pos, float scale, char *format, ...);
void bmfont_center(bm_font *font, cpVect pos, float scale, char *format, ...);
void bmfont_right(bm_font *font, cpVect pos, float scale, char *format, ...);
void bmfont_render(bm_font *font, int align, float x, float y, float scale, char *text);


#endif

