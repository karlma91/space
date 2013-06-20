#include <stdlib.h>
#include <stdio.h>
#include "waffle_utils.h"
#include "SDL_log.h"

#define RESOURCE_VERSION 1

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

void waffle_init()
{
	zzip_error_t zzip_err;
#if __ANDROID__
	game_data = zzip_dir_open(APK_PATH, &zzip_err); // NB! is actually .apk, not game_data
#else
	game_data = zzip_dir_open("game_data.zip", &zzip_err);
	if (!game_data) {
		SDL_Log("ERROR: game_data.zip could not be found!\n"
				"Run ./zip_res.sh to compress current game data");
		exit(-1);
	}
#endif

	ZZIP_FILE *zf = waffle_open("ver");
	if (zf) {
		char buffer[4096];
		int filesize = zzip_file_read(zf, buffer, 4096);
		buffer[filesize] = 0;
		zzip_file_close(zf);

		int version = 0;
		sscanf(buffer, "%d", &version);

		if (version != RESOURCE_VERSION) {
			SDL_Log("ERROR: you need to update game_data.zip, found version %d, but needs version %d.\n"
					"Run ./zip_res.sh to compress current game data", version, RESOURCE_VERSION);
			exit(-1);
		}

	} else {
		SDL_Log("ERROR: could not open version file! Make sure your game_data.zip is up to date!\n"
				"Run ./zip_res.sh to compress current game data");
		exit(-1);
	}

}

void waffle_destroy()
{
	zzip_dir_close(game_data);
}

//TODO create general read to buffer method

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
