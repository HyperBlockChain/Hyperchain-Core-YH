/*Copyright 2017 hyperchain.net (Hyper Block Chain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT。
/*
/*Permission is hereby granted, free of charge, to any person obtaining a copy of this 
/*software and associated documentation files (the "Software"), to deal in the Software
/*without restriction, including without limitation the rights to use, copy, modify, merge,
/*publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
/*to whom the Software is furnished to do so, subject to the following conditions:
/*
/*The above copyright notice and this permission notice shall be included in all copies or
/*substantial portions of the Software.
/*
/*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
/*INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
/*PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
/*FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
/*OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/*DEALINGS IN THE SOFTWARE.
*/
#ifndef _SHA2_H_
#define _SHA2_H_

//#define uchar unsigned char
//#define uint unsigned long
//
//struct s_hashValues
//{
//	uint h0;
//	uint h1;
//	uint h2;
//	uint h3;
//	uint h4;
//	uint h5;
//	uint h6;
//	uint h7;
//};
//
//uint k[64] = 
//{0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
// 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
// 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
// 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
// 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
// 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
// 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
// 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
//};
////{0x982f8a42, 0x91443771, 0xcffbc0b5, 0xa5dbb5e9, 0x5bc25639, 0xf111f159, 0xa4823f92, 0xd55e1cab, //
//// 0x98aa07d8, 0x015b8312, 0xbe853124, 0xc37d0c55, 0x745dbe72, 0xfeb1de80, 0xa706dc9b, 0x74f19bc1, //
//// 0xc1699be4, 0x8647beef, 0xc69dc10f, 0xcca10c24, 0x6f2ce92d, 0xaa84744a, 0xdca9b05c, 0xda88f976, //
//// 0x52513e98, 0x6dc631a8, 0xc82703b0, 0xc77f59bf, 0xf30be0c6, 0x4791a7d5, 0x5163ca06, 0x67292914, //
//// 0x850ab727, 0x38211b2e, 0xfc6d2c4d, 0x130d3853, 0x54730a65, 0xbb0a6a76, 0x2ec9c281, 0x852c7292, //
//// 0xa1e8bfa2, 0x4b661aa8, 0x708b4bc2, 0xa3516cc7, 0x19e892d1, 0x240699d6, 0x85350ef4, 0x70a06a10, //
//// 0x16c1a419, 0x086c371e, 0x4c774827, 0xb5bcb034, 0xb30c1c39, 0x4aaad84e, 0x4fca9c5b, 0xf36f2e68, //
//// 0xee828f74, 0x6f63a578, 0x1478c884, 0x0802c78c, 0xfaffbe90, 0xeb6c50a4, 0xf7a3f9be, 0xf27871c6  //
////};

#include <stdio.h>
#include <stdlib.h>

#define SHA256_ROTL(a,b) (((a>>(32-b))&(0x7fffffff>>(31-b)))|(a<<b))
#define SHA256_SR(a,b) ((a>>b)&(0x7fffffff>>(b-1)))
#define SHA256_Ch(x,y,z) ((x&y)^((~x)&z))
#define SHA256_Maj(x,y,z) ((x&y)^(x&z)^(y&z))
#define SHA256_E0(x) (SHA256_ROTL(x,30)^SHA256_ROTL(x,19)^SHA256_ROTL(x,10))
#define SHA256_E1(x) (SHA256_ROTL(x,26)^SHA256_ROTL(x,21)^SHA256_ROTL(x,7))
#define SHA256_O0(x) (SHA256_ROTL(x,25)^SHA256_ROTL(x,14)^SHA256_SR(x,3))
#define SHA256_O1(x) (SHA256_ROTL(x,15)^SHA256_ROTL(x,13)^SHA256_SR(x,10))

extern void GetSHA256(unsigned char* sha256, const char* str, long long length)
{
    char *pp, *ppend;
    long l, i, W[64], T1, T2, A, B, C, D, E, F, G, H, H0, H1, H2, H3, H4, H5, H6, H7;
    H0 = 0x6a09e667, H1 = 0xbb67ae85, H2 = 0x3c6ef372, H3 = 0xa54ff53a;
    H4 = 0x510e527f, H5 = 0x9b05688c, H6 = 0x1f83d9ab, H7 = 0x5be0cd19;
    long K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    };
    l = length + ((length % 64 >= 56) ? (128 - length % 64) : (64 - length % 64));
    if (!(pp = (char*)malloc((unsigned long)l))) return;
    for (i = 0; i < length; pp[i + 3 - 2 * (i % 4)] = str[i], i++);
    for (pp[i + 3 - 2 * (i % 4)] = 128, i++; i < l; pp[i + 3 - 2 * (i % 4)] = 0, i++);
    *((long*)(pp + l - 4)) = length << 3;
    *((long*)(pp + l - 8)) = length >> 29;
    for (ppend = pp + l; pp < ppend; pp += 64){
        for (i = 0; i < 16; W[i] = ((long*)pp)[i], i++);
        for (i = 16; i < 64; W[i] = (SHA256_O1(W[i - 2]) + W[i - 7] + SHA256_O0(W[i - 15]) + W[i - 16]), i++);
        A = H0, B = H1, C = H2, D = H3, E = H4, F = H5, G = H6, H = H7;
        for (i = 0; i < 64; i++){
            T1 = H + SHA256_E1(E) + SHA256_Ch(E, F, G) + K[i] + W[i];
            T2 = SHA256_E0(A) + SHA256_Maj(A, B, C);
            H = G, G = F, F = E, E = D + T1, D = C, C = B, B = A, A = T1 + T2;
        }
        H0 += A, H1 += B, H2 += C, H3 += D, H4 += E, H5 += F, H6 += G, H7 += H;
    }
    free(pp - l);
	for (int i = 0; i < 4; ++i)
	{
		sha256[0  + i] = ((unsigned char*)&H0)[3 - i];
		sha256[4  + i] = ((unsigned char*)&H1)[3 - i];
		sha256[8  + i] = ((unsigned char*)&H2)[3 - i];
		sha256[12 + i] = ((unsigned char*)&H3)[3 - i];
		sha256[16 + i] = ((unsigned char*)&H4)[3 - i];
		sha256[20 + i] = ((unsigned char*)&H5)[3 - i];
		sha256[24 + i] = ((unsigned char*)&H6)[3 - i];
		sha256[28 + i] = ((unsigned char*)&H7)[3 - i];
	}
    
	/*unsigned char buf1[64];
	memset(buf1, 0, 64);
	sprintf((char*)buf1, "%08X%08X%08X%08X%08X%08X%08X%08X\n", H0, H1, H2, H3, H4, H5, H6, H7);*/
	//printf("%s", buf1);
	///*char *pBuf = sha256;*/
	//unsigned char buf2[32];
	////memset(buf2, 0, 64);
	//for (int i = 0; i < 32; ++i)
	//{
	//
	//	printf("%x", sha256[i]);
	//	
	//}
 //  // return sha256;
	//int idex = 0;
}

void displayHash(unsigned char hash[32])
{
	printf("0x");

	for (int i = 0; i < 32; ++i)
	{
		printf("%x", hash[i]);
	}

	printf("\n");
}
#endif
