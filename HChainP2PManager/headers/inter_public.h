/*copyright 2016-2018 hyperchain.net (Hyperchain)
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

#ifndef __INTER_PUBLIC_H__
#define __INTER_PUBLIC_H__

#include<iostream>
#include<vector>
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#include <iconv.h>
#endif
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gen_int.h"
#include "includeComm.h"
using namespace std;

#define MAX_SECS_COUNTER                (9999999999)
#define MAX_SEND_NAT_TRAVERSAL_NODE_NUM (2)
#define MAX_SEND_PEERLIST_RSP_PERIOD	(5*60)
#define MAX_SAVE_PEERLIST_PERIOD		(30*60)
#define MAX_SEND_CHAIN_STATE_RSP_PERIOD	(5*60)
#define MAX_RECV_UDP_BUF_LEN			(30*1024)
#define	MAX_NATTRAVERSAL_PERIOD			(10*60)
#define RANDTIME						(60)
#define LOCALBUDDYTIME                  (1*60)
#define GLOBALBUDDYTIME                 (1.5*60)
#define NEXTBUDDYTIME					(2*60)
#define LIST_BUDDY_RSP_NUM				(3)
#define BUDDYSCRIPT						("buddy_script")
#define AUTHKEY							("auth_key")
#define FILESIZES						(256)
#define FILESIZEL						(512)
#define BUFFERLEN						(1024)
#define PORT							(8111)
#define MAGICNUMBER						(116444736000000000)
#define SECONDLEN						(10000000)
#define TEST_SERVER1					("http://192.168.0.2/api/nodeinfo?node=")
#define TEST_SERVER2					("http://192.168.0.3/api/nodeinfo?node=")
#define ONE_SECOND						(1000)
#define ONE_MIN						    (60*ONE_SECOND)
#define INIT_TIME						(10)
#define ONE_KILO						(1024)
#define FIVE_MINI						(5*60)  
#define ONE_HOUR						(60*60)
#define UNICODE_POS					    (2)
#define DIVIDEND_NUM					(2)
#define MAX_NUM_LEN						(256)
#define ONE_LOCAL_BLOCK				    (1)
#define NOT_START_BUDDY_NUM				(1)
#define LEAST_START_GLOBAL_BUDDY_NUM    (2)
#define HYPERBLOCK_SYNC_TIMES			(2)

enum _ePoeReqState
{
	DEFAULT_REGISREQ_STATE = 0,
	RECV,
	SEND,
	STOP,
	CONFIRMING,
	CONFIRMED,
	REJECTED,

};

enum _eblocktype
{
	HYPER_BLOCK = 1,
	LOCAL_BLOCK
};

enum _eNodeState
{
	DEFAULT_NODE_STATE = 0,
	SYNC_DATA_STATE,
	IDLE_STATE,
	LOCAL_BUDDY_STATE,
	GLOBAL_BUDDY_STATE,
	ON_CHAIN_SUCCESS,
	ON_CHAIN_FAILED,
	NODE_MALICE
};

enum _eChainState
{
	CHAIN_DEFAULT_STATE = 0,
	CHAIN_CONFIRMING,
	CHAIN_CONFIRMED
};

#pragma pack(push,1)
typedef struct _tLocalChain
{
	uint16	iId;
	uint64	iAllChainNodeNum;
	_eChainState	eState;
}TGETFRIENDCHAININFO, *P_TGETFRIENDCHAININFO;

typedef struct _tPoeInfo
{
	string				cFileName;
	string				cCustomInfo;
	string				cRightOwner;
	string				cFileHash;
	int16				iFileState;
	uint64				tRegisTime;
	uint64				iFileSize;
	uint64				iBlocknum;

	_tPoeInfo()
	{
		cFileName = "";
		cCustomInfo = "";
		cRightOwner = "";
		cFileHash = "";
		iFileSize = 0;
		iFileState = DEFAULT_REGISREQ_STATE;
		tRegisTime = 0;
		iBlocknum = 0;
	}

}TEVIDENCEINFO, *P_TEVIDENCEINFO;

typedef struct _tChainQueryStru
{
	uint64		iBlockNo;
	uint64		iJoinedNodeNum;
	uint64		iLocalBlockNum;
	uint16		iLocalChainNum;

	uint64		tTimeStamp;
	_tPoeInfo tPoeRecordInfo;

	_tChainQueryStru()
	{
		iBlockNo = 0;
		iLocalChainNum = 0;
		iLocalBlockNum = 0;
		iJoinedNodeNum = 0;

		tTimeStamp = 0;
	}

}TBROWSERSHOWINFO, *P_TBROWSERSHOWINFO;
#pragma pack(pop)

typedef struct _tUpqueue
{
	uint64 uiID;
	string strHash;
	uint64 uiTime;
}TUPQUEUE, *P_TUPQUEUE;

typedef struct _tBlockInfo
{
	uint64 iBlockNo;
	uint64 iCreatTime;
	_tPoeInfo tPoeRecordInfo;
}TBLOCKINFO, *P_TBLOCKINFO;

typedef struct _tHBlockDlgInfo
{
	uint64 iBlockNo;
	uint64 iCreatTime;
	uint64 iLocalBlockNum;
	string strHHash;
}THBLOCKDLGINFO, *P_THBLOCKDLGINFO;

typedef struct _tNodeInfo
{
	uint64 uiNodeState;
	string strNodeIp;
}TNODEINFO, *P_TNODEINFO;

typedef struct _tBlockPersistStru
{
	uint8  ucBlockType;

	uint64 uiBlockId;
	uint64 uiReferHyperBlockId;
	uint64 uiBlockTimeStamp;
	uint64 uiLocalChainId;

	unsigned char strHyperBlockHash[DEF_SHA256_LEN];
	unsigned char strPreHash[DEF_SHA256_LEN];
	string strScript;
	string strAuth;
	unsigned char strHashSelf[DEF_SHA256_LEN];
	unsigned char strHashAll[DEF_SHA256_LEN];

	string strPayload;

	uint64 uiQueueID;
	_tBlockPersistStru()
	{
		ucBlockType = 0;
		uiBlockId = 0;
		uiBlockTimeStamp = 0;
		uiLocalChainId = 0;
		uiQueueID = 0;

		memset(strHyperBlockHash, 0, DEF_SHA256_LEN);
		memset(strPreHash, 0, DEF_SHA256_LEN);
		strScript = "";
		strAuth = "";
		memset(strHashSelf, 0, DEF_SHA256_LEN);
		memset(strHashAll, 0, DEF_SHA256_LEN);
		strPayload = "";
	}
}T_HYPERBLOCKDBINFO, *T_PHYPERBLOCKDBINFO;

typedef vector<T_HYPERBLOCKDBINFO>				VEC_T_HYPERBLOCKDBINFO;
typedef VEC_T_HYPERBLOCKDBINFO::iterator		ITR_VEC_T_HYPERBLOCKDBINFO;

typedef vector<P_TBLOCKINFO>					VEC_T_BLOCKINFO;
typedef VEC_T_BLOCKINFO::iterator				ITR_VEC_T_BLOCKINFO;

typedef vector<TNODEINFO>						VEC_T_NODEINFO;
typedef VEC_T_NODEINFO::iterator				ITR_VEC_T_NODEINFO;

typedef vector<TBROWSERSHOWINFO>				VEC_T_BROWSERSHOWINFO;
typedef VEC_T_BROWSERSHOWINFO::iterator			ITR_VEC_T_BROWSERSHOWINFO;

typedef vector<P_TEVIDENCEINFO>					VEC_T_EVIDENCEINFO;
typedef VEC_T_EVIDENCEINFO::iterator			ITR_VEC_T_EVIDENCEINFO;

typedef vector<P_TUPQUEUE>						VEC_T_UPQUEUEINFO;
typedef VEC_T_UPQUEUEINFO::iterator				ITR_VEC_T_UPQUEUEINFO;

typedef vector<P_THBLOCKDLGINFO>				VEC_T_HBLOCKDLGINFO;
typedef VEC_T_HBLOCKDLGINFO::iterator			ITR_VEC_T_HBLOCKDLGINFO;

#endif