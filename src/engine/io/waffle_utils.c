#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "waffle_utils.h"
#include "SDL.h"

#include "we_crypt.h"

#if __WIN32__
#define S_IRWXG 1
#define S_IROTH 1
#define S_IXOTH 1
#endif

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


void waffle_init(void)
{
#if LOAD_FROM_FOLDER
	/* do nothing */
#else
	byte key[20];
	we_genkey(key, "abcdefghijklmnopqrstuvwxyz*&_ #=");
	we_setkey(key);

	//zzip_init_io(&io_handler, 0);
	//io_handler.fd.read = &decrypt_block;

	zzip_error_t zzip_err;
	//SDL_Log("DEBUG: Loading game resources...");
#if __ANDROID__
	game_data = zzip_dir_open(APK_PATH, &zzip_err); // NB! is actually .apk, not game_data
	strcpy(&INTERNAL_PATH[0], SDL_AndroidGetInternalStoragePath());
#else
	//game_data = zzip_dir_open_ext_io(GAME_RESOURCES, &zzip_err, ext, &io_handler);
	game_data = zzip_dir_open("game_data.zip", &zzip_err);
	if (!game_data) {
		SDL_Log("ERROR: game data could not be loaded!\n"
				"Run ./zip_res.sh to compress current game data");
		//SDL_Log("%s",zzip_errno(zzip_err));
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
	int filesize = waffle_read_file_zip("ver", buffer, 4096);
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

ZZIP_FILE *waffle_open_zip(char *path)
{
	char full_path[256];

#if LOAD_FROM_FOLDER
	sprintf(&full_path[0], "game_data/%s", path);
	fprintf(stderr, "opening file: %s\n", full_path);
	return zzip_open(full_path, O_RDWR);
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
int waffle_read_zip(ZZIP_FILE *zf, char *buffer, int len)
{
	int filesize = zzip_read(zf, buffer, len);
	buffer[filesize] = 0;
	zzip_close(zf);

	return filesize;
}


int waffle_read_file_zip(char *filename, char *buffer, int len)
{
	Uint32 time_used = SDL_GetTicks();
	//SDL_Log("Reading file: '%s'",filename); //verbose
	ZZIP_FILE *fp = waffle_open_zip(filename);

	if (fp) {
		int filesize = waffle_read_zip(fp, buffer, len);
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

int waffle_remove(char *path)
{
	char fpath[200];
	sprintf(fpath,"bin/user_files/%s", path);
	return remove(fpath);
}

FILE *waffle_fopen(enum WAFFLE_DIR dir_type, const char *filename, const char *opentype)
{
	if (dir_type > 3) {
		SDL_Log("ERROR: Invalid directory type: %d", dir_type);
		exit(-1);
	}

	const char *folder = waffle_dirs[dir_type];
	char path[200] = "";
	char fopath[200] = "";

	strcpy(&path[0], folder);
	strcat(&path[0], filename);
	strcpy(fopath, path);
	SDL_Log("Opening: %s", path);
	int i;
	int last_slash = 199;
	for (i=0; i<200 && fopath[i]; i++) {
		last_slash = fopath[i] == '/' ? i : last_slash;
	}
	fopath[last_slash] = '\0';
	if (!strchr(fopath, 'r')) {
		mkpath(fopath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}


	return fopen(path, opentype);
}

we_diriter *waffle_get_diriter(enum WAFFLE_DIR dir_type, const char *path)
{
	char fullpath[200] = "";
	sprintf(fullpath, "%s%s",waffle_dirs[dir_type], path);

	DIR *d = opendir(fullpath);

	if (d){
		we_diriter *wedi = calloc(1, sizeof(we_diriter));
		strcpy(wedi->path, path);
		strcpy(wedi->cur_path, path);

		wedi->d = d;
		wedi->dir_type = dir_type;
		return wedi;
	}else{
		SDL_Log("Error openging folder %s", fullpath);
	}
	return NULL;
}

void waffle_free_diriter(we_diriter * wedi)
{
	if(wedi){
		closedir(wedi->d);
		free(wedi);
	}
}

int waffle_read_diriter_file(we_diriter * wedi, char *buffer, int n)
{
	if(waffle_isfile(wedi)){
		FILE *f = waffle_fopen(wedi->dir_type, wedi->path,"wr");
		return waffle_read(f,buffer,n);
	}else{
		SDL_Log("[waffle_read_diriter_file]: Cannot read filder to buffer");
		return -1;
	}
}

int waffle_dirnext(we_diriter * wedi)
{
    if (wedi) {
	wedi->dir = readdir(wedi->d);
	while (wedi->dir && (strcmp(wedi->dir->d_name, ".") == 0 || strcmp(wedi->dir->d_name, "..") == 0)) {
		wedi->dir = readdir(wedi->d);
	}
	if (wedi->dir) {
		strcpy(wedi->cur_path, wedi->path);
		if(waffle_isdir(wedi)){
			strcat(wedi->cur_path, "/");
			strcat(wedi->cur_path, wedi->dir->d_name);
		}
		return 1;
	}
    }
    return 0;
}

int waffle_isdir(we_diriter * wedi)
{
	struct stat s;
	char full[256];
	sprintf(full,"%s%s", waffle_dirs[wedi->dir_type], wedi->cur_path);

	SDL_Log("STAT: %s ", full);
	if(stat(full, &s) == 0){
		if (S_ISDIR(s.st_mode)){
			return 1;
		}
	}else{
		SDL_Log("Error doing stat()");
	}
	return 0;
}
int waffle_isfile(we_diriter * wedi)
{
	struct stat s;
	char full[256];
	sprintf(full,"%s%s", waffle_dirs[wedi->dir_type], wedi->cur_path);

	SDL_Log("STAT: %s ", full);
	if(stat(full, &s) == 0){
		if (S_ISREG(s.st_mode)){
			return 1;
		}
	}else{
		SDL_Log("ISFILE Error doing stat()");
	}
	return 0;
}

//Todo make sure that the caller is notified if the given file is not fully read!
int waffle_read(FILE *f, char *buffer, int len)
{
	int filesize = fread(buffer, 1, len, f);
	buffer[filesize] = 0;
	fclose(f);
	return filesize;
}


int we_mkdir(char *path, unsigned int mode)
{
#if defined(_WIN32)
	return mkdir(path);
#else
	return mkdir(path,mode);
#endif
}

/* Function with behaviour like `mkdir -p'
 * http://niallohiggins.com/2009/01/08/mkpath-mkdir-p-alike-in-c-for-unix/
 * */
int mkpath(const char *s, mode_t mode)
{
	char *q, *r = NULL, *path = NULL, *up = NULL;
	int rv;

	rv = -1;
	if (strcmp(s, ".") == 0 || strcmp(s, "/") == 0)
		return 0;
	if ((path = strdup(s)) == NULL)
		return 0;
	if ((q = strdup(s)) == NULL)
		return 0;
	if ((r = dirname(q)) == NULL)
		goto out;
	if ((up = strdup(r)) == NULL)
		return 0;
	if ((mkpath(up, mode) == -1) && (errno != EEXIST))
		goto out;
	if ((we_mkdir(path, mode) == -1) && (errno != EEXIST))
		rv = -1;
	else
		rv = 0;

	out:
	if (up != NULL)
		free(up);
	free(q);
	free(path);
	return (rv);
}
