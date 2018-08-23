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
#ifndef __P2P_PROTOCOL_H__
#define __P2P_PROTOCOL_H__

#include "../headers/commonstruct.h" 

enum _ep2pprotocoltype
{
	P2P_PROTOCOL_PING_REQ = 0,
	P2P_PROTOCOL_PING_RSP,
	P2P_PROTOCOL_PEERLIST_REQ,
	P2P_PROTOCOL_PEERLIST_RSP,
	P2P_PROTOCOL_GET_BLOCK_STATE_REQ,
	P2P_PROTOCOL_GET_BLOCK_STATE_RSP,
	P2P_PROTOCOL_ADD_BLOCK_REQ,
	P2P_PROTOCOL_ON_CHAIN_REQ,
	P2P_PROTOCOL_ON_CHAIN_RSP,
	P2P_PROTOCOL_ON_CHAIN_CONFIRM,
	P2P_PROTOCOL_ON_CHAIN_CONFIRM_RSP,
	P2P_PROTOCOL_ON_CHAIN_CONFIRM_FIN,
	P2P_PROTOCOL_REFUSE_REQ,
	P2P_PROTOCOL_GLOBAL_BUDDY_REQ,
	P2P_PROTOCOL_GLOBAL_BUDDY_RSP,
	P2P_PROTOCOL_COPY_HYPER_BLOCK_REQ,
	P2P_PROTOCOL_COPY_HYPER_BLOCK_RSP,
	P2P_PROTOCOL_COPY_BLOCK_REQ,
	P2P_PROTOCOL_COPY_BLOCK_RSP,
	P2P_PROTOCOL_ADD_BLOCK_RSP,
	P2P_PROTOCOL_GET_BLOCK_REQ,
	P2P_PROTOCOL_GET_BLOCK_RSP,
	P2P_PROTOCOL_WANT_NAT_TRAVERSAL_REQ,
	P2P_PROTOCOL_WANT_NAT_TRAVERSAL_RSP,
	P2P_PROTOCOL_SOME_ONE_NAT_TRAVERSAL_TO_YOU_REQ,
	P2P_PROTOCOL_SOME_ONE_NAT_TRAVERSAL_TO_YOU_RSP,
	P2P_PROTOCOL_NAT_TRAVERSAL_REQ,
	P2P_PROTOCOL_NAT_TRAVERSAL_RSP,
	P2P_PROTOCOL_GET_HYPERBLOCK_BY_NO_REQ,
	P2P_PROTOCOL_GET_HYPERBLOCK_BY_NO_RSP
};

enum _ep2pnetstate
{
	DEFAULT_NET = 0,
	OUTNET,
	INNERNET
};

enum _ep2pnatTraversalstate
{
	DEFAULT_NATTRAVERSAL_STATE = 0,
	GOOD,
	BAD
};
enum _ep2pprotocolresult
{
	P2P_PROTOCOL_NULL=0,
	P2P_PROTOCOL_SUCCESS=1,
	P2P_PROTOCOL_FAILED=2,
	P2P_PROTOCOL_NOTGETPEER=3
};

#pragma pack(1)
typedef struct _tp2pprotocoltype
{
	uint8 ucType;
	uint64 uiTimeStamp;


	_tp2pprotocoltype(){}
	_tp2pprotocoltype(uint8 t, uint64 timeStamp);

	_tp2pprotocoltype& operator = (const _tp2pprotocoltype& arRes);
	void SetP2pprotocoltype(uint8 Type, uint64 TimeStamp);
	void SetType(uint8 Type);
	void SetTimeStamp(uint64 TimeStamp);
	uint8 GetType()const;
	uint64 GetTimeStamp()const;
}T_P2PPROTOCOLTYPE, *T_PP2PPROTOCOLTYPE;
//
typedef struct _tp2pprotocolrsp
{
	T_P2PPROTOCOLTYPE	tType;
	uint8				iResult;

	_tp2pprotocolrsp(){}
	_tp2pprotocolrsp(T_P2PPROTOCOLTYPE t, uint8 result);
	_tp2pprotocolrsp& operator = (const _tp2pprotocolrsp& arRes);
	void SetP2pprotocolrsp(T_P2PPROTOCOLTYPE Type, uint8 Result);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetResult(uint8 Result);
	T_P2PPROTOCOLTYPE GetType()const;
	uint8 GetResult()const;
}T_P2PPROTOCOLRSP, *T_PP2PPROTOCOLRSP;

typedef struct _tp2pprotocolpingreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;
	int8			strName[MAX_NODE_NAME_LEN];
	uint64	uiMaxBlockNum;
	uint16  uiNodeState;

	_tp2pprotocolpingreq& operator = (const _tp2pprotocolpingreq& arRes);
	void SetP2pprotocolpingreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, int8 *Name, uint64 MaxBlockNum, uint16 NodeState);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	void SetName(int8 * Name);
	void SetMaxBlockNum(uint64 MaxBlockNum);
	void SetnodeState(uint16 NodeState);
	
	T_P2PPROTOCOLTYPE GetType()const;
	T_PEERADDRESS GetPeerAddr()const;
	int8 *  GetName();
	uint64 GetMaxBlockNum()const;
	uint16 GetnodeState()const;
}T_P2PPROTOCOLPINGREQ, *T_PP2PPROTOCOLPINGREQ;

typedef struct _tp2pprotocolpingrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerOutAddr;
	uint64 uiMaxBlockNum;

	_tp2pprotocolpingrsp& operator = (const _tp2pprotocolpingrsp& arRes);
	void SetP2pprotocolpingrsp(T_P2PPROTOCOLRSP Result, T_PEERADDRESS PeerOutAddr, uint64 MaxBlockNum);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetPeeroutAddr(T_PEERADDRESS PeerOutAddr);
	void SetMaxBlockNum(uint64 MaxBlockNum);

	T_P2PPROTOCOLRSP GetResult()const;
	T_PEERADDRESS GetPeeroutAddr()const;
	uint64 GetMaxBlockNum()const;

}T_P2PPROTOCOLPINGRSP, *T_PP2PPROTOCOLPINGRSP;

typedef struct _tp2pprotocolgethyperblockbynoreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64 uiBlockNum;

	_tp2pprotocolgethyperblockbynoreq(){}
	_tp2pprotocolgethyperblockbynoreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum);
	_tp2pprotocolgethyperblockbynoreq& operator = (const _tp2pprotocolgethyperblockbynoreq& arRes);
	void SetP2pprotocolgethyperblockbynoreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetBlockNum(uint64 BlockNum);
	T_P2PPROTOCOLTYPE GetType()const;
	uint64 GetBlockNum()const;
}T_P2PPROTOCOLGETHYPERBLOCKBYNOREQ, *T_PP2PPROTOCOLGETHYPERBLOCKBYNOREQ;

typedef struct _tp2pprotocolgethyperblockbynorsp
{
	T_P2PPROTOCOLRSP tResult;

	_tp2pprotocolgethyperblockbynorsp& operator = (const _tp2pprotocolgethyperblockbynorsp& arRes);
	void SetResult(T_P2PPROTOCOLRSP Result);
	T_P2PPROTOCOLRSP GetResult()const;
}T_P2PPROTOCOLGETHYPERBLOCKBYNORSP, *T_PP2PPROTOCOLGETHYPERBLOCKBYNORSP;


typedef struct _tp2pprotocolpeerlistreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;
	uint64	uiMaxBlockNum;
	uint16 uiCount;
	int8 strName[MAX_NODE_NAME_LEN];
	uint16 uiNodeState;

	_tp2pprotocolpeerlistreq(){}
	_tp2pprotocolpeerlistreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, uint64 MaxBlockNum, int8 *Name, uint16 NodeState);
	_tp2pprotocolpeerlistreq& operator = (const _tp2pprotocolpeerlistreq& arRes);
	void SetP2pprotocolpeerlistreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, uint64 MaxBlockNum, uint16 Count, int8 *Name, uint16 NodeState);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	void SetMaxBlockNum(uint64 MaxBlockNum);
	void SetCount(uint16 Count);
	void SetName(int8 *Name);
	void SetNodeState(uint16 NodeState);

	T_P2PPROTOCOLTYPE GetType()const;
	T_PEERADDRESS GetPeerAddr()const;
	uint64 GetMaxBlockNum()const;
	uint16 GetCount()const;
	int8* GetName();
	uint16 GetNodeState()const;

}T_P2PPROTOCOLPEERLISTREQ, *T_PP2PPROTOCOLPEERLISTREQ;

typedef struct _tp2pprotocolpeerlistrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerAddr;
	int8			strName[MAX_NODE_NAME_LEN];
	uint16			 uiCount;
	uint64			 uiMaxBlockNum;
	uint16			 uiNodeState;

	_tp2pprotocolpeerlistrsp& operator = (const _tp2pprotocolpeerlistrsp& arRes);
	void SetP2pprotocolpeerlistrsp(T_P2PPROTOCOLRSP Result,	T_PEERADDRESS PeerAddr,int8 *Name,uint16 Count,uint64 MaxBlockNum,uint16 NodeState);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	void SetName(int8 *Name);
	void SetCount(uint16 Count);
	void SetMaxBlockNum(uint64 MaxBlockNum);
	void SetnodeState(uint16 NodeState);

	T_P2PPROTOCOLRSP GetResult()const;
	T_PEERADDRESS GetPeerAddr()const;
	int8 * GetName();
	uint16 GetCount()const;
	uint64 GetMaxBlockNum()const;
	uint16 GetnodeState()const;

}T_P2PPROTOCOLPEERLISTRSP, *T_PP2PPROTOCOLPEERLISTRSP;

typedef struct _tp2pprotocoladdblockreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64	uiBlockNum;
	uint16  uiSendTimes;
	uint64  uiBlockCount;

	_tp2pprotocoladdblockreq& operator = (const _tp2pprotocoladdblockreq& arRes);
	void SetP2pprotocoladdblockreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum, uint16 SendTimes, uint64 BlockCount);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetBlockNum(uint64 BlockNum);
	void SetSendTime(uint16 SendTimes);
	void SetBlockCount(uint64 BlockCount);

	T_P2PPROTOCOLTYPE GetType()const;
	uint64 GetBlockNum()const;
	uint16 GetSendTime()const;
	uint64 GetBlockCount()const;


}T_P2PPROTOCOLADDBLOCKREQ, *T_PP2PPROTOCOLADDBLOCKREQ;

typedef struct _tp2pprotocolonchainreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64 uiHyperBlockNum;
	uint64  uiBlockCount;

	_tp2pprotocolonchainreq& operator = (const _tp2pprotocolonchainreq& arRes);
	void SetP2pprotocolonchainreq(T_P2PPROTOCOLTYPE Type, uint64 HyperBlockNum, uint64  BlockCount);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetHyperBlockNum(uint64 HyperBlockNum);
	void SetBlockCount(uint64 BlockCount);
	T_P2PPROTOCOLTYPE GetType()const;
	uint64 GetHyperBlockNum()const;
	uint64 GetBlockCount()const;

}T_P2PPROTOCOLONCHAINREQ, *T_PP2PPROTOCOLONCHAINREQ;


typedef struct _tp2pprotocolonchainrsp
{
	T_P2PPROTOCOLRSP tResult;
	uint64 uiHyperBlockNum;

	uint64  uiBlockCount;

	int8 strHash[DEF_STR_HASH256_LEN];

	_tp2pprotocolonchainrsp& operator = (const _tp2pprotocolonchainrsp& arRes);
	void SetP2pprotocolonchainrsp(T_P2PPROTOCOLRSP Result, uint64 HyperBlockNum, uint64  BlockCount, int8 *Hash);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetHyperBlockNum(uint64 HyperBlockNum);
	void SetBlockCount(uint64 BlockCount);
	void SetHash(int8 *Hash);
	void SetInitHash(int Num);

	T_P2PPROTOCOLRSP GetResult()const;
	uint64 GetHyperBlockNum()const;
	uint64 GetBlockCount()const;
	int8 * GetHash();

}T_P2PPROTOCOLONCHAINRSP, *T_PP2PPROTOCOLONCHAINRSP;

typedef struct _tp2pprotocolonchainconfirm
{
	T_P2PPROTOCOLRSP tResult;
	int8 strHash[DEF_STR_HASH256_LEN];
	
	_tp2pprotocolonchainconfirm& operator = (const _tp2pprotocolonchainconfirm& arRes);
	void SetP2pprotocolonchainconfirm(T_P2PPROTOCOLRSP Result, int8 *Hash);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetHash(int8 *Hash);
	void SetInitHash(int Num);
	T_P2PPROTOCOLRSP GetResult()const;
	int8* GetHash();

}T_P2PPROTOCOLONCHAINCONFIRM, *T_PP2PPROTOCOLONCHAINCONFIRM;

typedef struct _tp2pprotocolonchainconfirmrsp
{
	T_P2PPROTOCOLRSP tResult;
	int8 strHash[DEF_STR_HASH256_LEN];

	_tp2pprotocolonchainconfirmrsp& operator = (const _tp2pprotocolonchainconfirmrsp& arRes);
	void SetP2pprotocolonchainconfirmrsp(T_P2PPROTOCOLRSP Result, int8 *Hash);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetHash(int8 *Hash);
	void SetInitHash(int Num);
	T_P2PPROTOCOLRSP GetResult()const;
	int8* GetHash();


}T_P2PPROTOCOLONCHAINCONFIRMRSP, *T_PP2PPROTOCOLONCHAINCONFIRMRSP;

typedef struct _tp2pprotocolonchainconfirmfin
{
	T_P2PPROTOCOLRSP tResult;
	int8 strHash[DEF_STR_HASH256_LEN];

	_tp2pprotocolonchainconfirmfin& operator = (const _tp2pprotocolonchainconfirmfin& arRes);
	void SetP2pprotocolonchainconfirmfin(T_P2PPROTOCOLRSP Result, int8 *Hash);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetHash(int8 *Hash);
	void SetInitHash(int Num);

	T_P2PPROTOCOLRSP GetResult()const;
	int8* GetHash();
}T_P2PPROTOCOLONCHAINCONFIRMFIN, *T_PP2PPROTOCOLONCHAINCONFIRMFIN;


typedef struct _tp2pprotocolrefusereq
{
	T_P2PPROTOCOLTYPE tType;
	int8 strHash[DEF_STR_HASH256_LEN];

	uint8 uSubType;

	_tp2pprotocolrefusereq& operator = (const _tp2pprotocolrefusereq& arRes);
	void SetP2pprotocolrefusereq(T_P2PPROTOCOLTYPE Type, int8 * Hash, uint8 SubType);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetHash(int8 * Hash);
	void SetInitHash(int Num);

	void SetSubType(uint8 SubType);

	T_P2PPROTOCOLTYPE GetType()const;
	int8 * GetHash();
	uint8 GetSubType()const;

}T_P2PPROTOCOLREFUSEREQ, *T_PP2PPROTOCOLREFUSEREQ;

typedef struct _tp2pprotocolglobalconsensusreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;

	uint64  uiBlockCount;
	uint64  uiChainCount;

	_tp2pprotocolglobalconsensusreq& operator = (const _tp2pprotocolglobalconsensusreq& arRes);
	void SetP2pprotocolglobalconsensusreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS	PeerAddr, uint64  BlockCount, uint64  ChainCount);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetPeerAddr(T_PEERADDRESS	PeerAddr);
	void SetBlockCount(uint64  BlockCount);
	void SetChainCount(uint64  ChainCount);

	T_P2PPROTOCOLTYPE GetType()const;
	T_PEERADDRESS GetPeerAddr()const;
	uint64 GetBlockCount()const;
	uint64 GetChainCount()const;


}T_P2PPROTOCOLGLOBALBUDDYREQ, *T_PP2PPROTOCOLGLOBALBUDDYREQ;


typedef struct _tp2pprotocolglobalconsensusrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerAddr;

	uint64  uiBlockCount;
	uint64  uiChainCount;

	_tp2pprotocolglobalconsensusrsp& operator = (const _tp2pprotocolglobalconsensusrsp& arRes);
	void SetP2pprotocolglobalconsensusrsp(T_P2PPROTOCOLRSP Result, T_PEERADDRESS PeerAddr, uint64 BlockCount, uint64 ChainCount);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	void SetBlockCount(uint64 BlockCount);
	void SetChainCount(uint64 ChainCount);

	T_P2PPROTOCOLRSP GetResult()const;
	T_PEERADDRESS GetPeerAddr()const;
	uint64 GetBlockCount()const;
	uint64 GetChainCount()const;
}T_P2PPROTOCOLGLOBALBUDDYRSP, *T_PP2PPROTOCOLGLOBALBUDDYRSP;


typedef struct _tp2pprotocolcopyhyperblockreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;
	uint64	uiBlockNum;
	uint16  uiSendTimes;
	uint64  uiBlockCount;
	uint64  uiChainCount;

	_tp2pprotocolcopyhyperblockreq& operator = (const _tp2pprotocolcopyhyperblockreq& arRes);
	void SetP2pprotocolcopyhyperblockreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, uint64 BlockNum, uint16 SendTimes, uint64 BlockCount, uint64 ChainCount);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	void SetBlockNum(uint64 BlockNum);
	void SetSendTimes(uint16 SendTimes);
	void SetBlockCount(uint64 BlockCount);
	void SetChainCount(uint64 ChainCount);
	T_P2PPROTOCOLTYPE GetType()const;
	T_PEERADDRESS GetPeerAddr()const;
	uint64 GetBlockNum()const;
	uint16 GetSendTimes()const;
	uint64 GetBlockCount()const;
	uint64 GetChainCount()const;
}T_P2PPROTOCOLCOPYHYPERBLOCKREQ, *T_PP2PPROTOCOLCOPYHYPERBLOCKREQ;


typedef struct _tp2pprotocolcopyhyperblockrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerAddr;
	uint64	uiBlockNum;
	uint16  uiSendTimes;
	uint64  uiBlockCount;
	uint64  uiChainCount;
	//bool	bMultipleSend;

	_tp2pprotocolcopyhyperblockrsp& operator = (const _tp2pprotocolcopyhyperblockrsp& arRes);
	void SetP2pprotocolcopyhyperblockrsp(T_P2PPROTOCOLRSP Result, T_PEERADDRESS PeerAddr, uint64 BlockNum, uint16 SendTimes, uint64 BlockCount, uint64 ChainCount);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	void SetBlockNum(uint64 BlockNum);
	void SetSendTimes(uint16 SendTimes);
	void SetBlockCount(uint64 BlockCount);
	void SetChainCount(uint64 ChainCount);
	T_P2PPROTOCOLRSP GetResult()const;
	T_PEERADDRESS GetPeerAddr()const;
	uint64 GetBlockNum()const;
	uint16 GetSendTimes()const;
	uint64 GetBlockCount()const;
	uint64 GetChainCount()const;
}T_P2PPROTOCOLCOPYHYPERBLOCKRSP, *T_PP2PPROTOCOLCOPYHYPERBLOCKRSP;

typedef struct _tp2pprotocolcopyblockreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64	uiBlockNum;
	uint16  uiSendTimes;

	_tp2pprotocolcopyblockreq& operator = (const _tp2pprotocolcopyblockreq& arRes);
	void SetP2pprotocolcopyblockreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum, uint16 SendTimes);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetBlockNum(uint64 BlockNum);
	void SetSendTimes(uint16 SendTimes);
	T_P2PPROTOCOLTYPE GetType()const;
	uint64 GetBlockNum()const;
	uint16 GetSendTimes()const;
}T_P2PPROTOCOLCOPYBLOCKREQ, *T_PP2PPROTOCOLCOPYBLOCKREQ;

typedef struct _tp2pprotocolcopyblockrsp
{
	T_P2PPROTOCOLRSP tResult;

	uint16  uiSendTimes;
	uint64  uiBlockCount;

	_tp2pprotocolcopyblockrsp& operator = (const _tp2pprotocolcopyblockrsp& arRes);
	void SetP2pprotocolcopyblockrsp(T_P2PPROTOCOLRSP Result, uint16 SendTimes, uint64 BlockCount);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetSendTimes(uint16 SendTimes);
	void SetBlockCount(uint64 BlockCount);
	T_P2PPROTOCOLRSP GetResult()const;
	uint16 GetSendTimes()const;
	uint64 GetBlockCount()const;
}T_P2PPROTOCOLCOPYBLOCKRSP, *T_PP2PPROTOCOLCOPYBLOCKRSP;

typedef struct _tp2pprotocolwantNatTraversalreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerOutAddr;
	T_PEERADDRESS	tPeerBeNatTraversaldAddr;

	_tp2pprotocolwantNatTraversalreq& operator = (const _tp2pprotocolwantNatTraversalreq& arRes);
	void SetP2pprotocolwantNatTraversalreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerOutAddr, T_PEERADDRESS PeerBeNatTraversaldAddr);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetPeerOutAddr(T_PEERADDRESS PeerOutAddr);
	void SetPeerBeNatTraversaldAddr(T_PEERADDRESS PeerBeNatTraversaldAddr);
	T_P2PPROTOCOLTYPE GetType()const;
	T_PEERADDRESS GetPeerOutAddr()const;
	T_PEERADDRESS GetPeerBeNatTraversaldAddr()const;
}T_P2PPROTOCOLWANTNATTRAVERSALREQ, *T_PP2PPROTOCOLWANTNATTRAVERSALREQ;

typedef struct _tp2pprotocolsomeonewantcallyoureq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerWantNatTraversalAddr;

	_tp2pprotocolsomeonewantcallyoureq& operator = (const _tp2pprotocolsomeonewantcallyoureq& arRes);
	void SetP2pprotocolsomeonewantcallyoureq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerWantNatTraversalAddr);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetPeerWantNatTraversalAddr(T_PEERADDRESS PeerWantNatTraversalAddr);
	T_P2PPROTOCOLTYPE GetType()const;
	T_PEERADDRESS GetPeerWantNatTraversalAddr()const;
}T_P2PPROTOCOLSOMEONEWANTCALLYOUREQ, *T_PP2PPROTOCOLSOMEONEWANTCALLYOUREQ;

typedef struct _tp2pprotocolNatTraversalaskreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;

	_tp2pprotocolNatTraversalaskreq& operator = (const _tp2pprotocolNatTraversalaskreq& arRes);
	void SetP2pprotocolNatTraversalaskreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS	PeerAddr);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetPeerAddr(T_PEERADDRESS	PeerAddr);
	T_P2PPROTOCOLTYPE GetType()const;
	T_PEERADDRESS GetPeerAddr()const;
}T_P2PPROTOCOLNATTRAVERSALASKREQ, *T_PP2PPROTOCOLNATTRAVERSALASKREQ;

typedef struct _tp2pprotocolgetchainstatereq
{
	T_P2PPROTOCOLTYPE tType;
	uint64	uiMaxBlockNum;

	_tp2pprotocolgetchainstatereq& operator = (const _tp2pprotocolgetchainstatereq& arRes);
	void SetP2pprotocolgetchainstatereq(T_P2PPROTOCOLTYPE Type, uint64	MaxBlockNum);
	void SetType(T_P2PPROTOCOLTYPE Type);
	void SetMaxBlockNum(uint64	MaxBlockNum);
	T_P2PPROTOCOLTYPE GetType()const;
	uint64 GetMaxBlockNum()const;
}T_P2PPROTOCOLGETCHAINSTATEREQ, *T_PP2PPROTOCOLGETCHAINSTATEREQ;

typedef struct _tp2pprotocolgetchainstatersp
{
	T_P2PPROTOCOLRSP tResult;
	uint64 uiCount;
	T_PEERADDRESS	tPeerAddr;

	_tp2pprotocolgetchainstatersp& operator = (const _tp2pprotocolgetchainstatersp& arRes);
	void SetP2pprotocolgetchainstatersp(T_P2PPROTOCOLRSP Result, uint64 Count, T_PEERADDRESS PeerAddr);
	void SetResult(T_P2PPROTOCOLRSP Result);
	void SetCountPeerAddr(uint64 Count);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	T_P2PPROTOCOLRSP GetResult()const;
	uint64 GetCount()const;
	T_PEERADDRESS GetPeerAddr()const;
}T_P2PPROTOCOLGETCHAINSTATERSP, *T_PP2PPROTOCOLGETCHAINSTATERSP;

#pragma pack()

class CP2pProtocol
{
public:
	
	CP2pProtocol();
	virtual ~CP2pProtocol();

};


#endif 
