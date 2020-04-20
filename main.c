/************************************************************************
Lab 9 Nios Software

Dong Kai Wang, Fall 2017
Christine Chen, Fall 2013

For use with ECE 385 Experiment 9
University of Illinois ECE Department
************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "aes.h"

// Pointer to base address of AES module, make sure it matches Qsys
volatile unsigned int * AES_PTR = (unsigned int *) 0x00000040;

// Execution mode: 0 for testing, 1 for benchmarking
int run_mode = 0;

/** charToHex
 *  Convert a single character to the 4-bit value it represents.
 *
 *  Input: a character c (e.g. 'A')
 *  Output: converted 4-bit value (e.g. 0xA)
 */
char charToHex(char c)
{
	char hex = c;

	if (hex >= '0' && hex <= '9')
		hex -= '0';
	else if (hex >= 'A' && hex <= 'F')
	{
		hex -= 'A';
		hex += 10;
	}
	else if (hex >= 'a' && hex <= 'f')
	{
		hex -= 'a';
		hex += 10;
	}
	return hex;
}

/** charsToHex
 *  Convert two characters to byte value it represents.
 *  Inputs must be 0-9, A-F, or a-f.
 *
 *  Input: two characters c1 and c2 (e.g. 'A' and '7')
 *  Output: converted byte value (e.g. 0xA7)
 */
char charsToHex(char c1, char c2)
{
	char hex1 = charToHex(c1);
	char hex2 = charToHex(c2);
	return (hex1 << 4) + hex2;
}

void RotWord(uchar* word){
	uchar rot = word[0];
	word[0] = word[1];
	word[1] = word[2];
	word[2] = word[3];
	word[3] = rot;
}

void SubWord(uchar* word){
	word[0] = aes_sbox[word[0]];
	word[1] = aes_sbox[word[1]];
	word[2] = aes_sbox[word[2]];
	word[3] = aes_sbox[word[3]];

}

void KeyExpansion(uchar* key, uchar* key_schedule){
	uchar temp[4];
	uchar w[176];

	//puts first key into key schedule
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			w[i*4 + j] = key[i * 4 + j];
			key_schedule[i*4 + j] = key[j *4 + i];
		}
	}
	int i = 4;
	while (i<44){
		for (int j = 0; j < 4; j++){
			temp[j] = w[i*4 + j - 4];
		}
		if(i % 4 == 0){
			RotWord(&temp);
			SubWord(&temp);
			temp[0] ^= (Rcon[i/4] >> 24);
		}
		for(int k = 0; k < 4; k++){
			w[i*4 + k] = w[(i-4)*4 + k] ^ temp[k];
		}
		i++;
	}
		for(int i = 0; i < 11; i++){
			for(int x = 0; x < 4; x++){
				for(int y = 0; y < 4; y++){
					key_schedule[i*16 + 4*x + y] = w[i*16 + y*4 + x];
				}
			}
		}
}



void AddRoundKey(uchar* state, uchar* RoundKey){
	for (int x = 0; x<16; x++){
		state[x] ^= RoundKey[x];
	}
}

void SubBytes(uchar* state){
	for(int x = 0; x<16; x++){
		state[x] = aes_sbox[state[x]];
	}
}

void ShiftRows(unsigned char *state) {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < i; j++)
			RotWord(&state[i * 4]);
	}
}

void MixColumns(uchar* state){
	uchar copy[4];
	copy[0] = gf_mul[state[0]][0] ^ gf_mul[state[4]][1] ^ state[8] ^ state[12];
	copy[1] = state[0] ^ gf_mul[state[4]][0] ^ gf_mul[state[8]][1] ^ state[12];
   	copy[2] = state[0] ^ state[4] ^ gf_mul[state[8]][0] ^ gf_mul[state[12]][1];
   	copy[3] = gf_mul[state[0]][1] ^ state[4] ^ state[8] ^ gf_mul[state[12]][0];
   	state[0] = copy[0];
   	state[4] = copy[1];
   	state[8] = copy[2];
   	state[12] = copy[3];

	copy[0] = gf_mul[state[1]][0] ^ gf_mul[state[5]][1] ^ state[9] ^ state[13];
	copy[1] = state[1] ^ gf_mul[state[5]][0] ^ gf_mul[state[9]][1] ^ state[13];
   	copy[2] = state[1] ^ state[5] ^ gf_mul[state[9]][0] ^ gf_mul[state[13]][1];
   	copy[3] = gf_mul[state[1]][1] ^ state[5] ^ state[9] ^ gf_mul[state[13]][0];
   	state[1] = copy[0];
   	state[5] = copy[1];
   	state[9] = copy[2];
   	state[13] = copy[3];

	copy[0] = gf_mul[state[2]][0] ^ gf_mul[state[6]][1] ^ state[10] ^ state[14];
	copy[1] = state[2] ^ gf_mul[state[6]][0] ^ gf_mul[state[10]][1] ^ state[14];
   	copy[2] = state[2] ^ state[6] ^ gf_mul[state[10]][0] ^ gf_mul[state[14]][1];
   	copy[3] = gf_mul[state[2]][1] ^ state[6] ^ state[10] ^ gf_mul[state[14]][0];
   	state[2] = copy[0];
   	state[6] = copy[1];
   	state[10] = copy[2];
   	state[14] = copy[3];

   	copy[0] = gf_mul[state[3]][0] ^ gf_mul[state[7]][1] ^ state[11] ^ state[15];
	copy[1] = state[3] ^ gf_mul[state[7]][0] ^ gf_mul[state[11]][1] ^ state[15];
   	copy[2] = state[3] ^ state[7] ^ gf_mul[state[11]][0] ^ gf_mul[state[15]][1];
   	copy[3] = gf_mul[state[3]][1] ^ state[7] ^ state[11] ^ gf_mul[state[15]][0];
   	state[3] = copy[0];
   	state[7] = copy[1];
   	state[11] = copy[2];
   	state[15] = copy[3];

}

/** encrypt
 *  Perform AES encryption in software.
 *
 *  Input: msg_ascii - Pointer to 32x 8-bit char array that contains the input message in ASCII format
 *         key_ascii - Pointer to 32x 8-bit char array that contains the input key in ASCII format
 *  Output:  msg_enc - Pointer to 4x 32-bit int array that contains the encrypted message
 *               key - Pointer to 4x 32-bit int array that contains the input key
 */
void encrypt(unsigned char * msg_ascii, unsigned char * key_ascii, unsigned int * msg_enc, unsigned int * key)
{
	// Implement this function
	uchar byte_in[16];
	uchar byte_out[16];
	uchar word[176];

	for(int i = 0; i < 4; i++){
		for(int j = 0; j<4;j++){
			byte_in[j*4+i] = charsToHex(msg_ascii[(i*4+j)*2], msg_ascii[2*(i*4+j)+1]);
			byte_out[i*4+j] = charsToHex(key_ascii[(i*4+j)*2], key_ascii[2*(i*4+j)+1]);
		}

	}
	KeyExpansion(&byte_out, &word);
	AddRoundKey(&byte_in, &word[0]);
	for(int i = 1; i < 10; i++){
		SubBytes(&byte_in);
		ShiftRows(&byte_in);
		MixColumns(&byte_in);
		AddRoundKey(&byte_in, &word[16*i]);
	}
	SubBytes(&byte_in);
	ShiftRows(&byte_in);
	AddRoundKey(&byte_in, &word[160]);

	key[0] = ((word[0]) << 24) |((word[4]) << 16) |((word[8]) << 8) |(word[12]);
	msg_enc[0] = ((byte_in[0]) << 24)| ((byte_in[4]) << 16) |((byte_in[8]) << 8) |(byte_in[12]);

	key[1] = ((word[1]) << 24) |((word[5]) << 16) |((word[9]) << 8) |(word[13]);
	msg_enc[1] = ((byte_in[1]) << 24)| ((byte_in[5]) << 16) |((byte_in[9]) << 8) |(byte_in[13]);

	key[2] = ((word[2]) << 24) |((word[6]) << 16) |((word[10]) << 8) |(word[14]);
	msg_enc[2] = ((byte_in[2]) << 24)| ((byte_in[6]) << 16) |((byte_in[10]) << 8) |(byte_in[14]);

	key[3] = ((word[3]) << 24) |((word[7]) << 16) |((word[11]) << 8) |(word[15]);
	msg_enc[3] = ((byte_in[3]) << 24)| ((byte_in[7]) << 16) |((byte_in[11]) << 8) |(byte_in[15]);

	AES_PTR[0] = key[0];
	AES_PTR[1] = key[1];
	AES_PTR[2] = key[2];
	AES_PTR[3] = key[3];
}


/** decrypt
 *  Perform AES decryption in hardware.
 *
 *  Input:  msg_enc - Pointer to 4x 32-bit int array that contains the encrypted message
 *              key - Pointer to 4x 32-bit int array that contains the input key
 *  Output: msg_dec - Pointer to 4x 32-bit int array that contains the decrypted message
 */
void decrypt(unsigned int * msg_enc, unsigned int * msg_dec, unsigned int * key)
{
	// Implement this function
	AES_PTR[4] = msg_enc[0];
	AES_PTR[5] = msg_enc[1];
	AES_PTR[6] = msg_enc[2];
	AES_PTR[7] = msg_enc[3];
	AES_PTR[14] = 0x0001;
	while(AES_PTR[15] == 0x0000) {
		continue;
	}
	msg_dec[0] = AES_PTR[11];
	msg_dec[1] = AES_PTR[10];
	msg_dec[2] = AES_PTR[9];
	msg_dec[3] = AES_PTR[8];
//	AES_PTR[0] = msg_dec[0]; //uncomment these lines to display first and last 2 bytes of decrypted message on hex displays
//	AES_PTR[1] = msg_dec[1];
//	AES_PTR[2] = msg_dec[2];
//	AES_PTR[3] = msg_dec[3];
	AES_PTR[14] = 0x0;
}

/** main
 *  Allows the user to enter the message, key, and select execution mode
 *
 */
int main()
{
	// Input Message and Key as 32x 8-bit ASCII Characters ([33] is for NULL terminator)
	unsigned char msg_ascii[33];
	unsigned char key_ascii[33];
	// Key, Encrypted Message, and Decrypted Message in 4x 32-bit Format to facilitate Read/Write to Hardware
	unsigned int key[4];
	unsigned int msg_enc[4];
	unsigned int msg_dec[4];

	printf("Select execution mode: 0 for testing, 1 for benchmarking: ");
	scanf("%d", &run_mode);

	if (run_mode == 0) {
		// Continuously Perform Encryption and Decryption
		while (1) {
			int i = 0;
			printf("\nEnter Message:\n");
			scanf("%s", msg_ascii);
			printf("\n");
			printf("\nEnter Key:\n");
			scanf("%s", key_ascii);
			printf("\n");
			encrypt(msg_ascii, key_ascii, msg_enc, key);
			printf("\nEncrpted message is: \n");
			for(i = 0; i < 4; i++){
				printf("%08x", msg_enc[i]);
			}
			printf("\n");
			decrypt(msg_enc, msg_dec, key);
			printf("\nDecrypted message is: \n");
			for(i = 0; i < 4; i++){
				printf("%08x", msg_dec[i]);
			}
			printf("\n");
		}
	}
	else {
		// Run the Benchmark
		int i = 0;
		int size_KB = 2;
		// Choose a random Plaintext and Key
		for (i = 0; i < 32; i++) {
			msg_ascii[i] = 'a';
			key_ascii[i] = 'b';
		}
		// Run Encryption
		clock_t begin = clock();
		for (i = 0; i < size_KB * 64; i++)
			encrypt(msg_ascii, key_ascii, msg_enc, key);
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		double speed = size_KB / time_spent;
		printf("Software Encryption Speed: %f KB/s \n", speed);
		// Run Decryption
		begin = clock();
		for (i = 0; i < size_KB * 64; i++)
			decrypt(msg_enc, msg_dec, key);
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		speed = size_KB / time_spent;
		printf("Hardware Encryption Speed: %f KB/s \n", speed);
	}
	return 0;
}
