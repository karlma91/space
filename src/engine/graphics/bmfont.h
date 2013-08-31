#ifndef BMFONT_H_
#define BMFONT_H_

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
    float       scale;
    int         base;
    int         line_height;
    int         tex_w;
    int         tex_h;
    int         tex_id;
    bm_char     chars[256];
    float       mmat[16];
} bm_font;

bm_font * bmfont_read_font(char *filename);
void bmfont_init(void);
void bmfont_render(bm_font *font, int align, float x, float y, char *format, ...);

#endif

