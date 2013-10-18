#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "draw.h"

extern GLuint *textures;
extern int texture_load(const char *file);

extern int texture_init(void);

extern const float TEX_MAP_FULL[8];

extern int texture_bind(int);

extern int TEX_WHITE;
extern int TEX_GLOW_DOT;
extern int TEX_GLOW;
extern int TEX_LIGHT;
extern int TEX_STARS;
extern int TEX_METAL;

#endif /* TEXTURE_H_ */
