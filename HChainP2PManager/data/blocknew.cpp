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

#include "../headers/commonstruct.h"
#include "../headers/merkle.h"

_tblockbaseinfonew& _tblockbaseinfonew::operator = (const _tblockbaseinfonew& arRes)
{
	if (this != &arRes)
	{
		uiID = arRes.uiID;
		uiTime = arRes.uiTime;
		tPreHash = arRes.tPreHash;
		tHashSelf = arRes.tHashSelf;
		memcpy(strAuth, arRes.strAuth, MAX_AUTH_LEN);
		scriptLength = arRes.scriptLength;
		if (strScript) free(strScript);
		if (arRes.scriptLength > 0)
			strScript = (int8*)malloc(arRes.scriptLength*sizeof(int8));
		else
			return *this;
		memcpy(strScript, arRes.strScript, arRes.scriptLength*sizeof(int8));
	}

	return *this;
}

_tblockbaseinfonew::_tblockbaseinfonew(uint64 ID, T_SHA256 preHash, uint64 t, int8* script, uint64 scLen, int8 auth[], T_SHA256 hashSelf)
{
	Set(ID, preHash, t, script, scLen, auth, hashSelf);
}

_tblockbaseinfonew::~_tblockbaseinfonew()
{
	if (strScript) free(strScript);
}

void _tblockbaseinfonew::Set(uint64 ID, T_SHA256 preHash, uint64 t, int8* script, uint64 scLen, int8 auth[], T_SHA256 hashSelf)
{
	uiID = ID;
	tPreHash = preHash;
	uiTime = t;	
	if (strScript) free(strScript);
	strScript = (int8*)malloc(scLen*sizeof(int8));
	memcpy(strScript, script, scLen);
	scriptLength = scLen;
	memcpy(strAuth, auth, MAX_AUTH_LEN);
	tHashSelf = hashSelf;
}


uint64 _tblockbaseinfonew::GetID()const
{
	return uiID;
}

uint64 _tblockbaseinfonew::GetScriptLength()const
{
	return scriptLength;
}

T_SHA256 _tblockbaseinfonew::GetPreviousHash()const
{
	return tPreHash;
}

T_SHA256 _tblockbaseinfonew::GetSelfHash()const
{
	return tHashSelf;
}

void _tblockbaseinfonew::SetScript(int8*script,uint64 scLen)
{
	if (strScript) free(strScript);
	scriptLength = scLen;
	strScript = (int8*)malloc(scLen*sizeof(int8));
	memcpy(strScript, script, scLen);
}

void _tblockbaseinfonew::SetAuth(int8 auth[])
{
	memcpy(strAuth, auth, MAX_AUTH_LEN);
}


void _tlocalblocknew::SetUserDefineDefault()
{
	class CBlockHeader
	{
	public:
		// header
		int32_t nVersion;
		T_SHA256 hashPrevBlock;
		T_SHA256 hashMerkleRoot;
		uint32_t nTime;
		uint32_t nBits;
		uint32_t nNonce;
	};
	
	userDefineLength = sizeof(CBlockHeader);
	CBlockHeader cbh;
	cbh.nVersion = 1;
	cbh.nBits = 32;
	userDefine = (int8*)malloc(sizeof(CBlockHeader));
	memcpy(userDefine, &cbh, sizeof(CBlockHeader));
}

_tlocalblocknew& _tlocalblocknew::operator = (const _tlocalblocknew& arRes)
{
	if (this != &arRes)
	{
		tHHash = arRes.tHHash;
		tBlockBaseInfo = arRes.tBlockBaseInfo;
		uiAtChainNum = arRes.uiAtChainNum;
		userDefineLength = arRes.userDefineLength;
		if (userDefine) free(userDefine);
		userDefine = (int8*)malloc(sizeof(int8)*arRes.userDefineLength);
		memcpy(userDefine, arRes.userDefine, userDefineLength*sizeof(int8));
	}

	return *this;
}

_tlocalblocknew::_tlocalblocknew(const _tblockbaseinfonew&bbi, T_SHA256 HHash, uint64 atChainNum, int8*ud, uint64 udLen)
{
	Set(bbi, HHash, atChainNum, ud, udLen);
}

_tlocalblocknew::~_tlocalblocknew()
{
	if (userDefine) free(userDefine);
	userDefine = NULL;
}

void _tlocalblocknew::Set(const _tblockbaseinfonew&bbi, T_SHA256 HHash, uint64 atChainNum, int8*ud, uint64 udLen)
{
	tBlockBaseInfo = bbi;
	tHHash = HHash;
	uiAtChainNum = atChainNum;
	if (userDefine) free(userDefine);
	userDefineLength = udLen;
	userDefine = (int8 *)malloc(udLen*sizeof(int8));
	memcpy(userDefine, ud, udLen*sizeof(int8));
}

void _tlocalblocknew::SetUserDefine(int8*ud, uint64 udLen)
{
	if (userDefine) free(userDefine);
	userDefineLength = udLen;
	userDefine = (int8 *)malloc(udLen*sizeof(int8));
	memcpy(userDefine, ud, udLen*sizeof(int8));
}

T_SHA256 _tlocalblocknew::GetParentHash()const
{
	return tHHash;
}

uint64 _tlocalblocknew::WhichChainAt()const
{
	return uiAtChainNum;
}

const _tblockbaseinfonew& _tlocalblocknew::GetBaseInfo()const
{
	return tBlockBaseInfo;
}


_thyperblocknew::_thyperblocknew(const _thyperblock&block)
{
	tHashAll = block.tHashAll;
	list<LIST_T_LOCALBLOCK>::const_iterator itrOuter = block.listPayLoad.begin();
	for (; itrOuter != block.listPayLoad.end(); itrOuter++)
	{
		LIST_T_LOCALBLOCK::const_iterator itrInner = (*itrOuter).begin();
		LIST_T_LOCALBLOCKNEW ltl;
		for (; itrInner != (*itrOuter).end(); itrInner++)
		{
			T_LOCALBLOCKNEW tlb;
			tlb.tHHash = (*itrInner).tHHash;
			tlb.uiAtChainNum = (*itrInner).uiAtChainNum;
			tlb.tBlockBaseInfo.tHashSelf = (*itrInner).tBlockBaseInfo.tHashSelf;
			tlb.tBlockBaseInfo.tPreHash = (*itrInner).tBlockBaseInfo.tPreHash;
			tlb.tBlockBaseInfo.uiID = (*itrInner).tBlockBaseInfo.uiID;
			tlb.tBlockBaseInfo.uiTime = (*itrInner).tBlockBaseInfo.uiTime;
			tlb.userDefine = NULL;
			tlb.userDefineLength = 0;
			ltl.push_back(tlb);
		}
		listPayLoad.push_back(ltl);
	}
	tBlockBaseInfo.tPreHash = block.tBlockBaseInfo.tPreHash;
	tBlockBaseInfo.tHashSelf = block.tBlockBaseInfo.tHashSelf;
	tBlockBaseInfo.uiID = block.tBlockBaseInfo.uiID;
	tBlockBaseInfo.uiTime = block.tBlockBaseInfo.uiTime;
	memcpy(tBlockBaseInfo.strAuth, block.tBlockBaseInfo.strAuth, MAX_AUTH_LEN);

	GetRoot();
}

_thyperblocknew& _thyperblocknew::operator = (const _thyperblocknew& arRes)
{
	if (this != &arRes)
	{
		tHashAll = arRes.tHashAll;
		tBlockBaseInfo = arRes.tBlockBaseInfo;

		{
			list<LIST_T_LOCALBLOCKNEW>::iterator itr = (const_cast<_thyperblocknew&>(arRes)).listPayLoad.begin();
			for (; itr != (const_cast<_thyperblocknew&>(arRes)).listPayLoad.end(); itr++)
			{
				list<T_LOCALBLOCKNEW> vectTemp;
				list<T_LOCALBLOCKNEW>::iterator subItr = (*itr).begin();

				for (; subItr != (*itr).end(); subItr++)
				{
					vectTemp.push_back(*subItr);
				}

				listPayLoad.push_back(vectTemp);
			}
		}
		merkleRoot = arRes.merkleRoot;
		leaves = arRes.leaves;
	}

	return *this;
}

std::vector<T_SHA256> _thyperblocknew::GetLeavesHash()const
{
	std::vector<T_SHA256> hashes;
	list<LIST_T_LOCALBLOCKNEW>::const_iterator itr = listPayLoad.begin();
	for (; itr != listPayLoad.end(); itr++)
	{
		list<T_LOCALBLOCKNEW>::const_iterator subItr = (*itr).begin();
		for (; subItr != (*itr).end(); subItr++)
			hashes.push_back((*subItr).tBlockBaseInfo.tHashSelf);
	}

	return hashes;
}

void _thyperblocknew::GetRoot()
{
	leaves = GetLeavesHash();
	if (leaves.size()<2) return;
	CMerkleTree merkleTree;
	merkleTree.create();
	merkleTree.add(&leaves[0], leaves.size());
	merkleTree.getRoot(merkleRoot);
	merkleTree.remove();
	tHashAll = merkleRoot;
}

void _thyperblocknew::Clear()
{
	list<LIST_T_LOCALBLOCKNEW>::iterator itrOuter = listPayLoad.begin();
	for (; itrOuter != listPayLoad.end(); itrOuter++)
	{
		itrOuter->clear();
	}
	listPayLoad.clear();
	leaves.clear();
}

T_SHA256 _thyperblocknew::GetMerkleRoot()const
{
	return merkleRoot;
}

T_SHA256 _thyperblocknew::GetHashAll()const
{
	return tHashAll;
}

const _tblockbaseinfonew& _thyperblocknew::GetBaseInfo()const
{
	return tBlockBaseInfo;
}

