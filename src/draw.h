#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"

#define CIRCLE_SMALL 16
#define CIRCLE_MEDIUM 32
#define CIRCLE_BIG 128
#define CIRCLE_EXTRA 256

//fra chipmunkDemo.h
typedef struct Color {
        float r, g, b, a;
} Color;

static inline void glColor_from_color(Color color){
        glColor4fv((GLfloat *)&color);
}

static inline Color RGBAColor(float r, float g, float b, float a){
        Color color = {r, g, b, a};
        return color;
}

static inline Color LAColor(float l, float a){
        Color color = {l, l, l, a};
        return color;
}
void draw_init();
void drawCircle(cpVect center, cpFloat angle, cpFloat radius, Color fill, Color line);