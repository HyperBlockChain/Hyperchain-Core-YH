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

#ifndef __HCHAINP2P_MANAGER__
#define __HCHAINP2P_MANAGER__

#include "utility/CallbackFuncObj.h"
#include "utility/ThreadObj.h"
#include "network/udpsocket.h"
#include "headers/inter_public.h"

class QtNotify;
typedef struct _tBuddyInfo 
{
	uint64 uiCurBuddyNo;			
	uint16 usBlockNum;				
	uint16 usChainNum;				
	uint8 eBuddyState;				

	uint64 GetCurBuddyNo()const;
	uint16 GetBlockNum()const;
	uint16 GetChainNum()const;
	uint8 GetBuddyState()const;

	void SetBlockNum(uint16 num);

}T_STRUCTBUDDYINFO, *T_PSTRUCTBUDDYINFO;

typedef struct _tOnChainHashInfo 
{
	uint64 uiTime;					
	string strHash;					

	uint64 GetTime()const;
	string GetHash()const;

	void Set(uint64 t, string h);

}T_ONCHAINHASHINFO, *T_PONCHAINHASHINFO;

typedef struct _tp2pmanagerstatus
{
	bool bStartGlobalFlag;			
	bool bHaveOnChainReq;			
	bool bHaveRecvCopyHyperBlock;   
	int32 uiMaxBlockNum;			
	uint16 usBuddyPeerCount;		
	uint16 uiNodeState;				
	uint64 uiSendRegisReqNum;		
	uint64 uiRecvRegisReqNum;			
	uint64 uiSendConfirmingRegisReqNum;
	uint64 uiRecvConfirmingRegisReqNum;
	T_HYPERBLOCK tPreHyperBlock;	
	T_LOCALBLOCK tPreLocalBlock;	
	uint64 uiStartTimeOfConsensus;	
	uint64 uiNextStartTimeNewest;	
	uint64 uiTimeOfConsensus;		
	uint16 usGlobalBuddyChainCount; 
	bool bLocalBuddyChainState;
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

	T_ONCHAINHASHINFO tOnChainHashInfo;

	uint64 uiSendPoeNum;
	uint64 uiRecivePoeNum;

	T_STRUCTBUDDYINFO tBuddyInfo;

	void clearStatus();

	_tp2pmanagerstatus()
	{
		clearStatus();
	}

	bool StartGlobalFlag()const;

	bool HaveOnChainReq()const;

	bool HaveRecvCopyHyperBlock()const;

	int32 GetMaxBlockNum()const;

	uint16 GetBuddyPeerCount()const;

	uint16 GetNodeState()const;

	uint64 GetSendRegisReqNum()const;

	uint64 GetRecvRegisReqNum()const;

	uint64 GetSendConfirmingRegisReqNum()const;

	uint64 GetRecvConfirmingRegisReqNum()const;

	uint64 GetStartTimeOfConsensus()const;

	uint64 GetNextStartTimeNewest()const;

	uint64 GetTimeOfConsensus()const;

	uint16 GetGlobalBuddyChainCount()const;

	bool LocalBuddyChainState()const;

	T_PEERADDRESS GetLocalBuddyAddr()const;

	T_HYPERBLOCK GetPreHyperBlock()const;
	uint64 GetSendPoeNum()const;

	void SetSendPoeNum(uint64 num);

	LIST_T_LOCALCONSENSUS& GetListOnChainReq();
	void SetP2pmanagerstatus(T_HYPERBLOCK PreHyperBlock, int32 MaxBlockNum, bool ClearlistGlobalBuddyChainInfo, bool ClearlistLocalBuddyChainInfo, 
		bool HaveOnChainReq, bool LocalBuddyChainState, uint64 NextStartTimeNewest, uint64 TimeOfConsensus, uint64 SendConfirmingRegisReqNum, uint16 NodeState);


	void SetStartGlobalFlag(bool flag);

	void SetHaveOnChainReq(bool haveOnChainReq);

	void SetHaveRecvCopyHyperBlock(bool haveRecvCopyHyperBlock);

	void SetLocalBuddyChainState(bool state);

	void SetBuddyPeerCount(uint16 count);

	void SetNodeState(uint16 state);

	void SetSendRegisReqNum(uint64 num);

	void SetRecvRegisReqNum(uint64 num);

	void SetSendConfirmingRegisReqNum(uint64 num);

	void SetRecvConfirmingRegisReqNum(uint64 num);

	void SetStartTimeOfConsensus(uint64 t);

	void SetNextStartTimeNewest(uint64 t);

	void SetTimeOfConsensus(uint64 t);

	void SetGlobalBuddyChainCount(uint16 count);

	void SetGlobalBuddyAddr(T_PEERADDRESS addr);

	void SetMaxBlockNum(int32 num);

	void SetPreHyperBlock(const T_HYPERBLOCK&block);

	void SetPreLocalBlock(const T_LOCALBLOCK&block);

	void ClearGlobalBuddyChainInfo();

	void ClearLocalBuddyChainInfo();

	uint64 GetRecvPoeNum()const;
	
	void SetRecvPoeNum(uint64 num);

	T_STRUCTBUDDYINFO GetBuddyInfo()const;

	void SetBuddyInfo(T_STRUCTBUDDYINFO info);

}T_P2PMANAGERSTATUS, *T_PP2PMANAGERSTATUS;

class CHChainP2PManager
{
public:
	CHChainP2PManager(void);
	~CHChainP2PManager(void);

public:
	bool SetQtNotify(QtNotify *pnotify);
	bool Init();
	bool Start();
	void Stop();
	void Join();
	void Teardown();

	////qt interface
	void AddNewBlock(TEVIDENCEINFO tFileInfo, const char *rand);
	void AddNewBlockEx(TEVIDENCEINFO tFileInfo, const char *rand, string& localhash, string& strtime);
	void AddNewScriptBlock(string script, const char *rand);
	void DeleteFailedPoe(string script, uint64 time);
	uint64 GetLocalLatestBlockNo();
	uint64 GetLatestHyperBlockNo();
	VEC_T_BLOCKINFO GetBlockInfoByIndex(uint64 start, uint64 end);
	string GetChainData();
	
	int64 GetOnChainState(string fileHash);
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
	VEC_T_PPEERCONF GetPeerInfo();
	uint16 GetStateOfCurrentConsensus(uint64 &blockNo, uint16 &blockNum, uint16 &chainNum);
	uint32 GetConnectedNodesNum();

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
	void GetHyperBlockNumInfoFromLocal();
	uint16 GetPoeRecordListNum();
	LIST_T_LOCALCONSENSUS GetPoeRecordList();

	void GetHyperBlockInfoFromP2P(uint64 start, uint64 end);
	//int testNum;
private:
	uint32 GetNodeNum(uint32 uiStart, uint32 uiEnd);

	T_FILEINFO GetFileInfoFormPayload(string str);
	string PrintAllPeerNodeList();			
	string PrintAllLocalBlocks();				
	string getBlockInfo(T_HYPERBLOCK hyperBlock);
	string PrintBlockNodeMap();			

	void WriteBlockLog(T_HYPERBLOCK hyperBlock);
	void SendGetPeerListReq(int8 *strName, uint32 uiIP, uint16 usPort);		
	void SavePeerList();													
	void SendBlockNodeMapReq();											
	void SendLoginReq(uint32 uiIP, uint16 usPort);							
	void SearchPeerList(char *strName, uint32 uiIPIN, uint16 usPortIN, uint32 uiIPOUT, uint16 usPortOUT, bool bFlag, uint64 uiTime, uint16 uiNodeState);t
	void SendBlockToPeer(uint32 uiIP, uint16 usPort, uint64 uiBlockNum);	
	void SendOneBlockToPeer(uint32 uiIP, uint16 usPort, uint64 uiBlockNum);
	void SendPeerList(uint32 uiIP, uint16 usPort);					
	void SendToOutPeerWantNatTraversalReq(T_PPEERINFO tPpeerInfo);					
	void GreateGenesisBlock();
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
	void copyLocalBuddyList(LIST_T_LOCALCONSENSUS &endList, LIST_T_LOCALCONSENSUS fromList);
	void SendConfirmReq(char* pszIP, unsigned short usPort, string hash, uint8 state);
	void SendConfirmRsp(char* pszIP, unsigned short usPort, string hash);
	void SendRefuseReq(char* pszIP, unsigned short usPort, string hash, uint8 type);
	void SendCopyLocalBlock(T_LOCALCONSENSUS localBlock);
	void ReOnChainFun();
	void GetNewHyperInfoAndReOnChain();
	uint16 HyperBlockInListOrNot(uint64 blockNum, uint64 blockCount, T_SHA256 tHashSelf);
	void SendConfirmFin(char* pszIP, unsigned short usPort, string hash);
	bool CurBuddyBlockInTheHyperBlock(T_HYPERBLOCK blockInfos);
	int GetChainUsedMemory();
	void ClearHalfMemoryCache();

	void ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPingRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPeerListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessPeerListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessWantNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessSomeNodeWantToConnectYouReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetBlockNodeMapReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessAddBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessCopyBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessOnChainConfirmMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetBlockNodeMapRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGlobalBuddyReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGlobalBuddyRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessCopyHyperBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessCopyHyperBlockRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetHyperBlockByNoReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void ProcessGetHyperBlockByNoRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen);
	void SendOneHyperBlockByNo(uint32 uiIP, uint16 usPort, uint64 uiBlockNum);
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

	static void* MemCacheProcessEntry(void*pParam);
	void MemCacheProcessImp();

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

	CThreadObj                              m_threadMemCacheProsess;
	TCallbackFuncObj<pThreadCallbackFunc>   m_funcMemCacheProcess;

	LIST_T_PPEERINFO m_PeerInfoList;
	CMutexObj		 m_MuxPeerInfoList;
	
	LIST_T_HYPERBLOCK   m_HchainBlockList;
	LIST_T_HYPERBLOCKNEW m_HchainBlockListNew;
	CMutexObj			m_MuxHchainBlockList;

	MAP_BLOCK_STATE m_BlockStateMap;
	CMutexObj       m_MuxBlockStateMap;

	CUdpSocket      m_UdpSocket;
	T_PEERINFO		m_MyPeerInfo;
	QtNotify		*m_qtnotify;

	
};
#endif