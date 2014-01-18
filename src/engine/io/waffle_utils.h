#ifndef WAFFLE_UTILS_H_
#define WAFFLE_UTILS_H_

#include <dirent.h>
#include <stdio.h>
#include "SDL_log.h"
#include "SDL_system.h"
#include "stdio.h"
#include "zzip/lib.h"

#if TARGET_OS_IPHONE


#elif __ANDROID__

#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "waffle_tag"
#define  SDL_Log(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

//extern char APK_PATH[200];
#else

#endif

enum WAFFLE_DIR {
	WAFFLE_DOCUMENTS,
	WAFFLE_PREFERENCES,
	WAFFLE_LIBRARY,
	WAFFLE_ZIP,
	WAFFLE_TMP
};

extern void waffle_init(void);
extern void waffle_destroy(void);

int checksum(char *data, int length);

/*
 * waffle_open
 *
 * opens a file relative to the zip-archive opened in waffle_init
 */
extern ZZIP_FILE *waffle_open_zip(char *path);

/* waffle_read
 *
 * Reads an open file into buffer of given length,
 * returns size of file read.
 * NB! zf is closed after use
 */
extern int waffle_read_zip(ZZIP_FILE *fp, char *buffer, int len);

/* waffle_read_file
 *
 * opens file with the given name and reads into buffer of given length,
 * closes the file and return size of file
 */
extern int waffle_read_file_zip(char *filename, char *buffer, int len);


/* waffle_internal_fopen
 *
 * Use this method to return a IO-stream to filename on the device internal storage
 */
extern FILE *waffle_fopen(enum WAFFLE_DIR dir_type, const char *filename, const char *opentype);



typedef struct _we_diriter{
	enum WAFFLE_DIR dir_type;
	char path[256];
	char cur_path[256];
	DIR *d;
	struct dirent *dir;
}we_diriter;

we_diriter *waffle_get_diriter(enum WAFFLE_DIR dir_type, const char *path);
void waffle_free_diriter(we_diriter * wedi);
int waffle_read_diriter_file(we_diriter * wedi, char *buffer, int n);
int waffle_dirnext(we_diriter * wedi);
int waffle_isdir(we_diriter * wedi);
int waffle_isfile(we_diriter * wedi);
int waffle_read(FILE *zf, char *buffer, int len);

int mkpath(const char *s, mode_t mode);
/*
 * return number of chars until next '\n'
 */
int waffle_next_line(char *file);




#endif //WAFFLE_UTILS_H_
