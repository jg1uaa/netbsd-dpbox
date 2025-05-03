#ifndef MD2MD5_H
#define MD2MD5_H

#include <stdint.h>

/* MD5POINTER defines a generic pointer type */
typedef unsigned char *MD5POINTER;

/* MD5UINT2 defines a two byte word */
typedef unsigned short int MD5UINT2;

/* MD5UINT4 defines a four byte word */
typedef uint32_t MD5UINT4;

/* MD5 context. */
typedef struct {
  MD5UINT4 state[4];                                   /* state (ABCD) */
  MD5UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputLen);
void MD5Final(unsigned char digest[16], MD5_CTX *context);

typedef char MD2barr16[16];

typedef struct MD2_CTX {
  MD2barr16 state, checksum;
  int32_t count;
  MD2barr16 buffer;
} MD2_CTX;


void MD2Init(MD2_CTX *context);
void MD2Update(MD2_CTX *context, char *input, int32_t inputLen);
void MD2Final(char *digest, MD2_CTX *context);

#endif
