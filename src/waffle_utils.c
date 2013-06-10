#include <stdlib.h>
#include <stdio.h>
#include "waffle_utils.h"


#if __ANDROID__

char APK_PATH[200] = "";

/*
 * Native method called from java to obtain full path to the .apk package
 */
void set_apk_path(const char *apk_path)
{
	strcpy(&APK_PATH[0], apk_path);
	SDL_Log("DEBUG: ANDROID_APK_PATH = %s", &APK_PATH[0]);
}

/*
 * Class:     org_libsdl_app_SDLActivity
 * Method:    set_apk_path
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_set_1apk_1path
  (JNIEnv *env, jclass cls, jstring apk_path) {
	const char* str = (*env)->GetStringUTFChars(env, apk_path, 0);
	set_apk_path(str);
}
#endif


static ZZIP_DIR *game_data = NULL;

int waffle_init()
{
	zzip_error_t zzip_err;
#if __ANDROID__
	game_data = zzip_dir_open(APK_PATH, &zzip_err); // NB! is actually .apk, not game_data
#else
	game_data = zzip_dir_open("game_data.zip", &zzip_err);
#endif

	return 0;
}

int waffle_destroy()
{
	zzip_dir_close(game_data);
	return 0;
}

ZZIP_FILE *waffle_open(char *path)
{
	char full_path[256];

#if __ANDROID__
	sprintf(&full_path[0], "assets/game_data/%s", path);
#else
	sprintf(&full_path[0], "%s", path);
#endif

	SDL_Log("DEBUG: full_path = %s", &full_path[0]);
	return zzip_file_open(game_data, full_path, 0);
}
