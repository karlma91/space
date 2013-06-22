#ifndef WAFFLE_UTILS_H_
#define WAFFLE_UTILS_H_

#include "SDL_log.h"

#if TARGET_OS_IPHONE


#elif __ANDROID__

#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "waffle_tag"
#define  SDL_Log(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  fprintf(TYPE,...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__);__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,"file: %s line:%d\n",__FILE__,__LINE__)

extern char APK_PATH[200];
#else

#endif


#include "zzip/lib.h"

extern void waffle_init();
extern void waffle_destroy();

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

#endif //WAFFLE_UTILS_H_
