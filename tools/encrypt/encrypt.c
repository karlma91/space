#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MB << 20
#define chunk_size (1 MB)
typedef unsigned char byte;

#include "we_crypt.h"

int main(int argc, char *argv[])
{
	byte *buf = malloc(chunk_size);
	byte order[32];
	if (argc == 5) {
		byte key[20];
		we_genkey(key, argv[4]);
		we_setkey(key);
	} else {
		byte key[20];
		we_genkey(key, "abcdefghijklmnopqrstuvwxyz*& _=#");
		we_setkey(key);
	}

	if (argc < 3) {
		fprintf(stderr, "wrong number of arguments. Expected 2 or more but got %d\n", argc-1);
		return -1;
	}

	FILE *in = fopen(argv[1],"rb");
	int fd_in = fileno(in);
	FILE *out = fopen(argv[2],"wb");
	FILE *check = NULL;
	if (argc >= 4) {
		check = fopen(argv[3],"wb");
	}

	if (!in) {
		fprintf(stderr, "ERROR: could not find input file: %s\n", argv[1]);
		return -1;
	}

	int i;
	int chunk = 0;
	int size = 0;
	unsigned long total_size = 0;
	do {
		fprintf(stderr,"processing chunk #%d\n", chunk++);
		size = encrypt_file(fd_in, buf, chunk_size);
		size = size + (size & 3 ? 4 - (size & 3) : 0); // rund opp til nærmeste hele int
		fwrite(buf,sizeof *buf, size, out);
		if (check) {
			for (i = 0; i < size; i +=4) {
				int eword = chr2int(buf+i);
				int dword = we_decrypt(total_size+i, eword);
				int2chr(buf+i, dword);
			}
			fwrite(buf,sizeof *buf, size, check);
		}
		total_size += size;
	} while (size);

	int err = ferror(in);
	if (err) {
		fprintf(stderr,"error reading file: error %d\n", err);
		return err;
	}
	err = ferror(out);
	if (err) {
		fprintf(stderr,"error writing file: error %d\n", err);
		return err;
	}

	fclose(out);
	fclose(in);
	if (check) fclose(check);
	fprintf(stderr, "Done! %d bytes processed\n", total_size);
	return 0;
}

