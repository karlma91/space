#ifndef WAFFLE_UTILS_H_
#define WAFFLE_UTILS_H_



#if TARGET_OS_IPHONE


#elif __ANDROID__

#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "waffle_tag"
#define  printf(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  fprintf(TYPE,...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__);__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,"file: %s line:%d\n",__FILE__,__LINE__)

#else

#endif


#endif //WAFFLE_UTILS_H_
