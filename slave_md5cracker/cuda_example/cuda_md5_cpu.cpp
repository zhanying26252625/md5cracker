// CPU implementation of MD5 

typedef unsigned int uint;

#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;

// prepare a 56-byte (maximum) wide md5 message by appending the 64-bit length
// we assume c0 is zero-padded
void md5_prep(char *c0)
{
	uint len = 0;
	char *c = c0;
	while(*c) {len++; c++;}
	c[0] = 0x80;			// bit 1 after the message
	((uint*)c0)[14] = len * 8;	// message length in bits
}

union md5hash
{
	uint ui[4];
	char ch[16];
};

string md5hash2string(md5hash& md5){
    
    uint* hash = (uint*)md5.ui;

    string retStr;
    for(int i = 0; i != 16; i++) { 
        char buf[128] = {0};
        sprintf(buf, "%02x", (uint)(((unsigned char *)hash)[i])); 
        retStr += string(buf);
    }

    return retStr;
}

int convert(char c){
    if( c>='0' && c<='9')
        return c-'0';
    else if(c>='a'&&c<='f')
        return c-'a'+10;
    else
        return 0;
}

void string2md5hash(string& str, md5hash& hash){

    const char* buf = str.c_str();

    for(int i=0,j=0;i<32;i+=2,j++){
        char f = convert( buf[i] );
        char s = convert(buf[i+1] );
        hash.ch[j] = f*16+s;
    }
}


void print_md5(uint* hash){

    for(int i = 0; i != 16; i++) { 
        printf( "%02x", (uint)(((unsigned char *)hash)[i])); 
    }

    printf("\n");
}

//////////////////////////////////////////////////////////////////////////////
/////////////       Ron Rivest's MD5 C Implementation       //////////////////
//////////////////////////////////////////////////////////////////////////////

/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */


/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z))) 

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (uint)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (uint)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (uint)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (uint)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define getw(in, i) ((in)[(i)])

/* Basic MD5 step. Transform buf based on in.
 */
void md5_cpu_v2(const uint *in, uint &a, uint &b, uint &c, uint &d)
{
	const uint a0 = 0x67452301;
	const uint b0 = 0xEFCDAB89;
	const uint c0 = 0x98BADCFE;
	const uint d0 = 0x10325476;

	//Initialize hash value for this chunk:
	a = a0;
	b = b0;
	c = c0;
	d = d0;

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, getw(in,  0), S11, 3614090360U); /* 1 */
  FF ( d, a, b, c, getw(in,  1), S12, 3905402710U); /* 2 */
  FF ( c, d, a, b, getw(in,  2), S13,  606105819U); /* 3 */
  FF ( b, c, d, a, getw(in,  3), S14, 3250441966U); /* 4 */
  FF ( a, b, c, d, getw(in,  4), S11, 4118548399U); /* 5 */
  FF ( d, a, b, c, getw(in,  5), S12, 1200080426U); /* 6 */
  FF ( c, d, a, b, getw(in,  6), S13, 2821735955U); /* 7 */
  FF ( b, c, d, a, getw(in,  7), S14, 4249261313U); /* 8 */
  FF ( a, b, c, d, getw(in,  8), S11, 1770035416U); /* 9 */
  FF ( d, a, b, c, getw(in,  9), S12, 2336552879U); /* 10 */
  FF ( c, d, a, b, getw(in, 10), S13, 4294925233U); /* 11 */
  FF ( b, c, d, a, getw(in, 11), S14, 2304563134U); /* 12 */
  FF ( a, b, c, d, getw(in, 12), S11, 1804603682U); /* 13 */
  FF ( d, a, b, c, getw(in, 13), S12, 4254626195U); /* 14 */
  FF ( c, d, a, b, getw(in, 14), S13, 2792965006U); /* 15 */
  FF ( b, c, d, a, getw(in, 15), S14, 1236535329U); /* 16 */
 
  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, getw(in,  1), S21, 4129170786U); /* 17 */
  GG ( d, a, b, c, getw(in,  6), S22, 3225465664U); /* 18 */
  GG ( c, d, a, b, getw(in, 11), S23,  643717713U); /* 19 */
  GG ( b, c, d, a, getw(in,  0), S24, 3921069994U); /* 20 */
  GG ( a, b, c, d, getw(in,  5), S21, 3593408605U); /* 21 */
  GG ( d, a, b, c, getw(in, 10), S22,   38016083U); /* 22 */
  GG ( c, d, a, b, getw(in, 15), S23, 3634488961U); /* 23 */
  GG ( b, c, d, a, getw(in,  4), S24, 3889429448U); /* 24 */
  GG ( a, b, c, d, getw(in,  9), S21,  568446438U); /* 25 */
  GG ( d, a, b, c, getw(in, 14), S22, 3275163606U); /* 26 */
  GG ( c, d, a, b, getw(in,  3), S23, 4107603335U); /* 27 */
  GG ( b, c, d, a, getw(in,  8), S24, 1163531501U); /* 28 */
  GG ( a, b, c, d, getw(in, 13), S21, 2850285829U); /* 29 */
  GG ( d, a, b, c, getw(in,  2), S22, 4243563512U); /* 30 */
  GG ( c, d, a, b, getw(in,  7), S23, 1735328473U); /* 31 */
  GG ( b, c, d, a, getw(in, 12), S24, 2368359562U); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, getw(in,  5), S31, 4294588738U); /* 33 */
  HH ( d, a, b, c, getw(in,  8), S32, 2272392833U); /* 34 */
  HH ( c, d, a, b, getw(in, 11), S33, 1839030562U); /* 35 */
  HH ( b, c, d, a, getw(in, 14), S34, 4259657740U); /* 36 */
  HH ( a, b, c, d, getw(in,  1), S31, 2763975236U); /* 37 */
  HH ( d, a, b, c, getw(in,  4), S32, 1272893353U); /* 38 */
  HH ( c, d, a, b, getw(in,  7), S33, 4139469664U); /* 39 */
  HH ( b, c, d, a, getw(in, 10), S34, 3200236656U); /* 40 */
  HH ( a, b, c, d, getw(in, 13), S31,  681279174U); /* 41 */
  HH ( d, a, b, c, getw(in,  0), S32, 3936430074U); /* 42 */
  HH ( c, d, a, b, getw(in,  3), S33, 3572445317U); /* 43 */
  HH ( b, c, d, a, getw(in,  6), S34,   76029189U); /* 44 */
  HH ( a, b, c, d, getw(in,  9), S31, 3654602809U); /* 45 */
  HH ( d, a, b, c, getw(in, 12), S32, 3873151461U); /* 46 */
  HH ( c, d, a, b, getw(in, 15), S33,  530742520U); /* 47 */
  HH ( b, c, d, a, getw(in,  2), S34, 3299628645U); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, getw(in,  0), S41, 4096336452U); /* 49 */
  II ( d, a, b, c, getw(in,  7), S42, 1126891415U); /* 50 */
  II ( c, d, a, b, getw(in, 14), S43, 2878612391U); /* 51 */
  II ( b, c, d, a, getw(in,  5), S44, 4237533241U); /* 52 */
  II ( a, b, c, d, getw(in, 12), S41, 1700485571U); /* 53 */
  II ( d, a, b, c, getw(in,  3), S42, 2399980690U); /* 54 */
  II ( c, d, a, b, getw(in, 10), S43, 4293915773U); /* 55 */
  II ( b, c, d, a, getw(in,  1), S44, 2240044497U); /* 56 */
  II ( a, b, c, d, getw(in,  8), S41, 1873313359U); /* 57 */
  II ( d, a, b, c, getw(in, 15), S42, 4264355552U); /* 58 */
  II ( c, d, a, b, getw(in,  6), S43, 2734768916U); /* 59 */
  II ( b, c, d, a, getw(in, 13), S44, 1309151649U); /* 60 */
  II ( a, b, c, d, getw(in,  4), S41, 4149444226U); /* 61 */
  II ( d, a, b, c, getw(in, 11), S42, 3174756917U); /* 62 */
  II ( c, d, a, b, getw(in,  2), S43,  718787259U); /* 63 */
  II ( b, c, d, a, getw(in,  9), S44, 3951481745U); /* 64 */

	a += a0;
	b += b0;
	c += c0;
	d += d0;

}


