/*Copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or https://opensource.org/licenses/MIT.

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef __HYPERCHAIN_DB_H__
#define __HYPERCHAIN_DB_H__

#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm> 

#ifndef _WIN32
# define __STDC_LIMIT_MACROS
# include <stdint.h>
#else
#include <cstdint>
#endif

#include "../db/dbmgr.h"
#include "../HChainP2PManager/headers/inter_public.h"

using namespace std;


typedef map<uint64, T_HYPERBLOCKDBINFO> LocalBlockDB;

typedef map<uint64, LocalBlockDB> LocalChainDB;


struct HyperBlockDB
{
	T_HYPERBLOCKDBINFO hyperBlock;	
	LocalChainDB mapLocalChain;		

	const T_HYPERBLOCKDBINFO& GetHyperBlock()const;
	LocalChainDB& GetMapLocalChain();


};


typedef map<uint64, HyperBlockDB> HyperchainDB;

class CHyperchainDB
{
public:

	static int cleanTmp(HyperchainDB &hyperchainDB);
	static int AddHyperBlockDataRecord(HyperchainDB &hyperchainDB, T_HYPERBLOCKDBINFO blockInfo);

public:
	CHyperchainDB();
	~CHyperchainDB();
public:

	static int saveHyperBlockToDB(const T_HYPERBLOCKDBINFO &hyperblock);

	static int saveHyperBlocksToDB(const vector<T_HYPERBLOCKDBINFO> &vHyperblock);

	static int getHyperBlocks(HyperchainDB &hyperchainDB, uint64 nStartHyperID, uint64 nEndHyperID);

	static int getAllHyperBlocks(HyperchainDB &hyperchainDB);

	static uint64 GetLatestHyperBlockNo();

	static int GetLatestHyperBlock(HyperBlockDB &hyperblockDB);

	static int GetHyperBlockNumInfo(std::list<uint64> &HyperBlockNum);

};
#endif 