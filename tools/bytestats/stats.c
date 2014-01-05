#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MB  << 20
#define chunk_size (1 MB)
typedef unsigned char byte;

long chr_stat[256];

int main(int argc, char *argv[])
{
	byte *buf = malloc(chunk_size);

	if (argc < 2) {
		fprintf(stderr, "wrong number of arguments. Expected 2 but got %d\n", argc-1);
		return -1;
	}

	int j;
	for (j = 1; j < argc; j++) {
		memset(chr_stat, 0, 256);
		char *fname = argv[j];
		char fstatname[250];
		snprintf(fstatname, 250, "%s_stat.txt", fname);

		FILE *in = fopen(fname,"rb");
		FILE *fstat = fopen(fstatname,"w");

		int i;
		int chunk = 0;
		int size = 0;
		unsigned long total_size = 0;
		do {
			fprintf(stderr,"processing chunk #%d\n", chunk++);
			size = fread(buf, sizeof *buf, chunk_size, in);
			total_size += size;
			for (i = 0; i < size; i++) {
				chr_stat[buf[i]]++;
			}
		} while (size);

		for (i = 0; i < 256; i++) {
			fprintf(fstat, "%d\t%ld\n", i, chr_stat[i]);
		}

		fclose(in);
		fclose(fstat);
		fprintf(stderr, "Done! %d bytes read\n", total_size);
	}

	free(buf);
	return 0;
}
