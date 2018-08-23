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
#include "p2pprotocol.h"


CP2pProtocol::CP2pProtocol()
{
}

CP2pProtocol::~CP2pProtocol()
{
}

_tp2pprotocoltype::_tp2pprotocoltype(uint8 t,uint64 timeStamp)
{
	ucType = t;
	uiTimeStamp = timeStamp;
}

_tp2pprotocoltype& _tp2pprotocoltype::operator = (const _tp2pprotocoltype& arRes)
{
	if (this != &arRes)
	{
		ucType = arRes.ucType;
		uiTimeStamp = arRes.uiTimeStamp;
	}
	return *this;
}

void _tp2pprotocoltype::SetP2pprotocoltype(uint8 Type, uint64 TimeStamp)
{
	ucType = Type;
	uiTimeStamp = TimeStamp;
}

void _tp2pprotocoltype::SetType(uint8 Type)
{
	ucType = Type;
}

void _tp2pprotocoltype::SetTimeStamp(uint64 TimeStamp)
{
	uiTimeStamp = TimeStamp;
}

uint8 _tp2pprotocoltype::GetType()const
{
	return ucType;
}

uint64 _tp2pprotocoltype::GetTimeStamp()const
{
	return uiTimeStamp;
}

_tp2pprotocolrsp::_tp2pprotocolrsp(T_P2PPROTOCOLTYPE t, uint8 result)
{
	tType = t;
	iResult = result;
}

_tp2pprotocolrsp& _tp2pprotocolrsp::operator = (const _tp2pprotocolrsp& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		iResult = arRes.iResult;
	}
	return *this;
}

void _tp2pprotocolrsp::SetP2pprotocolrsp(T_P2PPROTOCOLTYPE Type, uint8 Result)
{
	tType = Type;
	iResult = Result;

}

void _tp2pprotocolrsp::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;

}

void _tp2pprotocolrsp::SetResult(uint8 Result)
{
	iResult = Result;
}

T_P2PPROTOCOLTYPE _tp2pprotocolrsp::GetType()const
{
	return tType;
}

uint8 _tp2pprotocolrsp::GetResult()const
{
	return iResult;
}



_tp2pprotocolpingreq& _tp2pprotocolpingreq::operator = (const _tp2pprotocolpingreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		tPeerAddr = arRes.tPeerAddr;
		strncpy(strName, arRes.strName, MAX_NODE_NAME_LEN);		
		uiMaxBlockNum = arRes.uiMaxBlockNum;
		uiNodeState = arRes.uiNodeState;
	}
	return *this;
}

void _tp2pprotocolpingreq::SetP2pprotocolpingreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, int8 *Name, uint64 MaxBlockNum, uint16 NodeState)
{
	tType = Type;
	tPeerAddr = PeerAddr;
	strncpy(strName, Name, MAX_NODE_NAME_LEN);
	uiMaxBlockNum = MaxBlockNum;
	uiNodeState = NodeState;
}

void _tp2pprotocolpingreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolpingreq::SetPeerAddr(T_PEERADDRESS PeerAddr)
{
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolpingreq::SetName(int8 * Name)
{
	strncpy(strName, Name, MAX_NODE_NAME_LEN);

}

void _tp2pprotocolpingreq::SetMaxBlockNum(uint64 MaxBlockNum)
{
	uiMaxBlockNum = MaxBlockNum;
}

void _tp2pprotocolpingreq::SetnodeState(uint16 NodeState)
{
	uiNodeState = NodeState;
}

T_P2PPROTOCOLTYPE _tp2pprotocolpingreq::GetType()const
{
	return tType;
}

T_PEERADDRESS _tp2pprotocolpingreq::GetPeerAddr()const
{
	return tPeerAddr;
}

int8 *  _tp2pprotocolpingreq::GetName()
{
	return strName;
}

uint64 _tp2pprotocolpingreq::GetMaxBlockNum()const
{
	return uiMaxBlockNum;
}

uint16 _tp2pprotocolpingreq::GetnodeState()const
{
	return uiNodeState;
}



_tp2pprotocolpingrsp& _tp2pprotocolpingrsp::operator = (const _tp2pprotocolpingrsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		tPeerOutAddr = arRes.tPeerOutAddr;
		uiMaxBlockNum = arRes.uiMaxBlockNum;
	}
	return *this;
}

void _tp2pprotocolpingrsp::SetP2pprotocolpingrsp(T_P2PPROTOCOLRSP Result, T_PEERADDRESS PeerOutAddr, uint64 MaxBlockNum)
{
	tResult = Result;
	tPeerOutAddr = PeerOutAddr;
	uiMaxBlockNum = MaxBlockNum;
}

void _tp2pprotocolpingrsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}

void _tp2pprotocolpingrsp::SetPeeroutAddr(T_PEERADDRESS PeerOutAddr)
{
	tPeerOutAddr = PeerOutAddr;
}

void _tp2pprotocolpingrsp::SetMaxBlockNum(uint64 MaxBlockNum)
{
	uiMaxBlockNum = MaxBlockNum;
}

T_P2PPROTOCOLRSP _tp2pprotocolpingrsp::GetResult()const
{
	return tResult;
}

T_PEERADDRESS _tp2pprotocolpingrsp::GetPeeroutAddr()const
{
	return tPeerOutAddr;
}

uint64 _tp2pprotocolpingrsp::GetMaxBlockNum()const
{
	return uiMaxBlockNum;
}



_tp2pprotocolgethyperblockbynoreq::_tp2pprotocolgethyperblockbynoreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum)
{
	tType = Type;
	uiBlockNum = BlockNum;
}

_tp2pprotocolgethyperblockbynoreq& _tp2pprotocolgethyperblockbynoreq::operator = (const _tp2pprotocolgethyperblockbynoreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		uiBlockNum = arRes.uiBlockNum;
	}
	return *this;
}

void _tp2pprotocolgethyperblockbynoreq::SetP2pprotocolgethyperblockbynoreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum)
{
	tType = Type;
	uiBlockNum = BlockNum;
}

void _tp2pprotocolgethyperblockbynoreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolgethyperblockbynoreq::SetBlockNum(uint64 BlockNum)
{
	uiBlockNum = BlockNum;
}

T_P2PPROTOCOLTYPE _tp2pprotocolgethyperblockbynoreq::GetType()const
{
	return tType;
}

uint64 _tp2pprotocolgethyperblockbynoreq::GetBlockNum()const
{
	return uiBlockNum;
}



_tp2pprotocolgethyperblockbynorsp& _tp2pprotocolgethyperblockbynorsp::operator = (const _tp2pprotocolgethyperblockbynorsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
	}
	return *this;
}

void _tp2pprotocolgethyperblockbynorsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}

T_P2PPROTOCOLRSP _tp2pprotocolgethyperblockbynorsp::GetResult()const
{
	return tResult;
}



_tp2pprotocolpeerlistreq::_tp2pprotocolpeerlistreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, uint64 MaxBlockNum, int8 *Name, uint16 NodeState)
{
	tType = Type;
	tPeerAddr = PeerAddr;
	uiMaxBlockNum = MaxBlockNum;
	strncpy(strName, Name, MAX_NODE_NAME_LEN);
	uiNodeState = NodeState;
}

_tp2pprotocolpeerlistreq& _tp2pprotocolpeerlistreq::operator = (const _tp2pprotocolpeerlistreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		tPeerAddr = arRes.tPeerAddr;
		uiMaxBlockNum = arRes.uiMaxBlockNum;
		uiCount = arRes.uiCount;
		strncpy(strName, arRes.strName, MAX_NODE_NAME_LEN);
		uiNodeState = arRes.uiNodeState;
	}
	return *this;
}

void _tp2pprotocolpeerlistreq::SetP2pprotocolpeerlistreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, uint64 MaxBlockNum, uint16 Count, int8 *Name, uint16 NodeState)
{
	tType = Type;
	tPeerAddr = PeerAddr;
	uiMaxBlockNum = MaxBlockNum;
	uiCount = Count;
	strncpy(strName, Name, MAX_NODE_NAME_LEN);
	uiNodeState = NodeState;
}

void _tp2pprotocolpeerlistreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolpeerlistreq::SetPeerAddr(T_PEERADDRESS PeerAddr)
{
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolpeerlistreq::SetMaxBlockNum(uint64 MaxBlockNum)
{
	uiMaxBlockNum = MaxBlockNum;
}

void _tp2pprotocolpeerlistreq::SetCount(uint16 Count)
{
	uiCount = Count;
}

void _tp2pprotocolpeerlistreq::SetName(int8 *Name)
{
	strncpy(strName, Name, MAX_NODE_NAME_LEN);
}

void _tp2pprotocolpeerlistreq::SetNodeState(uint16 NodeState)
{
	uiNodeState = NodeState;
}

T_P2PPROTOCOLTYPE _tp2pprotocolpeerlistreq::GetType()const
{
	return tType;
}

T_PEERADDRESS _tp2pprotocolpeerlistreq::GetPeerAddr()const
{
	return tPeerAddr;
} 

uint64 _tp2pprotocolpeerlistreq::GetMaxBlockNum()const
{
	return uiMaxBlockNum;
}

uint16 _tp2pprotocolpeerlistreq::GetCount()const
{
	return uiCount;
}

int8* _tp2pprotocolpeerlistreq::GetName()
{
	return strName;
}

uint16 _tp2pprotocolpeerlistreq::GetNodeState()const
{
	return uiNodeState;
}



_tp2pprotocolpeerlistrsp& _tp2pprotocolpeerlistrsp::operator = (const _tp2pprotocolpeerlistrsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		tPeerAddr = arRes.tPeerAddr;
		strncpy(strName, arRes.strName, MAX_NODE_NAME_LEN);
		uiCount = arRes.uiCount;
		uiMaxBlockNum = arRes.uiMaxBlockNum;
		uiNodeState = arRes.uiNodeState;
	}
	return *this;
}

void _tp2pprotocolpeerlistrsp::SetP2pprotocolpeerlistrsp(T_P2PPROTOCOLRSP Result, T_PEERADDRESS PeerAddr, int8 *Name, uint16 Count, uint64 MaxBlockNum, uint16 NodeState)
{
	tResult = Result;
	tPeerAddr = PeerAddr;
	strncpy(strName, Name, MAX_NODE_NAME_LEN);
	uiCount = Count;
	uiMaxBlockNum = MaxBlockNum;
	uiNodeState = NodeState;
}

void _tp2pprotocolpeerlistrsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}

void _tp2pprotocolpeerlistrsp::SetPeerAddr(T_PEERADDRESS PeerAddr)
{
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolpeerlistrsp::SetName(int8 *Name)
{
	strncpy(strName, Name, MAX_NODE_NAME_LEN);
}

void _tp2pprotocolpeerlistrsp::SetCount(uint16 Count)
{
	uiCount = Count;
}

void _tp2pprotocolpeerlistrsp::SetMaxBlockNum(uint64 MaxBlockNum)
{
	uiMaxBlockNum = MaxBlockNum;
}

void _tp2pprotocolpeerlistrsp::SetnodeState(uint16 NodeState)
{
	uiNodeState = NodeState;
}

T_P2PPROTOCOLRSP _tp2pprotocolpeerlistrsp::GetResult()const
{
	return tResult;
}

T_PEERADDRESS _tp2pprotocolpeerlistrsp::GetPeerAddr()const
{
	return tPeerAddr;
}

int8 * _tp2pprotocolpeerlistrsp::GetName()
{
	return strName;
}

uint16 _tp2pprotocolpeerlistrsp::GetCount()const
{
	return uiCount;
}

uint64 _tp2pprotocolpeerlistrsp::GetMaxBlockNum()const
{
	return uiMaxBlockNum;
}

uint16 _tp2pprotocolpeerlistrsp::GetnodeState()const
{
	return uiNodeState;
}



_tp2pprotocoladdblockreq& _tp2pprotocoladdblockreq::operator = (const _tp2pprotocoladdblockreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		uiBlockNum = arRes.uiBlockNum;
		uiSendTimes = arRes.uiSendTimes;
		uiBlockCount = arRes.uiBlockCount;
	}
	return *this;
}

void _tp2pprotocoladdblockreq::SetP2pprotocoladdblockreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum, uint16 SendTimes, uint64 BlockCount)
{
	tType = Type;
	uiBlockNum = BlockNum;
	uiSendTimes = SendTimes;
	uiBlockCount = BlockCount;
}

void _tp2pprotocoladdblockreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocoladdblockreq::SetBlockNum(uint64 BlockNum)
{
	uiBlockNum = BlockNum;
}

void _tp2pprotocoladdblockreq::SetSendTime(uint16 SendTimes)
{
	uiSendTimes = SendTimes;
}

void _tp2pprotocoladdblockreq::SetBlockCount(uint64 BlockCount)
{
	uiBlockCount = BlockCount;
}

T_P2PPROTOCOLTYPE _tp2pprotocoladdblockreq::GetType()const
{
	return tType;
}

uint64 _tp2pprotocoladdblockreq::GetBlockNum()const
{
	return uiBlockNum;
}

uint16 _tp2pprotocoladdblockreq::GetSendTime()const
{
	return uiSendTimes;
}

uint64 _tp2pprotocoladdblockreq::GetBlockCount()const
{
	return uiBlockCount;
}



_tp2pprotocolonchainreq& _tp2pprotocolonchainreq::operator = (const _tp2pprotocolonchainreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		uiHyperBlockNum = arRes.uiHyperBlockNum;
		uiBlockCount = arRes.uiBlockCount;
	}
	return *this;
}

void _tp2pprotocolonchainreq::SetP2pprotocolonchainreq(T_P2PPROTOCOLTYPE Type, uint64 HyperBlockNum, uint64  BlockCount)
{
	tType = Type;
	uiHyperBlockNum = HyperBlockNum;
	uiBlockCount = BlockCount;
}

void _tp2pprotocolonchainreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolonchainreq::SetHyperBlockNum(uint64 HyperBlockNum)
{
	uiHyperBlockNum = HyperBlockNum;
}

void _tp2pprotocolonchainreq::SetBlockCount(uint64 BlockCount)
{
	uiBlockCount = BlockCount;
}

T_P2PPROTOCOLTYPE _tp2pprotocolonchainreq::GetType()const
{
	return tType;
}

uint64 _tp2pprotocolonchainreq::GetHyperBlockNum()const
{
	return uiHyperBlockNum;
}

uint64 _tp2pprotocolonchainreq::GetBlockCount()const
{
	return uiBlockCount;
}




_tp2pprotocolonchainrsp& _tp2pprotocolonchainrsp::operator = (const _tp2pprotocolonchainrsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		uiHyperBlockNum = arRes.uiHyperBlockNum;	
		uiBlockCount = arRes.uiBlockCount;
		memcpy(strHash, arRes.strHash, DEF_STR_HASH256_LEN);	
	}
	return *this;
}

void _tp2pprotocolonchainrsp::SetP2pprotocolonchainrsp(T_P2PPROTOCOLRSP Result, uint64 HyperBlockNum, uint64  BlockCount, int8 *Hash)
{
	tResult = Result;
	uiHyperBlockNum = HyperBlockNum;
	uiBlockCount = BlockCount;
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}


void _tp2pprotocolonchainrsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}

void _tp2pprotocolonchainrsp::SetHyperBlockNum(uint64 HyperBlockNum)
{
	uiHyperBlockNum = HyperBlockNum;
}

void _tp2pprotocolonchainrsp::SetBlockCount(uint64 BlockCount)
{
	uiBlockCount = BlockCount;
}

void _tp2pprotocolonchainrsp::SetInitHash(int Num)
{
	memset(strHash, Num, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolonchainrsp::SetHash(int8 *Hash)
{
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

T_P2PPROTOCOLRSP _tp2pprotocolonchainrsp::GetResult()const
{
	return tResult;
}

uint64 _tp2pprotocolonchainrsp::GetHyperBlockNum()const
{
	return uiHyperBlockNum;
}

uint64 _tp2pprotocolonchainrsp::GetBlockCount()const
{
	return uiBlockCount;
}

int8 * _tp2pprotocolonchainrsp::GetHash()
{
	return strHash;
}



_tp2pprotocolonchainconfirm& _tp2pprotocolonchainconfirm::operator = (const _tp2pprotocolonchainconfirm& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		memcpy(strHash, arRes.strHash, DEF_STR_HASH256_LEN);
	}
	return *this;
}

void _tp2pprotocolonchainconfirm::SetP2pprotocolonchainconfirm(T_P2PPROTOCOLRSP Result, int8 *Hash)
{
	tResult = Result;
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolonchainconfirm::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;

}

void _tp2pprotocolonchainconfirm::SetInitHash(int Num)
{
	memset(strHash, Num, DEF_STR_HASH256_LEN);

}
void _tp2pprotocolonchainconfirm::SetHash(int8 *Hash)
{
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

T_P2PPROTOCOLRSP _tp2pprotocolonchainconfirm::GetResult()const
{
	return tResult;
}

int8* _tp2pprotocolonchainconfirm::GetHash()
{
	return strHash;
}



_tp2pprotocolonchainconfirmrsp& _tp2pprotocolonchainconfirmrsp::operator = (const _tp2pprotocolonchainconfirmrsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		memcpy(strHash, arRes.strHash, DEF_STR_HASH256_LEN);
	}
	return *this;
}

void _tp2pprotocolonchainconfirmrsp::SetP2pprotocolonchainconfirmrsp(T_P2PPROTOCOLRSP Result, int8 *Hash)
{
	tResult = Result;
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolonchainconfirmrsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;

}

void _tp2pprotocolonchainconfirmrsp::SetInitHash(int Num)
{
	memset(strHash, Num, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolonchainconfirmrsp::SetHash(int8 *Hash)
{
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

T_P2PPROTOCOLRSP _tp2pprotocolonchainconfirmrsp::GetResult()const
{
	return tResult;
}

int8* _tp2pprotocolonchainconfirmrsp::GetHash()
{
	return strHash;
}



_tp2pprotocolonchainconfirmfin& _tp2pprotocolonchainconfirmfin::operator = (const _tp2pprotocolonchainconfirmfin& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		memcpy(strHash, arRes.strHash, DEF_STR_HASH256_LEN);
	}
	return *this;
}

void _tp2pprotocolonchainconfirmfin::SetP2pprotocolonchainconfirmfin(T_P2PPROTOCOLRSP Result, int8 *Hash)
{
	tResult = Result;
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolonchainconfirmfin::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;

}

void _tp2pprotocolonchainconfirmfin::SetInitHash(int Num)
{
	memset(strHash, Num, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolonchainconfirmfin::SetHash(int8 *Hash)
{
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

T_P2PPROTOCOLRSP _tp2pprotocolonchainconfirmfin::GetResult()const
{
	return tResult;
}

int8* _tp2pprotocolonchainconfirmfin::GetHash()
{
	return strHash;
}



_tp2pprotocolrefusereq& _tp2pprotocolrefusereq::operator = (const _tp2pprotocolrefusereq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		memcpy(strHash, arRes.strHash, DEF_STR_HASH256_LEN);
		uSubType = arRes.uSubType;
	}
	return *this;
}
void _tp2pprotocolrefusereq::SetP2pprotocolrefusereq(T_P2PPROTOCOLTYPE Type, int8 * Hash, uint8 SubType)
{
	tType = Type;
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
	uSubType = SubType;
}

void _tp2pprotocolrefusereq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolrefusereq::SetInitHash(int Num)
{
	memset(strHash, Num, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolrefusereq::SetHash(int8 * Hash)
{
	memcpy(strHash, Hash, DEF_STR_HASH256_LEN);
}

void _tp2pprotocolrefusereq::SetSubType(uint8 SubType)
{
	uSubType = SubType;
}

T_P2PPROTOCOLTYPE _tp2pprotocolrefusereq::GetType()const
{
	return tType;
}

int8 * _tp2pprotocolrefusereq::GetHash()
{
	return strHash;
}

uint8 _tp2pprotocolrefusereq::GetSubType()const
{
	return uSubType;
}


_tp2pprotocolglobalconsensusreq& _tp2pprotocolglobalconsensusreq::operator = (const _tp2pprotocolglobalconsensusreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		tPeerAddr = arRes.tPeerAddr;
		uiBlockCount = arRes.uiBlockCount;
		uiChainCount = arRes.uiChainCount;		
	}
	return *this;
}

void _tp2pprotocolglobalconsensusreq::SetP2pprotocolglobalconsensusreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS	PeerAddr, uint64  BlockCount, uint64  ChainCount)
{
	tType = Type;
	tPeerAddr = PeerAddr;
	uiBlockCount = BlockCount;
	uiChainCount = ChainCount;
}

void _tp2pprotocolglobalconsensusreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolglobalconsensusreq::SetPeerAddr(T_PEERADDRESS	PeerAddr)
{
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolglobalconsensusreq::SetBlockCount(uint64  BlockCount)
{
	uiBlockCount = BlockCount;
}

void _tp2pprotocolglobalconsensusreq::SetChainCount(uint64  ChainCount)
{
	uiChainCount = ChainCount;
}

T_P2PPROTOCOLTYPE _tp2pprotocolglobalconsensusreq::GetType()const
{
	return tType;
}

T_PEERADDRESS _tp2pprotocolglobalconsensusreq::GetPeerAddr()const
{
	return tPeerAddr;
}

uint64 _tp2pprotocolglobalconsensusreq::GetBlockCount()const
{
	return uiBlockCount;
}

uint64 _tp2pprotocolglobalconsensusreq::GetChainCount()const
{
	return uiChainCount;
}



_tp2pprotocolglobalconsensusrsp& _tp2pprotocolglobalconsensusrsp::operator = (const _tp2pprotocolglobalconsensusrsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		tPeerAddr = arRes.tPeerAddr;
		uiBlockCount = arRes.uiBlockCount;
		uiChainCount = arRes.uiChainCount;
	}
	return *this;
}

void _tp2pprotocolglobalconsensusrsp::SetP2pprotocolglobalconsensusrsp(T_P2PPROTOCOLRSP Result, T_PEERADDRESS PeerAddr, uint64 BlockCount, uint64 ChainCount)
{
	tResult = Result;
	tPeerAddr = PeerAddr;
	uiBlockCount = BlockCount;
	uiChainCount = ChainCount;
}

void _tp2pprotocolglobalconsensusrsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}
void _tp2pprotocolglobalconsensusrsp::SetPeerAddr(T_PEERADDRESS PeerAddr)
{
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolglobalconsensusrsp::SetBlockCount(uint64 BlockCount)
{
	uiBlockCount = BlockCount;
}

void _tp2pprotocolglobalconsensusrsp::SetChainCount(uint64 ChainCount)
{
	uiChainCount = ChainCount;
}

T_P2PPROTOCOLRSP _tp2pprotocolglobalconsensusrsp::GetResult()const
{
	return tResult;
}

T_PEERADDRESS _tp2pprotocolglobalconsensusrsp::GetPeerAddr()const
{
	return tPeerAddr;
}

uint64 _tp2pprotocolglobalconsensusrsp::GetBlockCount()const
{
	return uiBlockCount;
}

uint64 _tp2pprotocolglobalconsensusrsp::GetChainCount()const
{
	return uiChainCount;
}



_tp2pprotocolcopyhyperblockreq& _tp2pprotocolcopyhyperblockreq::operator = (const _tp2pprotocolcopyhyperblockreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		tPeerAddr = arRes.tPeerAddr;
		uiBlockNum = arRes.uiBlockNum;
		uiSendTimes = arRes.uiSendTimes;
		uiBlockCount = arRes.uiBlockCount;
		uiChainCount = arRes.uiChainCount;
	}
	return *this;

}

void _tp2pprotocolcopyhyperblockreq::SetP2pprotocolcopyhyperblockreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerAddr, uint64 BlockNum, uint16 SendTimes, uint64 BlockCount, uint64 ChainCount)
{
	tType = Type;
	tPeerAddr = PeerAddr;
	uiBlockNum = BlockNum;
	uiSendTimes = SendTimes;
	uiBlockCount = BlockCount;
	uiChainCount = ChainCount;
}

void _tp2pprotocolcopyhyperblockreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolcopyhyperblockreq::SetPeerAddr(T_PEERADDRESS PeerAddr)
{
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolcopyhyperblockreq::SetBlockNum(uint64 BlockNum)
{
	uiBlockNum = BlockNum;
}

void _tp2pprotocolcopyhyperblockreq::SetSendTimes(uint16 SendTimes)
{
	uiSendTimes = SendTimes;
}

void _tp2pprotocolcopyhyperblockreq::SetBlockCount(uint64 BlockCount)
{
	uiBlockCount = BlockCount;
}

void _tp2pprotocolcopyhyperblockreq::SetChainCount(uint64 ChainCount)
{
	uiChainCount = ChainCount;
}

T_P2PPROTOCOLTYPE _tp2pprotocolcopyhyperblockreq::GetType()const
{
	return tType;
}

T_PEERADDRESS _tp2pprotocolcopyhyperblockreq::GetPeerAddr()const
{
	return tPeerAddr;
}

uint64 _tp2pprotocolcopyhyperblockreq::GetBlockNum()const
{
	return uiBlockNum;
}

uint16 _tp2pprotocolcopyhyperblockreq::GetSendTimes()const
{
	return uiSendTimes;
}

uint64 _tp2pprotocolcopyhyperblockreq::GetBlockCount()const
{
	return uiBlockCount;
}

uint64 _tp2pprotocolcopyhyperblockreq::GetChainCount()const
{
	return uiChainCount;
}



_tp2pprotocolcopyhyperblockrsp& _tp2pprotocolcopyhyperblockrsp::operator = (const _tp2pprotocolcopyhyperblockrsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		tPeerAddr = arRes.tPeerAddr;
		uiBlockNum = arRes.uiBlockNum;
		uiSendTimes = arRes.uiSendTimes;
		uiBlockCount = arRes.uiBlockCount;
		uiChainCount = arRes.uiChainCount;
	}
	return *this;

}

void _tp2pprotocolcopyhyperblockrsp::SetP2pprotocolcopyhyperblockrsp(T_P2PPROTOCOLRSP Result, T_PEERADDRESS PeerAddr, uint64 BlockNum, uint16 SendTimes, uint64 BlockCount, uint64 ChainCount)
{
	tResult = Result;
	tPeerAddr = PeerAddr;
	uiBlockNum = BlockNum;
	uiSendTimes = SendTimes;
	uiBlockCount = BlockCount;
	uiChainCount = ChainCount;
}

void _tp2pprotocolcopyhyperblockrsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}

void _tp2pprotocolcopyhyperblockrsp::SetPeerAddr(T_PEERADDRESS PeerAddr)
{
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolcopyhyperblockrsp::SetBlockNum(uint64 BlockNum)
{
	uiBlockNum = BlockNum;
}

void _tp2pprotocolcopyhyperblockrsp::SetSendTimes(uint16 SendTimes)
{
	uiSendTimes = SendTimes;
}

void _tp2pprotocolcopyhyperblockrsp::SetBlockCount(uint64 BlockCount)
{
	uiBlockCount = BlockCount;
}

void _tp2pprotocolcopyhyperblockrsp::SetChainCount(uint64 ChainCount)
{
	uiChainCount = ChainCount;
}

T_P2PPROTOCOLRSP _tp2pprotocolcopyhyperblockrsp::GetResult()const
{
	return tResult;
}

T_PEERADDRESS _tp2pprotocolcopyhyperblockrsp::GetPeerAddr()const
{
	return tPeerAddr;
}

uint64 _tp2pprotocolcopyhyperblockrsp::GetBlockNum()const
{
	return uiBlockNum;
}

uint16 _tp2pprotocolcopyhyperblockrsp::GetSendTimes()const
{
	return uiSendTimes;
}

uint64 _tp2pprotocolcopyhyperblockrsp::GetBlockCount()const
{
	return uiBlockCount;
}

uint64 _tp2pprotocolcopyhyperblockrsp::GetChainCount()const
{
	return uiChainCount;
}



_tp2pprotocolcopyblockreq& _tp2pprotocolcopyblockreq::operator = (const _tp2pprotocolcopyblockreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		uiBlockNum = arRes.uiBlockNum;
		uiSendTimes = arRes.uiSendTimes;
	}
	return *this;
}

void _tp2pprotocolcopyblockreq::SetP2pprotocolcopyblockreq(T_P2PPROTOCOLTYPE Type, uint64 BlockNum, uint16 SendTimes)
{
	tType = Type;
	uiBlockNum = BlockNum;
	uiSendTimes = SendTimes;
}

void _tp2pprotocolcopyblockreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolcopyblockreq::SetBlockNum(uint64 BlockNum)
{
	uiBlockNum = BlockNum;
}

void _tp2pprotocolcopyblockreq::SetSendTimes(uint16 SendTimes)
{
	uiSendTimes = SendTimes;
}

T_P2PPROTOCOLTYPE _tp2pprotocolcopyblockreq::GetType()const
{
	return tType;
}

uint64 _tp2pprotocolcopyblockreq::GetBlockNum()const
{
	return uiBlockNum;
}

uint16 _tp2pprotocolcopyblockreq::GetSendTimes()const
{
	return uiSendTimes;
}



_tp2pprotocolcopyblockrsp& _tp2pprotocolcopyblockrsp::operator = (const _tp2pprotocolcopyblockrsp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		uiSendTimes = arRes.uiSendTimes;
		uiBlockCount = arRes.uiBlockCount;
	}
	return *this;
}

void _tp2pprotocolcopyblockrsp::SetP2pprotocolcopyblockrsp(T_P2PPROTOCOLRSP Result, uint16 SendTimes, uint64 BlockCount)
{
	tResult = Result;
	uiSendTimes = SendTimes;
	uiBlockCount = BlockCount;
}

void _tp2pprotocolcopyblockrsp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}

void _tp2pprotocolcopyblockrsp::SetSendTimes(uint16 SendTimes)
{
	uiSendTimes = SendTimes;
}

void _tp2pprotocolcopyblockrsp::SetBlockCount(uint64 BlockCount)
{
	uiBlockCount = BlockCount;
}

T_P2PPROTOCOLRSP _tp2pprotocolcopyblockrsp::GetResult()const
{
	return tResult;
}

uint16 _tp2pprotocolcopyblockrsp::GetSendTimes()const
{
	return uiSendTimes;
}

uint64 _tp2pprotocolcopyblockrsp::GetBlockCount()const
{
	return uiBlockCount;
}



_tp2pprotocolwantNatTraversalreq& _tp2pprotocolwantNatTraversalreq::operator = (const _tp2pprotocolwantNatTraversalreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		tPeerOutAddr = arRes.tPeerOutAddr;
		tPeerBeNatTraversaldAddr = arRes.tPeerBeNatTraversaldAddr;
	}
	return *this;
}

void _tp2pprotocolwantNatTraversalreq::SetP2pprotocolwantNatTraversalreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerOutAddr, T_PEERADDRESS PeerBeNatTraversaldAddr)
{
	tType = Type;
	tPeerOutAddr = PeerOutAddr;
	tPeerBeNatTraversaldAddr = PeerBeNatTraversaldAddr;
}

void _tp2pprotocolwantNatTraversalreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;

}
void _tp2pprotocolwantNatTraversalreq::SetPeerOutAddr(T_PEERADDRESS PeerOutAddr)
{
	tPeerOutAddr = PeerOutAddr;

}

void _tp2pprotocolwantNatTraversalreq::SetPeerBeNatTraversaldAddr(T_PEERADDRESS PeerBeNatTraversaldAddr)
{
	tPeerBeNatTraversaldAddr = PeerBeNatTraversaldAddr;

}

T_P2PPROTOCOLTYPE _tp2pprotocolwantNatTraversalreq::GetType()const
{
	return tType;
}

T_PEERADDRESS _tp2pprotocolwantNatTraversalreq::GetPeerOutAddr()const
{
	return tPeerOutAddr;
}

T_PEERADDRESS _tp2pprotocolwantNatTraversalreq::GetPeerBeNatTraversaldAddr()const
{
	return tPeerBeNatTraversaldAddr;
}



_tp2pprotocolsomeonewantcallyoureq& _tp2pprotocolsomeonewantcallyoureq::operator = (const _tp2pprotocolsomeonewantcallyoureq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		tPeerWantNatTraversalAddr = arRes.tPeerWantNatTraversalAddr;
	}
	return *this;
}

void _tp2pprotocolsomeonewantcallyoureq::SetP2pprotocolsomeonewantcallyoureq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS PeerWantNatTraversalAddr)
{
	tType = Type;
	tPeerWantNatTraversalAddr = PeerWantNatTraversalAddr;
}

void _tp2pprotocolsomeonewantcallyoureq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolsomeonewantcallyoureq::SetPeerWantNatTraversalAddr(T_PEERADDRESS PeerWantNatTraversalAddr)
{
	tPeerWantNatTraversalAddr = PeerWantNatTraversalAddr;
}

T_P2PPROTOCOLTYPE _tp2pprotocolsomeonewantcallyoureq::GetType()const
{
	return tType;
}

T_PEERADDRESS _tp2pprotocolsomeonewantcallyoureq::GetPeerWantNatTraversalAddr()const
{
	return tPeerWantNatTraversalAddr;
}



_tp2pprotocolNatTraversalaskreq& _tp2pprotocolNatTraversalaskreq::operator = (const _tp2pprotocolNatTraversalaskreq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		tPeerAddr = arRes.tPeerAddr;
	}
	return *this;
}

void _tp2pprotocolNatTraversalaskreq::SetP2pprotocolNatTraversalaskreq(T_P2PPROTOCOLTYPE Type, T_PEERADDRESS	PeerAddr)
{
	tType = Type;
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolNatTraversalaskreq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;

}

void _tp2pprotocolNatTraversalaskreq::SetPeerAddr(T_PEERADDRESS	PeerAddr)
{
	tPeerAddr = PeerAddr;

}

T_P2PPROTOCOLTYPE _tp2pprotocolNatTraversalaskreq::GetType()const
{
	return tType;
}

T_PEERADDRESS _tp2pprotocolNatTraversalaskreq::GetPeerAddr()const
{
	return tPeerAddr;
}



_tp2pprotocolgetchainstatereq& _tp2pprotocolgetchainstatereq::operator = (const _tp2pprotocolgetchainstatereq& arRes)
{
	if (this != &arRes)
	{
		tType = arRes.tType;
		uiMaxBlockNum = arRes.uiMaxBlockNum;
	}
	return *this;
}

void _tp2pprotocolgetchainstatereq::SetP2pprotocolgetchainstatereq(T_P2PPROTOCOLTYPE Type, uint64	MaxBlockNum)
{
	tType = Type;
	uiMaxBlockNum = MaxBlockNum;
}

void _tp2pprotocolgetchainstatereq::SetType(T_P2PPROTOCOLTYPE Type)
{
	tType = Type;
}

void _tp2pprotocolgetchainstatereq::SetMaxBlockNum(uint64	MaxBlockNum)
{
	uiMaxBlockNum = MaxBlockNum;
}

T_P2PPROTOCOLTYPE _tp2pprotocolgetchainstatereq::GetType()const
{
	return tType;
}

uint64 _tp2pprotocolgetchainstatereq::GetMaxBlockNum()const
{
	return uiMaxBlockNum;
}



_tp2pprotocolgetchainstatersp& _tp2pprotocolgetchainstatersp::operator = (const _tp2pprotocolgetchainstatersp& arRes)
{
	if (this != &arRes)
	{
		tResult = arRes.tResult;
		uiCount = arRes.uiCount;
		tPeerAddr = arRes.tPeerAddr;
	}
	return *this;
}

void _tp2pprotocolgetchainstatersp::SetP2pprotocolgetchainstatersp(T_P2PPROTOCOLRSP Result, uint64 Count, T_PEERADDRESS PeerAddr)
{
	tResult = Result;
	uiCount = Count;
	tPeerAddr = PeerAddr;
}

void _tp2pprotocolgetchainstatersp::SetResult(T_P2PPROTOCOLRSP Result)
{
	tResult = Result;
}

void _tp2pprotocolgetchainstatersp::SetCountPeerAddr(uint64 Count)
{
	uiCount = Count;
}

void _tp2pprotocolgetchainstatersp::SetPeerAddr(T_PEERADDRESS PeerAddr)
{
	tPeerAddr = PeerAddr;
}

T_P2PPROTOCOLRSP _tp2pprotocolgetchainstatersp::GetResult()const
{
	return tResult;
}

uint64 _tp2pprotocolgetchainstatersp::GetCount()const
{
	return uiCount;
}

T_PEERADDRESS _tp2pprotocolgetchainstatersp::GetPeerAddr()const
{
	return tPeerAddr;
}