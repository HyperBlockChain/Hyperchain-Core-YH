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
	bool  bStartGlobalFlag;
	bool  bHaveOnChainReq;
	bool  bHaveRecvCopyHyperBlock;
	bool bLocalBuddyChainState;
	int32 uiMaxBlockNum;
	uint16 usBuddyPeerCount;
	uint16 uiNodeState;
	uint64 uiSendRegisReqNum;
	uint64 uiSendConfirmingRegisReqNum;
	uint64 uiRecvConfirmingRegisReqNum;
	uint64 uiRecvRegisReqNum;
	T_HYPERBLOCK tPreHyperBlock;
	T_LOCALBLOCK tPreLocalBlock;
	uint64 uiStartTimeOfConsensus;
	uint64 uiNextStartTimeNewest;
	uint64 uiTimeOfConsensus;
	uint16 usGlobalBuddyChainCount;
	T_PEERADDRESS tLocalBuddyAddr;
	T_LOCALCONSENSUS curBuddyBlock;

	CMutexObj		 MuxlistOnChainReq;
	LIST_T_LOCALCONSENSUS listOnChainReq;

	CMutexObj		 MuxlistLocalBuddyChainInfo;
	LIST_T_LOCALCONSENSUS listLocalBuddyChainInfo;
	
	CMutexObj		 MuxlistGlobalBuddyChainInfo;
	LIST_LIST_GLOBALBUDDYINFO listGlobalBuddyChainInfo;

	CMutexObj		 MuxlistRecvLocalBuddyRsp;
	LIST_T_BUDDYINFO  listRecvLocalBuddyRsp;

	CMutexObj		 MuxlistCurBuddyRsp;
	LIST_T_BUDDYINFOSTATE listCurBuddyRsp;

	CMutexObj		 MuxlistRecvLocalBuddyReq;
	LIST_T_BUDDYINFO  listRecvLocalBuddyReq;

	CMutexObj		 MuxlistCurBuddyReq;
	LIST_T_BUDDYINFOSTATE listCurBuddyReq;

	CMutexObj			MuxMapSearchOnChain;
	MAP_T_SEARCHONCHAIN mapSearchOnChain;

	void clearStatus()
	{
		bStartGlobalFlag = false;
		bHaveOnChainReq = false;
		bLocalBuddyChainState = false;
		bHaveRecvCopyHyperBlock = false;
		usGlobalBuddyChainCount = 1;
		uiMaxBlockNum = 0;
		uiNodeState = DEFAULT_REGISREQ_STATE;
		usBuddyPeerCount = 0;
		uiSendRegisReqNum = 0;
		uiSendConfirmingRegisReqNum = 0;
		uiRecvRegisReqNum = 0;
		uiRecvConfirmingRegisReqNum = 0;
		uiStartTimeOfConsensus = 0;
		uiTimeOfConsensus = 0;
		listLocalBuddyChainInfo.clear();
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

	void AddNewBlock(TEVIDENCEINFO tFileInfo, const char *rand);
	uint64 GetCurBlockNumOfMyself();
	uint64 GetCurBlockNumOfAllNode();
	VEC_T_BLOCKINFO GetBlockInfoByIndex(uint64 start, uint64 end);
	string GetChainData();
	
	int64 GetOnChainState(string fileHash);
	uint64 GetLatestHyperBlockNo();
	uint32 GetStrongHyperBlockNo();
	uint32 GetAverageHyperBlockNo();
	uint32 GetWeakHyperBlockNo();
	uint32 GetOfflineHyperBlockNo();
	uint32 GetSendRegisReqNum();
	uint32 GetSendConfirmingRegisReqNum();
	uint32 GetRecvRegisReqNum();
	uint32 GetRecvConfirmingRegisReqNum();
	bool MatchFile(string &checkFileHash, P_TEVIDENCEINFO pCheckInfo);
	uint64 GetAllNodesInTheNet();
	void GetSendingRate(string &sendRate);
	void GetSentSize(string &allSendSize);
	void GetRecvingRate(string &recvRate);
	void GetRecvedSize(string &allRecvSize);

	VEC_T_NODEINFO GetOtherLocalChain(uint16 chainNum);
	VEC_T_NODEINFO GetMyLocalChain();
	
	void GetNodeDescription(string &info, string &ip, uint16 &uiport); 

	VEC_T_HYPERBLOCKDBINFO ChainDataPersist();

	VEC_T_BROWSERSHOWINFO GetBlockInfoByHash(string &hash);
	VEC_T_BROWSERSHOWINFO GetBlockInfoByTime(string &time);
	VEC_T_BROWSERSHOWINFO GetBlockInfoByBlockNum(string &info);
	VEC_T_BROWSERSHOWINFO GetBlockInfoByCustomInfo(string &cusInfo);

	uint64 GetElaspedTimeOfCurrentConsensus();
	uint64 GetStartTimeOfConsensus();

	void SaveHyperBlockToLocal(T_HYPERBLOCK tHyperBlock);
	void SaveLocalBlockToLocal(T_HYPERBLOCK tHyperBlock);
	void GetHyperBlockInfoFromLocal();
	uint16 GetPoeRecordListNum();
private:
	uint32 GetNodeNum(uint32 uiStart, uint32 uiEnd);

	string getPeerListInfo();			
	string getBlockInfo();				
	string getBlockInfo(T_HYPERBLOCK hyperBlock);
	string getBlockStateInfo();			

	void WriteBlockLog(T_HYPERBLOCK hyperBlock);
	void SendGetPeerListReq(int8 *strName, uint32 uiIP, uint16 usPort);		
	void SavePeerList();													
	void SendGetChainStateReq();											
	void SendLoginReq(uint32 uiIP, uint16 usPort);							
	void SearchPeerList(char *strName, uint32 uiIPIN, uint16 usPortIN, uint32 uiIPOUT, uint16 usPortOUT, bool bFlag, uint64 uiTime, uint16 uiNodeState);
	void SendBlockToPeer(uint32 uiIP, uint16 usPort, uint64 uiBlockNum);	
	void SendPeerListToPeer(uint32 uiIP, uint16 usPort);					
	void SendToOutPeerWantNatTraversalReq(T_PPEERINFO tPpeerInfo);					
	void CreateGenesisHyperBlock();
	bool JudgExistAtGlobalBuddy(LIST_T_LOCALCONSENSUS listLocalBuddyChainInfo);
	void SendDataToPeer(char *buf, uint32 bufLen);
	void ChangeLocalBlockPreHash(LIST_T_LOCALCONSENSUS &localList);
	void ReOnChain(T_LOCALCONSENSUS localInfo);
	void SendOnChainRsp(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void SendLocalBuddyReq();
	void GetOnChainInfo();
	void StartGlobalBuddy();
	bool JudgExistAtLocalBuddy(LIST_T_LOCALCONSENSUS localList,T_LOCALCONSENSUS localBlockInfo);
	void SendGlobalBuddyReq();
	bool CreatHyperBlock();
	void SendNodeState(uint8 nodeState);
	void GetHyperBlockByNo(uint64 blockNum);
	void CopyLocalBuddyList(LIST_T_LOCALCONSENSUS &endList, LIST_T_LOCALCONSENSUS fromList);
	void SendConfirmReq(char* pszIP, unsigned short usPort, string hash, uint8 state);
	void SendConfirmRsp(char* pszIP, unsigned short usPort, string hash);
	void SendRefuseReq(char* pszIP, unsigned short usPort, string hash, uint8 type);
	void SendCopyLocalBlock(T_LOCALCONSENSUS localBlock);
	void ReOnChainFunc();
	void GetNewHyperInfoAndReOnChain();
	uint16 HyperBlockInListOrNot(uint64 blockNum, uint64 blockCount, T_SHA256 tHashSelf);
	void SendConfirmFin(char* pszIP, unsigned short usPort, string hash);
	bool CurBuddyBlockInTheHyperBlock(T_HYPERBLOCK blockInfos);
	void SendOneHyperBlockByNo(uint32 uiIP, uint16 usPort, uint64 uiBlockNum);

	void ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPingRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPeerListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPeerListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessWantNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessSomeNodeWantToConnectYouReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetBlockStateReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessAddBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessCopyBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainConfirmMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetBlockStateRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGlobalBuddyReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGlobalBuddyRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessCopyHyperBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessCopyHyperBlockRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetHyperBlockByNoReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetHyperBlockByNoRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainAck1RspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainAck2RspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainFinRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainConfirmRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessRefuseReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainConfirmFinMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);

	static void* UdpProcessEntry(void* pParam);			
	void UdpProcessImp();								

	static void* UpdateDataProcessEntry(void* pParam);
	void UpdateDataProcessImp();						

	static void* WriteStatusProcessEntry(void* pParam); 
	void WriteStatusProcessImp();

	static void* RecvLocalBuddyPackReqThreadEntry(void* pParam); 
	void RecvLocalBuddyPackReqThreadEntryImp();

	static void* RecvLocalBuddyPackRspThreadEntry(void* pParam); 
	void RecvLocalBuddyPackRspThreadEntryImp();

	static void* LocalBuddyThreadEntry(void* pParam); 
	void LocalBuddyThreadEntryImp();
	
	static void* SearchOnChainStateEntry(void *pParam);
	void SearchOnChainStateEntryImp();

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