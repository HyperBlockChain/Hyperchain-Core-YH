﻿/*copyright 2016-2018 hyperchain.net (Hyperchain)
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
#ifndef __COMMON_STRUCT_H__
#define __COMMON_STRUCT_H__

#include "includeComm.h"
#include "gen_int.h"
#include "inter_public.h"
#include "../utility/MutexObj.h"
#include "../utility/event_impl.h"

using namespace std;

#define DEF_ONE_DAY     (60 * 60 * 24)
#define MAX_IP_LEN		(32)
#define MAX_RECV_BUF_LEN (1024)
#define MAX_SEND_BUF_LEN (1024)
#define MAX_FILE_NAME_LEN (1024)
#define MAX_NODE_NAME_LEN (64)
#define MAX_CUSTOM_INFO_LEN (512)
#define MAX_SCRIPT_LEN (1024*2)
#define MAX_AUTH_LEN (64)
#define MAX_QUEED_LEN (32)
#define LISTEN_PORT (8115)

typedef struct _tsha256
{
	unsigned char pID[DEF_SHA256_LEN];

	_tsha256 operator = (const _tsha256& arRes)
	{
		if(this != &arRes)
		{
			for(int i=0; i<DEF_SHA256_LEN; i++)
				pID[i] = arRes.pID[i];
		}
		return *this;
	}

	bool operator == (const _tsha256& arRes) const
	{
		if(this == &arRes)
			return true;

		for(int i=0; i<DEF_SHA256_LEN; i++)
		{
			if(pID[i] != arRes.pID[i])
				return false;
		}
		return true;
	}

	bool operator < (const _tsha256& arRes) const
	{
		if(this == &arRes)
			return false;

		for(int i=0; i<DEF_SHA256_LEN; i++)
		{
			if(pID[i] < arRes.pID[i])
			{
				return true;
			}
			else if(pID[i] == arRes.pID[i])
			{
				continue;
			}
			else
				return false;

		}
		return false;
	}

	bool operator > (const _tsha256& arRes) const
	{
		if(this == &arRes)
			return false;

		for(int i=0; i<DEF_SHA256_LEN; i++)
		{
			if(pID[i] > arRes.pID[i])
			{
				return true;
			}
			else if(pID[i] == arRes.pID[i])
			{
				continue;
			}
			else
				return false;

		}
		return false;
	}

}T_SHA256, *T_PSHA256;

typedef struct _tsha512
{
	unsigned char pID[DEF_SHA512_LEN];

	_tsha512 operator = (const _tsha512& arRes)
	{
		if (this != &arRes)
		{
			for (int i = 0; i<DEF_SHA512_LEN; i++)
				pID[i] = arRes.pID[i];
		}
		return *this;
	}

	bool operator == (const _tsha512& arRes) const
	{
		if (this == &arRes)
			return true;

		for (int i = 0; i<DEF_SHA512_LEN; i++)
		{
			if (pID[i] != arRes.pID[i])
				return false;
		}
		return true;
	}

	bool operator < (const _tsha512& arRes) const
	{
		if (this == &arRes)
			return false;

		for (int i = 0; i<DEF_SHA512_LEN; i++)
		{
			if (pID[i] < arRes.pID[i])
			{
				return true;
			}
			else if (pID[i] == arRes.pID[i])
			{
				continue;
			}
			else
				return false;

		}
		return false;
	}

	bool operator > (const _tsha512& arRes)
	{
		if (this == &arRes)
			return false;

		for (int i = 0; i<DEF_SHA512_LEN; i++)
		{
			if (pID[i] > arRes.pID[i])
			{
				return true;
			}
			else if (pID[i] == arRes.pID[i])
			{
				continue;
			}
			else
				return false;

		}
		return false;
	}

}T_SHA512, *T_PSHA512;

#pragma pack(1)

enum _ep2pprotocoltypestate
{
	DEFAULT_STATE = 0,
	SEND_ON_CHAIN_RSP,
	RECV_ON_CHAIN_RSP,
	SEND_CONFIRM,
	RECV_CONFIRM,
	SEND_CONFIRM_RSP,
	RECV_CONFIRM_RSP
};

enum _erecvpagestate
{
	DEFAULT_RECV_STATE = 0,
	RECV_RSP,
	RECV_REQ
};
enum _eerrorno
{
	DEFAULT_ERROR_NO = 0,
	ERROR_NOT_NEWEST,
	ERROR_EXIST
};

typedef struct _tpeeraddress
{

	uint32	uiIP;
	uint32	uiPort;

	_tpeeraddress& operator = (const _tpeeraddress& arRes)
	{
		if (this != &arRes)
		{
			uiIP = arRes.uiIP;
			uiPort = arRes.uiPort;
		}

		return *this;
	}
}T_PEERADDRESS, *T_PPEERADDRESS;

typedef struct _tblockbaseinfo
{
	uint64 uiID;
	T_SHA256 tPreHash;
	uint64 uiTime;
	int8  strScript[MAX_SCRIPT_LEN];
	int8  strAuth[MAX_AUTH_LEN];
	T_SHA256 tHashSelf;

	_tblockbaseinfo()
	{
		uiID = 0;
		uiTime = 0;
		memset(strScript, 0, MAX_SCRIPT_LEN);
		memset(strAuth, 0, MAX_AUTH_LEN);
		memset(tPreHash.pID, 0, DEF_SHA256_LEN);
		memset(tHashSelf.pID, 0, DEF_SHA256_LEN);
	}

	_tblockbaseinfo& operator = (const _tblockbaseinfo& arRes)
	{
		if (this != &arRes)
		{
			uiID = arRes.uiID;
			uiTime = arRes.uiTime;
			tPreHash = arRes.tPreHash;
			tHashSelf = arRes.tHashSelf;
			memcpy(strScript, arRes.strScript, MAX_SCRIPT_LEN);
			memcpy(strAuth, arRes.strAuth, MAX_AUTH_LEN);
		}

		return *this;
	}

}T_BLOCKBASEINFO, *T_PBLOCKBASEINFO;

typedef struct _tblockbaseinfonew
{
	uint64 uiID;
	T_SHA256 tPreHash;
	uint64 uiTime;
	int8 *strScript;
	uint64 scriptLength;
	int8  strAuth[MAX_AUTH_LEN];
	T_SHA256 tHashSelf;

	_tblockbaseinfonew()
	{
		uiID = 0;
		uiTime = 0;
		strScript = NULL;
		scriptLength = 0;
		memset(strAuth, 0, MAX_AUTH_LEN);
		memset(tPreHash.pID, 0, DEF_SHA256_LEN);
		memset(tHashSelf.pID, 0, DEF_SHA256_LEN);
	}

	_tblockbaseinfonew& operator = (const _tblockbaseinfonew& arRes)
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

}T_BLOCKBASEINFONEW, *T_PBLOCKBASEINFONEW;

typedef struct _fileinfo
{
	uint64 uiFileSize;
	char fileName[MAX_FILE_NAME_LEN];
	char customInfo[MAX_CUSTOM_INFO_LEN];
	char tFileOwner[MAX_CUSTOM_INFO_LEN];
	T_SHA512 tFileHash;
	uint64 uiFileCreateTime;

	_fileinfo()
	{
		uiFileSize = 0;
		uiFileCreateTime = 0;
		memset(fileName, 0, MAX_FILE_NAME_LEN);
		memset(customInfo, 0, MAX_CUSTOM_INFO_LEN);
		memset(tFileHash.pID, 0, DEF_SHA512_LEN);
		memset(tFileOwner, 0, MAX_CUSTOM_INFO_LEN);
	}

	_fileinfo& operator = (const _fileinfo& arRes)
	{
		if (this != &arRes)
		{
			uiFileSize = arRes.uiFileSize;
			uiFileCreateTime = arRes.uiFileCreateTime;
			memcpy(fileName, arRes.fileName, MAX_FILE_NAME_LEN);
			memcpy(customInfo, arRes.customInfo, MAX_CUSTOM_INFO_LEN);
			memcpy(tFileHash.pID, arRes.tFileHash.pID, DEF_SHA512_LEN);
			memcpy(tFileOwner, arRes.tFileOwner, MAX_CUSTOM_INFO_LEN);
		}
		return *this;
	}
}T_FILEINFO, *T_PFILEINFO;

typedef struct _tprivateblock
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHHash;

	T_FILEINFO tPayLoad;

	_tprivateblock()
	{
		memset(tHHash.pID, 0, DEF_SHA256_LEN);
	}

	_tprivateblock& operator = (const _tprivateblock& arRes)
	{
		if (this != &arRes)
		{
			tHHash = arRes.tHHash;
			tBlockBaseInfo = arRes.tBlockBaseInfo;
			tPayLoad = arRes.tPayLoad;

		}

		return *this;
	}

}T_PRIVATEBLOCK, *T_PPRIVATEBLOCK;

typedef struct _tlocalblock
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHHash;
	uint64  uiAtChainNum;

	T_PRIVATEBLOCK tPayLoad;

	_tlocalblock()
	{
		memset(tHHash.pID, 0, DEF_SHA256_LEN);
		uiAtChainNum = 1;
	}

	_tlocalblock& operator = (const _tlocalblock& arRes)
	{
		if (this != &arRes)
		{
			tHHash = arRes.tHHash;
			tBlockBaseInfo = arRes.tBlockBaseInfo;
			uiAtChainNum = arRes.uiAtChainNum;
			tPayLoad = arRes.tPayLoad;

		}

		return *this;
	}
}T_LOCALBLOCK, *T_PLOCALBLOCK;

typedef struct _tlocalblocknew
{
	_tblockbaseinfonew   tBlockBaseInfo;
	T_SHA256 tHHash;
	uint64  uiAtChainNum;
	int8 *userDefine;
	uint64 userDefineLength;

	_tlocalblocknew()
	{
		userDefine = NULL;
		userDefineLength = 0;
		memset(tHHash.pID, 0, DEF_SHA256_LEN);
		uiAtChainNum = 1;

		SetUserDefineDefault();
	}

	void SetUserDefineDefault()
	{
		class CBlockHeader
		{
		public:

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

	_tlocalblocknew& operator = (const _tlocalblocknew& arRes)
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
}T_LOCALBLOCKNEW, *T_PLOCALBLOCKNEW;

typedef list<T_LOCALBLOCK> LIST_T_LOCALBLOCK;
typedef LIST_T_LOCALBLOCK::iterator ITR_LIST_T_LOCALBLOCK;
typedef list<T_LOCALBLOCKNEW> LIST_T_LOCALBLOCKNEW;
typedef LIST_T_LOCALBLOCKNEW::iterator ITR_LIST_T_LOCALBLOCKNEW;

typedef struct _thyperblock
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHashAll;

	list<LIST_T_LOCALBLOCK> listPayLoad;

	_thyperblock()
	{
		memset(tHashAll.pID, 0, DEF_SHA256_LEN);
	}

	_thyperblock& operator = (const _thyperblock& arRes)
	{
		if (this != &arRes)
		{
			tHashAll = arRes.tHashAll;
			tBlockBaseInfo = arRes.tBlockBaseInfo;

			{
				list<LIST_T_LOCALBLOCK>::iterator itr = (const_cast<_thyperblock&>(arRes)).listPayLoad.begin();
				for (; itr != (const_cast<_thyperblock&>(arRes)).listPayLoad.end(); itr++)
				{
					list<T_LOCALBLOCK> vectTemp;
					list<T_LOCALBLOCK>::iterator subItr = (*itr).begin();

					for (; subItr != (*itr).end(); subItr++)
					{
						vectTemp.push_back(*subItr);
					}

					listPayLoad.push_back(vectTemp);
				}
			}
		}

		return *this;
	}
}T_HYPERBLOCK, *T_PHYPERBLOCK;

typedef enum merkle_tree_error {
	SUCCESS = 0,
	OUT_Of_MEMORY,
	ILLEGAL_PARAM,
	ILLEGAL_STATE,
	ROOT_MISMATCH,
	UNSPECIFIED
} mt_error;

class CMerkleTree
{
#define HASH_LENGTH   32
#define TREE_LEVELS   20
#define MAX_ELEMS     524288

#define ERROR_CHECK(f) do {mt_error r = f;if (r != SUCCESS) {return r;}} while (0)

private:

	typedef struct merkle_tree_array_list {
		uint32 elems;
		T_SHA256 *data;

		merkle_tree_array_list()
		{
			elems = 0;
			data = NULL;
		}
	} mt_al;

	class CMerkleTreeArrayList
	{
	public:
		CMerkleTreeArrayList();
		CMerkleTreeArrayList(mt_al*mtal);
		~CMerkleTreeArrayList();

		mt_al *create(void);

		void remove();

		mt_error add(const T_SHA256&h);

		mt_error update(const T_SHA256&h, uint32 offset);

		mt_error addOrUpdate(const T_SHA256&h, uint32 offset);

		const T_SHA256* getOffset(uint32 offset);

		inline uint32 getSize()
		{
			if (!m_merkleTreeArrayList) return 0;

			return m_merkleTreeArrayList->elems;
		}

	private:

		int isPowerOfTwo(uint32_t v);

	private:
		mt_al* m_merkleTreeArrayList;

	};

	typedef struct merkle_tree {
		uint32 elems;
		CMerkleTreeArrayList *levels[TREE_LEVELS];

		merkle_tree()
		{
			elems = 0;
			for (size_t i = 0; i<TREE_LEVELS; i++)
				levels[i] = NULL;
		}
	} mt_t;

public:
	mt_t* create(void);

	void remove();

	mt_error add(const T_SHA256*v, size_t count);

	uint32 getSize();

	mt_error verify(const T_SHA256&v, uint32 offset);

	mt_error getRoot(T_SHA256&root);

private:
	mt_error add(const T_SHA256&v);

	int exists(uint32 offset);

	uint32 hasNextLevel(uint32 curLevel);

	const T_SHA256* findRight(uint32 offset, int32 level);

	int getRight(uint32 offset);

	int getLeft(uint32 offset);

	static mt_error hash(const T_SHA256 left, const T_SHA256 right,
		T_SHA256&messageDigest);

private:
	mt_t*  m_merkleTree;

};

typedef struct _thyperblocknew
{
	_tblockbaseinfonew   tBlockBaseInfo;
	T_SHA256 tHashAll;
	list<LIST_T_LOCALBLOCKNEW> listPayLoad;
	T_SHA256 merkleRoot;
	std::vector<T_SHA256> leaves;

	_thyperblocknew()
	{
		memset(tHashAll.pID, 0, DEF_SHA256_LEN);
	}

	_thyperblocknew(const _thyperblock&block)
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
				ltl.push_back(tlb);
			}
			listPayLoad.push_back(ltl);
		}
		tBlockBaseInfo.tPreHash = block.tBlockBaseInfo.tPreHash;
		tBlockBaseInfo.tHashSelf = block.tBlockBaseInfo.tHashSelf;
		tBlockBaseInfo.uiID = block.tBlockBaseInfo.uiID;
		tBlockBaseInfo.uiTime = block.tBlockBaseInfo.uiTime;
		memcpy(tBlockBaseInfo.strAuth, block.tBlockBaseInfo.strAuth, MAX_AUTH_LEN);

		getRoot();
	}

	_thyperblocknew& operator = (const _thyperblocknew& arRes)
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

	std::vector<T_SHA256> getLeavesHash()const
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

	void getRoot()
	{
		leaves = getLeavesHash();
		if (leaves.size()<2) return;
		CMerkleTree merkleTree;
		merkleTree.create();
		merkleTree.add(&leaves[0], leaves.size());
		merkleTree.getRoot(merkleRoot);
		merkleTree.remove();
		tHashAll = merkleRoot;
	}

	void clear()
	{
		list<LIST_T_LOCALBLOCKNEW>::iterator itrOuter = listPayLoad.begin();
		for (; itrOuter != listPayLoad.end(); itrOuter++)
		{
			itrOuter->clear();
		}
		listPayLoad.clear();
		leaves.clear();
	}

}T_HYPERBLOCKNEW, *T_PHYPERBLOCKNEW;

typedef struct _thyperblocksend
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHashAll;

	_thyperblocksend()
	{
		memset(tHashAll.pID, 0, DEF_SHA256_LEN);
	}

	_thyperblocksend& operator = (const _thyperblocksend& arRes)
	{
		if (this != &arRes)
		{
			tHashAll = arRes.tHashAll;
			tBlockBaseInfo = arRes.tBlockBaseInfo;
		}
		return *this;
	}
}T_HYPERBLOCKSEND, *T_PHYPERBLOCKSEND;

typedef struct _tchainStateinfo
{
	uint64 uiBlockNum;

	_tchainStateinfo& operator = (const _tchainStateinfo& arRes)
	{
		if (this != &arRes)
		{
			uiBlockNum = arRes.uiBlockNum;
		}
		return *this;
	}
}T_CHAINSTATEINFO, *T_PCHAINSTATEINFO;

typedef struct _tpeerinfo
{
	T_PEERADDRESS tPeerInfoByMyself;
	T_PEERADDRESS tPeerInfoByOther;
	uint16 uiState;
	uint16 uiNatTraversalState;
	uint64 uiTime;
	int8 strName[MAX_NODE_NAME_LEN];
	uint16 uiNodeState;

	_tpeerinfo()
	{
		tPeerInfoByMyself.uiIP = 0;
		tPeerInfoByMyself.uiPort = LISTEN_PORT;
		tPeerInfoByOther.uiIP = 0;
		tPeerInfoByOther.uiPort = LISTEN_PORT;
		uiState = 0;
		uiNatTraversalState = 0;
		uiTime = 0;
		uiNodeState = DEFAULT_REGISREQ_STATE;
		memset(strName, 0, MAX_NODE_NAME_LEN);
	}

	_tpeerinfo& operator = (const _tpeerinfo& arRes)
	{
		if (this != &arRes)
		{
			tPeerInfoByMyself.uiIP = arRes.tPeerInfoByMyself.uiIP;
			tPeerInfoByMyself.uiPort = arRes.tPeerInfoByMyself.uiPort;
			tPeerInfoByOther.uiIP = arRes.tPeerInfoByOther.uiIP;
			tPeerInfoByOther.uiPort = arRes.tPeerInfoByOther.uiPort;
			uiState = arRes.uiState;
			uiNatTraversalState = arRes.uiNatTraversalState;
			uiTime = arRes.uiTime;
			strncpy(strName, arRes.strName, MAX_NODE_NAME_LEN);
			uiNodeState = arRes.uiNodeState;

		}
		return *this;
	}
}T_PEERINFO, *T_PPEERINFO;

typedef struct _tblockstateaddr
{
	T_PEERADDRESS tPeerAddr;
	T_PEERADDRESS tPeerAddrOut;

	_tblockstateaddr& operator = (const _tblockstateaddr& arRes)
	{
		if (this != &arRes)
		{
			tPeerAddr = arRes.tPeerAddr;
			tPeerAddrOut = arRes.tPeerAddrOut;

		}
		return *this;
	}
}T_BLOCKSTATEADDR, *T_PBLOCKSTATEADDR;

typedef struct _tlocalconsensus
{
	T_BLOCKSTATEADDR tPeer;
	T_LOCALBLOCK  tLocalBlock;
	char strID[MAX_QUEED_LEN];
	uint64 uiRetryTime;
	char strFileHash[DEF_SHA512_LEN+1];

	_tlocalconsensus()
	{

		memset(strID, 0, MAX_QUEED_LEN);
		memset(strFileHash, 0, DEF_SHA512_LEN + 1);

		uiRetryTime = 0;

	}

	_tlocalconsensus& operator = (const _tlocalconsensus& arRes)
	{
		if (this != &arRes)
		{
			tPeer = arRes.tPeer;

			tLocalBlock = arRes.tLocalBlock;
			memcpy(strID, arRes.strID, MAX_QUEED_LEN);
			memcpy(strFileHash, arRes.strFileHash, DEF_SHA512_LEN + 1);
			uiRetryTime = arRes.uiRetryTime;

		}
		return *this;
	}
}T_LOCALCONSENSUS, *T_PLOCALCONSENSUS;

typedef struct _tglobalconsenus
{
	T_BLOCKSTATEADDR tPeer;
	T_LOCALBLOCK  tLocalBlock;
	uint64 uiAtChainNum;
}T_GLOBALCONSENSUS, *T_PGLOBALCONSENSUS;

typedef struct _tbuddyinfo
{
	uint8 tType;
	uint8 bufLen;
	char *recvBuf;
	T_PEERADDRESS tPeerAddrOut;
}T_BUDDYINFO, *T_PBUDDYINFO;

typedef list<T_LOCALCONSENSUS> LIST_T_LOCALCONSENSUS;
typedef LIST_T_LOCALCONSENSUS::iterator ITR_LIST_T_LOCALCONSENSUS;

typedef list<T_PLOCALCONSENSUS> LIST_T_PLOCALCONSENSUS;
typedef LIST_T_PLOCALCONSENSUS::iterator ITR_LIST_T_PLOCALCONSENSUS;

typedef struct _tbuddyinfostate
{

	int8 strBuddyHash[DEF_STR_HASH256_LEN];
	uint8 uibuddyState;

	T_PEERADDRESS tPeerAddrOut;

	LIST_T_LOCALCONSENSUS localList;
	_tbuddyinfostate()
	{
		memset(strBuddyHash, 0, DEF_STR_HASH256_LEN);
		uibuddyState = DEFAULT_STATE;
	}
}T_BUDDYINFOSTATE, *T_PBUDDYINFOSTATE;

typedef struct _tsearchinfo
{

	uint64 uiHyperID;
	uint64 uiTime;
}T_SEARCHINFO, *T_PSEARCHINFO;

typedef list<LIST_T_LOCALCONSENSUS> LIST_LIST_GLOBALBUDDYINFO;
typedef LIST_LIST_GLOBALBUDDYINFO::iterator ITR_LIST_LIST_GLOBALBUDDYINFO;

typedef list<T_BUDDYINFO> LIST_T_BUDDYINFO;
typedef LIST_T_BUDDYINFO::iterator ITR_LIST_T_BUDDYINFO;

typedef list<T_PBUDDYINFOSTATE> LIST_T_PBUDDYINFOSTATE;
typedef LIST_T_PBUDDYINFOSTATE::iterator ITR_LIST_T_PBUDDYINFOSTATE;

typedef list<T_BUDDYINFOSTATE> LIST_T_BUDDYINFOSTATE;
typedef LIST_T_BUDDYINFOSTATE::iterator ITR_LIST_T_BUDDYINFOSTATE;

typedef map<string,T_SEARCHINFO> MAP_T_SEARCHONCHAIN;
typedef MAP_T_SEARCHONCHAIN::iterator ITR_MAP_T_SEARCHONCHAIN;

#pragma pack()

typedef list<T_PPEERINFO> LIST_T_PPEERINFO;
typedef LIST_T_PPEERINFO::iterator ITR_LIST_T_PPEERINFO;

typedef list<T_HYPERBLOCK> LIST_T_HYPERBLOCK;
typedef LIST_T_HYPERBLOCK::iterator ITR_LIST_T_HYPERBLOCK;
typedef list<T_HYPERBLOCKNEW> LIST_T_HYPERBLOCKNEW;
typedef LIST_T_HYPERBLOCKNEW::iterator ITR_LIST_T_HYPERBLOCKNEW;

typedef list<T_PBLOCKSTATEADDR> LIST_T_PBLOCKSTATEADDR;
typedef LIST_T_PBLOCKSTATEADDR::iterator ITR_LIST_T_PBLOCKSTATEADDR;

typedef map<uint64, LIST_T_PBLOCKSTATEADDR> MAP_BLOCK_STATE;
typedef MAP_BLOCK_STATE::iterator ITR_MAP_BLOCK_STATE;

typedef struct _tpeerconf
{
	T_PEERADDRESS tPeerAddr;
	T_PEERADDRESS tPeerAddrOut;
	uint16 uiPeerState;
	int8 strName[MAX_NODE_NAME_LEN];
}T_PEERCONF, *T_PPEERCONF;

typedef std::vector<T_PPEERCONF>	VEC_T_PPEERCONF;
typedef VEC_T_PPEERCONF::iterator   ITR_VEC_T_PPEERCONF;

typedef struct _tconffile
{
	uint16			uiSaveNodeNum;
	uint32			uiLocalIP;
	uint32			uiLocalPort;
	string          strLocalNodeName;
	string			strLogDir;
	VEC_T_PPEERCONF vecPeerConf;
}T_CONFFILE, *T_PCONFFILE;

class CCommonStruct
{
private:

	CCommonStruct();
	virtual ~CCommonStruct();

public:
	void static gettimeofday_update(struct timeval *ptr);
	static int CompareHash(const T_SHA256& arhashLocal, const T_SHA256& arhashGlobal);
	static void Hash256ToStr(char* getStr, T_PSHA256 phash);
	static void StrToHash256(unsigned char *des, char* getStr);
	static void Hash512ToStr(char* getStr, T_PSHA512 phash);
	static void hash512tostring(char* out, const char* szHash);
	static void StrToHash512(unsigned char *des, char* getStr);
	static T_SHA256 DistanceHash(const T_SHA256& arLeft, const T_SHA256& arRight);
	static string HashToString(const T_SHA256& arGuid);
	static T_SHA256 StringToHash(const char *str);
	static void ReplaceAll(string& str,const string& old_value,const string& new_value);
	static void ReparePath(string& astrPath);

	static bool ReadConfig();
	static string GetLocalIp();
	static char* Time2String(time_t time1);
private:
#ifdef WIN32
	void static win_gettimeofday(struct timeval *tp);
#endif	
};

extern T_CONFFILE	g_confFile;

#endif
