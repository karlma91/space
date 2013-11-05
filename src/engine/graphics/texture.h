#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "draw.h"
#include "we_data.h"
#include "sprite.h"

#define MAX_TEXNAME_LEN 50
extern const float TEX_MAP_FULL[8];
extern array *tex_units;

extern int texture_load(const char *file);
extern int texture_init(void);
extern GLint texture_get_current(void);
extern int texture_bind_virt(int);
extern int texture_bind(int);

extern sprite_subimg texture_normalize_uv(int tex_id, sprite_subimg subimg);

typedef struct tex_unit {
	char name[MAX_TEXNAME_LEN];
	GLuint gl_tex;
	GLint tex_min_filter;
	GLint tex_mag_filter;
	GLint tex_wrap_s;
	GLint tex_wrap_t;
	int w, h;
	float load_time;
} tex_unit;

#endif /* TEXTURE_H_ */
