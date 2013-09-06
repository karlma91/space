#ifndef WAFFLE_UTILS_H_
#define WAFFLE_UTILS_H_

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
extern ZZIP_FILE *waffle_open(char *path);

/* waffle_read
 *
 * Reads an open file into buffer of given length,
 * returns size of file read.
 * NB! zf is closed after use
 */
extern int waffle_read(ZZIP_FILE *fp, char *buffer, int len);

/* waffle_read_file
 *
 * opens file with the given name and reads into buffer of given length,
 * closes the file and return size of file
 */
extern int waffle_read_file(char *filename, char *buffer, int len);


/* waffle_internal_fopen
 *
 * Use this method to return a IO-stream to filename on the device internal storage
 */
extern FILE *waffle_internal_fopen(enum WAFFLE_DIR dir_type, const char *filename, const char *opentype);

/*
 * return number of chars until next '\n'
 */
int waffle_next_line(char *file);


#endif //WAFFLE_UTILS_H_