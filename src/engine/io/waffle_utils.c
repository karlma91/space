#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "waffle_utils.h"
#include "SDL.h"

#include "we_crypt.h"

#define RESOURCE_VERSION 9 // changed: 05.11.13

/* Pass '-DLOAD_FROM_FOLDER=1' to compiler for reading files directly from game_data/... */
#ifndef LOAD_FROM_FOLDER
#define LOAD_FROM_FOLDER 0
#endif

#if TARGET_OS_IPHONE
#define GAME_RESFOLDER "res"
#define GAME_RESOURCES GAME_RESFOLDER".dat"
#elif __ANDROID__
#define GAME_RESFOLDER "res"
#define GAME_RESOURCES GAME_RESFOLDER".dat"
#else
#define GAME_RESFOLDER "res" //"game_data"
#define GAME_RESOURCES GAME_RESFOLDER".dat" //".zip"
#endif

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


static zzip_ssize_t decrypt_block(int fd, void *data, zzip_size_t len);
static ZZIP_DIR *game_data = NULL;

static zzip_strings_t ext[] = { ".zip", ".ZIP", ".dat", ".DAT", "", 0};
static zzip_plugin_io_handlers io_handler;

void waffle_init(void)
{
#if LOAD_FROM_FOLDER
	/* do nothing */
#else
	byte key[20];
	we_genkey(key, "abcdefghijklmnopqrstuvwxyz*&_ #=");
	we_setkey(key);

	zzip_init_io(&io_handler, 0);
	io_handler.fd.read = &decrypt_block;

	zzip_error_t zzip_err;
	//SDL_Log("DEBUG: Loading game resources...");
#if __ANDROID__
	game_data = zzip_dir_open(APK_PATH, &zzip_err); // NB! is actually .apk, not game_data
	strcpy(&INTERNAL_PATH[0], SDL_AndroidGetInternalStoragePath());
#else
	game_data = zzip_dir_open_ext_io(GAME_RESOURCES, &zzip_err, ext, &io_handler);
	if (!game_data) {
		SDL_Log("ERROR: game data could not be loaded!\n"
				"Run ./zip_res.sh to compress current game data");
		exit(-1);
	}
#endif /* __ANDROID__ */
	ZZIP_DIRENT zd;
	zzip_rewinddir(game_data);
	do {
		zzip_dir_read(game_data, &zd);
		fprintf(stderr, "file: %s\n", zd.d_name);
	} while (strcmp("ver", zd.d_name));
	//zzip_dir_stat(game_data, )
	//SDL_Log("Checking if game resources are up to date...");
#endif /* LOAD_FROM_FOLDER */

	char buffer[4096];
	int filesize = waffle_read_file("ver", buffer, 4096);
	if (filesize) {
		int version = -1;
		sscanf(buffer, "%d", &version);
		if (version != RESOURCE_VERSION) {
			SDL_Log("ERROR: you need to update game_data.zip, found version %d, but expected version %d", version, RESOURCE_VERSION);
			exit(-1);
		}
	} else {
		SDL_Log("ERROR: could not open version file! Make sure your game_data.zip is up to date!\n"
				"Run ./zip_res.sh to compress cunrrent game data");
		exit(-1);
	}
	SDL_Log("Game data ready for loading\n");
}

void waffle_destroy(void)
{
#if !LOAD_FROM_FOLDER
	zzip_dir_close(game_data);
#endif
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

#if LOAD_FROM_FOLDER
	sprintf(&full_path[0], "game_data/%s", path);
	fprintf(stderr, "opening file: %s\n", full_path);
	return zzip_open(full_path, ZZIP_CASELESS);
#else
#if __ANDROID__
	sprintf(&full_path[0], "assets/game_data/%s", path);
#else
	sprintf(&full_path[0], "%s", path);
#endif /* __ANDROID__ */
	fprintf(stderr, "opening file: %s\n", full_path);
	return zzip_file_open(game_data, full_path, 0);
	//return zzip_open_ext_io(full_path, O_RDONLY, ZZIP_ONLYZIP | ZZIP_CASELESS, ext, &io_handler);
#endif /* LOAD_FROM_FOLDER */
}

//Todo make sure that the caller is notified if the given file is not fully read!
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

static zzip_ssize_t decrypt_block(int fd, void *data, zzip_size_t len)
{
	int warn = 0;
	byte *dst = data;
	zzip_off_t start_pos = lseek(fd, 0, SEEK_CUR);

	int pad_start = 0, pad_end = 0;
	if (start_pos & 3) {
		//fprintf(stderr, "Warning! pos not a factor of 4! Changing [pos, len]: [%lld, %lu] -> ", start_pos, len); //TODO support pos not a factor of 4!
		pad_start = start_pos & 3;
		start_pos = lseek(fd, -pad_start, SEEK_CUR);
		//if (start_pos & 3) fprintf(stderr, "\nERROR: could not change file offset correctly\n");
		len += pad_start;
		//fprintf(stderr, "[%lld, %lu] with start padding of %d byte\n", start_pos, len, pad_start);
		//warn=1;
	}
	if (len & 3) {
		//fprintf(stderr, "Warning! length not a factor of 4! len %ld -> ", len);
		pad_end = 4 - len & 3;
		len += pad_end;
		//fprintf(stderr, "%ld with end padding of %d byte\n", len, pad_end);
		//warn=1;
	}

	byte src[len];
	zzip_size_t size = read(fd, src, len); // read encrypted data to tmp buffer
	lseek(fd, -pad_end, SEEK_CUR); /* move to expected seek position */

	if (size & 3) {
		fprintf(stderr, "Warning! file size not a factor of 4!\n");
		warn=1;
	}
	//zzip_off_t end = start_pos + (size - pad_end);
	//fprintf(stderr, "io: read %lld\tsize: %lub\tend: %lld\n", start_pos, size, end);

	int src_i = 0, dst_i = 0, fpos;
	/* decrypt block to buffer*/
	for (fpos = start_pos, src_i = 0; src_i < len; src_i += 4, fpos += 4) {
		int eword, dword;
		eword = chr2int(&src[src_i]);
		dword = we_decrypt(fpos, eword);
		//if (warn) fprintf(stderr, "%08x%c", dword, fpos & 0x3f ? ' ' : '\n');
		int2chr(&src[src_i], dword);
	}

	/* copy decrypted block to dst data */
	src_i = pad_start; // skip padding
	size = len - pad_end - pad_start; // ignore padding from total size
	for (dst_i = 0; dst_i < size; dst_i++, src_i++) {
		dst[dst_i] = src[src_i];
	}
	//if (warn) fprintf(stderr, "\n");
	return size;
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
