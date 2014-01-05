/*
 * we_crypt.h
 *
 *  Created on: 3. jan. 2014
 *      Author: Mathias
 */

#ifndef WE_CRYPT_H_
#define WE_CRYPT_H_

#define MB << 20
#define chunk_size (1 MB)
typedef unsigned char byte;

int chr2int(byte buf[4]);
void int2chr(byte buf[4], int num);
int bits2int(byte bits[32]);
int bits2int_inv(byte bits[32]);
void int2bits(byte bits[32], unsigned int n);

void we_genkey(byte key[20], char str[32]); //supported symbols: a-z,*,&,(space),_,=,#
int we_encrypt(off_t pos, int word);
int we_decrypt(off_t pos, int word);
void we_setkey(byte key[20]);
void we_rmkey(byte key[20]);

int encrypt_file(int fd, char *buf, int len);

#endif /* WE_CRYPT_H_ */
