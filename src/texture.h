#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "draw.h"

extern GLuint *textures;
extern int texture_load(char *file);

extern int texture_init();

extern const float TEX_MAP_FULL[8];

extern int texture_bind();

#endif /* TEXTURE_H_ */
