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

#ifndef __HCHAINP2P_MANAGER__
#define __HCHAINP2P_MANAGER__

#include "utility/CallbackFuncObj.h"
#include "utility/ThreadObj.h"
#include "network/udpsocket.h"
#include "headers/inter_public.h"
#include <time.h>
#include <mutex>
typedef struct _tp2pmanagerstatus
{
	int32 uiMaxBlockNum;
	uint16 uiNodeState;
	uint16 usBuddyPeerCount;
	uint64 uiSendRegisReqNum;
	uint64 uiSendConfirmingRegisReqNum;
	uint64 uiRecvConfirmingRegisReqNum;
	uint64 uiRecvRegisReqNum;
	T_HYPERBLOCK tPreHyperBlock;
	T_LOCALBLOCK tPreLocalBlock;
	uint64 uiStartTimeOfConsensus;
	uint64 uiTimeOfConsensus;

	//std::mutex muLock;
	void clearStatus()
	{
		uiMaxBlockNum = 0;
		usBuddyPeerCount = 0;
		uiNodeState = DEFAULT_REGISREQ_STATE;	
		uiSendRegisReqNum = 0;
		uiSendConfirmingRegisReqNum = 0;
		uiRecvRegisReqNum = 0;
		uiRecvConfirmingRegisReqNum = 0;
		uiStartTimeOfConsensus = 0;
		uiTimeOfConsensus = 0;
	}

	_tp2pmanagerstatus()
	{
		clearStatus();
	}
}T_P2PMANAGERSTATUS, *T_PP2PMANAGERSTATUS;

class CHChainP2PManager
{
public:
	CHChainP2PManager(void);
	~CHChainP2PManager(void);

public:
	bool Init();
	bool Start();
	void Stop();
	void Join();
	void Teardown();

	void AddNewBlock(TEVIDENCEINFO tFileInfo);
	uint64 GetLocalLatestBlockNo();
	uint64 GetLatestHyperBlockNo();
	VEC_T_BLOCKINFO GetBlockInfoByIndex(uint64 start, uint64 end);
	string GetChainData();
	uint32 GetStrongNodeNum();
	uint32 GetAverageNodeNum();
	uint32 GetWeakNodeNum();
	uint32 GetOfflineNodeNum();
	uint32 GetSendRegisReqNum();
	uint32 GetSendConfirmingRegisReqNum();
	uint32 GetRecvRegisReqNum();
	uint32 GetRecvConfirmingRegisReqNum();
	bool VerifyPoeRecord(string &checkFileHash, P_TEVIDENCEINFO pCheckInfo);
	uint64 GetAllNodesInTheNet();
	void GetSendingRate(string &sendRate);
	void GetSentSize(string &allSendSize);
	void GetRecvingRate(string &recvRate);
	void GetRecvedSize(string &allRecvSize);

	VEC_T_NODEINFO GetOtherLocalChain(uint16 chainNum);
	VEC_T_NODEINFO GetMyLocalChain();
	
	void GetNodeDescription(string &info, string &ip, uint16 &uiport);

	VEC_T_HYPERBLOCKDBINFO ChainDataPersist();

	VEC_T_QUERYSHOWINFO GetBlockInfoByHash(string &hash);
	VEC_T_QUERYSHOWINFO GetBlockInfoByTime(string &time);
	VEC_T_QUERYSHOWINFO GetBlockInfoByBlockNum(string &info);
	VEC_T_QUERYSHOWINFO GetBlockInfoByCustomInfo(string &cusInfo);

	uint64 GetElaspedTimeOfCurrentConsensus();
	uint64 GetStartTimeOfConsensus();

private:
	uint32 GetNodeNum(uint32 uiStart, uint32 uiEnd);

	string PrintAllPeerNodeList();
	string PrintAllLocalBlocks();
	string PrintBlockNodeMap();

	void SendGetPeerListReq(int8 *strName, uint32 uiIP, uint16 usPort);		
	void SavePeerList();													
	void SendBlockNodeMapReq();
	void SendLoginReq(uint32 uiIP, uint16 usPort);							
	void SearchPeerList(char *strName, uint32 uiIPIN, uint16 usPortIN, uint32 uiIPOUT, uint16 usPortOUT, bool bFlag, uint64 uiTime, uint16 uiNodeState);
	void SendBlockToPeer(uint32 uiIP, uint16 usPort, uint64 uiBlockNum);	
	void SendPeerList(uint32 uiIP, uint16 usPort);
	void SendToOutPeerWantNATTraversalReq(T_PPEERINFO tPpeerInfo);
	void GreateGenesisBlock();
	void SendDataToPeer(char *buf, uint32 bufLen);
	void SendNodeState(uint8 nodeState);

	
	void ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPingRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPeerListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPeerListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessWantNATTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessNATTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessSomeNodeWantToConnectYouReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetBlockNodeMapReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetBlockNodeMapRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessAddBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);

	static void* UdpProcessEntry(void* pParam);			
	void UdpProcessImp();								

	static void* UpdateDataProcessEntry(void* pParam);
	void UpdateDataProcessImp();						

	static void* WriteStatusProcessEntry(void* pParam); 
	void WriteStatusProcessImp();

	CThreadObj                              m_threadUdpProcess;
	TCallbackFuncObj<pThreadCallbackFunc>   m_funcUdpProcess;

	CThreadObj                              m_threadUpdateDataProcess;
	TCallbackFuncObj<pThreadCallbackFunc>   m_funcUpdateDataProcess;

	CThreadObj                              m_threadWriteStatusProcess;
	TCallbackFuncObj<pThreadCallbackFunc>   m_funcWriteStatusProcess;

	LIST_T_PPEERINFO m_PeerInfoList;
	CMutexObj		 m_MuxPeerInfoList;
	
	LIST_T_HYPERBLOCK   m_HchainBlockList;
	CMutexObj			m_MuxHchainBlockList;

	MAP_BLOCK_STATE m_BlockStateMap;
	CMutexObj       m_MuxBlockStateMap;

	CUdpSocket      m_UdpSocket;
	T_PEERINFO		m_MyPeerInfo;

	
};
#endif//__HCHAINP2P_MANAGER__