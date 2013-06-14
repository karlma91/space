#ifndef WAFFLE_UTILS_H_
#define WAFFLE_UTILS_H_

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

extern int waffle_init();
extern int waffle_destroy();

/*
 * waffle_open
 *
 * opens a file relative to the zip-archive opened in waffle_init
 * use zzip_read(ZZIP_FILE * fp, void * buf, zzip_size_t len) to read
 * the returned ZZIP_FILE to buf
 *
 */
extern ZZIP_FILE *waffle_open(char *path);

#endif //WAFFLE_UTILS_H_
