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
using namespace std;

#define MAX_SECS_COUNTER (9999999999)

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

	_tPoeInfo()
	{
		cFileName = "";
		cCustomInfo = "";
		cRightOwner = "";
		cFileHash = "";
		iFileSize = 0;
		iFileState = DEFAULT_REGISREQ_STATE; 
		tRegisTime = 0;   
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

}TQUERYSHOWINFO, *P_TQUERYSHOWINFO;
#pragma pack(pop)

typedef struct _tBlockInfo
{
	uint64 iBlockNo;
	uint64 iCreatTime;
	_tPoeInfo tPoeRecordInfo;
}TBLOCKINFO, *P_TBLOCKINFO;    

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

	string strHyperBlockHash;
	string strPreHash;  
	string strScript;
	string strAuth;
	string strHashSelf;
	string strHashAll; 

	string strPayload;

	_tBlockPersistStru()
	{
		ucBlockType = 0;
		uiBlockId = 0;
		uiBlockTimeStamp = 0;
		uiLocalChainId = 0;

		strHyperBlockHash = "";
		strPreHash = "";
		strScript = "";
		strAuth = "";
		strHashSelf = "";
		strHashAll = "";
		strPayload = "";
	}
}T_HYPERBLOCKDBINFO, *T_PHYPERBLOCKDBINFO;


typedef vector<T_HYPERBLOCKDBINFO>				VEC_T_HYPERBLOCKDBINFO;
typedef VEC_T_HYPERBLOCKDBINFO::iterator		ITR_VEC_T_HYPERBLOCKDBINFO;

typedef vector<P_TBLOCKINFO>					VEC_T_BLOCKINFO;
typedef VEC_T_BLOCKINFO::iterator				ITR_VEC_T_BLOCKINFO;

typedef vector<TNODEINFO>						VEC_T_NODEINFO;
typedef VEC_T_NODEINFO::iterator				ITR_VEC_T_NODEINFO;

typedef vector<TQUERYSHOWINFO>				VEC_T_QUERYSHOWINFO;
typedef VEC_T_QUERYSHOWINFO::iterator			ITR_VEC_T_QUERYSHOWINFO;

typedef vector<P_TEVIDENCEINFO>					VEC_T_EVIDENCEINFO;
typedef VEC_T_EVIDENCEINFO::iterator			ITR_VEC_T_EVIDENCEINFO;

#endif