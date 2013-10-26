#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "draw.h"
#include "we_data.h"

extern const float TEX_MAP_FULL[8];
extern array *textures;

extern int texture_load(const char *file);
extern int texture_init(void);
extern GLint texture_get_current(void);
extern int texture_bind_virt(int);

extern int TEX_WHITE;
extern int TEX_GLOW_DOT;
extern int TEX_GLOW;
extern int TEX_LIGHT;
extern int TEX_STARS;
extern int TEX_METAL;

#endif /* TEXTURE_H_ */
