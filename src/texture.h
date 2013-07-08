#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "draw.h"

extern GLuint *textures;
extern int texture_load(char *file);

extern int texture_init();

extern const float TEX_MAP_FULL[8];

extern int texture_bind();

extern int TEX_GLOW_DOT;
extern int TEX_GLOW;
extern int TEX_LIGHT;

#endif /* TEXTURE_H_ */
