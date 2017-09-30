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

//
//#include "sha2.h"
//
//#include "includeComm.h"
//#include <iostream>
//#include <string.h>
//using namespace std;
//
///*

//uint32 preProcess(uint8* &ppMessage, uint8* message, uint32 len)
//{

//	uint32 paddingSize = (((len + 1) % 64) > 56) ? (56 + (64 - ((len) % 64))) : 56 - ((len) % 64); // TODO: Clean this up and optimize
//	uint32 ppMessageLength = len + paddingSize;
//	

//	ppMessage = (uint8*)calloc(ppMessageLength + 8, 1);
//	memcpy(ppMessage, message, len);
//	ppMessage[len] = 0x80;
//

//	uint32 msgBitLen[2] = {0, 0};
//	if ((len * 8) > 0xFFFFFFF) ++msgBitLen[1]; msgBitLen[0] += (len * 8);
//

//	for (int i = 0; i < 4; ++i)
//	{
//		(ppMessage + ppMessageLength)[i]	 = (msgBitLen[1] >> ((3 - i) << 3)) & 0xFF;
//		(ppMessage + ppMessageLength)[i + 4] = (msgBitLen[0] >> ((3 - i) << 3)) & 0xFF;
//	}
//
//	return (ppMessageLength + 8);
//} 
//
//void scheduler(uint8* messageChunk, uint32* schedulerArray)
//{

//	for (int i = 0; i < 16; ++i)
//	{
//		schedulerArray[i] = messageChunk[(i << 2)    ] << 24 |
//							messageChunk[(i << 2) + 1] << 16 |
//							messageChunk[(i << 2) + 2] <<  8 |
//							messageChunk[(i << 2) + 3];
//	}
//
//	uint32 s0, s1;			
//	for (uint32 i = 16; i < 64; ++i)
//	{
//		s0 = (_rotr(schedulerArray[i-15], 7) ^ (_rotr(schedulerArray[i-15], 18)) ^ (schedulerArray[i-15] >> 3));
//		s1 = (_rotr(schedulerArray[i-2], 17) ^ (_rotr(schedulerArray[i-2],  19)) ^ (schedulerArray[i-2] >> 10));
//		schedulerArray[i] = schedulerArray[i-16] + s0 + schedulerArray[i-7] + s1;
//	}
//}
//
//void compressor(uint32* schedulerArray, s_hashValues &hValues)
//{

//	uint32 a = hValues.h0;
//	uint32 b = hValues.h1;
//	uint32 c = hValues.h2;
//	uint32 d = hValues.h3;
//	uint32 e = hValues.h4;
//	uint32 f = hValues.h5;
//	uint32 g = hValues.h6;
//	uint32 h = hValues.h7;
//	

//	uint32 S1, ch, temp1, S0, maj, temp2;
//	for (uint32 i = 0; i < 64; ++i)
//	{

//		S1 = _rotr(e, 6) ^ _rotr(e, 11) ^ _rotr(e, 25);
//		ch = (e & f) ^ ((~e) & g);
//		temp1 = h + S1 + ch + k[i] + schedulerArray[i];
//		S0 = _rotr(a, 2) ^ _rotr(a, 13) ^ _rotr(a, 22);
//		maj = (a & b) ^ (a & c) ^ (b & c);
//		temp2 = S0 + maj;

//		h = g;
//		g = f;
//		f = e;
//		e = d + temp1;
//		d = c;
//		c = b;
//		b = a;
//		a = temp1 + temp2;
//	}
//

//	hValues.h0 += a;
//	hValues.h1 += b;
//	hValues.h2 += c;
//	hValues.h3 += d;
//	hValues.h4 += e;
//	hValues.h5 += f;
//	hValues.h6 += g;
//	hValues.h7 += h;
//}
//

//void SHA256(uint8 finalHash[32], uint8* message, uint32 len)
//{

//	s_hashValues hash;
//

//	hash.h0 = 0x6a09e667;
//	hash.h1 = 0xbb67ae85;
//	hash.h2 = 0x3c6ef372;
//	hash.h3 = 0xa54ff53a;
//	hash.h4 = 0x510e527f;
//	hash.h5 = 0x9b05688c;
//	hash.h6 = 0x1f83d9ab;
//	hash.h7 = 0x5be0cd19;
//

//	//hash.h0 = 0x67e6096a;
//	//hash.h1 = 0x85ae67bb;
//	//hash.h2 = 0x72f36e3c;
//	//hash.h3 = 0x3af54fa5;
//	//hash.h4 = 0x7f520e51;
//	//hash.h5 = 0x8c68059b;
//	//hash.h6 = 0xabd9831f;
//	//hash.h7 = 0x19cde05b;
//

//	uint8* ppMsg = NULL;
//	uint32 ppMsgLen = preProcess(ppMsg, message, len);
//	uint32 nbChunks = ppMsgLen >> 6 ;	.
//

//	uint32* schedulerArray = (uint32*) malloc(256);
//

//	for (uint32 i = 0; i < nbChunks; ++i)
//	{
//		scheduler(&ppMsg[i << 6], schedulerArray);
//		compressor(schedulerArray, hash);
//	}
//
//	// Cleanup
//	free(schedulerArray);
//	free(ppMsg);
//

//	for (int i = 0; i < 4; ++i)
//	{
//		finalHash[0  + i] = ((uint8*)&hash.h0)[3 - i];
//		finalHash[4  + i] = ((uint8*)&hash.h1)[3 - i];
//		finalHash[8  + i] = ((uint8*)&hash.h2)[3 - i];
//		finalHash[12 + i] = ((uint8*)&hash.h3)[3 - i];
//		finalHash[16 + i] = ((uint8*)&hash.h4)[3 - i];
//		finalHash[20 + i] = ((uint8*)&hash.h5)[3 - i];
//		finalHash[24 + i] = ((uint8*)&hash.h6)[3 - i];
//		finalHash[28 + i] = ((uint8*)&hash.h7)[3 - i];
//	}
//}
//
//void displayHash(uint8 hash[32])
//{
//	printf("0x");
//	
//	for (int i = 0; i < 32; ++i)
//	{
//		printf("%x", hash[i]);
//	}
//
//	printf("\n");
//}
//
////int _tmain(int argc, _TCHAR* argv[])
////{

////	uchar test[] = "The quick brown fox jumps over the lazy dog";
////	uchar finalHash[32];
////	SHA256(finalHash, test, 43);
////
////	displayHash(finalHash);
////
////	// 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
////	return 0;
////}