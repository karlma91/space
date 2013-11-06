#include <stdlib.h>
#include <stdio.h>
#include "waffle_utils.h"
#include "SDL.h"

#define RESOURCE_VERSION 9 // changed: 05.11.13

#if __ANDROID__

static char APK_PATH[200] = "";
static char INTERNAL_PATH[200] = "";

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

void waffle_init(void)
{
	zzip_error_t zzip_err;
	//SDL_Log("DEBUG: Loading game resources...");
#if __ANDROID__
	game_data = zzip_dir_open(APK_PATH, &zzip_err); // NB! is actually .apk, not game_data
	strcpy(&INTERNAL_PATH[0], SDL_AndroidGetInternalStoragePath());
#else
	game_data = zzip_dir_open("game_data.zip", &zzip_err);
	if (!game_data) {
		SDL_Log("ERROR: game_data.zip could not be found!\n"
				"Run ./zip_res.sh to compress current game data");
		exit(-1);
	}
#endif

	//SDL_Log("Checking if game resources are up to date...");
	ZZIP_FILE *zf = waffle_open("ver");
	if (zf) {
		char buffer[4096];
		int filesize = zzip_file_read(zf, buffer, 4096);
		buffer[filesize] = 0;
		zzip_file_close(zf);

		int version = 0;
		sscanf(buffer, "%d", &version);

		if (version != RESOURCE_VERSION) {
			SDL_Log("ERROR: you need to update game_data.zip, found version %d, but expected version %d", version, RESOURCE_VERSION);
			exit(-1);
		}

	} else {
		SDL_Log("ERROR: could not open version file! Make sure your game_data.zip is up to date!\n"
				"Run ./zip_res.sh to compress current game data");
		exit(-1);
	}
}

void waffle_destroy(void)
{
	zzip_dir_close(game_data);
}


/*
 * a simple checksum method
 */
int checksum(char *data, int length)
{
	unsigned int sum = 0;

	do {
		sum = (sum >> 1) | (sum << 31);
		sum += *data;
		++data;
	} while (--length);

	return sum;
}


ZZIP_FILE *waffle_open(char *path)
{
	char full_path[256];

#if __ANDROID__
	sprintf(&full_path[0], "assets/game_data/%s", path);
#else
	sprintf(&full_path[0], "%s", path);
#endif
	return zzip_file_open(game_data, full_path, 0);
}

//Todo make sure that the caller is notified if the given file is not fully read
int waffle_read(ZZIP_FILE *zf, char *buffer, int len)
{
	int filesize = zzip_read(zf, buffer, len);
	buffer[filesize] = 0;
	zzip_close(zf);

	return filesize;
}


int waffle_read_file(char *filename, char *buffer, int len)
{
	Uint32 time_used = SDL_GetTicks();
	//SDL_Log("Reading file: '%s'",filename); //verbose
	ZZIP_FILE *fp = waffle_open(filename);

	if (fp) {
		int filesize = waffle_read(fp, buffer, len);
		if (filesize) {
#if !ARCADE_MODE
			time_used = SDL_GetTicks() - time_used;
			SDL_Log("DEBUG: File loaded: #%08x\tsize: %5.3fkB\tname: '%s\ttime: %.3f'", checksum(buffer, filesize), filesize / 1024.0f, filename, time_used / 1000.0);
#endif
			return filesize;
		} else {
			SDL_Log("ERROR: could not read file '%s' - error code: %d", filename, game_data->errcode);
			return 0;
		}
	} else {
		SDL_Log("ERROR: could not open file '%s' - error code: %d", filename, game_data->errcode);
		return 0;
	}
}

int waffle_next_line(char *file)
{
    int i = 0;
    while(file[i] != '\n'){
        if(file[i] == '\0')
            return -1;
        i++;
    }
    return i;
}

//TODO make sure folders exists
#if __ANDROID__
const static char *waffle_dirs[4] = {
	INTERNAL_PATH,INTERNAL_PATH,INTERNAL_PATH,INTERNAL_PATH
};
#elif TARGET_OS_IPHONE
const static char *waffle_dirs[4] = {
	"../Documents/","../Library/Preferences/","../Library/","../tmp/"
};
#else // win, linux, mac
#if ARCADE_MODE
const static char *waffle_dirs[4] = {
	"", "", "",""
};
#else
const static char *waffle_dirs[4] = {
	"bin/user_files/", "bin/user_data/settings/", "bin/user_data/","bin/tmp/"
};
#endif
#endif

FILE *waffle_internal_fopen(enum WAFFLE_DIR dir_type,const char *filename, const char *opentype)
{
	if (dir_type > 3) {
		SDL_Log("ERROR: Invalid directory type: %d", dir_type);
		exit(-1);
	}

	const char *folder = waffle_dirs[dir_type];
	char path[200] = "";
	strcpy(&path[0], folder);
	strcat(&path[0], filename);
	return fopen(path, opentype);
}


/*
int waffle_write_internal(char *filename, char *data)
{

	return 0;
}

int waffle_read_internal(char *filename, char *data)
{

	return 0;
}
*/
