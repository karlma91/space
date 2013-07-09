



typedef struct _bm_char{
    int id;
    int x;
    int y;
    int w;
    int h;
    int x_offset;
    int y_offset;
    int x_advance;
} bm_char;


struct bm_font{
    float        scale;
    int          base, line_h, w, h;
    int          tex_id;
    int          base_list;
    bm_char      chars[256];
    float        mmat[16];
};

int bmfont_read_font(char *filename)
{

}

