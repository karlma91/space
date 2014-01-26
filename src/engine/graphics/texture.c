#include <stdio.h>
#include <string.h>

#include "texture.h"
#include "SDL.h"
#include "SDL_image.h"

#include "../io/waffle_utils.h"
#include "draw.h"

#if __ANDROID__ // | TARGET_OS_IPHONE
#define TEXTURE_RESOLUTION "_low"
#else
#define TEXTURE_RESOLUTION ""
#endif

#define MAX_IMAGE_BUFFER (1024*1024*20)


typedef struct PVRTextureHeaderV3 { /* Reference: PVRTTexture.h from ImgTec's PVR SDK */
	Uint32	u32Version;			//Version of the file header, used to identify it.
	Uint32	u32Flags;			//Various format flags.
	Uint64	u64PixelFormat;		//The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
	Uint32	u32ColourSpace;		//The Colour Space of the texture, currently either linear RGB or sRGB.
	Uint32	u32ChannelType;		//Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
	Uint32	u32Height;			//Height of the texture.
	Uint32	u32Width;			//Width of the texture.
	Uint32	u32Depth;			//Depth of the texture. (Z-slices)
	Uint32	u32NumSurfaces;		//Number of members in a Texture Array.
	Uint32	u32NumFaces;		//Number of faces in a Cube Map. Maybe be a value other than 6.
	Uint32	u32MIPMapCount;		//Number of MIP Maps in the texture - NB: Includes top level.
	Uint32	u32MetaDataSize;	//Size of the accompanying meta data.
} PVRTextureHeaderV3;


const float TEX_MAP_FULL[8] = {0,1, 1,1, 0,0, 1,0};

array *tex_units; /* <tex_unit> */
hashmap *hm_name2tex = NULL;
static int tex_counter = 0; //textures goes from 1 and up

char *buffer = NULL;

static GLenum GL_ENUM_TYPE = GL_UNSIGNED_BYTE;
static GLint gl_tex_id_virt = -1, gl_tex_id = -1;

Uint32 time_start;

static int pvr_load(const char *file, PVRTextureHeaderV3 *pvr)
{
#if TARGET_OS_IPHONE
	if (pvr->u32Version == 0x03525650) {
		SDL_Log("Loading pvr texture...");

		tex_unit tex;
		int mipmaps = pvr->u32MIPMapCount;
		int w = pvr->u32Width;
		int h = pvr->u32Height;
		tex.w = w; tex.h = h;
		strncpy(tex.name,file, MAX_TEXNAME_LEN);

		++tex_counter;
		glGenTextures(1, &tex.gl_tex);
		glBindTexture(GL_TEXTURE_2D, tex.gl_tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


#define max(a,b) (a>b?a:b)
        int m;
		char *buffer = ((char *)pvr);
		int pre_size = pvr->u32MetaDataSize + sizeof *pvr;
		buffer += pre_size;
        for (m = 0; m < mipmaps; m++, w /=2, h /=2) {
        	int size = ( max(w, 8) * max(h, 8) * 4 + 7) / 8;
        	glCompressedTexImage2D(GL_TEXTURE_2D, m, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, w, h, 0, size, buffer);
        	buffer += size;
        }
		array_set_safe(tex_units, tex_counter, &tex);
		//hm_add(hm_name2tex, file, (void *)(*((int **)&tex_counter)));
		hm_add(hm_name2tex, file, tex_counter);
		tex.load_time = (SDL_GetTicks() - time_start) / 1000.0;
		SDL_Log("DEBUG: PVR texture loaded: %s, metasize: %d, loading time: %.3f", file, pre_size, tex.load_time);
		return tex_counter;
	}
#endif
	return 0;
}

int texture_load(const char *file)
{
#if !LOAD_TEXTURES
	return 0;
#endif
	if (!buffer) SDL_Log("ERROR: tried to load texture before initializing texture loader!");

	int tex_id_ext = (int) hm_get(hm_name2tex, file);
	if (tex_id_ext) {
		//SDL_Log("have texture: %s\n", file);
		return tex_id_ext;
	}

	char filepath[64];
	sprintf(filepath,"textures/%s", file);

	/* append texture resolution suffix */
	const char *file_suffix = &file[strlen(file) - 4];

	filepath[strlen(filepath) - 4] = 0; // ignore file suffix
	strcat(filepath, TEXTURE_RESOLUTION);
	strcat(filepath, file_suffix);

	time_start = SDL_GetTicks();

	SDL_RWops *rw;
	int filesize = waffle_read_file_zip(filepath, buffer, MAX_IMAGE_BUFFER);
	if (!filesize) {
		SDL_Log("ERROR: Unable to find texture: %s", file);
		return 1;
	}

	rw = SDL_RWFromMem(&buffer[0], filesize);

	if (pvr_load(file, (PVRTextureHeaderV3 *)buffer)) {
		SDL_FreeRW(rw);
		return 1;
	}

	SDL_Surface* img_load = IMG_Load_RW(rw, 0);
	SDL_FreeRW(rw);

	if (img_load) {
		tex_unit tex;
		int premult_alpha = 0;
		int mipmaps = 0;
		tex.w = img_load->w;
		tex.h = img_load->h;
		++tex_counter;

		strncpy(tex.name,file, MAX_TEXNAME_LEN);

		SDL_Surface *img = SDL_ConvertSurfaceFormat(img_load, SDL_PIXELFORMAT_ARGB8888,0);
		SDL_FreeSurface(img_load);

		/*Generate an OpenGL 2D texture from the SDL_Surface*.*/
		glGenTextures(1, &tex.gl_tex);
		//texture_bind(tex.gl_tex);
		glBindTexture(GL_TEXTURE_2D, tex.gl_tex);

		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		/* pre-multiply alpha */
		if (premult_alpha) {
			int i;
			int size = img->w * img->h;
			for (i=0 ; i < size; i++) {
				unsigned char *pixel = (unsigned char *)((int *)img->pixels + i);
				int b = pixel[0], g = pixel[1], r = pixel[2], a = pixel[3];
				pixel[0] = b * a / 255;
				pixel[1] = g * a / 255;
				pixel[2] = r * a / 255;
			}
		}

		glTexImage2D(GL_TEXTURE_2D, mipmaps, GL_RGBA, tex.w, tex.h, 0, GL_BGRA, GL_ENUM_TYPE, img->pixels);
		if (mipmaps == 0) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		SDL_FreeSurface(img);
		array_set_safe(tex_units, tex_counter, &tex);
		//TODO create combined hashmap/array data structure for storing data directly
		//hm_add(hm_name2tex, file, (void *)(*((int **)&tex_counter)));
		hm_add(hm_name2tex, file, tex_counter);
		tex.load_time = (SDL_GetTicks() - time_start) / 1000.0;
		SDL_Log("DEBUG: Texture loaded: %s, loading time: %.3f", file, tex.load_time);
		return tex_counter;

	} else {
		SDL_Log("ERROR: Unable to load texture: %s\n IMG_ERROR: %s\n", filepath, IMG_GetError());
		return 0;
	}
}

char * texture_get_name(TEXTURE_ID id)
{
	hashiterator *it = hm_get_iterator(hm_name2tex);
	TEXTURE_ID id_temp = (int)hm_iterator_next(it);
	while (id_temp != 0) {
		if (id_temp == id) {
			char *name = (char*)hm_iterator_get_key(it);
			hm_iterator_destroy(it);
			return name;
		} else {
			id_temp = (int)hm_iterator_next(it);
		}
	}
	hm_iterator_destroy(it);
	return NULL;
}

#include "SDL_endian.h"
int texture_init(void)
{
	buffer = calloc(1, MAX_IMAGE_BUFFER);
	tex_units = array_new(sizeof(tex_unit));
	hm_name2tex = hm_create();

	return 0;
}

sprite_subimg texture_normalize_uv(int tex_id, sprite_subimg subimg)
{
	tex_unit *tex = array_get(tex_units, tex_id);
	int w = tex->w;
	int h = tex->h;
	subimg.x1 /=w;
	subimg.y1 /=h;
	subimg.x2 /=w;
	subimg.y2 /=h;
	return subimg;
}

GLint texture_get_current_virt(void)
{
	return gl_tex_id_virt;
}

int texture_destroy(void)
{
	int i;
	for (i = 0; i < tex_counter; i++) {
		glDeleteTextures(1, array_get(tex_units, i));
	}
	array_destroy(tex_units);
	hm_destroy(hm_name2tex);
	free(buffer);
	return 0;
}

void texture_bind_clear(void)
{
	gl_tex_id = -1;
}

int texture_bind_virt(int tex_id) {
#if !LOAD_TEXTURES
	return 1;
#endif
	if (tex_id != gl_tex_id_virt && tex_id >= 0) {
		gl_tex_id_virt = tex_id;
		return 0;
	}
	return 1;
}

int texture_bind(int tex_id) {
#if !LOAD_TEXTURES
	return 1;
#endif
	if (tex_id != gl_tex_id && tex_id >= 0) {
		gl_tex_id = tex_id;
		glBindTexture(GL_TEXTURE_2D, ((tex_unit *)array_get(tex_units, tex_id))->gl_tex);
		return 0;
	}
	return 1;
}
