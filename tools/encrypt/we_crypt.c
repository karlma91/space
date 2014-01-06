#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "../../src/engine/include/we_crypt.h"

//TODO support partly encryption of files? (for encryption of android assets)

static byte order[32];
static byte xor_mask[4];

int chr2int(byte buf[4])
{
	return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
}
void int2chr(byte buf[4], int num)
{
	buf[0] = (num >> 24) & 0xFF;
	buf[1] = (num >> 16) & 0xFF;
	buf[2] = (num >> 8) & 0xFF;
	buf[3] = num & 0xFF;
}

int bits2int(byte bits[32])
{
	int i, n = 0;
	for (i = 0; i < 32; i++) {
		n |= (bits[i] & 0x1) << order[i];
	}
	return n;
}

int bits2int_inv(byte bits[32])
{
	byte inv_order[256];
	int i, n = 0;
	for (i = 0; i < 32; i++) {
		inv_order[order[i]] = i;
	}
	for (i = 0; i < 32; i++) {
		n |= (bits[i] & 0x1) << inv_order[i];
	}
	return n;
}

void int2bits(byte bits[32], unsigned int n)
{
	int sum = 0;
	//fprintf(stderr,"0x%0X = ", n);
	int i;
	for (i = 0; i < 32; i++, n >>= 1) {
		bits[i] = n & 0x1;
		//fprintf(stderr,"%d", bits[i]);
		if (bits[i]) sum++;
	}
	//fprintf(stderr," [sum: %d]\n", sum);
}


/*
 * supported symbols: a-z,*,&,(space),_,=,#
 */

void we_genkey(byte key[20], char str[32])
{
	int i, shift, bp=0;
	byte chrs[256];
	for(i=0;i<256;i++) chrs[i] = 0xff;
	for(i=0;i<20;i++) key[i] = 0x00;
	int offset = 'z'-'a';
	for (i = 'a'; i <= 'z'; i++) {
		chrs[i] = i - (byte)'a';
	}
	chrs[(byte)'*'] = ++offset;
	chrs[(byte)'&'] = ++offset;
	chrs[(byte)' '] = ++offset;
	chrs[(byte)'_'] = ++offset;
	chrs[(byte)'='] = ++offset;
	chrs[(byte)'#'] = ++offset;

	fprintf(stderr, "decoding: '%s'\n", str);

	for (i=0, bp = 0; bp < 160; i++, bp += 5) {
		shift = bp & 0x7;
		byte index = chrs[(byte)str[i]];
		if (index >= 0 && index < 32) {
			key[bp >> 3] |= index << shift;
			key[(bp+7) >> 3] |= index >> (8-shift);
		} else {
			fprintf(stderr, "!%c!", str[i]);
		}
	}

	fprintf(stderr, "key:\n{");
	for (i=0; i<20; i++) {
		fprintf(stderr, "0x%02x, ", key[i]);
	}
	fprintf(stderr, "}\n");
}

static void genorder(byte key[20])
{
	int i, mx, n, shift, bp = 0;
	fprintf(stderr, "byte order[32] = {\n");
	byte available[32];
	for (i=0;i<32;i++) available[i] = i;
	for (i = 0, mx = 31; i < 32; i++, mx--) {
		int bits, mask;
		if (mx > 15) {
			bits = 5;
			mask = 0x1F;
		} else if (mx > 7) {
			bits = 4;
			mask = 0x0F;
		} else if (mx > 3) {
			bits = 3;
			mask = 0x07;
		} else if (mx > 1) {
			bits = 2;
			mask = 0x03;
		} else if (mx > 0) {
			bits = 0;
			mask = 0x01;
		} else {
			bits = 0;
			mask = 0x00;
		}

		n = key[bp >> 3];
		shift = bp & 0x7;
		n >>= shift;
		n |= key[(bp+7) >> 3] << (8-shift);
		n &= mask;

		if (n > mx) {
			fprintf(stderr, " ");
			n -= mx;
		}

		// find the nth available number
		int j;
		int num;
		for (j=0; j < 32; j++) {
			num = available[j];
			if (num < 32) {
				if (n > 0) {
					n--;
				} else {
					available[j] = 32;
					break;
				}
			}
		}
		order[i] = num;
		fprintf(stderr, "% 2d,%s", num, (i+1)&7 ? " " : "\n");
		bp += bits;
	}
	fprintf(stderr, "};\n");
}

static void genmask(byte key[20])
{
	int i;
	for (i=0; i < 4; i++) {
		xor_mask[i] = key[i+16];
	}
	fprintf(stderr, "byte mask[4] = {0x%02x, 0x%02x, 0x%02x, 0x%02x};\n",xor_mask[0],xor_mask[1],xor_mask[2],xor_mask[3]);
}

int we_encrypt(off_t pos, int word)
{
	int i;
	byte c[4];
	byte bits[32];

	// shuffle
	int2bits(bits, word);
	word = bits2int(bits); //TODO use offset on order as well

	// xor
	int2chr(c, word);
	for (i=0; i<4; i++) {
		byte mask_off = ((pos+i) & 0xFF) ^ ((pos>>8) & 0xFF) ^ ((pos>>16) & 0xFF) ^ ((pos>>24) & 0xFF);
		c[i] ^= xor_mask[i] + mask_off; //TODO use random gen with seed based on parts of key to generate offset
	}
	word = chr2int(c);
	return word;
}

int we_decrypt(off_t pos, int word)
{
	int i;
	byte c[4];
	byte bits[32];

	//xor
	int2chr(c, word);
	for (i=0; i<4; i++) {
		byte mask_off = ((pos+i) & 0xFF) ^ ((pos>>8) & 0xFF) ^ ((pos>>16) & 0xFF) ^ ((pos>>24) & 0xFF);
		c[i] ^= xor_mask[i] + mask_off;
	}
	word = chr2int(c);

	// shuffle
	int2bits(bits, word);
	word = bits2int_inv(bits);
	return word;
}

void we_setkey(byte key[20])
{
	int i;
	genorder(key);
	genmask(key);
	for (i=0; i<20;i++) key[i] = 0xff;
}

void we_rmkey(byte key[20])
{
	int i;
	for (i=0; i<20;i++) key[i] = 0x00;
	genorder(key);
	genmask(key);
}

int encrypt_file(int fd, char *buf, int len)
{
	off_t pos = lseek(fd, 0, SEEK_CUR);
	ssize_t size = read(fd, buf, len);

	if (size & 3) {
		fprintf(stderr, "Warning! size not a factor of 4!\n");
	}

	ssize_t i;
	for (i=0; i < size; i+=4) {
		int eword, dword, word;
		word = chr2int(buf+i);
		eword = we_encrypt(pos+i, word);
		int2chr(buf+i,eword);
	}
	return size;
}

