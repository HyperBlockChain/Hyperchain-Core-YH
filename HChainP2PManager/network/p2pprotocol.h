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

}T_P2PPROTOCOLTYPE, *T_PP2PPROTOCOLTYPE;

typedef struct _tp2pprotocolrsp
{
	T_P2PPROTOCOLTYPE	tType;
	uint8				iResult;

}T_P2PPROTOCOLRSP, *T_PP2PPROTOCOLRSP;

typedef struct _tp2pprotocolpingreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;
	int8			strName[MAX_NODE_NAME_LEN];
	uint64	uiMaxBlockNum;
	uint16  uiNodeState;
}T_P2PPROTOCOLPINGREQ, *T_PP2PPROTOCOLPINGREQ;

typedef struct _tp2pprotocolpingrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerOutAddr;
	uint64 uiMaxBlockNum;
}T_P2PPROTOCOLPINGRSP, *T_PP2PPROTOCOLPINGRSP;

typedef struct _tp2pprotocolgethyperblockbynoreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64 uiBlockNum;

}T_P2PPROTOCOLGETHYPERBLOCKBYNOREQ, *T_PP2PPROTOCOLGETHYPERBLOCKBYNOREQ;

typedef struct _tp2pprotocolgethyperblockbynorsp
{
	T_P2PPROTOCOLRSP tResult;

}T_P2PPROTOCOLGETHYPERBLOCKBYNORSP, *T_PP2PPROTOCOLGETHYPERBLOCKBYNORSP;

typedef struct _tp2pprotocolpeerlistreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;
	uint64	uiMaxBlockNum;
	uint16 uiCount;
	int8 strName[MAX_NODE_NAME_LEN];
	uint16 uiNodeState;
}T_P2PPROTOCOLPEERLISTREQ, *T_PP2PPROTOCOLPEERLISTREQ;

typedef struct _tp2pprotocolpeerlistrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerAddr;
	int8			strName[MAX_NODE_NAME_LEN];
	uint16			 uiCount;
	uint64			 uiMaxBlockNum;
	uint16			 uiNodeState;
}T_P2PPROTOCOLPEERLISTRSP, *T_PP2PPROTOCOLPEERLISTRSP;

typedef struct _tp2pprotocoladdblockreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64	uiBlockNum;
	uint16  uiSendTimes;
	uint64  uiBlockCount;
}T_P2PPROTOCOLADDBLOCKREQ, *T_PP2PPROTOCOLADDBLOCKREQ;

typedef struct _tp2pprotocolonchainreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64 uiHyperBlockNum;
	uint64  uiBlockCount;
}T_P2PPROTOCOLONCHAINREQ, *T_PP2PPROTOCOLONCHAINREQ;

typedef struct _tp2pprotocolonchainrsp
{
	T_P2PPROTOCOLRSP tResult;
	uint64 uiHyperBlockNum;

	uint64  uiBlockCount;

	int8 strHash[DEF_STR_HASH256_LEN];
}T_P2PPROTOCOLONCHAINRSP, *T_PP2PPROTOCOLONCHAINRSP;

typedef struct _tp2pprotocolonchainconfirm
{
	T_P2PPROTOCOLRSP tResult;
	int8 strHash[DEF_STR_HASH256_LEN];

}T_P2PPROTOCOLONCHAINCONFIRM, *T_PP2PPROTOCOLONCHAINCONFIRM;

typedef struct _tp2pprotocolonchainconfirmrsp
{
	T_P2PPROTOCOLRSP tResult;
	int8 strHash[DEF_STR_HASH256_LEN];

}T_P2PPROTOCOLONCHAINCONFIRMRSP, *T_PP2PPROTOCOLONCHAINCONFIRMRSP;

typedef struct _tp2pprotocolonchainconfirmfin
{
	T_P2PPROTOCOLRSP tResult;
	int8 strHash[DEF_STR_HASH256_LEN];

}T_P2PPROTOCOLONCHAINCONFIRMFIN, *T_PP2PPROTOCOLONCHAINCONFIRMFIN;

typedef struct _tp2pprotocolrefusereq
{
	T_P2PPROTOCOLTYPE tType;
	int8 strHash[DEF_STR_HASH256_LEN];

	uint8 uSubType;
}T_P2PPROTOCOLREFUSEREQ, *T_PP2PPROTOCOLREFUSEREQ;

typedef struct _tp2pprotocolglobalconsensusreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;

	uint64  uiBlockCount;
	uint64  uiChainCount;

}T_P2PPROTOCOLGLOBALBUDDYREQ, *T_PP2PPROTOCOLGLOBALBUDDYREQ;

typedef struct _tp2pprotocolglobalconsensusrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerAddr;

	uint64  uiBlockCount;
	uint64  uiChainCount;

}T_P2PPROTOCOLGLOBALBUDDYRSP, *T_PP2PPROTOCOLGLOBALBUDDYRSP;

typedef struct _tp2pprotocolcopyhyperblockreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;
	uint64	uiBlockNum;
	uint16  uiSendTimes;
	uint64  uiBlockCount;
	uint64  uiChainCount;

}T_P2PPROTOCOLCOPYHYPERBLOCKREQ, *T_PP2PPROTOCOLCOPYHYPERBLOCKREQ;

typedef struct _tp2pprotocolcopyhyperblockrsp
{
	T_P2PPROTOCOLRSP tResult;
	T_PEERADDRESS	tPeerAddr;
		uint64	uiBlockNum;
	uint16  uiSendTimes;
	uint64  uiBlockCount;
	uint64  uiChainCount;

}T_P2PPROTOCOLCOPYHYPERBLOCKRSP, *T_PP2PPROTOCOLCOPYHYPERBLOCKRSP;

typedef struct _tp2pprotocolcopyblockreq
{
	T_P2PPROTOCOLTYPE tType;
	uint64	uiBlockNum;

	uint16  uiSendTimes;
}T_P2PPROTOCOLCOPYBLOCKREQ, *T_PP2PPROTOCOLCOPYBLOCKREQ;

typedef struct _tp2pprotocolcopyblockrsp
{
	T_P2PPROTOCOLRSP tResult;

	uint16  uiSendTimes;
	uint64  uiBlockCount;
}T_P2PPROTOCOLCOPYBLOCKRSP, *T_PP2PPROTOCOLCOPYBLOCKRSP;

typedef struct _tp2pprotocolwantNatTraversalreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerOutAddr;
	T_PEERADDRESS	tPeerBeNatTraversaldAddr;
}T_P2PPROTOCOLWANTNATTRAVERSALREQ, *T_PP2PPROTOCOLWANTNATTRAVERSALREQ;

typedef struct _tp2pprotocolsomeonewantcallyoureq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerWantNatTraversalAddr;
}T_P2PPROTOCOLSOMEONEWANTCALLYOUREQ, *T_PP2PPROTOCOLSOMEONEWANTCALLYOUREQ;

typedef struct _tp2pprotocolNatTraversalaskreq
{
	T_P2PPROTOCOLTYPE tType;
	T_PEERADDRESS	tPeerAddr;
}T_P2PPROTOCOLNATTRAVERSALASKREQ, *T_PP2PPROTOCOLNATTRAVERSALASKREQ;

typedef struct _tp2pprotocolgetchainstatereq
{
	T_P2PPROTOCOLTYPE tType;
	uint64	uiMaxBlockNum;
}T_P2PPROTOCOLGETCHAINSTATEREQ, *T_PP2PPROTOCOLGETCHAINSTATEREQ;

typedef struct _tp2pprotocolgetchainstatersp
{
	T_P2PPROTOCOLRSP tResult;
	uint64 uiCount;
	T_PEERADDRESS	tPeerAddr;
}T_P2PPROTOCOLGETCHAINSTATERSP, *T_PP2PPROTOCOLGETCHAINSTATERSP;

#pragma pack()

class CP2pProtocol
{
public:

	CP2pProtocol();
	virtual ~CP2pProtocol();

};

#endif
