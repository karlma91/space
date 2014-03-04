#include "monitor.h"
#include "../../space/game.h"
static int i = 0;

enum{T_F,T_I,T_C,T_S};
static struct{
    int type;
    char name[32];
    float fv;
    int iv;
    cpVect cv;
    char sv[32];
} prints[32];

void monitor_float(char *name, float  var)
{
    prints[i].type = T_F;
    strncpy(prints[i].name, name, 32);
    prints[i].fv = var;
    i = i<31?i+1:i;
}
void monitor_int(char *name, int  var)
{
    prints[i].type = T_I;
    strncpy(prints[i].name,name, 32);
    prints[i].iv = var;
    i = i<31?i+1:i;
}
void monitor_string(char *name, char * var)
{
    prints[i].type = T_S;
    strncpy(prints[i].name,name, 32);
    strncpy(prints[i].sv,var, 32);
    i = i<31?i+1:i;
}
void monitor_cpvect(char *name, cpVect var)
{
    prints[i].type = T_C;
    strncpy(prints[i].name,name, 32);
    prints[i].cv = var;
    i = i<31?i+1:i;
}

void monitor_render(view *cam)
{
    int j = 0;
    draw_color(COL_BLACK);
    draw_box(1,cpv(-cam->view_width/2, 250-(i-1)*30), cpv(250,i*30), 0, 0);
    draw_color(COL_WHITE);
    while(j<i) {
            cpVect p = cpv(-cam->view_width/2 ,250-j*30);
            switch(prints[j].type){
            case T_F:
                bmfont_left(FONT_NORMAL,p,30,"%s:%.4f",prints[j].name, prints[j].fv);
                break;
            case T_I:
                bmfont_left(FONT_NORMAL,p,30,"%s:%d",prints[j].name, prints[j].iv);
                break;
            case T_S:
                bmfont_left(FONT_NORMAL,p,30,"%s:%s",prints[j].name, prints[j].sv);
                break;
            case T_C:
                bmfont_left(FONT_NORMAL,p,30,"%s:(%.1f,%.1f)",prints[j].name, prints[j].cv.x, prints[j].cv.y);
                break;
            }
            j++;
    }
    i = 0;
}
