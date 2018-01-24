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
#include "HChainP2PManager.h"
#include "network/p2pprotocol.h"
#include "headers/UUFile.h"
#include "headers/inter_public.h"
#include "crypto/sha2.h"
#include <mutex>
#include <thread>
#include "../db/dbmgr.h"
#include "interface/QtInterface.h"
#include "HttpUnit/HttpUnit.h"
#include <QJsonDocument>  
#include <QDebug> 
#include <QJsonObject>
#include "db/HyperchainDB.h"
#define LEAST_START_GLOBAL_BUDDY_NUM    (2)
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
#define MAX_SEND_LOGIN_PEER_NODE        (2)
#define HYPERBLOCK_SYNC_TIMES			(2)
#define TEMP_BUF_LEN					(256)
#define MAX_IP_LEN						(32)
#define ONE_SECOND						(1000)
#define ONE_MIN							(60*ONE_SECOND)
#define NOT_START_BUDDY_NUM				(1)
#define BIN_UINT						(1024)
#define FIVE_MIN					(5*60)
#define HALF_AN_HOUR				(30*60)
#define ONE_HOUR					(60*60)

#define UNICODE_POS					(2)

#define ONE_LOCAL_BLOCK				(1)


#ifdef WIN32
extern void win_gettimeofday(struct timeval *tp);
#define GETTIMEOFDAY(ptr)	win_gettimeofday(ptr)
#else
#define GETTIMEOFDAY(ptr)	gettimeofday(ptr, 0)
#endif

extern UUFile m_uufile;
extern T_CONFFILE	g_confFile;
extern void SHA256(unsigned char* sha256, const char* str, long long length);

CMutexObj g_MuxP2pManagerStatus;
T_P2PMANAGERSTATUS g_tP2pManagerStatus;

std::mutex muLock;
bool g_bWriteStatusIsSet = true;

void convertUnCharToStr(char* str, unsigned char* UnChar, int ucLen)
{
	int i = 0;
	for (i = 0; i < ucLen; i++)
	{
		sprintf(str + i * UNICODE_POS, "%02x", UnChar[i]);
	}
}

struct Cmpare
{
	bool operator()(const T_PPEERINFO st1, const T_PPEERINFO  st2) const
	{
		return (st1->uiTime < st2->uiTime);
	}
};


struct CmpareRecvLocalBuddyReq
{
	bool operator()(const T_BUDDYINFO st1, const T_BUDDYINFO  st2) const
	{
		return (st1.tType < st2.tType);
	}
};

struct CmpareOnChain
{
	bool operator()(const T_LOCALCONSENSUS st1, const T_LOCALCONSENSUS  st2) const
	{
		return (st1.tLocalBlock.tBlockBaseInfo.tHashSelf < st2.tLocalBlock.tBlockBaseInfo.tHashSelf);
	}
};
struct CmpareOnChainLocal
{   
	bool operator()(const T_LOCALBLOCK st1, const T_LOCALBLOCK st2) const
	{
		return (st1.tBlockBaseInfo.tHashSelf < st2.tBlockBaseInfo.tHashSelf);
	}
};

struct CmpareGlobalBuddy
{
	bool operator()(LIST_T_LOCALCONSENSUS st1, LIST_T_LOCALCONSENSUS  st2) const
	{
		ITR_LIST_T_LOCALCONSENSUS itr1 = st1.begin();
		ITR_LIST_T_LOCALCONSENSUS itr2 = st2.begin();

		return ((*itr1).tLocalBlock.tBlockBaseInfo.tHashSelf < (*itr2).tLocalBlock.tBlockBaseInfo.tHashSelf);
	}
};

CHChainP2PManager::CHChainP2PManager(void)
{
}
CHChainP2PManager::~CHChainP2PManager(void)
{
}

void CHChainP2PManager::SendNodeState(uint8 nodeState)
{
}

void CHChainP2PManager::GetHyperBlockInfoFromLocal()
{
	HyperBlockDB hyperchainDB;

	CHyperchainDB::GetLatestHyperBlock(hyperchainDB);

	if (hyperchainDB.hyperBlock.uiBlockId != 0)
	{
		T_HYPERBLOCK hyperBlock;
		hyperBlock.tBlockBaseInfo.uiID = hyperchainDB.hyperBlock.uiBlockId;
		hyperBlock.tBlockBaseInfo.uiTime = hyperchainDB.hyperBlock.uiBlockTimeStamp;
		strncpy(hyperBlock.tBlockBaseInfo.strAuth, hyperchainDB.hyperBlock.strAuth.c_str(), MAX_AUTH_LEN);
		strncpy(hyperBlock.tBlockBaseInfo.strScript, hyperchainDB.hyperBlock.strScript.c_str(), MAX_SCRIPT_LEN);
		memcpy((char*)hyperBlock.tBlockBaseInfo.tHashSelf.pID, (char*)hyperchainDB.hyperBlock.strHashSelf, DEF_SHA256_LEN);
		
		memcpy((char*)hyperBlock.tBlockBaseInfo.tPreHash.pID, (char*)hyperchainDB.hyperBlock.strPreHash, DEF_SHA256_LEN);
		memcpy((char*)hyperBlock.tHashAll.pID, (char*)hyperchainDB.hyperBlock.strHashAll, DEF_SHA256_LEN);
		
		uint64 chainNumTemp  = 1;
		bool index = false;
		LIST_T_LOCALBLOCK listLocakBlockTemp;
		LocalChainDB::iterator itrMap = hyperchainDB.mapLocalChain.begin();
		for (itrMap; itrMap != hyperchainDB.mapLocalChain.end(); itrMap++)
		{
			index = true;
			LIST_T_LOCALBLOCK listLocalBlockTemp;
			LocalBlockDB::iterator itrMapBlock = (*itrMap).second.begin();
			for (itrMapBlock; itrMapBlock != (*itrMap).second.end(); itrMapBlock++)
			{
				T_LOCALBLOCK pLocalTemp;
				strncpy(pLocalTemp.tBlockBaseInfo.strAuth, (*itrMapBlock).second.strAuth.c_str(), MAX_AUTH_LEN);
				strncpy(pLocalTemp.tBlockBaseInfo.strScript, (*itrMapBlock).second.strScript.c_str(), MAX_SCRIPT_LEN);
				memcpy((char*)pLocalTemp.tBlockBaseInfo.tHashSelf.pID, (char*)(*itrMapBlock).second.strHashSelf, DEF_SHA256_LEN);
				memcpy((char*)pLocalTemp.tBlockBaseInfo.tPreHash.pID, (char*)(*itrMapBlock).second.strPreHash, DEF_SHA256_LEN);
				pLocalTemp.tBlockBaseInfo.uiID = (*itrMapBlock).second.uiBlockId;
				pLocalTemp.tBlockBaseInfo.uiTime = (*itrMapBlock).second.uiBlockTimeStamp;

				memcpy((char*)pLocalTemp.tHHash.pID, (char*)(*itrMapBlock).second.strHyperBlockHash, DEF_SHA256_LEN);
				pLocalTemp.uiAtChainNum = (*itrMapBlock).second.uiLocalChainId;
			
				string tempTest = (*itrMapBlock).second.strPayload.c_str();
				int nPos = tempTest.find("fileName=");
				int nPos1 = tempTest.find("fileHash=");
				string fileHash = tempTest.substr(nPos1 + 9, nPos - (nPos1 + 9));
				char tempBuf[DEF_SHA512_LEN];
				memset(tempBuf, 0, DEF_SHA512_LEN);
				strncpy(tempBuf, fileHash.c_str(), DEF_SHA512_LEN);

				nPos1 = tempTest.find("fileCustom=");
				string fileName = tempTest.substr(nPos+9, nPos1-(nPos+9));
				strncpy(pLocalTemp.tPayLoad.tPayLoad.fileName, fileName.c_str(), fileName.length());

				nPos = tempTest.find("fileSize=");
				string fileCustom = tempTest.substr(nPos1+1, nPos);
				strncpy(pLocalTemp.tPayLoad.tPayLoad.customInfo, fileCustom.c_str(), fileCustom.length());

				memset(pLocalTemp.tPayLoad.tPayLoad.tFileOwner, 0, MAX_CUSTOM_INFO_LEN);
				
				pLocalTemp.tPayLoad.tPayLoad.uiFileSize = INIT_SIZE;
				listLocalBlockTemp.push_back(pLocalTemp);
			}
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			hyperBlock.listPayLoad.push_back(listLocalBlockTemp);
		}
		
		if (index)
		{
			m_HchainBlockList.push_back(hyperBlock);
			WriteBlockLog(hyperBlock);
		}

		if (g_tP2pManagerStatus.uiMaxBlockNum < hyperBlock.tBlockBaseInfo.uiID)
		{
			g_tP2pManagerStatus.uiMaxBlockNum = hyperBlock.tBlockBaseInfo.uiID;
			g_tP2pManagerStatus.tPreHyperBlock = hyperBlock;
		}
		
	}
	
}
bool CHChainP2PManager::Init()
{
	strncpy(m_MyPeerInfo.strName, g_confFile.strLocalNodeName.c_str(), MAX_NODE_NAME_LEN);
	m_MyPeerInfo.tPeerInfoByMyself.uiIP = g_confFile.uiLocalIP;
	m_MyPeerInfo.tPeerInfoByMyself.uiPort = g_confFile.uiLocalPort;
	m_MyPeerInfo.tPeerInfoByOther.uiIP = 0;
	m_MyPeerInfo.tPeerInfoByOther.uiPort = 0;
	m_MyPeerInfo.uiTime = INIT_TIME;
	m_MyPeerInfo.uiState = DEFAULT_NET;
	g_tP2pManagerStatus.usBuddyPeerCount = g_confFile.uiSaveNodeNum;
	g_tP2pManagerStatus.uiNodeState = DEFAULT_REGISREQ_STATE;

	if (g_tP2pManagerStatus.uiMaxBlockNum == 0)
	{
		CreateGenesisHyperBlock();
	}
	else
	{
		g_tP2pManagerStatus.uiStartTimeOfConsensus = time(NULL);
		
	}


	ITR_VEC_T_PPEERCONF itr = g_confFile.vecPeerConf.begin();
	for (; itr != g_confFile.vecPeerConf.end(); itr++)
	{
		T_PPEERINFO pPeerInfo = new T_PEERINFO;

		pPeerInfo->tPeerInfoByMyself.uiIP = (*itr)->tPeerAddr.uiIP;
		pPeerInfo->tPeerInfoByMyself.uiPort = (*itr)->tPeerAddr.uiPort; 
		pPeerInfo->tPeerInfoByOther.uiIP = (*itr)->tPeerAddrOut.uiIP;
		pPeerInfo->tPeerInfoByOther.uiPort = (*itr)->tPeerAddrOut.uiPort;
		if ((pPeerInfo->tPeerInfoByMyself.uiIP == pPeerInfo->tPeerInfoByOther.uiIP)
			&& (pPeerInfo->tPeerInfoByMyself.uiPort == pPeerInfo->tPeerInfoByOther.uiPort))
		{
			pPeerInfo->uiState = OUTNET;
			pPeerInfo->uiNatTraversalState = GOOD;
		}
		else
		{
			pPeerInfo->uiState = INNERNET;
			pPeerInfo->uiNatTraversalState = DEFAULT_NATTRAVERSAL_STATE;
		}
		pPeerInfo->uiNodeState = DEFAULT_REGISREQ_STATE;
		pPeerInfo->uiTime = INIT_TIME;
		strncpy(pPeerInfo->strName, (*itr)->strName, MAX_NODE_NAME_LEN);
		m_PeerInfoList.push_back(pPeerInfo);

		m_PeerInfoList.sort(Cmpare());
	}
	
	m_funcUdpProcess.Set(UdpProcessEntry, this);
	m_funcUpdateDataProcess.Set(UpdateDataProcessEntry, this);
	m_funcWriteStatusProcess.Set(WriteStatusProcessEntry, this);
	
	std::thread RecvLocalBuddyPackReqThread(RecvLocalBuddyPackReqThreadEntry, this);
	RecvLocalBuddyPackReqThread.detach();

	std::thread RecvLocalBuddyPackRspThread(RecvLocalBuddyPackRspThreadEntry, this);
	RecvLocalBuddyPackRspThread.detach();

	std::thread LocalBuddyThread(LocalBuddyThreadEntry, this);
	LocalBuddyThread.detach();

	std::thread SearchOnChainStateThread(SearchOnChainStateEntry, this);
	SearchOnChainStateThread.detach();

	char* pszLocalIP = NULL;

	struct in_addr addPeerIP;
	addPeerIP.s_addr = g_confFile.uiLocalIP;
	
	if (1 != m_UdpSocket.Init(inet_ntoa(addPeerIP), g_confFile.uiLocalPort))
		return false;
	m_UdpSocket.SetUsed(true);

	SLEEP(1* ONE_SECOND);
	uint16 uiIndex = 0;
	ITR_VEC_T_PPEERCONF itrConf = g_confFile.vecPeerConf.begin();
	for (; itrConf != g_confFile.vecPeerConf.end(); itrConf++)
	{
		if (uiIndex > MAX_SEND_LOGIN_PEER_NODE)
		{
			break;
		}

		if ((*itrConf)->uiPeerState == OUTNET)
		{
			SendLoginReq((*itrConf)->tPeerAddr.uiIP, (*itrConf)->tPeerAddr.uiPort);
			uiIndex++;
		}
	}
	return true;
}
bool CHChainP2PManager::Start()
{
	if (0 != m_threadUdpProcess.Start(&m_funcUdpProcess))
		return false;

	if (0 != m_threadUpdateDataProcess.Start(&m_funcUpdateDataProcess))
		return false;

	if (0 != m_threadWriteStatusProcess.Start(&m_funcWriteStatusProcess))
		return false;

	return true;
}
void CHChainP2PManager::Stop()
{
	m_threadUdpProcess.Kill();
	m_threadUpdateDataProcess.Kill();
	m_threadWriteStatusProcess.Kill();
}
void CHChainP2PManager::Join()
{
	m_threadUdpProcess.Join();
	m_threadUpdateDataProcess.Join();
	m_threadWriteStatusProcess.Join();
}
void CHChainP2PManager::Teardown()
{

}
void *CHChainP2PManager::SearchOnChainStateEntry(void* pParam)
{
	CHChainP2PManager* pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->SearchOnChainStateEntryImp();
	return NULL;
}

void CHChainP2PManager::SearchOnChainStateEntryImp()
{
	while (1)
	{
		{
			CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxMapSearchOnChain);
			ITR_MAP_T_SEARCHONCHAIN itr = g_tP2pManagerStatus.mapSearchOnChain.begin();
			for (itr; itr != g_tP2pManagerStatus.mapSearchOnChain.end();)
			{
				uint64 timeNow = (uint64)time(NULL);
				if (timeNow - (*itr).second.uiTime > FIVE_MIN*2)
				{
					itr = g_tP2pManagerStatus.mapSearchOnChain.erase(itr);
				}
				else
				{
					itr++;
				}
			}
		}

		SLEEP(5 * ONE_MIN);
	}
}
void* CHChainP2PManager::LocalBuddyThreadEntry(void* pParam)
{
	CHChainP2PManager* pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->LocalBuddyThreadEntryImp();
	return NULL;
}

void CHChainP2PManager::ReOnChainFun()
{
	T_LOCALCONSENSUS localInfo;
	ITR_LIST_T_LOCALCONSENSUS itrList = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
	for (; itrList != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itrList++)
	{
		if ((*itrList).tPeer.tPeerAddr.uiIP == m_MyPeerInfo.tPeerInfoByMyself.uiIP
			&& (*itrList).tPeer.tPeerAddr.uiPort == m_MyPeerInfo.tPeerInfoByMyself.uiPort)
		{
			localInfo = (*itrList);
			char localHash[TEMP_BUF_LEN] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localInfo.tLocalBlock.tBlockBaseInfo.tHashSelf);

			g_tP2pManagerStatus.listLocalBuddyChainInfo.clear();
			g_tP2pManagerStatus.listOnChainReq.push_front(localInfo);
			break;
		}
	}
}
void CHChainP2PManager::GetNewHyperInfoAndReOnChain()
{
	uint i = 1;
	while (i<2)
	{
		if (g_tP2pManagerStatus.bHaveOnChainReq)
		{
			GetHyperBlockByNo(g_tP2pManagerStatus.uiMaxBlockNum + 1);
			i++;
			SLEEP(1 * ONE_SECOND);
		}
		else
		{
			break;
		}
	}
}
void CHChainP2PManager::LocalBuddyThreadEntryImp()
{

	bool indexGlobal = true;
	bool bIndexCreate = true;
	bool bFirstTimes = true;
	bool bCreatBlock = false;
	while (1)
	{		
		if ((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) <= LOCALBUDDYTIME)
		{
			bIndexCreate = true;
			bFirstTimes = true;
			bCreatBlock = false;
			uint64 tempNum = 0;
			uint64 tempNum1 = 0;
			{	
				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
				tempNum = g_tP2pManagerStatus.listCurBuddyRsp.size();

				CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyReq);
				tempNum1 = g_tP2pManagerStatus.listCurBuddyReq.size();
			}
			if (tempNum != 0 || tempNum1 != 0)
			{
				SLEEP(2 * ONE_SECOND);
				continue;
			}

			indexGlobal = false;
			
			if(!g_tP2pManagerStatus.bHaveOnChainReq)
			{
				GetOnChainInfo();
			}

			uint64 tempNum2 = 0;
			{
				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
				tempNum2 = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
			}
			if (tempNum2 == 0)
			{
				SLEEP(2 * ONE_SECOND);
				continue;
			}
			
			SendLocalBuddyReq();
			
			SLEEP(2 * ONE_SECOND);
		}
		else if (((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) > LOCALBUDDYTIME) && ((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) <= GLOBALBUDDYTIME))
		{
			uint64 tempNum = 0;
			{
				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
				tempNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
			}
			if (tempNum == 0 ||tempNum == 1)
			{
				SLEEP(2 * ONE_SECOND);
				continue;
			}

			if (!indexGlobal)
			{
				indexGlobal = true;
				StartGlobalBuddy();
			}
			else
			{
				SendGlobalBuddyReq();
			}
			SLEEP(2 * ONE_SECOND);
			continue;
		}
		else if (((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) > GLOBALBUDDYTIME) && ((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) <= NEXTBUDDYTIME))
		{
			g_tP2pManagerStatus.bStartGlobalFlag = false;
			indexGlobal = false;
		
			uint64 tempNum = 0;
			uint64 tempNum1 = 0;
			
			if (bFirstTimes)
			{
				{
					CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
					tempNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();

					CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
					tempNum1 = g_tP2pManagerStatus.listGlobalBuddyChainInfo.size();
				}
				if (tempNum != 0 && tempNum1 != 0)
				{
					bCreatBlock = CreatHyperBlock();
					if (!bCreatBlock)
					{
						GetNewHyperInfoAndReOnChain();
					}
				}
				else if (tempNum == 0 && tempNum1 == 0)
				{
					bIndexCreate = false;
				}
				
				bFirstTimes = false;
			}
			else
			{
				{
					GetNewHyperInfoAndReOnChain();
				}
				
			}
			SLEEP(2 * ONE_SECOND);
		}
		else
		{
			{
				CAutoMutexLock muxAutoG(g_MuxP2pManagerStatus);
  				if (g_tP2pManagerStatus.bHaveOnChainReq)
				{
					CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
					if (g_tP2pManagerStatus.listLocalBuddyChainInfo.size() == 1)
					{
						ReOnChainFun();
					}
					else
					{
						g_tP2pManagerStatus.listLocalBuddyChainInfo.clear();
					}

					CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
					g_tP2pManagerStatus.listGlobalBuddyChainInfo.clear();
					if (g_tP2pManagerStatus.bHaveOnChainReq)
					{
						g_tP2pManagerStatus.uiStartTimeOfConsensus += (NEXTBUDDYTIME);
					}
					g_tP2pManagerStatus.bHaveOnChainReq = false;
					g_tP2pManagerStatus.bLocalBuddyChainState = false;
					

				}
				else
				{
					{
						if (g_tP2pManagerStatus.uiStartTimeOfConsensus == g_tP2pManagerStatus.uiNextStartTimeNewest)
						{
							g_tP2pManagerStatus.uiNextStartTimeNewest = g_tP2pManagerStatus.uiStartTimeOfConsensus;
							g_tP2pManagerStatus.uiNextStartTimeNewest += (NEXTBUDDYTIME);
							g_tP2pManagerStatus.uiStartTimeOfConsensus = g_tP2pManagerStatus.uiNextStartTimeNewest;
						}
						else
						{
							if (g_tP2pManagerStatus.uiNextStartTimeNewest == 0)
							{
								g_tP2pManagerStatus.uiNextStartTimeNewest = g_tP2pManagerStatus.uiStartTimeOfConsensus;
								g_tP2pManagerStatus.uiNextStartTimeNewest += (NEXTBUDDYTIME);
							}

							g_tP2pManagerStatus.uiStartTimeOfConsensus = g_tP2pManagerStatus.uiNextStartTimeNewest;
						}
						
					}
				}
			}
			SLEEP(2 * ONE_SECOND);
		}
	}
}
void* CHChainP2PManager::RecvLocalBuddyPackRspThreadEntry(void* pParam)
{
	CHChainP2PManager* pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->RecvLocalBuddyPackRspThreadEntryImp();
	return NULL;        

}
void CHChainP2PManager::RecvLocalBuddyPackRspThreadEntryImp()
{

	uint8 listBuddyRspNum = 0;
	while (1)
	{
		if ((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) > LOCALBUDDYTIME)
		{
			{

				CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
				g_tP2pManagerStatus.listCurBuddyRsp.clear();

				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
				g_tP2pManagerStatus.listRecvLocalBuddyRsp.clear();
			}
			SLEEP(2 * ONE_SECOND);
			continue;
		}

		uint64 tempNumTest = 0;
		uint64 tempNum = 0;
		{
			CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
			tempNum = g_tP2pManagerStatus.listCurBuddyRsp.size();

			CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
			tempNumTest = g_tP2pManagerStatus.listRecvLocalBuddyRsp.size();
		}

		if (tempNumTest == 0)
		{
			SLEEP(2 * ONE_SECOND);
			continue;
		}

		if (tempNum > LIST_BUDDY_RSP_NUM)
		{
			SLEEP(2 * ONE_SECOND);
			continue;
		}
		else
		{
			T_BUDDYINFO localInfo;
			{
				CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
				localInfo = g_tP2pManagerStatus.listRecvLocalBuddyRsp.front();
				g_tP2pManagerStatus.listRecvLocalBuddyRsp.pop_front();
			}
			g_tP2pManagerStatus.tLocalBuddyAddr = localInfo.tPeerAddrOut;

			char pszPeerIP[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = localInfo.tPeerAddrOut.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));


			ProcessOnChainRspMsg(pszPeerIP, localInfo.tPeerAddrOut.uiPort, localInfo.recvBuf, localInfo.bufLen);
		}
	}
}

void* CHChainP2PManager::RecvLocalBuddyPackReqThreadEntry(void* pParam)
{
	CHChainP2PManager* pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->RecvLocalBuddyPackReqThreadEntryImp();
	return NULL;

}
void CHChainP2PManager::RecvLocalBuddyPackReqThreadEntryImp()
{

	uint8 listBuddyRspNum = 0;
	bool getReqFromList = true;
	while (1)
	{
		if ((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) > LOCALBUDDYTIME)
		{
			{
				CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyReq);
				g_tP2pManagerStatus.listCurBuddyReq.clear();
			
				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
				g_tP2pManagerStatus.listRecvLocalBuddyReq.clear();
			}
			SLEEP(2 * ONE_SECOND);
			continue;
		}

		uint64 tempNum = 0;
		uint64 tempNum1 = 0;
		uint64 tempNum2 = 0;
		{
			CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyReq);
			tempNum1 = g_tP2pManagerStatus.listCurBuddyReq.size();

			CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
			tempNum = g_tP2pManagerStatus.listRecvLocalBuddyReq.size();
			
		}
		

		if (tempNum == 0)
		{
			SLEEP(2 * ONE_SECOND);
			continue;
		}

		if (tempNum1 > LIST_BUDDY_RSP_NUM)
		{
			SLEEP(2 * ONE_SECOND);
			continue;
		}
		else
		{
			T_BUDDYINFO localInfo;
			{
				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
				localInfo = g_tP2pManagerStatus.listRecvLocalBuddyReq.front();
				g_tP2pManagerStatus.listRecvLocalBuddyReq.pop_front();
			}
			g_tP2pManagerStatus.tLocalBuddyAddr = localInfo.tPeerAddrOut;

			char pszPeerIP[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = localInfo.tPeerAddrOut.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			ProcessOnChainReqMsg(pszPeerIP, localInfo.tPeerAddrOut.uiPort, localInfo.recvBuf, localInfo.bufLen);
		}		
	}
}
void* CHChainP2PManager::UpdateDataProcessEntry(void* pParam)
{
	CHChainP2PManager* pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->UpdateDataProcessImp();
	return NULL;

}
void CHChainP2PManager::UpdateDataProcessImp()
{
	m_threadUpdateDataProcess.ThreadStarted();


	time_t tmSaveUseList;
	time(&tmSaveUseList);

	time_t tmSendChainStateRsp;
	time(&tmSendChainStateRsp);

	time_t tmSendPeerlistRsp;
	time(&tmSendPeerlistRsp);
	
	while (1)
	{
		time_t tmNow;
		time(&tmNow);
		if (MAX_SAVE_PEERLIST_PERIOD < (tmNow - tmSaveUseList))
		{
			tmSaveUseList = tmNow;
			
		}
		
		if (MAX_SEND_CHAIN_STATE_RSP_PERIOD < (tmNow - tmSendChainStateRsp))
		{
			tmSendChainStateRsp = tmNow;
			
		}
		
		if (MAX_SEND_PEERLIST_RSP_PERIOD < (tmNow - tmSendPeerlistRsp))
		{
			tmSendPeerlistRsp = tmNow;

			CAutoMutexLock muxAuto(m_MuxPeerInfoList);
			int i = 0;
			m_PeerInfoList.sort(Cmpare());
			ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
			for (; itr != m_PeerInfoList.end(); itr++)
			{
				if (i < g_tP2pManagerStatus.usBuddyPeerCount)
				{
					SendGetPeerListReq((*itr)->strName, (*itr)->tPeerInfoByOther.uiIP, (*itr)->tPeerInfoByOther.uiPort);
					i++;   
				}
				else
				{
					break;
				}
			}
		}

		SLEEP(60 * ONE_SECOND);
	}
}

void CHChainP2PManager::WriteBlockLog(T_HYPERBLOCK hyperBlock)
{
	string strStatuFile = g_confFile.strLogDir;
	strStatuFile += "p2p_status.log";

	FILE* fp = fopen(strStatuFile.c_str(), "a");
	if (NULL == fp)
		return;

	string strTxt("");
	strTxt = getBlockInfo(hyperBlock);
	fprintf(fp, "%s\n", strTxt.c_str());
	fflush(fp);

	fclose(fp);
}


void* CHChainP2PManager::WriteStatusProcessEntry(void* pParam)
{
	CHChainP2PManager* pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->WriteStatusProcessImp();
	return NULL;

}
void CHChainP2PManager::WriteStatusProcessImp()
{
	m_threadWriteStatusProcess.ThreadStarted();
	time_t tmNatTraversalTime;
	time(&tmNatTraversalTime);
	
	SLEEP(60 * ONE_SECOND);

	while (!g_bWriteStatusIsSet)
	{
		SLEEP(ONE_SECOND);
	}
	

	string strStatuFile = g_confFile.strLogDir;
	strStatuFile += "p2p_status_all.log";

	FILE* fp = fopen(strStatuFile.c_str(), "a");
	if (NULL == fp)
		return;

	while (1)
	{
		SLEEP(5 * ONE_MIN);
		string strTxt("");
		
		strTxt = getBlockInfo();
		fprintf(fp, "%s\n", strTxt.c_str());
		fflush(fp);
	}
	fclose(fp);
}

string CHChainP2PManager::getPeerListInfo()
{
	string retData = "";
	char buf[TEMP_BUF_LEN] = { 0 };
	retData += "==================PEERLISTINFO==================\n";
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);

	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		retData += "NAME=";
		retData += (*itr)->strName;

		retData += "  IPIN=";
		struct in_addr addPeerIP;
		addPeerIP.s_addr = (*itr)->tPeerInfoByMyself.uiIP;
		retData += inet_ntoa(addPeerIP);

		retData += "  PORTIN=";   
		memset(buf, 0, TEMP_BUF_LEN);
		sprintf(buf, "%d", (*itr)->tPeerInfoByMyself.uiPort);
		retData += buf;

		retData += "  IPOUT=";
		addPeerIP.s_addr = (*itr)->tPeerInfoByOther.uiIP;
		retData += inet_ntoa(addPeerIP);
		   
		retData += "  PORTOUT=";
		memset(buf, 0, TEMP_BUF_LEN);
		sprintf(buf, "%d", (*itr)->tPeerInfoByOther.uiPort);
		retData += buf;

		retData += "\n";
	}

	retData += "================================================\n";
	return retData;
}

#ifdef WIN32
#else
int code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0) return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, (size_t*)&inlen, pout, (size_t*)&outlen) == -1) return -1;
	iconv_close(cd);
	return 0;
}

int u2g(char *inbuf, int inlen, char *outbuf, int outlen)
{
	return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}

int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}
#endif

string CHChainP2PManager::getBlockInfo()
{
	string retData = "";
	char buf[TEMP_BUF_LEN] = { 0 };
	retData += "==================BLOCKINFO==================\n";

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		retData += "HyperBlockNo=";
		memset(buf, 0, TEMP_BUF_LEN);
		sprintf(buf, "%d\n", itr->tBlockBaseInfo.uiID);
		retData += buf;

		retData += "ChainNum=";
		memset(buf, 0, TEMP_BUF_LEN);
		sprintf(buf, "%d\n", itr->listPayLoad.size());
		retData += buf;

		retData += "HyperBlockHash=";
		memset(buf, 0, TEMP_BUF_LEN);
		CCommonStruct::Hash256ToStr(buf, &itr->tBlockBaseInfo.tHashSelf);
		retData += buf;
		retData += "\n";

		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
		for (; subItr != itr->listPayLoad.end(); subItr++)
		{
			uint16 chainNum = 0;
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				chainNum += 1;
				T_PLOCALBLOCK localBlock = &(*ssubItr);

				T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
				T_PFILEINFO fileInfo = &(privateBlock->tPayLoad);
				  
				retData += "chainNo=";
				memset(buf, 0, TEMP_BUF_LEN);
				sprintf(buf, "%d   ", chainNum);
				retData += buf;

				retData += "localBlockHash=";
				memset(buf, 0, TEMP_BUF_LEN);
				CCommonStruct::Hash256ToStr(buf, &ssubItr->tBlockBaseInfo.tHashSelf);
				retData += buf;
				retData += "   ";
				
				retData += "PreBlockHash=";
				memset(buf, 0, TEMP_BUF_LEN);
				CCommonStruct::Hash256ToStr(buf, &ssubItr->tBlockBaseInfo.tPreHash);
				retData += buf;
				retData += "   ";

				retData += "fileHash=";
				memset(buf, 0, TEMP_BUF_LEN);
				CCommonStruct::Hash512ToStr(buf, &fileInfo->tFileHash);
				retData += buf;
				retData += "\n";
			}
			retData += "\n";
		}

	}

	retData += "================================================\n";
	return retData;
}
string CHChainP2PManager::getBlockInfo(T_HYPERBLOCK hyperBlock)
{
	string retData = "";
	char buf[TEMP_BUF_LEN] = { 0 };
	retData += "==================BLOCKINFO==================\n";


	{
		retData += "Block No:	";
		memset(buf, 0, TEMP_BUF_LEN);
		sprintf(buf, "%d\n", hyperBlock.tBlockBaseInfo.uiID);
		retData += buf;

		retData += "Block Hash:	";
		memset(buf, 0, TEMP_BUF_LEN);
		retData += "0x";
		CCommonStruct::Hash256ToStr(buf, &hyperBlock.tBlockBaseInfo.tHashSelf);
		retData += buf;
		retData += "\n";

		retData += "Time:		";
		memset(buf, 0, TEMP_BUF_LEN);
		struct tm * t;
		t = localtime((time_t*)&hyperBlock.tBlockBaseInfo.uiTime);
		sprintf(buf, "%d-%d-%d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
		retData += buf;

		retData += "Previous Hash:	";
		retData += "0x";
		memset(buf, 0, TEMP_BUF_LEN);
		CCommonStruct::Hash256ToStr(buf, &hyperBlock.tBlockBaseInfo.tPreHash);
		retData += buf;
		retData += "\n";

		retData += "Extra:		";

		retData += "Payload:\n";

		retData += "\n";

		uint16 chainNum = 0;
		list<LIST_T_LOCALBLOCK>::iterator subItr = hyperBlock.listPayLoad.begin();
		for (; subItr != hyperBlock.listPayLoad.end(); subItr++)
		{
			chainNum += 1;
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);

				T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
				T_PFILEINFO fileInfo = &(privateBlock->tPayLoad);

				retData += "HyperBlockHash:	";
				memset(buf, 0, TEMP_BUF_LEN);
				retData += "0x";
				CCommonStruct::Hash256ToStr(buf, &hyperBlock.tBlockBaseInfo.tHashSelf);
				retData += buf;
				retData += "";

				retData += "AtChainNo:";
				memset(buf, 0, TEMP_BUF_LEN);
				sprintf(buf, "%d ", chainNum);
				retData += buf;

				retData += "LocalBlockHash:";
				memset(buf, 0, TEMP_BUF_LEN);
				retData += "0x";
				CCommonStruct::Hash256ToStr(buf, &localBlock->tBlockBaseInfo.tHashSelf);
				retData += buf;
				retData += "";


				retData += "CustomInfo=";
				retData += fileInfo->customInfo;
				retData += "";

				retData += "\n";

			}
		}

	}

	retData += "================================================\n";
	return retData;
}
string CHChainP2PManager::getBlockStateInfo()
{
	string retData = "";
	char buf[TEMP_BUF_LEN] = { 0 };
	retData += "==================BLOCKSTATEINFO==================\n";

	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	ITR_MAP_BLOCK_STATE itr = m_BlockStateMap.begin();
	for (; itr != m_BlockStateMap.end(); itr++)
	{
		retData += "BlockNo=";
		memset(buf, 0, TEMP_BUF_LEN);
		sprintf(buf, "%d", (*itr).first);
		retData += buf;
		
		retData += "  NodeNum=";
		memset(buf, 0, TEMP_BUF_LEN);
		sprintf(buf, "%d", (*itr).second.size());
		retData += buf;
		retData += "\n";

	}

	retData += "================================================\n";
	return retData;
}
void* CHChainP2PManager::UdpProcessEntry(void* pParam)
{	
	CHChainP2PManager* pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->UdpProcessImp();
	return NULL;

}
void CHChainP2PManager::UdpProcessImp()
{
	m_threadUdpProcess.ThreadStarted();

	char* pRecvBuf = new char[MAX_RECV_UDP_BUF_LEN];
	unsigned int uiRecvBufLen = MAX_RECV_UDP_BUF_LEN;
	char pszPeerIP[MAX_IP_LEN] = "";
	unsigned int     uiPeerIP = 0;
	unsigned short usPeerPort = 0;
	
	int iResult = 0;

	CUdpSocket* pCurUdpSocket = &m_UdpSocket;
	int iTempTimes = 0;
	while (1)
	{
		memset(pRecvBuf, 0, MAX_RECV_UDP_BUF_LEN);
		uiRecvBufLen = MAX_RECV_UDP_BUF_LEN;
		memset(pszPeerIP, 0, MAX_IP_LEN);
		usPeerPort = 0;
		uiPeerIP = 0;

		iResult = pCurUdpSocket->Recv(uiPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);

		if (1 != iResult)
		{
			SLEEP(10);
			continue;
		}

		struct in_addr addPeerIP;
		addPeerIP.s_addr = uiPeerIP;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));

		T_PP2PPROTOCOLTYPE ptType = (T_PP2PPROTOCOLTYPE)(pRecvBuf);

		switch (ptType->ucType)
		{
		case P2P_PROTOCOL_PING_REQ:
			ProcessPingReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_PING_RSP:
			ProcessPingRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_PEERLIST_REQ:
			ProcessPeerListReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_PEERLIST_RSP:
			ProcessPeerListRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_WANT_NAT_TRAVERSAL_REQ:
			ProcessWantNatTraversalReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_SOME_ONE_NAT_TRAVERSAL_TO_YOU_REQ:
			ProcessSomeNodeWantToConnectYouReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_NAT_TRAVERSAL_REQ:
			ProcessNatTraversalReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_ADD_BLOCK_REQ:
		{
			ProcessAddBlockReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		}
		case P2P_PROTOCOL_ON_CHAIN_REQ:
		{
			T_BUDDYINFO localBuddyInfo;
			localBuddyInfo.tPeerAddrOut.uiIP = inet_addr(pszPeerIP);
			localBuddyInfo.tPeerAddrOut.uiPort = usPeerPort;
			localBuddyInfo.tType = RECV_REQ;
			localBuddyInfo.bufLen = uiRecvBufLen;
			localBuddyInfo.recvBuf = (char*)malloc(uiRecvBufLen);
			memcpy(localBuddyInfo.recvBuf, pRecvBuf, uiRecvBufLen);
			
			bool index = false;
			CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
			LIST_T_BUDDYINFO::iterator itr = g_tP2pManagerStatus.listRecvLocalBuddyReq.begin();
			for (itr; itr != g_tP2pManagerStatus.listRecvLocalBuddyReq.end(); itr ++)
			{
				if ((*itr).tPeerAddrOut.uiIP == localBuddyInfo.tPeerAddrOut.uiIP)
				{
					index = true;
					break;
				}
			}

			if (!index)
			{
				g_tP2pManagerStatus.listRecvLocalBuddyReq.push_back(localBuddyInfo);
				g_tP2pManagerStatus.uiRecvRegisReqNum += 1;
				g_tP2pManagerStatus.uiRecvConfirmingRegisReqNum += 1;
			}
			break;
		}
		case P2P_PROTOCOL_ON_CHAIN_RSP:
		{
			T_BUDDYINFO localBuddyInfo;
			localBuddyInfo.tPeerAddrOut.uiIP = inet_addr(pszPeerIP);
			localBuddyInfo.tPeerAddrOut.uiPort = usPeerPort;
			localBuddyInfo.tType = RECV_RSP;
			localBuddyInfo.bufLen = uiRecvBufLen;
			localBuddyInfo.recvBuf = (char*)malloc(uiRecvBufLen);
			memcpy(localBuddyInfo.recvBuf, pRecvBuf, uiRecvBufLen);
			
			bool index = false;
			CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
			LIST_T_BUDDYINFO::iterator itr = g_tP2pManagerStatus.listRecvLocalBuddyRsp.begin();
			for (itr; itr != g_tP2pManagerStatus.listRecvLocalBuddyRsp.end(); itr++)
			{
				if ((*itr).tPeerAddrOut.uiIP == localBuddyInfo.tPeerAddrOut.uiIP)
				{
					index = true;
					break;
				}
			}
			if (!index)
			{
				g_tP2pManagerStatus.listRecvLocalBuddyRsp.push_back(localBuddyInfo);
			}
			break;
		}
		case P2P_PROTOCOL_REFUSE_REQ:
		{
			ProcessRefuseReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		}
		case P2P_PROTOCOL_ON_CHAIN_CONFIRM:
		{
			ProcessOnChainConfirmMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		}
		case P2P_PROTOCOL_ON_CHAIN_CONFIRM_RSP:
		{
			ProcessOnChainConfirmRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		}
		case P2P_PROTOCOL_ON_CHAIN_CONFIRM_FIN:
		{
			ProcessOnChainConfirmFinMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		}
		case P2P_PROTOCOL_COPY_BLOCK_REQ:
			ProcessCopyBlockReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_GLOBAL_BUDDY_REQ:
			ProcessGlobalBuddyReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_GLOBAL_BUDDY_RSP:
			ProcessGlobalBuddyRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_COPY_HYPER_BLOCK_REQ:
			ProcessCopyHyperBlockReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_COPY_HYPER_BLOCK_RSP:
			break;
		case P2P_PROTOCOL_GET_BLOCK_STATE_REQ:
			ProcessGetBlockStateReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_GET_BLOCK_STATE_RSP:
			ProcessGetBlockStateRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_GET_HYPERBLOCK_BY_NO_REQ:
			ProcessGetHyperBlockByNoReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_GET_HYPERBLOCK_BY_NO_RSP:
			break;
		default:
			break;

		}

	}
	delete[] pRecvBuf;
	pRecvBuf = NULL;
}

void CHChainP2PManager::SendToOutPeerWantNatTraversalReq(T_PPEERINFO tPpeerInfo)
{
	T_P2PPROTOCOLWANTNATTRAVERSALREQ tNatTraversalReq;

	tNatTraversalReq.tType.ucType = P2P_PROTOCOL_WANT_NAT_TRAVERSAL_REQ;
	tNatTraversalReq.tPeerOutAddr.uiIP = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	tNatTraversalReq.tPeerOutAddr.uiPort = m_MyPeerInfo.tPeerInfoByOther.uiPort;
	tNatTraversalReq.tPeerBeNatTraversaldAddr.uiIP= tPpeerInfo->tPeerInfoByOther.uiIP;
	tNatTraversalReq.tPeerBeNatTraversaldAddr.uiPort = tPpeerInfo->tPeerInfoByOther.uiPort;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	tNatTraversalReq.tType.uiTimeStamp = timeTemp.tv_sec;

	uint32 uiIp = 0;
	uint16 usPort = 0;
	int index = 0;
	while (index < MAX_SEND_LOGIN_PEER_NODE)
	{
		ITR_VEC_T_PPEERCONF itrConfPeer = g_confFile.vecPeerConf.begin();
		for (; itrConfPeer != g_confFile.vecPeerConf.end(); itrConfPeer++)
		{
			if ((*itrConfPeer)->uiPeerState == OUTNET)
			{
				uiIp = (*itrConfPeer)->tPeerAddrOut.uiIP;
				usPort = (*itrConfPeer)->tPeerAddrOut.uiPort;
				index++;
				break;
			}
		}

		if (uiIp == 0 && usPort == 0)
		{
			break;
		}

		char pszPeerIP[MAX_IP_LEN] = "";
		struct in_addr addPeerIP;
		addPeerIP.s_addr = uiIp;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));
		
		m_UdpSocket.Send(uiIp, usPort, (char*)&tNatTraversalReq, sizeof(tNatTraversalReq));
	}
}

void CHChainP2PManager::SendGetPeerListReq(int8 *strName, uint32 uiIP, uint16 usPort)
{
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);

	T_P2PPROTOCOLPEERLISTREQ pP2pProtocolPeerlistReq;

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}

	pP2pProtocolPeerlistReq.tPeerAddr.uiIP = g_confFile.uiLocalIP;
	pP2pProtocolPeerlistReq.tPeerAddr.uiPort = g_confFile.uiLocalPort;
	pP2pProtocolPeerlistReq.tType.ucType = P2P_PROTOCOL_PEERLIST_REQ;
	pP2pProtocolPeerlistReq.uiMaxBlockNum = g_tP2pManagerStatus.uiMaxBlockNum;
	pP2pProtocolPeerlistReq.uiNodeState = g_tP2pManagerStatus.uiNodeState;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolPeerlistReq.tType.uiTimeStamp = timeTemp.tv_sec;
	strncpy(pP2pProtocolPeerlistReq.strName, g_confFile.strLocalNodeName.c_str(), MAX_NODE_NAME_LEN);

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));

	m_UdpSocket.Send(uiIP, usPort, (char*)&pP2pProtocolPeerlistReq, sizeof(pP2pProtocolPeerlistReq));
}
void CHChainP2PManager::SavePeerList()
{

	string strBuf = "<?xml version=\"1.0\" encoding=\"utf - 8\"?>\n";
	strBuf += "<peerinfo>\n";

	int i = 0;

	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (i < g_tP2pManagerStatus.usBuddyPeerCount)
		{
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr)->tPeerInfoByMyself.uiIP;
			char tempBuf[MAX_IP_LEN]; 
			memset(tempBuf, 0, MAX_IP_LEN);

			strBuf += "   <nodeinfo>\n";
			strBuf += "      <serverip>";
			strBuf += inet_ntoa(addPeerIP);
			strBuf += "</serverip>\n";

			strBuf += "      <serverport>";
			sprintf(tempBuf, "%d", (*itr)->tPeerInfoByMyself.uiPort);
			strBuf += tempBuf;
			strBuf += "</serverport>\n";

			addPeerIP.s_addr = (*itr)->tPeerInfoByOther.uiIP;
			strBuf += "      <outserverip>";
			strBuf += inet_ntoa(addPeerIP);
			strBuf += "</outserverip>\n";

			strBuf += "      <outserverport>";
			memset(tempBuf, 0, MAX_IP_LEN);
			sprintf(tempBuf, "%d", (*itr)->tPeerInfoByOther.uiPort);
			strBuf += tempBuf;
			strBuf += "</outserverport>\n";

			strBuf += "      <nodestate>";
			memset(tempBuf, 0, MAX_IP_LEN);
			sprintf(tempBuf, "%d", (*itr)->uiState);
			strBuf += tempBuf;
			strBuf += "</nodestate>\n";
		
			strBuf += "      <nodename>";
			strBuf += (*itr)->strName;
			strBuf += "</nodename>\n";

			strBuf += "   </nodeinfo>\n";
			i++;
		}
		else
		{
			break;
		}
	}

	strBuf += "</peerinfo>";

	string localPath = m_uufile.GetAppPath();
	string peerListPath = localPath + "peerlist.xml";

	m_uufile.ReparePath(peerListPath);

	m_uufile.SaveFile(peerListPath, strBuf);
}
void CHChainP2PManager::SendLoginReq(uint32 uiIP, uint16 usPort)
{
	T_P2PPROTOCOLPINGREQ tPingReq;

	tPingReq.tPeerAddr.uiIP = g_confFile.uiLocalIP;
	tPingReq.tPeerAddr.uiPort = g_confFile.uiLocalPort;
	tPingReq.tType.ucType = P2P_PROTOCOL_PING_REQ;
	tPingReq.uiMaxBlockNum = g_tP2pManagerStatus.uiMaxBlockNum;
	tPingReq.uiNodeState = g_tP2pManagerStatus.uiNodeState;
	strncpy(tPingReq.strName, m_MyPeerInfo.strName, MAX_NODE_NAME_LEN);
	struct timeval timePtr;
	GETTIMEOFDAY(&timePtr);
	tPingReq.tType.uiTimeStamp = timePtr.tv_sec;
	
	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));

	m_UdpSocket.Send(uiIP, usPort, (char*)&tPingReq, sizeof(tPingReq));
}

void CHChainP2PManager::GetHyperBlockByNo(uint64 blockNum)
{
	T_P2PPROTOCOLGETHYPERBLOCKBYNOREQ tGetHyperBlockByNoReq;

	tGetHyperBlockByNoReq.tType.ucType = P2P_PROTOCOL_GET_HYPERBLOCK_BY_NO_REQ;
	struct timeval timePtr;
	GETTIMEOFDAY(&timePtr);
	tGetHyperBlockByNoReq.tType.uiTimeStamp = timePtr.tv_sec;
	tGetHyperBlockByNoReq.uiBlockNum = blockNum;

	SendDataToPeer((char*)&tGetHyperBlockByNoReq, sizeof(tGetHyperBlockByNoReq));
	
}
void CHChainP2PManager::ProcessPeerListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	GETTIMEOFDAY(&tvStart);

	T_PP2PPROTOCOLPEERLISTRSP pP2pProtocolPeerListRsp = (T_PP2PPROTOCOLPEERLISTRSP)(pBuf);
	if (P2P_PROTOCOL_SUCCESS != pP2pProtocolPeerListRsp->tResult.iResult)
	{
		return;
	}
	T_PPEERINFO	pPeerInfo = NULL;
	T_PPEERINFO pPeerInfoTemp = NULL;
	pPeerInfo = (T_PPEERINFO)(pP2pProtocolPeerListRsp + 1);   

	SearchPeerList(pP2pProtocolPeerListRsp->strName, pP2pProtocolPeerListRsp->tPeerAddr.uiIP, pP2pProtocolPeerListRsp->tPeerAddr.uiPort, inet_addr(pszIP), usPort, true, pP2pProtocolPeerListRsp->tResult.tType.uiTimeStamp, pP2pProtocolPeerListRsp->uiNodeState);
	
	for (int i = 0; i<pP2pProtocolPeerListRsp->uiCount; i++)
	{
		pPeerInfoTemp = pPeerInfo + i;
		
		uint64 temp = 0;
		SearchPeerList(pPeerInfoTemp->strName, pPeerInfoTemp->tPeerInfoByMyself.uiIP, pPeerInfoTemp->tPeerInfoByMyself.uiPort, pPeerInfoTemp->tPeerInfoByOther.uiIP, pPeerInfoTemp->tPeerInfoByOther.uiPort, false, temp, pPeerInfoTemp->uiNodeState);
	}

	if (pP2pProtocolPeerListRsp->uiMaxBlockNum < g_tP2pManagerStatus.uiMaxBlockNum)
	{
		
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPeerListRsp->uiMaxBlockNum);
	}

	GETTIMEOFDAY(&tvEnd);
	tvSpac.tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
	
}
void CHChainP2PManager::ProcessPeerListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	GETTIMEOFDAY(&tvStart);

	T_PP2PPROTOCOLPEERLISTREQ pP2pProtocolPeerListReq = (T_PP2PPROTOCOLPEERLISTREQ)(pBuf);

	SearchPeerList(pP2pProtocolPeerListReq->strName, pP2pProtocolPeerListReq->tPeerAddr.uiIP, pP2pProtocolPeerListReq->tPeerAddr.uiPort, inet_addr(pszIP), usPort, true, pP2pProtocolPeerListReq->tType.uiTimeStamp, pP2pProtocolPeerListReq->uiNodeState);
	
	SendPeerListToPeer(inet_addr(pszIP), usPort);

	if (pP2pProtocolPeerListReq->uiMaxBlockNum < g_tP2pManagerStatus.uiMaxBlockNum)
	{
		
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPeerListReq->uiMaxBlockNum);
	}
}
void CHChainP2PManager::ProcessGetHyperBlockByNoReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLGETHYPERBLOCKBYNOREQ pP2pProtocolGetHyperBlockByNoReq = (T_PP2PPROTOCOLGETHYPERBLOCKBYNOREQ)(pBuf);

	SendOneHyperBlockByNo(inet_addr(pszIP), usPort, pP2pProtocolGetHyperBlockByNoReq->uiBlockNum);
}

void CHChainP2PManager::SendOneHyperBlockByNo(uint32 uiIP, uint16 usPort, uint64 uiBlockNum)
{
	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		if ((*itr).tBlockBaseInfo.uiID == uiBlockNum)
		{
			T_PP2PPROTOCOLCOPYHYPERBLOCKREQ pP2pProtocolCopyHyperBlockReq = NULL;
			int blockNum = 0;
			list<LIST_T_LOCALBLOCK>::iterator itrH = (*itr).listPayLoad.begin();
			for (; itrH != (*itr).listPayLoad.end(); itrH++)
			{
				{
					blockNum += (*itrH).size();
				}
			}

			int ipP2pProtocolCopyHyperBlockReqLen = sizeof(T_P2PPROTOCOLCOPYHYPERBLOCKREQ) + sizeof(T_HYPERBLOCKSEND) + blockNum * sizeof(T_LOCALBLOCK);
			pP2pProtocolCopyHyperBlockReq = (T_PP2PPROTOCOLCOPYHYPERBLOCKREQ)malloc(ipP2pProtocolCopyHyperBlockReqLen);

			pP2pProtocolCopyHyperBlockReq->tType.ucType = P2P_PROTOCOL_COPY_HYPER_BLOCK_REQ;
			struct timeval timeTemp;
			GETTIMEOFDAY(&timeTemp);
			pP2pProtocolCopyHyperBlockReq->tType.uiTimeStamp = timeTemp.tv_sec;
			pP2pProtocolCopyHyperBlockReq->uiSendTimes = HYPERBLOCK_SYNC_TIMES;
			pP2pProtocolCopyHyperBlockReq->uiBlockCount = blockNum;
			pP2pProtocolCopyHyperBlockReq->uiChainCount = (*itr).listPayLoad.size();
			pP2pProtocolCopyHyperBlockReq->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
			pP2pProtocolCopyHyperBlockReq->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;
			pP2pProtocolCopyHyperBlockReq->uiBlockNum = (*itr).tBlockBaseInfo.uiID;

			T_PHYPERBLOCKSEND pHyperBlockSend;
			pHyperBlockSend = (T_PHYPERBLOCKSEND)(pP2pProtocolCopyHyperBlockReq + 1);
			pHyperBlockSend->tBlockBaseInfo = (*itr).tBlockBaseInfo;
			pHyperBlockSend->tHashAll = (*itr).tHashAll;

			T_PLOCALBLOCK pPeerInfos;
			if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
				pPeerInfos = NULL;
			else
				pPeerInfos = (T_PLOCALBLOCK)(pHyperBlockSend + 1);


			uint8 chainNum = 0;;
			int i = 0;
			itrH = (*itr).listPayLoad.begin();
			for (; itrH != (*itr).listPayLoad.end(); itrH++)
			{
				chainNum++;
				ITR_LIST_T_LOCALBLOCK subItrH = itrH->begin();
				for (; subItrH != itrH->end(); subItrH++)
				{
					pPeerInfos[i].tBlockBaseInfo = (*subItrH).tBlockBaseInfo;
					pPeerInfos[i].tHHash = (*subItrH).tHHash;
					pPeerInfos[i].uiAtChainNum = chainNum;
					pPeerInfos[i].tPayLoad = (*subItrH).tPayLoad;
					i++;
				}
			}

			m_UdpSocket.Send(uiIP, usPort, (char*)pP2pProtocolCopyHyperBlockReq, ipP2pProtocolCopyHyperBlockReqLen);

			free(pP2pProtocolCopyHyperBlockReq);
			pP2pProtocolCopyHyperBlockReq = NULL;
		}
	}
}

void CHChainP2PManager::ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	GETTIMEOFDAY(&tvStart);

	T_PP2PPROTOCOLPINGREQ pP2pProtocolPingReq = (T_PP2PPROTOCOLPINGREQ)(pBuf);

	T_P2PPROTOCOLPINGRSP pP2pPingRsp;
	pP2pPingRsp.tResult.tType.ucType = P2P_PROTOCOL_PING_RSP;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pPingRsp.tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pPingRsp.tResult.iResult = P2P_PROTOCOL_SUCCESS;
	pP2pPingRsp.uiMaxBlockNum = g_tP2pManagerStatus.uiMaxBlockNum;
	pP2pPingRsp.tPeerOutAddr.uiIP = inet_addr(pszIP);
	pP2pPingRsp.tPeerOutAddr.uiPort = usPort;
	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)&pP2pPingRsp, sizeof(pP2pPingRsp));

	SearchPeerList(pP2pProtocolPingReq->strName, pP2pProtocolPingReq->tPeerAddr.uiIP, pP2pProtocolPingReq->tPeerAddr.uiPort, inet_addr(pszIP), usPort, true, pP2pProtocolPingReq->tType.uiTimeStamp, pP2pProtocolPingReq->uiNodeState);
	
	SLEEP(1*ONE_SECOND);
	SendPeerListToPeer(inet_addr(pszIP), usPort);

	if (pP2pProtocolPingReq->uiMaxBlockNum < g_tP2pManagerStatus.uiMaxBlockNum)
	{
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPingReq->uiMaxBlockNum);
	}

	GETTIMEOFDAY(&tvEnd);
	tvSpac.tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
}
void CHChainP2PManager::ProcessPingRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	GETTIMEOFDAY(&tvStart);

	T_PP2PPROTOCOLPINGRSP pP2pProtocolPingRsp = (T_PP2PPROTOCOLPINGRSP)(pBuf);
	m_MyPeerInfo.tPeerInfoByOther.uiIP = pP2pProtocolPingRsp->tPeerOutAddr.uiIP;
	m_MyPeerInfo.tPeerInfoByOther.uiPort = pP2pProtocolPingRsp->tPeerOutAddr.uiPort;

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = pP2pProtocolPingRsp->tPeerOutAddr.uiIP;;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));

	if (pP2pProtocolPingRsp->uiMaxBlockNum > g_tP2pManagerStatus.uiMaxBlockNum)
	{
		g_tP2pManagerStatus.uiMaxBlockNum = pP2pProtocolPingRsp->uiMaxBlockNum;
	}
}
void CHChainP2PManager::ProcessWantNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLWANTNATTRAVERSALREQ pP2pProtocolWantNatTraversalReq = (T_PP2PPROTOCOLWANTNATTRAVERSALREQ)(pBuf);

	T_P2PPROTOCOLSOMEONEWANTCALLYOUREQ tWantNatTraversalReq;
	tWantNatTraversalReq.tType.ucType = P2P_PROTOCOL_SOME_ONE_NAT_TRAVERSAL_TO_YOU_REQ;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	tWantNatTraversalReq.tType.uiTimeStamp = timeTemp.tv_sec;
	tWantNatTraversalReq.tPeerWantNatTraversalAddr.uiIP = pP2pProtocolWantNatTraversalReq->tPeerOutAddr.uiIP;
	tWantNatTraversalReq.tPeerWantNatTraversalAddr.uiPort = pP2pProtocolWantNatTraversalReq->tPeerOutAddr.uiPort;

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = pP2pProtocolWantNatTraversalReq->tPeerBeNatTraversaldAddr.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	m_UdpSocket.Send(pP2pProtocolWantNatTraversalReq->tPeerBeNatTraversaldAddr.uiIP, pP2pProtocolWantNatTraversalReq->tPeerBeNatTraversaldAddr.uiPort, (char*)&tWantNatTraversalReq, sizeof(tWantNatTraversalReq));
}
void CHChainP2PManager::ProcessSomeNodeWantToConnectYouReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLSOMEONEWANTCALLYOUREQ pP2pProtocolSomeNodeWantConnectYouReq = (T_PP2PPROTOCOLSOMEONEWANTCALLYOUREQ)(pBuf);

	T_P2PPROTOCOLNATTRAVERSALASKREQ pP2pProtocolNatTraversalAskReq;
	pP2pProtocolNatTraversalAskReq.tType.ucType = P2P_PROTOCOL_NAT_TRAVERSAL_REQ;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolNatTraversalAskReq.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolNatTraversalAskReq.tPeerAddr.uiIP = g_confFile.uiLocalIP;
	pP2pProtocolNatTraversalAskReq.tPeerAddr.uiPort = g_confFile.uiLocalPort;

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = pP2pProtocolSomeNodeWantConnectYouReq->tPeerWantNatTraversalAddr.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));

	m_UdpSocket.Send(pP2pProtocolSomeNodeWantConnectYouReq->tPeerWantNatTraversalAddr.uiIP, pP2pProtocolSomeNodeWantConnectYouReq->tPeerWantNatTraversalAddr.uiPort, (char*)&pP2pProtocolNatTraversalAskReq, sizeof(pP2pProtocolNatTraversalAskReq));
}
void CHChainP2PManager::ProcessNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLNATTRAVERSALASKREQ pP2pProtocolNatTraversalAskReq = (T_PP2PPROTOCOLNATTRAVERSALASKREQ)(pBuf);

	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if ((pP2pProtocolNatTraversalAskReq->tPeerAddr.uiIP == (*itr)->tPeerInfoByMyself.uiIP)
			&& (pP2pProtocolNatTraversalAskReq->tPeerAddr.uiPort == (*itr)->tPeerInfoByMyself.uiPort)
			&& (inet_addr(pszIP) == (*itr)->tPeerInfoByOther.uiIP)
			&& (usPort == (*itr)->tPeerInfoByOther.uiPort))
		{
			(*itr)->uiTime = time(NULL);
			(*itr)->uiNatTraversalState = GOOD;
			break;
		}
	}

}

void CHChainP2PManager::ProcessAddBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLADDBLOCKREQ pP2pProtocolAddBlockReqRecv = (T_PP2PPROTOCOLADDBLOCKREQ)(pBuf);

	T_HYPERBLOCK blockInfos;
	T_PHYPERBLOCKSEND pHyperBlockInfosTemp;
	pHyperBlockInfosTemp = (T_PHYPERBLOCKSEND)(pP2pProtocolAddBlockReqRecv + 1);
	blockInfos.tBlockBaseInfo = pHyperBlockInfosTemp->tBlockBaseInfo;
	blockInfos.tHashAll = pHyperBlockInfosTemp->tHashAll;

	uint16 uiRetNum = 0;
	uiRetNum = HyperBlockInListOrNot(pP2pProtocolAddBlockReqRecv->uiBlockNum, pP2pProtocolAddBlockReqRecv->uiBlockCount, blockInfos.tBlockBaseInfo.tHashSelf);
	if (uiRetNum == ERROR_EXIST)
	{
		return;
	}

	T_PLOCALBLOCK pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALBLOCK)(pHyperBlockInfosTemp + 1);

	int chainNumTemp = 1;
	LIST_T_LOCALBLOCK listLocakBlockTemp;
	for (int i = 0; i<pP2pProtocolAddBlockReqRecv->uiBlockCount; i++)
	{
		T_LOCALBLOCK pLocalTemp;
		pLocalTemp = *(pLocalBlockTemp + i);

		if (pLocalTemp.uiAtChainNum == chainNumTemp)
		{
			listLocakBlockTemp.push_back(pLocalTemp);
		}
		else
		{
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
			chainNumTemp = pLocalTemp.uiAtChainNum;
			listLocakBlockTemp.clear();
			listLocakBlockTemp.push_back(pLocalTemp);
		}

		if (i == pP2pProtocolAddBlockReqRecv->uiBlockCount - 1)
		{
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
		}
	}
	
	m_HchainBlockList.push_back(blockInfos);
	char pszPeerIP1[MAX_IP_LEN] = { 0 };
	struct in_addr addPeerIP1;
	addPeerIP1.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	strcpy(pszPeerIP1, inet_ntoa(addPeerIP1));
	{
		SaveHyperBlockToLocal(blockInfos);
		SaveLocalBlockToLocal(blockInfos);
	}
	WriteBlockLog(blockInfos);


	if (g_tP2pManagerStatus.uiMaxBlockNum < pHyperBlockInfosTemp->tBlockBaseInfo.uiID)
	{
		g_tP2pManagerStatus.uiMaxBlockNum = pHyperBlockInfosTemp->tBlockBaseInfo.uiID;
		g_tP2pManagerStatus.tPreHyperBlock = blockInfos;
	}

	if (uiRetNum == 1)
	{
		g_tP2pManagerStatus.tPreHyperBlock = blockInfos;
	}
	T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;
	pBlockStateAddr->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	pBlockStateAddr->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;
	pBlockStateAddr->tPeerAddrOut.uiIP = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	pBlockStateAddr->tPeerAddrOut.uiPort = m_MyPeerInfo.tPeerInfoByOther.uiPort;

	LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
	listPblockStateAddr.push_back(pBlockStateAddr);
	m_BlockStateMap[pP2pProtocolAddBlockReqRecv->uiBlockNum] = listPblockStateAddr;

	char pszPeerIP[MAX_IP_LEN] = "";
	char pszPeerIPOut[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	struct in_addr addPeerIPOut;
	addPeerIP.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	addPeerIPOut.s_addr = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}


	if (pP2pProtocolAddBlockReqRecv->uiSendTimes == 0)
		return;

	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolAddBlockReqRecv->tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolAddBlockReqRecv->uiSendTimes = pP2pProtocolAddBlockReqRecv->uiSendTimes - 1;

	SendDataToPeer(pBuf, uiBufLen);
}
void CHChainP2PManager::ProcessCopyBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLCOPYBLOCKREQ pP2pProtocolCopyBlockReqRecv = (T_PP2PPROTOCOLCOPYBLOCKREQ)(pBuf);

	T_PLOCALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALCONSENSUS)(pP2pProtocolCopyBlockReqRecv + 1);

	char localHash[TEMP_BUF_LEN] = { 0 };
	CCommonStruct::Hash256ToStr(localHash, &pLocalBlockTemp->tLocalBlock.tBlockBaseInfo.tHashSelf);

	bool index;

	index = false;
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	ITR_LIST_T_LOCALCONSENSUS itrList = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
	for (; itrList != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itrList++)
	{
		if (((*itrList).tLocalBlock.tBlockBaseInfo.tHashSelf == pLocalBlockTemp->tLocalBlock.tBlockBaseInfo.tHashSelf))
		{
			index = true;
			break;
		}
	}

	if (index)
		return;

	g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back(*pLocalBlockTemp);
	g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
	ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);

	g_tP2pManagerStatus.uiRecvRegisReqNum += 1;   
	g_tP2pManagerStatus.uiRecvConfirmingRegisReqNum += 1;
}
void CHChainP2PManager::ChangeLocalBlockPreHash(LIST_T_LOCALCONSENSUS &localList)
{
	int localSize = localList.size();
	T_PLOCALCONSENSUS localInfo;
	ITR_LIST_T_LOCALCONSENSUS itr = localList.begin();
	ITR_LIST_T_LOCALCONSENSUS itrNext = itr++;
	memset(itrNext->tLocalBlock.tBlockBaseInfo.tPreHash.pID, 0, DEF_SHA256_LEN);

	uint16 num = 1;
	while (num < localSize)
	{
		(*itr).tLocalBlock.tBlockBaseInfo.tPreHash = (*itrNext).tLocalBlock.tBlockBaseInfo.tHashSelf;
		itr++;
		itrNext++;
		num += 1;
	}
}
void CHChainP2PManager::CopyLocalBuddyList(LIST_T_LOCALCONSENSUS &endList, LIST_T_LOCALCONSENSUS fromList)
{
	ITR_LIST_T_LOCALCONSENSUS itrList = fromList.begin();
	for (; itrList != fromList.end(); itrList++)
	{
		T_LOCALCONSENSUS tempBlock;
		(tempBlock).tLocalBlock = (*itrList).tLocalBlock;
		(tempBlock).tPeer = (*itrList).tPeer;
		endList.push_back(tempBlock);
	}
}

void CHChainP2PManager::SendOnChainRsp(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINREQ pP2pProtocolOnChainReqRecv = (T_PP2PPROTOCOLONCHAINREQ)(pBuf);

	if (pP2pProtocolOnChainReqRecv->uiHyperBlockNum != g_tP2pManagerStatus.uiMaxBlockNum + 1)
	{
		return;
	}

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	uint8 nodeSize = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
	if (nodeSize == 0)
	{
		return;
	}

	bool index = false;

	if (nodeSize == ONE_LOCAL_BLOCK || pP2pProtocolOnChainReqRecv->uiBlockCount == ONE_LOCAL_BLOCK)
	{
		index = true;
	}

	if (!index)
	{
		return;
	}

	bool sendCopyIndex = false;
	if (nodeSize > 1 && pP2pProtocolOnChainReqRecv->uiBlockCount == ONE_LOCAL_BLOCK)
	{
		sendCopyIndex = true;
	}

	g_tP2pManagerStatus.bLocalBuddyChainState = true;

	T_BUDDYINFOSTATE buddyInfo;

	CopyLocalBuddyList(buddyInfo.localList, g_tP2pManagerStatus.listLocalBuddyChainInfo);

	T_PLOCALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALCONSENSUS)(pP2pProtocolOnChainReqRecv + 1);

	for (int i = 0; i < pP2pProtocolOnChainReqRecv->uiBlockCount; i++)
	{
		T_LOCALCONSENSUS  LocalBlockInfo;
		LocalBlockInfo = *(pLocalBlockTemp + i);

		char localHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &LocalBlockInfo.tLocalBlock.tBlockBaseInfo.tHashSelf);

		index = JudgExistAtLocalBuddy(buddyInfo.localList, LocalBlockInfo);

		if (index)
			continue;

		buddyInfo.localList.push_back(LocalBlockInfo);
		buddyInfo.localList.sort(CmpareOnChain());
		ChangeLocalBlockPreHash(buddyInfo.localList);
	}

	T_PLOCALCONSENSUS pPeerInfosTemp;
	uint8 blockNumTemp = buddyInfo.localList.size();
	int pPeerInfosTempLen = blockNumTemp * sizeof(T_LOCALCONSENSUS);
	pPeerInfosTemp = (T_PLOCALCONSENSUS)malloc(pPeerInfosTempLen);
	int j = 0;
	ITR_LIST_T_LOCALCONSENSUS itrTemp = buddyInfo.localList.begin();
	for (; itrTemp != buddyInfo.localList.end(); itrTemp++)
	{
		pPeerInfosTemp[j] = (*itrTemp);
		j++;
	}
	buddyInfo.tPeerAddrOut.uiIP = inet_addr(pszIP);
	buddyInfo.tPeerAddrOut.uiPort = usPort;
	buddyInfo.uibuddyState = SEND_ON_CHAIN_RSP;
	T_SHA256 tempHash;
	memset(tempHash.pID, 0, DEF_SHA256_LEN);
	GetSHA256(tempHash.pID, (const char*)(pPeerInfosTemp), pPeerInfosTempLen);
	
	char strLocalHashTemp[TEMP_BUF_LEN] = { 0 };
	memset(strLocalHashTemp, 0, TEMP_BUF_LEN);
	CCommonStruct::Hash256ToStr(strLocalHashTemp, &tempHash);

	memset(buddyInfo.strBuddyHash, 0, DEF_STR_HASH256_LEN);
	memcpy(buddyInfo.strBuddyHash, strLocalHashTemp, DEF_STR_HASH256_LEN);

	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyReq);
	ITR_LIST_T_BUDDYINFOSTATE itrReq = g_tP2pManagerStatus.listCurBuddyReq.begin();
	for (itrReq; itrReq != g_tP2pManagerStatus.listCurBuddyReq.end(); itrReq++)
	{
		if (0 == memcmp((*itrReq).strBuddyHash, buddyInfo.strBuddyHash, DEF_STR_HASH256_LEN))
		{
			return;
		}
	}
	g_tP2pManagerStatus.listCurBuddyReq.push_back(buddyInfo);
	
	T_PP2PPROTOCOLONCHAINRSP pP2pProtocolOnChainRsp = NULL;

	
	uint8 blockNum = buddyInfo.localList.size();
	int ipP2pProtocolOnChainRspLen = sizeof(T_P2PPROTOCOLONCHAINRSP) + blockNum * sizeof(T_LOCALCONSENSUS);
	pP2pProtocolOnChainRsp = (T_PP2PPROTOCOLONCHAINRSP)malloc(ipP2pProtocolOnChainRspLen);

	pP2pProtocolOnChainRsp->tResult.tType.ucType = P2P_PROTOCOL_ON_CHAIN_RSP;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolOnChainRsp->tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolOnChainRsp->tResult.iResult = P2P_PROTOCOL_SUCCESS;
	pP2pProtocolOnChainRsp->uiBlockCount = blockNum;
	pP2pProtocolOnChainRsp->uiHyperBlockNum = g_tP2pManagerStatus.uiMaxBlockNum + 1;
	memset(pP2pProtocolOnChainRsp->strHash, 0, DEF_STR_HASH256_LEN);
	memcpy(pP2pProtocolOnChainRsp->strHash, strLocalHashTemp, DEF_STR_HASH256_LEN);

	T_PLOCALCONSENSUS pPeerInfos;
	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
		pPeerInfos = NULL;
	else
		pPeerInfos = (T_PLOCALCONSENSUS)(pP2pProtocolOnChainRsp + 1);

	int i = 0;
	ITR_LIST_T_LOCALCONSENSUS itr = buddyInfo.localList.begin();
	for (; itr != buddyInfo.localList.end(); itr++)
	{
		pPeerInfos[i] = (*itr);
		i++;
	}

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolOnChainRsp, ipP2pProtocolOnChainRspLen);

	free(pP2pProtocolOnChainRsp);
	pP2pProtocolOnChainRsp = NULL;

}
void CHChainP2PManager::ProcessOnChainReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINREQ pP2pProtocolOnChainReqRecv = (T_PP2PPROTOCOLONCHAINREQ)(pBuf);

	SendOnChainRsp(pszIP, usPort, pBuf, uiBufLen);
}

void CHChainP2PManager::GetOnChainInfo()
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	
	{
		if (g_tP2pManagerStatus.listOnChainReq.empty())
		{
			return;
		}

		T_LOCALCONSENSUS onChainInfo = g_tP2pManagerStatus.listOnChainReq.front();
		g_tP2pManagerStatus.listOnChainReq.pop_front();

		g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back(onChainInfo);
		g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
		ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);

		g_tP2pManagerStatus.uiNodeState = CONFIRMING;

		g_tP2pManagerStatus.curBuddyBlock = onChainInfo;

		char localHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &onChainInfo.tLocalBlock.tBlockBaseInfo.tHashSelf);
	}

	g_tP2pManagerStatus.bHaveOnChainReq = true;

	return;
}
void CHChainP2PManager::SendLocalBuddyReq()
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	T_PP2PPROTOCOLONCHAINREQ pP2pProtocolOnChainReq = NULL;
	uint8 blockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
	int ipP2pProtocolOnChainReqLen = sizeof(T_P2PPROTOCOLONCHAINREQ) + blockNum*sizeof(T_LOCALCONSENSUS);
	pP2pProtocolOnChainReq = (T_PP2PPROTOCOLONCHAINREQ)malloc(ipP2pProtocolOnChainReqLen);

	pP2pProtocolOnChainReq->tType.ucType = P2P_PROTOCOL_ON_CHAIN_REQ;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolOnChainReq->tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolOnChainReq->uiBlockCount = blockNum;
	pP2pProtocolOnChainReq->uiHyperBlockNum = g_tP2pManagerStatus.uiMaxBlockNum + 1;

	T_PLOCALCONSENSUS pPeerInfos = NULL;
	pPeerInfos = (T_PLOCALCONSENSUS)(pP2pProtocolOnChainReq + 1);

	uint8 i = 0;
	ITR_LIST_T_LOCALCONSENSUS itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
	for (; itr != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itr++)
	{
		pPeerInfos[i].tLocalBlock = (*itr).tLocalBlock;
		pPeerInfos[i].tPeer = (*itr).tPeer;
		i++;

		char localHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &(*itr).tLocalBlock.tBlockBaseInfo.tHashSelf);
	}

	SendDataToPeer((char*)pP2pProtocolOnChainReq, ipP2pProtocolOnChainReqLen);

	free(pP2pProtocolOnChainReq);
	pP2pProtocolOnChainReq = NULL;
}

void CHChainP2PManager::StartGlobalBuddy()
{
	g_tP2pManagerStatus.bStartGlobalFlag = true;
	g_tP2pManagerStatus.usGlobalBuddyChainCount = 1;
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
	g_tP2pManagerStatus.listRecvLocalBuddyReq.clear();

	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	if (g_tP2pManagerStatus.listLocalBuddyChainInfo.size() == 0 || g_tP2pManagerStatus.listLocalBuddyChainInfo.size() == 1)
	{
			return;
	}
	LIST_T_LOCALCONSENSUS::iterator itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.end();
	itr--;
	if (((*itr).tPeer.tPeerAddr.uiIP == m_MyPeerInfo.tPeerInfoByMyself.uiIP) && ((*itr).tPeer.tPeerAddr.uiPort == m_MyPeerInfo.tPeerInfoByMyself.uiPort))
	{
		T_PP2PPROTOCOLGLOBALBUDDYREQ pP2pProtocolGlobalBuddyReq = NULL;

		uint8 blockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
		int ipP2pProtocolGlobalBuddyReqLen = sizeof(T_P2PPROTOCOLGLOBALBUDDYREQ) + blockNum * sizeof(T_GLOBALCONSENSUS);
		pP2pProtocolGlobalBuddyReq = (T_PP2PPROTOCOLGLOBALBUDDYREQ)malloc(ipP2pProtocolGlobalBuddyReqLen);

		pP2pProtocolGlobalBuddyReq->tType.ucType = P2P_PROTOCOL_GLOBAL_BUDDY_REQ;
		struct timeval timeTemp;
		GETTIMEOFDAY(&timeTemp);
		pP2pProtocolGlobalBuddyReq->tType.uiTimeStamp = timeTemp.tv_sec;
		pP2pProtocolGlobalBuddyReq->uiBlockCount = blockNum;
		pP2pProtocolGlobalBuddyReq->uiChainCount = g_tP2pManagerStatus.usGlobalBuddyChainCount;
		pP2pProtocolGlobalBuddyReq->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
		pP2pProtocolGlobalBuddyReq->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;

		T_PGLOBALCONSENSUS pPeerInfos;
		if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
			pPeerInfos = NULL;
		else
			pPeerInfos = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyReq + 1);

		int i = 0;
		ITR_LIST_T_LOCALCONSENSUS itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
		for (; itr != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itr++)
		{
			pPeerInfos[i].tLocalBlock = (*itr).tLocalBlock;
			pPeerInfos[i].tPeer = (*itr).tPeer;
			pPeerInfos[i].uiAtChainNum = 1;
			i++;

			char localHash[TEMP_BUF_LEN] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &(*itr).tLocalBlock.tBlockBaseInfo.tHashSelf);
		}

		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
		g_tP2pManagerStatus.listGlobalBuddyChainInfo.push_back(g_tP2pManagerStatus.listLocalBuddyChainInfo);

		SendDataToPeer((char*)pP2pProtocolGlobalBuddyReq, ipP2pProtocolGlobalBuddyReqLen);


		free(pP2pProtocolGlobalBuddyReq);
		pP2pProtocolGlobalBuddyReq = NULL;
	}
}
void CHChainP2PManager::ProcessOnChainConfirmRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINCONFIRMRSP pP2pProtocolOnChainConfirmRspRecv = (T_PP2PPROTOCOLONCHAINCONFIRMRSP)(pBuf);

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyRsp.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyRsp.end();)
	{
		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainConfirmRspRecv->strHash, DEF_STR_HASH256_LEN))
		{

			if (pP2pProtocolOnChainConfirmRspRecv->tResult.iResult == P2P_PROTOCOL_SUCCESS)
			{
				bool index = false;
				ITR_LIST_T_LOCALCONSENSUS itrSub = (*itr).localList.begin();
				for (; itrSub != (*itr).localList.end(); itrSub++)
				{
					index = JudgExistAtLocalBuddy(g_tP2pManagerStatus.listLocalBuddyChainInfo, (*itrSub));
					if (index)
						continue;

					char localHash[TEMP_BUF_LEN] = { 0 };
					CCommonStruct::Hash256ToStr(localHash, &(*itrSub).tLocalBlock.tBlockBaseInfo.tHashSelf);

					CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
					g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back((*itrSub));
					g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
					ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
					SendCopyLocalBlock((*itrSub));

					g_tP2pManagerStatus.uiRecvRegisReqNum += 1;
					g_tP2pManagerStatus.uiRecvConfirmingRegisReqNum += 1;
				}
			}

			SendConfirmFin(pszIP, usPort, (*itr).strBuddyHash);
			itr = g_tP2pManagerStatus.listCurBuddyRsp.erase(itr);
		}
		else
		{
			char pszPeerIP[MAX_IP_LEN] = { 0 };
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr).tPeerAddrOut.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			SendRefuseReq(pszPeerIP, (*itr).tPeerAddrOut.uiPort, (*itr).strBuddyHash, RECV_REQ);
			itr = g_tP2pManagerStatus.listCurBuddyRsp.erase(itr);
		}
	}

	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyReq);
	ITR_LIST_T_BUDDYINFOSTATE itrReq = g_tP2pManagerStatus.listCurBuddyReq.begin();
	for (itrReq; itrReq != g_tP2pManagerStatus.listCurBuddyReq.end();)
	{
		if (0 == strncmp((*itrReq).strBuddyHash, pP2pProtocolOnChainConfirmRspRecv->strHash, DEF_STR_HASH256_LEN))
		{
			itrReq = g_tP2pManagerStatus.listCurBuddyReq.erase(itrReq);
		}
		else
		{
			char pszPeerIP[MAX_IP_LEN] = { 0 };
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itrReq).tPeerAddrOut.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			SendRefuseReq(pszPeerIP, (*itrReq).tPeerAddrOut.uiPort, (*itrReq).strBuddyHash, RECV_RSP);
			itrReq = g_tP2pManagerStatus.listCurBuddyReq.erase(itrReq);
		}
	}

	CAutoMutexLock muxAuto3(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
	g_tP2pManagerStatus.listRecvLocalBuddyRsp.clear();

	CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
	g_tP2pManagerStatus.listRecvLocalBuddyReq.clear();
}

void CHChainP2PManager::ProcessRefuseReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLREFUSEREQ pP2pProtocolRefuseReq = (T_PP2PPROTOCOLREFUSEREQ)(pBuf);

	if (pP2pProtocolRefuseReq->uSubType == RECV_RSP)
	{
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
		ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyRsp.begin();
		for (itr; itr != g_tP2pManagerStatus.listCurBuddyRsp.end();)
		{
		
			if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolRefuseReq->strHash, DEF_STR_HASH256_LEN))
		
			{
				
				itr = g_tP2pManagerStatus.listCurBuddyRsp.erase(itr);
			}
			else
			{
				itr++;
			}
		}
	}
	else if (pP2pProtocolRefuseReq->uSubType == RECV_REQ)
	{
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyReq);
		ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyReq.begin();
		for (itr; itr != g_tP2pManagerStatus.listCurBuddyReq.end(); )
		{
			if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolRefuseReq->strHash, DEF_STR_HASH256_LEN))
			{

				itr = g_tP2pManagerStatus.listCurBuddyReq.erase(itr);
			}
			else
			{
				itr++;
			}
		}
	}
}

void CHChainP2PManager::SendCopyLocalBlock(T_LOCALCONSENSUS localBlock)
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	uint8 nodeSize = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
	if (nodeSize > NOT_START_BUDDY_NUM)
	{
		T_PP2PPROTOCOLCOPYBLOCKREQ pP2pProtocolCopyBlockReq = NULL;
		int ipP2pProtocolCopyBlockReqLen = sizeof(T_P2PPROTOCOLCOPYBLOCKREQ) + sizeof(T_LOCALCONSENSUS);
		pP2pProtocolCopyBlockReq = (T_PP2PPROTOCOLCOPYBLOCKREQ)malloc(ipP2pProtocolCopyBlockReqLen);

		pP2pProtocolCopyBlockReq->tType.ucType = P2P_PROTOCOL_COPY_BLOCK_REQ;
		struct timeval timeTemp;
		GETTIMEOFDAY(&timeTemp);
		pP2pProtocolCopyBlockReq->tType.uiTimeStamp = timeTemp.tv_sec;

		T_PLOCALCONSENSUS pLocalBlockInfo = (T_PLOCALCONSENSUS)(pP2pProtocolCopyBlockReq + 1);
		pLocalBlockInfo->tLocalBlock = localBlock.tLocalBlock;
		pLocalBlockInfo->tPeer = localBlock.tPeer;

		ITR_LIST_T_LOCALCONSENSUS itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
		for (; itr != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itr++)
		{
			if ((*itr).tLocalBlock.tBlockBaseInfo.tHashSelf == localBlock.tLocalBlock.tBlockBaseInfo.tHashSelf)
			{
				continue;
			}

			char pszPeerIP[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr).tPeer.tPeerAddr.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));
			
			char localHash[TEMP_BUF_LEN] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localBlock.tLocalBlock.tBlockBaseInfo.tHashSelf);

			m_UdpSocket.Send((*itr).tPeer.tPeerAddr.uiIP, (*itr).tPeer.tPeerAddr.uiPort, (char*)pP2pProtocolCopyBlockReq, ipP2pProtocolCopyBlockReqLen);
		}

		free(pP2pProtocolCopyBlockReq);
		pP2pProtocolCopyBlockReq = NULL;
	}
}
void CHChainP2PManager::ProcessOnChainConfirmFinMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINCONFIRM pP2pProtocolOnChainConfirmRecv = (T_PP2PPROTOCOLONCHAINCONFIRM)(pBuf);


	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
	ITR_LIST_T_BUDDYINFOSTATE itrRsp = g_tP2pManagerStatus.listCurBuddyRsp.begin();
	for (itrRsp; itrRsp != g_tP2pManagerStatus.listCurBuddyRsp.end();)
	{
	
		if (0 == strncmp((*itrRsp).strBuddyHash, pP2pProtocolOnChainConfirmRecv->strHash, DEF_STR_HASH256_LEN))
		{
			itrRsp = g_tP2pManagerStatus.listCurBuddyRsp.erase(itrRsp);
		}
		else
		{
			char pszPeerIP[MAX_IP_LEN] = { 0 };
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itrRsp).tPeerAddrOut.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			SendRefuseReq(pszPeerIP, (*itrRsp).tPeerAddrOut.uiPort, (*itrRsp).strBuddyHash, RECV_REQ);
			itrRsp = g_tP2pManagerStatus.listCurBuddyRsp.erase(itrRsp);
		}
	}

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyReq);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyReq.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyReq.end();)
	{
		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainConfirmRecv->strHash, DEF_STR_HASH256_LEN))
		{

			if (pP2pProtocolOnChainConfirmRecv->tResult.iResult == P2P_PROTOCOL_SUCCESS)
			{
				(*itr).uibuddyState = RECV_CONFIRM;
				bool index = false;
				ITR_LIST_T_LOCALCONSENSUS itrSub = (*itr).localList.begin();
				for (; itrSub != (*itr).localList.end(); itrSub++)
				{
					CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
					index = JudgExistAtLocalBuddy(g_tP2pManagerStatus.listLocalBuddyChainInfo, (*itrSub));
					if (index)
						continue;

					char localHash[TEMP_BUF_LEN] = { 0 };
					CCommonStruct::Hash256ToStr(localHash, &(*itrSub).tLocalBlock.tBlockBaseInfo.tHashSelf);

					g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back((*itrSub));
					g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
					ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
					SendCopyLocalBlock((*itrSub));

					g_tP2pManagerStatus.uiRecvRegisReqNum += 1;
					g_tP2pManagerStatus.uiRecvConfirmingRegisReqNum += 1;
				}

			}
			
			itr = g_tP2pManagerStatus.listCurBuddyReq.erase(itr);
		}
		else
		{
			char pszPeerIP[MAX_IP_LEN] = { 0 };
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr).tPeerAddrOut.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			SendRefuseReq(pszPeerIP, (*itr).tPeerAddrOut.uiPort, (*itr).strBuddyHash, RECV_RSP);
	
			itr = g_tP2pManagerStatus.listCurBuddyReq.erase(itr);

		}

	}

	
	CAutoMutexLock muxAuto3(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
	g_tP2pManagerStatus.listRecvLocalBuddyRsp.clear();

	CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
	g_tP2pManagerStatus.listRecvLocalBuddyReq.clear();

}
void CHChainP2PManager::ProcessOnChainConfirmMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINCONFIRM pP2pProtocolOnChainConfirmRecv = (T_PP2PPROTOCOLONCHAINCONFIRM)(pBuf);

	
	
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyReq);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyReq.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyReq.end(); itr ++)
	{

		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainConfirmRecv->strHash, DEF_STR_HASH256_LEN))
		{

			if (pP2pProtocolOnChainConfirmRecv->tResult.iResult == P2P_PROTOCOL_SUCCESS)
			{
				(*itr).uibuddyState = RECV_CONFIRM; 
			
				SendConfirmRsp(pszIP, usPort, (*itr).strBuddyHash);
			}
		}
	}
}

bool CHChainP2PManager::JudgExistAtLocalBuddy(LIST_T_LOCALCONSENSUS localList, T_LOCALCONSENSUS localBlockInfo)
{
	bool index = false;
	ITR_LIST_T_LOCALCONSENSUS itrList = localList.begin();
	for (; itrList != localList.end(); itrList++)
	{
		if (((*itrList).tPeer.tPeerAddr.uiIP == localBlockInfo.tPeer.tPeerAddr.uiIP)
			&& ((*itrList).tPeer.tPeerAddrOut.uiIP == localBlockInfo.tPeer.tPeerAddrOut.uiIP)
			&& (*itrList).tLocalBlock.tBlockBaseInfo.tHashSelf == localBlockInfo.tLocalBlock.tBlockBaseInfo.tHashSelf)
		{
			char localHash[TEMP_BUF_LEN] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localBlockInfo.tLocalBlock.tBlockBaseInfo.tHashSelf);

			index = true;
			break;
		}
	}

	return index;
}
void CHChainP2PManager::SendConfirmReq(char* pszIP, unsigned short usPort, string hash, uint8 state)
{
	T_PP2PPROTOCOLONCHAINCONFIRM pP2pProtocolOnChainConfirm = NULL;

	int ipP2pProtocolOnChainConfirmLen = sizeof(T_P2PPROTOCOLONCHAINCONFIRM);
	pP2pProtocolOnChainConfirm = (T_PP2PPROTOCOLONCHAINCONFIRM)malloc(ipP2pProtocolOnChainConfirmLen);

	pP2pProtocolOnChainConfirm->tResult.tType.ucType = P2P_PROTOCOL_ON_CHAIN_CONFIRM;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolOnChainConfirm->tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolOnChainConfirm->tResult.iResult = state;

	
	memset(pP2pProtocolOnChainConfirm->strHash, 0, DEF_STR_HASH256_LEN);
	memcpy(pP2pProtocolOnChainConfirm->strHash, hash.c_str(), DEF_STR_HASH256_LEN);

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolOnChainConfirm, ipP2pProtocolOnChainConfirmLen);

	g_tP2pManagerStatus.bLocalBuddyChainState = false;

	free(pP2pProtocolOnChainConfirm);
	pP2pProtocolOnChainConfirm = NULL;
}
void CHChainP2PManager::SendRefuseReq(char* pszIP, unsigned short usPort, string hash, uint8 type)
{
	T_PP2PPROTOCOLREFUSEREQ pP2pProtocolRefuseReq= NULL;

	int ipP2pProtocolRefuseReqLen = sizeof(T_P2PPROTOCOLREFUSEREQ);
	pP2pProtocolRefuseReq = (T_PP2PPROTOCOLREFUSEREQ)malloc(ipP2pProtocolRefuseReqLen);

	pP2pProtocolRefuseReq->tType.ucType = P2P_PROTOCOL_REFUSE_REQ;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolRefuseReq->tType.uiTimeStamp = timeTemp.tv_sec;

	memset(pP2pProtocolRefuseReq->strHash, 0, DEF_STR_HASH256_LEN);
	memcpy(pP2pProtocolRefuseReq->strHash, hash.c_str(), DEF_STR_HASH256_LEN);
	pP2pProtocolRefuseReq->uSubType = type;

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolRefuseReq, ipP2pProtocolRefuseReqLen);

	g_tP2pManagerStatus.bLocalBuddyChainState = false;

	free(pP2pProtocolRefuseReq);
	ipP2pProtocolRefuseReqLen = NULL;
}

void CHChainP2PManager::SendConfirmFin(char* pszIP, unsigned short usPort, string hash)
{
	T_PP2PPROTOCOLONCHAINCONFIRMFIN pP2pProtocolOnChainConfirmFin = NULL;

	int ipP2pProtocolOnChainConfirmFinLen = sizeof(T_P2PPROTOCOLONCHAINCONFIRMFIN);
	pP2pProtocolOnChainConfirmFin = (T_PP2PPROTOCOLONCHAINCONFIRMFIN)malloc(ipP2pProtocolOnChainConfirmFinLen);

	pP2pProtocolOnChainConfirmFin->tResult.tType.ucType = P2P_PROTOCOL_ON_CHAIN_CONFIRM_FIN;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolOnChainConfirmFin->tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolOnChainConfirmFin->tResult.iResult = P2P_PROTOCOL_SUCCESS;

	memset(pP2pProtocolOnChainConfirmFin->strHash, 0, DEF_STR_HASH256_LEN);
	memcpy(pP2pProtocolOnChainConfirmFin->strHash, hash.c_str(), DEF_STR_HASH256_LEN);

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolOnChainConfirmFin, ipP2pProtocolOnChainConfirmFinLen);

	g_tP2pManagerStatus.bLocalBuddyChainState = false;

	free(pP2pProtocolOnChainConfirmFin);
	pP2pProtocolOnChainConfirmFin = NULL;
}
void CHChainP2PManager::SendConfirmRsp(char* pszIP, unsigned short usPort, string hash)
{
	T_PP2PPROTOCOLONCHAINCONFIRMRSP pP2pProtocolOnChainConfirmRsp = NULL;

	int ipP2pProtocolOnChainConfirmRspLen = sizeof(T_P2PPROTOCOLONCHAINCONFIRMRSP);
	pP2pProtocolOnChainConfirmRsp = (T_PP2PPROTOCOLONCHAINCONFIRMRSP)malloc(ipP2pProtocolOnChainConfirmRspLen);

	pP2pProtocolOnChainConfirmRsp->tResult.tType.ucType = P2P_PROTOCOL_ON_CHAIN_CONFIRM_RSP;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolOnChainConfirmRsp->tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolOnChainConfirmRsp->tResult.iResult = P2P_PROTOCOL_SUCCESS;
	
	memset(pP2pProtocolOnChainConfirmRsp->strHash, 0, DEF_STR_HASH256_LEN);
	memcpy(pP2pProtocolOnChainConfirmRsp->strHash, hash.c_str(), DEF_STR_HASH256_LEN);

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolOnChainConfirmRsp, ipP2pProtocolOnChainConfirmRspLen);

	g_tP2pManagerStatus.bLocalBuddyChainState = false;

	free(pP2pProtocolOnChainConfirmRsp);
	pP2pProtocolOnChainConfirmRsp = NULL;
}
void CHChainP2PManager::ProcessOnChainRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINRSP pP2pProtocolOnChainRspRecv = (T_PP2PPROTOCOLONCHAINRSP)(pBuf);

	uint8 nodeSize = 0;
	{
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
		nodeSize = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
	}
	if (nodeSize == 0)
	{
		return;
	}

	bool index = false;

	if (nodeSize == ONE_LOCAL_BLOCK || pP2pProtocolOnChainRspRecv->uiBlockCount == ONE_LOCAL_BLOCK)
	{
		index = true;
	}

	if (!index)
	{
		return;
	}

	if (pP2pProtocolOnChainRspRecv->uiHyperBlockNum != g_tP2pManagerStatus.uiMaxBlockNum + 1)
	{
		SendRefuseReq(pszIP, usPort, pP2pProtocolOnChainRspRecv->strHash, RECV_REQ);
		return;
	}

	CAutoMutexLock muxAuto3(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyRsp.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyRsp.end(); itr++)
	{
		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainRspRecv->strHash, DEF_STR_HASH256_LEN))
		{
			return;
		}
	}

	T_BUDDYINFOSTATE buddyInfo;
	
	CAutoMutexLock muxAuto5(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	CopyLocalBuddyList(buddyInfo.localList, g_tP2pManagerStatus.listLocalBuddyChainInfo);

	T_PLOCALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALCONSENSUS)(pP2pProtocolOnChainRspRecv + 1);

	g_tP2pManagerStatus.bLocalBuddyChainState = true;

	index = false;
	for (int i = 0; i < pP2pProtocolOnChainRspRecv->uiBlockCount; i++)
	{
		T_LOCALCONSENSUS  LocalBlockInfo;
		LocalBlockInfo = *(pLocalBlockTemp + i);

		char localHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &LocalBlockInfo.tLocalBlock.tBlockBaseInfo.tHashSelf);

		index = JudgExistAtLocalBuddy(buddyInfo.localList, LocalBlockInfo);

		if (index)
			continue;

		buddyInfo.localList.push_back(LocalBlockInfo);
		buddyInfo.localList.sort(CmpareOnChain());
		ChangeLocalBlockPreHash(buddyInfo.localList);
		
	}
	buddyInfo.tPeerAddrOut.uiIP = inet_addr(pszIP);
	buddyInfo.tPeerAddrOut.uiPort = usPort;
	buddyInfo.uibuddyState = RECV_ON_CHAIN_RSP;
	memset(buddyInfo.strBuddyHash, 0, DEF_STR_HASH256_LEN);
	memcpy(buddyInfo.strBuddyHash, pP2pProtocolOnChainRspRecv->strHash, DEF_STR_HASH256_LEN);
	g_tP2pManagerStatus.listCurBuddyRsp.push_back(buddyInfo);
	SendConfirmReq(pszIP, usPort, pP2pProtocolOnChainRspRecv->strHash, P2P_PROTOCOL_SUCCESS);
}

bool CHChainP2PManager::JudgExistAtGlobalBuddy(LIST_T_LOCALCONSENSUS listLocalBuddyChainInfo)
{
	bool index = false;
	int num = 0;
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
	ITR_LIST_LIST_GLOBALBUDDYINFO itr = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
	for (itr; itr != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itr++)
	{
		num++;
		ITR_LIST_T_LOCALCONSENSUS subItrStart = itr->begin();
		ITR_LIST_T_LOCALCONSENSUS subItrEnd = itr->end(); subItrEnd--;

		ITR_LIST_T_LOCALCONSENSUS localStart = listLocalBuddyChainInfo.begin();
		ITR_LIST_T_LOCALCONSENSUS localEnd = listLocalBuddyChainInfo.end(); localEnd--;
		char startHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(startHash, &(*subItrStart).tLocalBlock.tBlockBaseInfo.tHashSelf);
		char endHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(endHash, &(*subItrEnd).tLocalBlock.tBlockBaseInfo.tHashSelf);

		char startHashtemp[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(startHashtemp, &(*localStart).tLocalBlock.tBlockBaseInfo.tHashSelf);
		char endHashtemp[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(endHashtemp, &(*localEnd).tLocalBlock.tBlockBaseInfo.tHashSelf);

		if ((localStart->tLocalBlock.tBlockBaseInfo.tHashSelf == subItrStart->tLocalBlock.tBlockBaseInfo.tHashSelf)
			&& (localEnd->tLocalBlock.tBlockBaseInfo.tHashSelf == subItrEnd->tLocalBlock.tBlockBaseInfo.tHashSelf))
		{
			index = true;
			break;
		}
	}

	if (!index)
	{
		if (listLocalBuddyChainInfo.size() >= LEAST_START_GLOBAL_BUDDY_NUM)
		{
			g_tP2pManagerStatus.listGlobalBuddyChainInfo.push_back(listLocalBuddyChainInfo);
			g_tP2pManagerStatus.listGlobalBuddyChainInfo.sort(CmpareGlobalBuddy());
		}
	}

	return index;
}


void CHChainP2PManager::SendDataToPeer(char *buf, uint32 bufLen)
{
	int index1 = 0;
	CAutoMutexLock muxAutoPeerList(m_MuxPeerInfoList);
	int tempSendPeerNum = g_tP2pManagerStatus.usBuddyPeerCount;
	int peerNum = m_PeerInfoList.size();
	if (peerNum < g_tP2pManagerStatus.usBuddyPeerCount)
	{
		tempSendPeerNum = peerNum;
	}

	ITR_LIST_T_PPEERINFO itrAdd = m_PeerInfoList.begin();
	for (; itrAdd != m_PeerInfoList.end(); itrAdd++)
	{
		if (index1 < tempSendPeerNum)
		{
			char pszPeerIP[MAX_IP_LEN] = {0};
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itrAdd)->tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			if ((*itrAdd)->tPeerInfoByOther.uiIP != m_MyPeerInfo.tPeerInfoByOther.uiIP)
			{
				m_UdpSocket.Send((*itrAdd)->tPeerInfoByOther.uiIP, (*itrAdd)->tPeerInfoByOther.uiPort, buf, bufLen);
			}
			index1++;
		}
		else
			break;
	}
}

void CHChainP2PManager::SendGlobalBuddyReq()
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	bool isEndNode = false;
	LIST_T_LOCALCONSENSUS::iterator itrLocal = g_tP2pManagerStatus.listLocalBuddyChainInfo.end();
	itrLocal--;
	if ((*itrLocal).tPeer.tPeerAddr.uiIP == m_MyPeerInfo.tPeerInfoByMyself.uiIP
		&& (*itrLocal).tPeer.tPeerAddr.uiPort == m_MyPeerInfo.tPeerInfoByMyself.uiPort)
	{
		isEndNode = true;
	}
	
	if (!isEndNode)
	{
		char pszPeerIP[TEMP_BUF_LEN] = { 0 };
		struct in_addr addPeerIP;
		addPeerIP.s_addr = (*itrLocal).tPeer.tPeerAddr.uiIP;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));

		return;
	}
	T_PP2PPROTOCOLGLOBALBUDDYREQ pP2pProtocolGlobalBuddyReq = NULL;
	uint8 blockNum = 0;
	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
	ITR_LIST_LIST_GLOBALBUDDYINFO itr = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
	for (itr; itr != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itr++)
	{
		
		{
			blockNum += itr->size();
		}
	}

	int ipP2pProtocolGlobalBuddyReqLen = sizeof(T_P2PPROTOCOLGLOBALBUDDYREQ) + blockNum * sizeof(T_GLOBALCONSENSUS);
	pP2pProtocolGlobalBuddyReq = (T_PP2PPROTOCOLGLOBALBUDDYREQ)malloc(ipP2pProtocolGlobalBuddyReqLen);

	pP2pProtocolGlobalBuddyReq->tType.ucType = P2P_PROTOCOL_GLOBAL_BUDDY_REQ;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolGlobalBuddyReq->tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolGlobalBuddyReq->uiBlockCount = blockNum;
	int tempNum = g_tP2pManagerStatus.listGlobalBuddyChainInfo.size();
	pP2pProtocolGlobalBuddyReq->uiChainCount = tempNum;
	pP2pProtocolGlobalBuddyReq->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	pP2pProtocolGlobalBuddyReq->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;

	T_PGLOBALCONSENSUS pPeerInfos;
	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
		pPeerInfos = NULL;
	else
		pPeerInfos = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyReq + 1);

	uint8 i = 0;
	uint8 chainNum = 0;
	ITR_LIST_LIST_GLOBALBUDDYINFO itrSend = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
	for (; itrSend != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itrSend++)
	{
		chainNum++;
		ITR_LIST_T_LOCALCONSENSUS subItr = itrSend->begin();
		for (; subItr != itrSend->end(); subItr++)
		{
			pPeerInfos[i].tLocalBlock = (*subItr).tLocalBlock;
			pPeerInfos[i].tPeer = (*subItr).tPeer;
			pPeerInfos[i].uiAtChainNum = chainNum;
			i++;

			char localHash[TEMP_BUF_LEN] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &(*subItr).tLocalBlock.tBlockBaseInfo.tHashSelf);
		}
	}
	SendDataToPeer((char*)pP2pProtocolGlobalBuddyReq, ipP2pProtocolGlobalBuddyReqLen);

	free(pP2pProtocolGlobalBuddyReq);
	pP2pProtocolGlobalBuddyReq = NULL;
}

void CHChainP2PManager::SaveHyperBlockToLocal(T_HYPERBLOCK tHyperBlock)
{
	T_HYPERBLOCKDBINFO hyperBlockInfo;
	hyperBlockInfo.ucBlockType = HYPER_BLOCK;
	hyperBlockInfo.uiBlockId = tHyperBlock.tBlockBaseInfo.uiID;
	hyperBlockInfo.strAuth = tHyperBlock.tBlockBaseInfo.strAuth;

	memcpy((char*)hyperBlockInfo.strHashAll, (char*)tHyperBlock.tHashAll.pID, DEF_SHA256_LEN);
	memcpy((char*)hyperBlockInfo.strHashSelf, (char*)tHyperBlock.tBlockBaseInfo.tHashSelf.pID, DEF_SHA256_LEN);
	memcpy((char*)hyperBlockInfo.strHyperBlockHash, (char*)tHyperBlock.tBlockBaseInfo.tHashSelf.pID, DEF_SHA256_LEN);
	 
	memcpy((char*)hyperBlockInfo.strPreHash, (char*)tHyperBlock.tBlockBaseInfo.tPreHash.pID, DEF_SHA256_LEN);

	hyperBlockInfo.strPayload = "";
	hyperBlockInfo.strScript = tHyperBlock.tBlockBaseInfo.strScript;
	hyperBlockInfo.uiBlockTimeStamp = tHyperBlock.tBlockBaseInfo.uiTime;
	hyperBlockInfo.uiLocalChainId = 0;
	hyperBlockInfo.uiReferHyperBlockId = tHyperBlock.tBlockBaseInfo.uiID;
	hyperBlockInfo.uiQueueID = 0;
	CHyperchainDB::saveHyperBlockToDB(hyperBlockInfo);
}

void CHChainP2PManager::SaveLocalBlockToLocal(T_HYPERBLOCK tHyperBlock)
{
	uint64 blockNum = 0;
	list<LIST_T_LOCALBLOCK>::iterator subItr = tHyperBlock.listPayLoad.begin();
	for (; subItr != tHyperBlock.listPayLoad.end(); subItr++)
	{
		list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
		for (; ssubItr != (*subItr).end(); ssubItr++)
		{
			blockNum += 1;
			T_HYPERBLOCKDBINFO hyperBlockInfo;
			hyperBlockInfo.ucBlockType = LOCAL_BLOCK;
			hyperBlockInfo.uiBlockId = blockNum;
			hyperBlockInfo.strAuth = (*ssubItr).tBlockBaseInfo.strAuth;
			memset(hyperBlockInfo.strHashAll, 0, DEF_SHA256_LEN);

			char localHash[TEMP_BUF_LEN] = { 0 };
			memcpy((char*)hyperBlockInfo.strHashSelf, (char*)(*ssubItr).tBlockBaseInfo.tHashSelf.pID, DEF_SHA256_LEN);		
			memcpy((char*)hyperBlockInfo.strHyperBlockHash, (char*)tHyperBlock.tBlockBaseInfo.tHashSelf.pID, DEF_SHA256_LEN);
			memcpy((char*)hyperBlockInfo.strPreHash, (char*)(*ssubItr).tBlockBaseInfo.tPreHash.pID, DEF_SHA256_LEN);
			hyperBlockInfo.strScript = (*ssubItr).tBlockBaseInfo.strScript;
			hyperBlockInfo.uiBlockTimeStamp = (*ssubItr).tBlockBaseInfo.uiTime;
			hyperBlockInfo.uiLocalChainId = (*ssubItr).uiAtChainNum;
			hyperBlockInfo.uiReferHyperBlockId = tHyperBlock.tBlockBaseInfo.uiID;
			hyperBlockInfo.uiQueueID = 0;

			string fileTest = (*ssubItr).tPayLoad.tPayLoad.fileName;

			hyperBlockInfo.strPayload = (*ssubItr).tPayLoad.tPayLoad.customInfo;

			char strBuf[MAX_BUF_LEN];
			memset(strBuf, 0, MAX_BUF_LEN);
			CCommonStruct::Hash512ToStr(strBuf, &(*ssubItr).tPayLoad.tPayLoad.tFileHash);
			char buf[MAX_BUF_LEN];
			memset(buf, 0, MAX_BUF_LEN);;
			memcpy(buf, strBuf, MAX_BUF_LEN);
			string tempBuf = buf;
			hyperBlockInfo.strPayload += "fileHash=";
			hyperBlockInfo.strPayload += tempBuf;
			hyperBlockInfo.strPayload += "fileName=";
			hyperBlockInfo.strPayload += (*ssubItr).tPayLoad.tPayLoad.fileName;
			hyperBlockInfo.strPayload += "fileCustom=";
			hyperBlockInfo.strPayload += (*ssubItr).tPayLoad.tPayLoad.customInfo;
			hyperBlockInfo.strPayload += "fileSize=";
			sprintf(buf, "%d", (*ssubItr).tPayLoad.tPayLoad.uiFileSize);
			hyperBlockInfo.strPayload += buf;

			CHyperchainDB::saveHyperBlockToDB(hyperBlockInfo);
		}
	}
}
bool CHChainP2PManager::CreatHyperBlock()
{
	bool isEndNode = false;
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	LIST_T_LOCALCONSENSUS::iterator itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.end();
	itr--;
	if ((*itr).tPeer.tPeerAddr.uiIP == m_MyPeerInfo.tPeerInfoByMyself.uiIP
		&& (*itr).tPeer.tPeerAddr.uiPort == m_MyPeerInfo.tPeerInfoByMyself.uiPort)
	{
		isEndNode = true;
	}

	if (isEndNode)
	{
		T_HYPERBLOCK tHyperChainBlock;
		tHyperChainBlock.tBlockBaseInfo.uiID = g_tP2pManagerStatus.uiMaxBlockNum + 1;
		tHyperChainBlock.tBlockBaseInfo.uiTime = g_tP2pManagerStatus.uiStartTimeOfConsensus + NEXTBUDDYTIME;
		memcpy(tHyperChainBlock.tBlockBaseInfo.strScript, "CustomizedScript", MAX_SCRIPT_LEN);
		memcpy(tHyperChainBlock.tBlockBaseInfo.strAuth, "AuthInfo", MAX_AUTH_LEN);
		tHyperChainBlock.tBlockBaseInfo.tPreHash = g_tP2pManagerStatus.tPreHyperBlock.tBlockBaseInfo.tHashSelf;

	
		uint16 blockNum = 0;
		LIST_T_LOCALBLOCK listLocalBlockInfo;
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
		ITR_LIST_LIST_GLOBALBUDDYINFO itr = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
		for (; itr != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itr++)
		{
			ITR_LIST_T_LOCALCONSENSUS subItr = itr->begin();
			for (; subItr != itr->end(); subItr++)
			{
				blockNum += 1;
				listLocalBlockInfo.push_back((*subItr).tLocalBlock);

				char localHash[TEMP_BUF_LEN] = { 0 };
				CCommonStruct::Hash256ToStr(localHash, &(*subItr).tLocalBlock.tBlockBaseInfo.tHashSelf);
			}
			listLocalBlockInfo.sort(CmpareOnChainLocal());
			tHyperChainBlock.listPayLoad.push_back(listLocalBlockInfo);
			listLocalBlockInfo.clear();
		}

		memset(tHyperChainBlock.tHashAll.pID, 0, DEF_SHA256_LEN);
		GetSHA256(tHyperChainBlock.tBlockBaseInfo.tHashSelf.pID, (const char*)(&tHyperChainBlock), sizeof(tHyperChainBlock));

		
		CAutoMutexLock muxAuto1(m_MuxHchainBlockList);
		m_HchainBlockList.push_back(tHyperChainBlock);
		char pszPeerIP1[MAX_IP_LEN] = { 0 };
		struct in_addr addPeerIP1;
		addPeerIP1.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
		strcpy(pszPeerIP1, inet_ntoa(addPeerIP1));
		{
			SaveHyperBlockToLocal(tHyperChainBlock);
			SaveLocalBlockToLocal(tHyperChainBlock);
		}
		WriteBlockLog(tHyperChainBlock);
		
		if (0 != strcmp(g_tP2pManagerStatus.curBuddyBlock.strID, ""))
		{
			T_SEARCHINFO searchInfo;
			searchInfo.uiHyperID = tHyperChainBlock.tBlockBaseInfo.uiID;
			searchInfo.uiTime = time(NULL);
			CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxMapSearchOnChain);
			g_tP2pManagerStatus.mapSearchOnChain[g_tP2pManagerStatus.curBuddyBlock.strID] = searchInfo;
		}

		char localHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &tHyperChainBlock.tBlockBaseInfo.tHashSelf);


		T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;
		pBlockStateAddr->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
		pBlockStateAddr->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;
		pBlockStateAddr->tPeerAddrOut.uiIP = m_MyPeerInfo.tPeerInfoByOther.uiIP;
		pBlockStateAddr->tPeerAddrOut.uiPort = m_MyPeerInfo.tPeerInfoByOther.uiPort;

		LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
		listPblockStateAddr.push_back(pBlockStateAddr);
		m_BlockStateMap[tHyperChainBlock.tBlockBaseInfo.uiID] = listPblockStateAddr;

		char pszPeerIP[MAX_IP_LEN] = "";
		char pszPeerIPOut[MAX_IP_LEN] = "";
		struct in_addr addPeerIP;
		struct in_addr addPeerIPOut;
		addPeerIP.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
		addPeerIPOut.s_addr = m_MyPeerInfo.tPeerInfoByOther.uiIP;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));
		strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));

		g_tP2pManagerStatus.tPreHyperBlock = tHyperChainBlock;
		g_tP2pManagerStatus.uiMaxBlockNum += 1;
		g_tP2pManagerStatus.listGlobalBuddyChainInfo.clear();
		g_tP2pManagerStatus.listLocalBuddyChainInfo.clear();
		g_tP2pManagerStatus.bHaveOnChainReq = false;
		g_tP2pManagerStatus.bLocalBuddyChainState = false;

		g_tP2pManagerStatus.uiNextStartTimeNewest = tHyperChainBlock.tBlockBaseInfo.uiTime;
		
		g_tP2pManagerStatus.uiTimeOfConsensus = time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus;
		
		g_tP2pManagerStatus.uiSendConfirmingRegisReqNum -= 1;
		g_tP2pManagerStatus.uiNodeState = CONFIRMED;
		
		
		T_PP2PPROTOCOLCOPYHYPERBLOCKREQ pP2pProtocolCopyHyperBlockReq = NULL;
		blockNum = 0;
		list<LIST_T_LOCALBLOCK>::iterator itrH = tHyperChainBlock.listPayLoad.begin();
		for (; itrH != tHyperChainBlock.listPayLoad.end(); itrH++)
		{
			
			{
				blockNum += (*itrH).size();
			}
		}

		int ipP2pProtocolCopyHyperBlockReqLen = sizeof(T_P2PPROTOCOLCOPYHYPERBLOCKREQ) + sizeof(T_HYPERBLOCKSEND) + blockNum * sizeof(T_LOCALBLOCK);
		pP2pProtocolCopyHyperBlockReq = (T_PP2PPROTOCOLCOPYHYPERBLOCKREQ)malloc(ipP2pProtocolCopyHyperBlockReqLen);

		pP2pProtocolCopyHyperBlockReq->tType.ucType = P2P_PROTOCOL_COPY_HYPER_BLOCK_REQ;
		struct timeval timeTemp;
		GETTIMEOFDAY(&timeTemp);
		pP2pProtocolCopyHyperBlockReq->tType.uiTimeStamp = timeTemp.tv_sec;
		pP2pProtocolCopyHyperBlockReq->uiSendTimes = HYPERBLOCK_SYNC_TIMES;
		pP2pProtocolCopyHyperBlockReq->uiBlockCount = blockNum;
		pP2pProtocolCopyHyperBlockReq->uiChainCount = tHyperChainBlock.listPayLoad.size();
		pP2pProtocolCopyHyperBlockReq->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
		pP2pProtocolCopyHyperBlockReq->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;
		pP2pProtocolCopyHyperBlockReq->uiBlockNum = tHyperChainBlock.tBlockBaseInfo.uiID;
		
		T_PHYPERBLOCKSEND pHyperBlockSend;
		pHyperBlockSend = (T_PHYPERBLOCKSEND)(pP2pProtocolCopyHyperBlockReq + 1);
		pHyperBlockSend->tBlockBaseInfo = tHyperChainBlock.tBlockBaseInfo;
		pHyperBlockSend->tHashAll = tHyperChainBlock.tHashAll;

		T_PLOCALBLOCK pPeerInfos;
		if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
			pPeerInfos = NULL;
		else
			pPeerInfos = (T_PLOCALBLOCK)(pHyperBlockSend + 1);

		
		uint8 chainNum = 0;;
		int i = 0;
		itrH = tHyperChainBlock.listPayLoad.begin();
		for (; itrH != tHyperChainBlock.listPayLoad.end(); itrH++)
		{
			chainNum++;
			ITR_LIST_T_LOCALBLOCK subItrH = itrH->begin();
			for (; subItrH != itrH->end(); subItrH++)
			{
				pPeerInfos[i].tBlockBaseInfo = (*subItrH).tBlockBaseInfo;
				pPeerInfos[i].tHHash = (*subItrH).tHHash;
				pPeerInfos[i].uiAtChainNum = chainNum;
				pPeerInfos[i].tPayLoad = (*subItrH).tPayLoad;
				i++;
			}
		}

		SendDataToPeer((char*)pP2pProtocolCopyHyperBlockReq, ipP2pProtocolCopyHyperBlockReqLen);

		free(pP2pProtocolCopyHyperBlockReq);
		pP2pProtocolCopyHyperBlockReq = NULL;
	}

	return isEndNode;
}

void CHChainP2PManager::ProcessGlobalBuddyReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLGLOBALBUDDYREQ pP2pProtocolGlobalBuddyReqRecv = (T_PP2PPROTOCOLGLOBALBUDDYREQ)(pBuf);

	if (!g_tP2pManagerStatus.bStartGlobalFlag)
	{
		return;
	}
	
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	if (g_tP2pManagerStatus.listLocalBuddyChainInfo.empty())
	{
		return;
	}

	bool isEndNode = false;
	LIST_T_LOCALCONSENSUS::iterator itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.end();
	itr--;
	if ((*itr).tPeer.tPeerAddr.uiIP == m_MyPeerInfo.tPeerInfoByMyself.uiIP 
		&& (*itr).tPeer.tPeerAddr.uiPort == m_MyPeerInfo.tPeerInfoByMyself.uiPort)
	{
		isEndNode = true;
	}
	
	if (isEndNode)
	{
	
		T_PGLOBALCONSENSUS  pLocalBlockTemp;
		pLocalBlockTemp = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyReqRecv + 1);

		bool index;
		LIST_T_LOCALCONSENSUS listLocalConsensusInfo;
		uint8 uiChainCountNum = 0;

		for (int i = 0; i < pP2pProtocolGlobalBuddyReqRecv->uiBlockCount; i++)
		{
			T_GLOBALCONSENSUS  localBlockInfo;
			localBlockInfo.tLocalBlock = (*(pLocalBlockTemp + i)).tLocalBlock;
			localBlockInfo.tPeer = (*(pLocalBlockTemp + i)).tPeer;
			localBlockInfo.uiAtChainNum = (*(pLocalBlockTemp + i)).uiAtChainNum;

			T_LOCALCONSENSUS localInfo;
			localInfo.tLocalBlock = localBlockInfo.tLocalBlock;
			localInfo.tPeer = localBlockInfo.tPeer;

			if (uiChainCountNum != localBlockInfo.uiAtChainNum)
			{
				uiChainCountNum = localBlockInfo.uiAtChainNum;
				if (listLocalConsensusInfo.size() != 0)
				{
					index = JudgExistAtGlobalBuddy(listLocalConsensusInfo);
					listLocalConsensusInfo.clear();
				}
			}

			char localHash[TEMP_BUF_LEN] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localInfo.tLocalBlock.tBlockBaseInfo.tHashSelf);

			listLocalConsensusInfo.push_back(localInfo);

			if (i == pP2pProtocolGlobalBuddyReqRecv->uiBlockCount - 1)
			{
				index = JudgExistAtGlobalBuddy(listLocalConsensusInfo);
				listLocalConsensusInfo.clear();
			}
		}

		
		T_PP2PPROTOCOLGLOBALBUDDYRSP pP2pProtocolGlobalBuddyRsp = NULL;
		
		uint8 blockNum = 0;
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
		ITR_LIST_LIST_GLOBALBUDDYINFO itrGlobal = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
		for (; itrGlobal != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itrGlobal++)
		{
			{
				blockNum += itrGlobal->size();
			}
		}
		
		int ipP2pProtocolGlobalBuddyRspLen = sizeof(T_P2PPROTOCOLGLOBALBUDDYRSP) + blockNum * sizeof(T_GLOBALCONSENSUS);
		pP2pProtocolGlobalBuddyRsp = (T_PP2PPROTOCOLGLOBALBUDDYRSP)malloc(ipP2pProtocolGlobalBuddyRspLen);

		pP2pProtocolGlobalBuddyRsp->tResult.tType.ucType = P2P_PROTOCOL_GLOBAL_BUDDY_RSP;
		struct timeval timeTemp;
		GETTIMEOFDAY(&timeTemp);
		pP2pProtocolGlobalBuddyRsp->tResult.tType.uiTimeStamp = timeTemp.tv_sec;
		pP2pProtocolGlobalBuddyRsp->tResult.iResult = P2P_PROTOCOL_SUCCESS;
		pP2pProtocolGlobalBuddyRsp->uiBlockCount = blockNum;
		uint64 tempNum = g_tP2pManagerStatus.listGlobalBuddyChainInfo.size();
		pP2pProtocolGlobalBuddyRsp->uiChainCount = tempNum;
		pP2pProtocolGlobalBuddyRsp->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
		pP2pProtocolGlobalBuddyRsp->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;

		T_PGLOBALCONSENSUS pPeerInfos;
		if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
			pPeerInfos = NULL;
		else
			pPeerInfos = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyRsp + 1);

		uint8 i = 0;
		uint8 chainNum = 0;
		itrGlobal = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
		for (; itrGlobal != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itrGlobal++)
		{
			chainNum++;
			ITR_LIST_T_LOCALCONSENSUS subItr = itrGlobal->begin();
			for (; subItr != itrGlobal->end(); subItr++)
			{		
				pPeerInfos[i].tLocalBlock = (*subItr).tLocalBlock;
				pPeerInfos[i].tPeer = (*subItr).tPeer;
				pPeerInfos[i].uiAtChainNum = chainNum;
				i++;

				char localHash[TEMP_BUF_LEN] = { 0 };
				CCommonStruct::Hash256ToStr(localHash, &(*subItr).tLocalBlock.tBlockBaseInfo.tHashSelf);
			}
		}
		char pszPeerIP[MAX_IP_LEN] = { 0 };
		struct in_addr addPeerIP;
		addPeerIP.s_addr = pP2pProtocolGlobalBuddyReqRecv->tPeerAddr.uiIP;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));

		if (pP2pProtocolGlobalBuddyReqRecv->tPeerAddr.uiIP != m_MyPeerInfo.tPeerInfoByOther.uiIP)
		{
			m_UdpSocket.Send(pP2pProtocolGlobalBuddyReqRecv->tPeerAddr.uiIP, pP2pProtocolGlobalBuddyReqRecv->tPeerAddr.uiPort, (char*)pP2pProtocolGlobalBuddyRsp, ipP2pProtocolGlobalBuddyRspLen);
		}
		free(pP2pProtocolGlobalBuddyRsp);
		pP2pProtocolGlobalBuddyRsp = NULL;
	}
	else
	{
		ITR_LIST_T_LOCALCONSENSUS endItr = g_tP2pManagerStatus.listLocalBuddyChainInfo.end();
		endItr--;

		char pszPeerIP[MAX_IP_LEN] = { 0 };
		char pszPeerIPOut[MAX_IP_LEN] = { 0 };
		struct in_addr addPeerIP;
		addPeerIP.s_addr = endItr->tPeer.tPeerAddr.uiIP;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));

		addPeerIP.s_addr = endItr->tPeer.tPeerAddrOut.uiIP;
		strcpy(pszPeerIPOut, inet_ntoa(addPeerIP));


		m_UdpSocket.Send(endItr->tPeer.tPeerAddr.uiIP, endItr->tPeer.tPeerAddr.uiPort, pBuf, uiBufLen);
	}
}

void CHChainP2PManager::ProcessGlobalBuddyRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLGLOBALBUDDYRSP pP2pProtocolGlobalBuddyRsp = (T_PP2PPROTOCOLGLOBALBUDDYRSP)(pBuf);

	if (P2P_PROTOCOL_SUCCESS != pP2pProtocolGlobalBuddyRsp->tResult.iResult)
	{
		return;
	}


	T_PGLOBALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyRsp + 1);

	bool index;
	LIST_T_LOCALCONSENSUS listLocalConsensusInfo;
	uint64 uiChainCountNum = 0;

	for (int i = 0; i < pP2pProtocolGlobalBuddyRsp->uiBlockCount; i++)
	{
		
		T_GLOBALCONSENSUS  localBlockInfo;
		localBlockInfo.tLocalBlock = (*(pLocalBlockTemp + i)).tLocalBlock;
		localBlockInfo.tPeer = (*(pLocalBlockTemp + i)).tPeer;
		localBlockInfo.uiAtChainNum = (*(pLocalBlockTemp + i)).uiAtChainNum;

		T_LOCALCONSENSUS  blockInfo;
		blockInfo.tLocalBlock = localBlockInfo.tLocalBlock;
		blockInfo.tPeer = localBlockInfo.tPeer;

		if (uiChainCountNum != localBlockInfo.uiAtChainNum)
		{
			uiChainCountNum = localBlockInfo.uiAtChainNum;
			if (listLocalConsensusInfo.size() != 0)
			{
				index = JudgExistAtGlobalBuddy(listLocalConsensusInfo);
				listLocalConsensusInfo.clear();
			}
		}


		listLocalConsensusInfo.push_back(blockInfo);
		char localHash[TEMP_BUF_LEN] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &localBlockInfo.tLocalBlock.tBlockBaseInfo.tHashSelf);

		if (i == pP2pProtocolGlobalBuddyRsp->uiBlockCount - 1)
		{
			index = JudgExistAtGlobalBuddy(listLocalConsensusInfo);
			listLocalConsensusInfo.clear();
		}
	}
}
uint16 CHChainP2PManager::HyperBlockInListOrNot(uint64 blockNum, uint64 blockCount, T_SHA256 tHashSelf)
{
	uint16 retNum = DEFAULT_ERROR_NO;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	CAutoMutexLock muxAutoMap(m_MuxBlockStateMap);
	ITR_LIST_T_HYPERBLOCK itrList = m_HchainBlockList.begin();
	for (; itrList != m_HchainBlockList.end(); itrList++)
	{
		if ((*itrList).tBlockBaseInfo.uiID == blockNum)
		{
			
			{
				uint8 oldHyperLocalBlockNum = 0;
				list<LIST_T_LOCALBLOCK>::iterator itrH = (*itrList).listPayLoad.begin();
				for (; itrH != (*itrList).listPayLoad.end(); itrH++)
				{
					{
						oldHyperLocalBlockNum += itrH->size();
					}
				}

				if (blockCount > oldHyperLocalBlockNum)
				{
					itrList = m_HchainBlockList.erase(itrList);
					retNum = ERROR_NOT_NEWEST;
					break;
				}
				else
				{
					retNum = ERROR_EXIST;
					break;
				}
			}
		}
		else
		{
			continue;
		}
	}

	return retNum;
}
void CHChainP2PManager::ProcessCopyHyperBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLCOPYHYPERBLOCKREQ pP2pProtocolCopyHyperBlockReqRecv = (T_PP2PPROTOCOLCOPYHYPERBLOCKREQ)(pBuf);

	
	T_HYPERBLOCK blockInfos;
	T_PHYPERBLOCKSEND pHyperBlockInfosTemp;
	pHyperBlockInfosTemp = (T_PHYPERBLOCKSEND)(pP2pProtocolCopyHyperBlockReqRecv + 1);
	blockInfos.tBlockBaseInfo = pHyperBlockInfosTemp->tBlockBaseInfo;
	blockInfos.tHashAll = pHyperBlockInfosTemp->tHashAll;


	uint16 uiRetNum = 0;
	uiRetNum = HyperBlockInListOrNot(pP2pProtocolCopyHyperBlockReqRecv->uiBlockNum, pP2pProtocolCopyHyperBlockReqRecv->uiBlockCount, blockInfos.tBlockBaseInfo.tHashSelf);
	if (uiRetNum == ERROR_EXIST)
	{
		return;
	}

	T_PLOCALBLOCK pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALBLOCK)(pHyperBlockInfosTemp + 1);

	uint64 chainNumTemp = 1;
	LIST_T_LOCALBLOCK listLocakBlockTemp;
	for (int i = 0; i<pP2pProtocolCopyHyperBlockReqRecv->uiBlockCount; i++)
	{
		T_LOCALBLOCK pLocalTemp;
		pLocalTemp = *(pLocalBlockTemp + i);

		if (pLocalTemp.uiAtChainNum == chainNumTemp)
		{
			listLocakBlockTemp.push_back(pLocalTemp);
		}
		else
		{
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
			chainNumTemp = pLocalTemp.uiAtChainNum;
			listLocakBlockTemp.clear();
			listLocakBlockTemp.push_back(pLocalTemp);  
		}

		if (i == pP2pProtocolCopyHyperBlockReqRecv->uiBlockCount - 1)
		{
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
		}
	}
	
	m_HchainBlockList.push_back(blockInfos);
	
	char pszPeerIP1[MAX_IP_LEN] = { 0 };
	struct in_addr addPeerIP1;
	addPeerIP1.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	strcpy(pszPeerIP1, inet_ntoa(addPeerIP1));
	{
		SaveHyperBlockToLocal(blockInfos);
		SaveLocalBlockToLocal(blockInfos);
	}
	
	WriteBlockLog(blockInfos);

	g_tP2pManagerStatus.bHaveRecvCopyHyperBlock = true;


	if (g_tP2pManagerStatus.uiMaxBlockNum < pHyperBlockInfosTemp->tBlockBaseInfo.uiID)
	{
		g_tP2pManagerStatus.uiMaxBlockNum = pHyperBlockInfosTemp->tBlockBaseInfo.uiID;
		g_tP2pManagerStatus.tPreHyperBlock = blockInfos;
	}

	if (uiRetNum == 1)
	{
		g_tP2pManagerStatus.tPreHyperBlock = blockInfos;
	}
	T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;
	pBlockStateAddr->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	pBlockStateAddr->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;
	pBlockStateAddr->tPeerAddrOut.uiIP = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	pBlockStateAddr->tPeerAddrOut.uiPort = m_MyPeerInfo.tPeerInfoByOther.uiPort;

	LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
	listPblockStateAddr.push_back(pBlockStateAddr);
	m_BlockStateMap[pP2pProtocolCopyHyperBlockReqRecv->uiBlockNum] = listPblockStateAddr;
	    
	char pszPeerIP[MAX_IP_LEN] = "";
	char pszPeerIPOut[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	struct in_addr addPeerIPOut; 
	addPeerIP.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	addPeerIPOut.s_addr = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}


	if (pP2pProtocolCopyHyperBlockReqRecv->uiSendTimes == 0)   
		return;

	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolCopyHyperBlockReqRecv->tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolCopyHyperBlockReqRecv->uiSendTimes = pP2pProtocolCopyHyperBlockReqRecv->uiSendTimes - 1;
	SendDataToPeer(pBuf, uiBufLen);
	
	bool index = false;

	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	if (g_tP2pManagerStatus.listLocalBuddyChainInfo.empty())
	{
	}

	
	CAutoMutexLock muxAutoG(g_MuxP2pManagerStatus);
	index = CurBuddyBlockInTheHyperBlock(blockInfos);
	

	if (!index)
	{
		ReOnChainFun();
	}
	CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
	g_tP2pManagerStatus.listGlobalBuddyChainInfo.clear();
	g_tP2pManagerStatus.bHaveOnChainReq = false;
	g_tP2pManagerStatus.bLocalBuddyChainState = false;
	g_tP2pManagerStatus.uiNextStartTimeNewest = pHyperBlockInfosTemp->tBlockBaseInfo.uiTime;
	g_tP2pManagerStatus.bStartGlobalFlag = false;
}
bool CHChainP2PManager::CurBuddyBlockInTheHyperBlock(T_HYPERBLOCK blockInfos)
{
	bool index = false;
	list<LIST_T_LOCALBLOCK>::iterator itr = blockInfos.listPayLoad.begin();
	for (; itr != blockInfos.listPayLoad.end(); itr++)
	{
		ITR_LIST_T_LOCALBLOCK subItr = itr->begin();
		for (subItr; subItr != itr->end(); subItr++)
		{
			if (subItr->tBlockBaseInfo.tHashSelf == g_tP2pManagerStatus.curBuddyBlock.tLocalBlock.tBlockBaseInfo.tHashSelf)
			{
				index = true;
				char localHash[TEMP_BUF_LEN] = { 0 };
				CCommonStruct::Hash256ToStr(localHash, &g_tP2pManagerStatus.curBuddyBlock.tLocalBlock.tBlockBaseInfo.tHashSelf);
				if (0 != strcmp(g_tP2pManagerStatus.curBuddyBlock.strID, ""))
				{
					T_SEARCHINFO searchInfo;
					searchInfo.uiHyperID = blockInfos.tBlockBaseInfo.uiID;
					searchInfo.uiTime = time(NULL);
					CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxMapSearchOnChain);
					g_tP2pManagerStatus.mapSearchOnChain[g_tP2pManagerStatus.curBuddyBlock.strID] = searchInfo;
				}
				g_tP2pManagerStatus.uiSendConfirmingRegisReqNum -= 1;
				g_tP2pManagerStatus.listLocalBuddyChainInfo.clear();
				break;
			}
		}

		if (index)
		{
			break;
		}
		
	}
	return index;
}
void CHChainP2PManager::ProcessGetBlockStateReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{ 
	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	T_PP2PPROTOCOLGETCHAINSTATERSP pP2pProtocolGetChainStateRsp = NULL;

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}

	int blockNum = m_BlockStateMap.size();

	int ipP2pProtocolGetChainStateRspLen = sizeof(T_P2PPROTOCOLGETCHAINSTATERSP) + blockNum * sizeof(T_CHAINSTATEINFO);
	pP2pProtocolGetChainStateRsp = (T_PP2PPROTOCOLGETCHAINSTATERSP)malloc(ipP2pProtocolGetChainStateRspLen);

	pP2pProtocolGetChainStateRsp->tResult.tType.ucType = P2P_PROTOCOL_GET_BLOCK_STATE_RSP;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolGetChainStateRsp->tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolGetChainStateRsp->tResult.iResult = P2P_PROTOCOL_SUCCESS;
	pP2pProtocolGetChainStateRsp->uiCount = blockNum;
	pP2pProtocolGetChainStateRsp->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	pP2pProtocolGetChainStateRsp->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;

	T_PCHAINSTATEINFO pPeerInfos;
	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
		pPeerInfos = NULL;
	else
		pPeerInfos = (T_PCHAINSTATEINFO)(pP2pProtocolGetChainStateRsp + 1);

	int i = 0;
	ITR_MAP_BLOCK_STATE itr = m_BlockStateMap.begin();
	for (; itr != m_BlockStateMap.end(); itr++)
	{
		pPeerInfos[i].uiBlockNum = (*itr).first;
		i++;
	}

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolGetChainStateRsp, ipP2pProtocolGetChainStateRspLen);

	free(pP2pProtocolGetChainStateRsp);
	pP2pProtocolGetChainStateRsp = NULL;
}
void CHChainP2PManager::ProcessGetBlockStateRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	GETTIMEOFDAY(&tvStart);

	T_PP2PPROTOCOLGETCHAINSTATERSP pP2pProtocolGetChainStateRsp = (T_PP2PPROTOCOLGETCHAINSTATERSP)(pBuf);
	if (P2P_PROTOCOL_SUCCESS != pP2pProtocolGetChainStateRsp->tResult.iResult)
	{
		return;
	}

	T_PCHAINSTATEINFO	pPeerInfo = NULL;
	T_PCHAINSTATEINFO pPeerInfoTemp = NULL;
	pPeerInfo = (T_PCHAINSTATEINFO)(pP2pProtocolGetChainStateRsp + 1);

	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	for (int i = 0; i<pP2pProtocolGetChainStateRsp->uiCount; i++)
	{
		bool bIndex = false;
		pPeerInfoTemp = pPeerInfo + i;
		ITR_MAP_BLOCK_STATE itr = m_BlockStateMap.find(pPeerInfoTemp->uiBlockNum);
		if (itr != m_BlockStateMap.end())
		{
			ITR_LIST_T_PBLOCKSTATEADDR itrSub = (*itr).second.begin();
			for (; itrSub != (*itr).second.end(); itrSub++)
			{
				if (((*itrSub)->tPeerAddr.uiIP == pP2pProtocolGetChainStateRsp->tPeerAddr.uiIP)
					&&((*itrSub)->tPeerAddr.uiPort == pP2pProtocolGetChainStateRsp->tPeerAddr.uiPort)
					&&(*itrSub)->tPeerAddrOut.uiIP == inet_addr(pszIP) && ((*itrSub)->tPeerAddrOut.uiPort == usPort))
				{
					bIndex = true;
					break;
				}
			}

			if (!bIndex)
			{
				T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;
				pBlockStateAddr->tPeerAddr.uiIP = pP2pProtocolGetChainStateRsp->tPeerAddr.uiIP;
				pBlockStateAddr->tPeerAddr.uiPort = pP2pProtocolGetChainStateRsp->tPeerAddr.uiPort;
				pBlockStateAddr->tPeerAddrOut.uiIP = inet_addr(pszIP);
				pBlockStateAddr->tPeerAddrOut.uiPort = usPort;

				(*itr).second.push_back(pBlockStateAddr);

				char pszPeerIP[MAX_IP_LEN] = "";
				struct in_addr addPeerIP;
				addPeerIP.s_addr = pP2pProtocolGetChainStateRsp->tPeerAddr.uiIP;
				strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			}
			
		}
		
	}
}

void CHChainP2PManager::SearchPeerList(char *strName, uint32 uiIPIN, uint16 usPortIN, uint32 uiIPOUT, uint16 usPortOUT, bool bFlag, uint64 uiTime, uint16 uiNodeState)
{
	char pszPeerIP[MAX_IP_LEN] = "";
	char pszPeerIPOut[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	struct in_addr addPeerIPOut;
	addPeerIP.s_addr = uiIPIN;
	addPeerIPOut.s_addr = uiIPOUT;   
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));
	
	bool bIndex = false;
	if ((uiIPOUT == m_MyPeerInfo.tPeerInfoByOther.uiIP)
		&& (usPortOUT == m_MyPeerInfo.tPeerInfoByOther.uiPort)
		&& (uiIPIN == m_MyPeerInfo.tPeerInfoByMyself.uiIP)
		&& (usPortIN == m_MyPeerInfo.tPeerInfoByMyself.uiPort))
	{
		bIndex = true;
		return;
	}

	if (!bIndex)
	{
		if (0 == strncmp(m_MyPeerInfo.strName, strName, MAX_NODE_NAME_LEN)
			&& (uiIPIN == m_MyPeerInfo.tPeerInfoByMyself.uiIP)
			&& (usPortIN == m_MyPeerInfo.tPeerInfoByMyself.uiPort))
		{
			m_MyPeerInfo.tPeerInfoByOther.uiIP = uiIPOUT;
			m_MyPeerInfo.tPeerInfoByOther.uiPort = usPortOUT;

			if ((uiIPIN == uiIPOUT) && (usPortIN == usPortOUT))
			{
				m_MyPeerInfo.uiState = OUTNET;
			}
			else
			{
				m_MyPeerInfo.uiState = INNERNET;
			}

			addPeerIP.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
			addPeerIPOut.s_addr = m_MyPeerInfo.tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));
			strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));

			return;
		}
	}

	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	bool iIndex = false;
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{	
		if (0 == strncmp((*itr)->strName, strName, MAX_NODE_NAME_LEN)
			&&(uiIPIN == (*itr)->tPeerInfoByMyself.uiIP)
			&& (usPortIN == (*itr)->tPeerInfoByMyself.uiPort) 
			&& (uiIPOUT == (*itr)->tPeerInfoByOther.uiIP) 
			&& (usPortOUT == (*itr)->tPeerInfoByOther.uiPort))
		{
	
			uint64 timeTemp = time(NULL);
			if(bFlag)
			{
				(*itr)->uiTime = (timeTemp - uiTime);
			}
			
			(*itr)->uiNodeState = uiNodeState;
			iIndex = true;
		}
	}

	if (!iIndex)
	{
		T_PPEERINFO pPeerInfo = new T_PEERINFO;
		
		memcpy(pPeerInfo->strName ,strName, MAX_NODE_NAME_LEN);
		pPeerInfo->tPeerInfoByMyself.uiIP = uiIPIN;
		pPeerInfo->tPeerInfoByMyself.uiPort = usPortIN;
		pPeerInfo->tPeerInfoByOther.uiIP = uiIPOUT;
		pPeerInfo->tPeerInfoByOther.uiPort = usPortOUT;
		if ((pPeerInfo->tPeerInfoByMyself.uiIP == pPeerInfo->tPeerInfoByOther.uiIP)
			&& (pPeerInfo->tPeerInfoByMyself.uiPort == pPeerInfo->tPeerInfoByOther.uiPort))
		{
			pPeerInfo->uiState = OUTNET;
			pPeerInfo->uiNatTraversalState = GOOD;
		}
		else
		{
			pPeerInfo->uiState = INNERNET;
			pPeerInfo->uiNatTraversalState = DEFAULT_NATTRAVERSAL_STATE;
			SendToOutPeerWantNatTraversalReq(pPeerInfo);
		}
		if(bFlag)
		{
			uint64 timeTemp = time(NULL);
			pPeerInfo->uiTime = (timeTemp- uiTime); 
		}
		else
		{
			pPeerInfo->uiTime = INIT_TIME;
		}
		pPeerInfo->uiNodeState = uiNodeState;

		char pszPeerIP[MAX_IP_LEN] = "";
		struct in_addr addPeerIP;
		addPeerIP.s_addr = pPeerInfo->tPeerInfoByMyself.uiIP;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));
		if (0 != strForbIp1.compare(pszPeerIP))
		{
			m_PeerInfoList.push_back(pPeerInfo);

			m_PeerInfoList.sort(Cmpare());
		}
	}
}
void CHChainP2PManager::SendPeerListToPeer(uint32 uiIP, uint16 usPort)
{
	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);

	T_PP2PPROTOCOLPEERLISTRSP pP2pProtocolPeerlistRsp = NULL;

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}

	int tempCount = 0;
	int nodeNum = m_PeerInfoList.size();
	if (g_tP2pManagerStatus.usBuddyPeerCount >= nodeNum)
	{
		tempCount = nodeNum;
	}
	else
	{
		tempCount = g_tP2pManagerStatus.usBuddyPeerCount;
	}

	int iP2pProtocolPeerlistRspLen = sizeof(T_P2PPROTOCOLPEERLISTRSP) + tempCount * sizeof(T_PEERINFO);
	pP2pProtocolPeerlistRsp = (T_PP2PPROTOCOLPEERLISTRSP)malloc(iP2pProtocolPeerlistRspLen);

	pP2pProtocolPeerlistRsp->tResult.tType.ucType = P2P_PROTOCOL_PEERLIST_RSP;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	pP2pProtocolPeerlistRsp->tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolPeerlistRsp->tResult.iResult = P2P_PROTOCOL_SUCCESS;
	pP2pProtocolPeerlistRsp->uiMaxBlockNum = g_tP2pManagerStatus.uiMaxBlockNum;
	pP2pProtocolPeerlistRsp->tPeerAddr.uiIP = g_confFile.uiLocalIP;
	pP2pProtocolPeerlistRsp->tPeerAddr.uiPort = g_confFile.uiLocalPort;
	pP2pProtocolPeerlistRsp->uiCount = tempCount;
	pP2pProtocolPeerlistRsp->uiNodeState = g_tP2pManagerStatus.uiNodeState;
	strncpy(pP2pProtocolPeerlistRsp->strName,g_confFile.strLocalNodeName.c_str(), MAX_NODE_NAME_LEN);
	
	T_PPEERINFO pPeerInfos;
	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
		pPeerInfos = NULL;
	else
		pPeerInfos = (T_PPEERINFO)(pP2pProtocolPeerlistRsp + 1);

	int i = 0;
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for(;itr != m_PeerInfoList.end(); itr ++)
	{
		if (i < tempCount)
		{
			pPeerInfos[i].tPeerInfoByMyself.uiIP = (*itr)->tPeerInfoByMyself.uiIP;

			pPeerInfos[i].tPeerInfoByMyself.uiPort = (*itr)->tPeerInfoByMyself.uiPort;
			pPeerInfos[i].tPeerInfoByOther.uiIP = (*itr)->tPeerInfoByOther.uiIP;
			pPeerInfos[i].tPeerInfoByOther.uiPort = (*itr)->tPeerInfoByOther.uiPort;
			pPeerInfos[i].uiState = (*itr)->uiState;
			pPeerInfos[i].uiNatTraversalState = (*itr)->uiNatTraversalState;
			pPeerInfos[i].uiNodeState = (*itr)->uiNodeState;
			pPeerInfos[i].uiTime = (*itr)->uiTime;
			strncpy(pPeerInfos[i].strName, (*itr)->strName, MAX_NODE_NAME_LEN);

			char pszPeerIP[MAX_IP_LEN] = "";
			char pszPeerIPOut[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			struct in_addr addPeerIPOut;
			addPeerIP.s_addr = pPeerInfos[i].tPeerInfoByMyself.uiIP;
			addPeerIPOut.s_addr = pPeerInfos[i].tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));
			strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));


			addPeerIP.s_addr = (*itr)->tPeerInfoByMyself.uiIP;
			addPeerIPOut.s_addr = (*itr)->tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));
			strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));


			i++;
		}
		else
		{
			break;
		}
	}

	m_UdpSocket.Send(uiIP, usPort, (char*)pP2pProtocolPeerlistRsp, iP2pProtocolPeerlistRspLen);

	free(pP2pProtocolPeerlistRsp);
	pP2pProtocolPeerlistRsp = NULL;
}
void CHChainP2PManager::CreateGenesisHyperBlock()
{

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	CAutoMutexLock muxAuto1(m_MuxBlockStateMap);

	T_FILEINFO fileInfo;
	strncpy(fileInfo.fileName, "filename_GenesisHyperBlock", MAX_FILE_NAME_LEN);
	strncpy(fileInfo.customInfo, "custominfo_GenesisHyperBlock", MAX_CUSTOM_INFO_LEN);
	strncpy(fileInfo.tFileOwner, "fileowner_GenesisHyperBlock", MAX_CUSTOM_INFO_LEN);
	fileInfo.uiFileSize = 0;
	memset(fileInfo.tFileHash.pID, 1, DEF_SHA512_LEN);

	T_PRIVATEBLOCK tPrivateBlock;
	tPrivateBlock.tBlockBaseInfo.uiID = 0;
	tPrivateBlock.tBlockBaseInfo.uiTime = 0;
	strncpy(tPrivateBlock.tBlockBaseInfo.strScript, "CustomizedScript", MAX_SCRIPT_LEN);
	strncpy(tPrivateBlock.tBlockBaseInfo.strAuth, "AuthInfo", MAX_AUTH_LEN);
	tPrivateBlock.tPayLoad = fileInfo;
	memset(tPrivateBlock.tBlockBaseInfo.tPreHash.pID, 1, DEF_SHA256_LEN);
	memset(tPrivateBlock.tHHash.pID, 1, DEF_SHA256_LEN);
	memset(tPrivateBlock.tBlockBaseInfo.tHashSelf.pID, 1, DEF_SHA256_LEN);

	T_LOCALBLOCK tLocalBlock;
	tLocalBlock.tBlockBaseInfo.uiID = 0;
	tLocalBlock.tBlockBaseInfo.uiTime = 0;
	strncpy(tLocalBlock.tBlockBaseInfo.strScript, "CustomizedScript", MAX_SCRIPT_LEN);
	strncpy(tLocalBlock.tBlockBaseInfo.strAuth, "AuthInfo", MAX_AUTH_LEN);
	memset(tLocalBlock.tBlockBaseInfo.tPreHash.pID, 1, DEF_SHA256_LEN);
	memset(tLocalBlock.tHHash.pID, 1, DEF_SHA256_LEN);
	tLocalBlock.tPayLoad = tPrivateBlock;
	memset(tLocalBlock.tBlockBaseInfo.tHashSelf.pID, 1, DEF_SHA256_LEN);

	T_HYPERBLOCK tHyperChainBlock;
	tHyperChainBlock.tBlockBaseInfo.uiID = 0;
	tHyperChainBlock.tBlockBaseInfo.uiTime = time(NULL);
	strncpy(tHyperChainBlock.tBlockBaseInfo.strScript, "CustomizedScript", MAX_SCRIPT_LEN);
	strncpy(tHyperChainBlock.tBlockBaseInfo.strAuth, "AuthInfo", MAX_AUTH_LEN);
	memset(tHyperChainBlock.tBlockBaseInfo.tPreHash.pID, 1, DEF_SHA256_LEN);
	LIST_T_LOCALBLOCK ListLocalBlock;
	ListLocalBlock.push_back(tLocalBlock);
	tHyperChainBlock.listPayLoad.push_back(ListLocalBlock);
	memset(tHyperChainBlock.tHashAll.pID, 1, DEF_SHA256_LEN);
	memset(tHyperChainBlock.tBlockBaseInfo.tHashSelf.pID, 1, DEF_SHA256_LEN);

	m_HchainBlockList.push_back(tHyperChainBlock);
	SaveHyperBlockToLocal(tHyperChainBlock);
	WriteBlockLog(tHyperChainBlock);

	T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;
	pBlockStateAddr->tPeerAddr.uiIP = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	pBlockStateAddr->tPeerAddr.uiPort = m_MyPeerInfo.tPeerInfoByMyself.uiPort;
	pBlockStateAddr->tPeerAddrOut.uiIP = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	pBlockStateAddr->tPeerAddrOut.uiPort = m_MyPeerInfo.tPeerInfoByOther.uiPort;

	LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
	listPblockStateAddr.push_back(pBlockStateAddr);
	m_BlockStateMap[tLocalBlock.tBlockBaseInfo.uiID] = listPblockStateAddr;

	char pszPeerIP[MAX_IP_LEN] = "";
	char pszPeerIPOut[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	struct in_addr addPeerIPOut;
	addPeerIP.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	addPeerIPOut.s_addr = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	strcpy(pszPeerIPOut, inet_ntoa(addPeerIPOut));

	g_tP2pManagerStatus.tPreHyperBlock = tHyperChainBlock;
	g_tP2pManagerStatus.uiStartTimeOfConsensus = tHyperChainBlock.tBlockBaseInfo.uiTime;
	g_tP2pManagerStatus.bHaveOnChainReq = false;
}

void CHChainP2PManager::ReOnChain(T_LOCALCONSENSUS localInfo)
{
	if (g_tP2pManagerStatus.bHaveOnChainReq)
	{
		return;
	}

	g_tP2pManagerStatus.bHaveOnChainReq = true;

	if ((time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus) >= LOCALBUDDYTIME)
	{
		return;
	}

	g_tP2pManagerStatus.uiTimeOfConsensus = time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus;

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back(localInfo);
	g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
	ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
}
void CHChainP2PManager::AddNewBlock(TEVIDENCEINFO tFileInfo, const char *rand)
{
	
	g_tP2pManagerStatus.uiSendRegisReqNum += 1;
	g_tP2pManagerStatus.uiSendConfirmingRegisReqNum += 1;



	T_FILEINFO pFileInfo;
	strncpy(pFileInfo.fileName, tFileInfo.cFileName.c_str(), MAX_FILE_NAME_LEN);
	strncpy(pFileInfo.tFileOwner, tFileInfo.cRightOwner.c_str(), MAX_CUSTOM_INFO_LEN);
	pFileInfo.uiFileSize = tFileInfo.iFileSize;

	strncpy(pFileInfo.customInfo, tFileInfo.cCustomInfo.c_str(), MAX_CUSTOM_INFO_LEN);
	char tempBuf[DEF_SHA512_LEN];
	memset(tempBuf, 0, DEF_SHA512_LEN);
	strncpy(tempBuf, tFileInfo.cFileHash.c_str(), DEF_SHA512_LEN);
	CCommonStruct::StrToHash512(pFileInfo.tFileHash.pID, tempBuf);

	T_PRIVATEBLOCK tPrivateBlock;
	tPrivateBlock.tBlockBaseInfo.uiID = 1;
	tPrivateBlock.tBlockBaseInfo.uiTime = time(NULL);
	strncpy(tPrivateBlock.tBlockBaseInfo.strScript, "CustomizedScript", MAX_SCRIPT_LEN);
	strncpy(tPrivateBlock.tBlockBaseInfo.strAuth, "AuthInfo", MAX_AUTH_LEN);
	tPrivateBlock.tPayLoad = pFileInfo;
	memset(tPrivateBlock.tBlockBaseInfo.tPreHash.pID, 0, DEF_SHA256_LEN);
	tPrivateBlock.tHHash = g_tP2pManagerStatus.tPreHyperBlock.tBlockBaseInfo.tHashSelf;
	GetSHA256(tPrivateBlock.tBlockBaseInfo.tHashSelf.pID, (const char*)(&tPrivateBlock), sizeof(tPrivateBlock));

	T_LOCALBLOCK tLocalBlock;
	tLocalBlock.tBlockBaseInfo.uiID = 1;
	tLocalBlock.uiAtChainNum = 1;
	tLocalBlock.tBlockBaseInfo.uiTime = time(NULL);
	strncpy(tLocalBlock.tBlockBaseInfo.strScript, "CustomizedScript", MAX_SCRIPT_LEN);
	strncpy(tLocalBlock.tBlockBaseInfo.strAuth, "AuthInfo", MAX_AUTH_LEN);
	memset(tLocalBlock.tBlockBaseInfo.tPreHash.pID, 0, DEF_SHA256_LEN);
	tLocalBlock.tHHash = g_tP2pManagerStatus.tPreHyperBlock.tBlockBaseInfo.tHashSelf;
	tLocalBlock.tPayLoad = tPrivateBlock;
	GetSHA256(tLocalBlock.tBlockBaseInfo.tHashSelf.pID, (const char*)(&tLocalBlock), sizeof(tLocalBlock));

	T_LOCALCONSENSUS localConsensusInfo;
	localConsensusInfo.tPeer.tPeerAddr = m_MyPeerInfo.tPeerInfoByMyself;
	localConsensusInfo.tPeer.tPeerAddrOut = m_MyPeerInfo.tPeerInfoByOther;
	localConsensusInfo.tLocalBlock = tLocalBlock;
	strncpy(localConsensusInfo.strID, rand, MAX_QUEED_LEN);

	g_tP2pManagerStatus.listOnChainReq.push_back(localConsensusInfo);
}

void CHChainP2PManager::SendBlockToPeer(uint32 uiIP, uint16 usPort, uint64 uiBlockNum)
{
   	T_PP2PPROTOCOLADDBLOCKREQ pP2pProtocolAddBlockReq = NULL;
	
	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}  
	
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		if ((*itr).tBlockBaseInfo.uiID > uiBlockNum)
		{
			uint64 totalLocalBlockNum = 0;
			list<LIST_T_LOCALBLOCK>::iterator itrSub = itr->listPayLoad.begin();
			for (; itrSub != itr->listPayLoad.end(); itrSub++)
			{
				totalLocalBlockNum += (*itrSub).size();
			} 
			uint64 hyperBlockSize = ((totalLocalBlockNum)* sizeof(T_LOCALBLOCK)) + sizeof(T_HYPERBLOCKSEND);

			int iP2pProtocolAddBlockLen = sizeof(T_P2PPROTOCOLADDBLOCKREQ) + hyperBlockSize;

			pP2pProtocolAddBlockReq = (T_PP2PPROTOCOLADDBLOCKREQ)malloc(iP2pProtocolAddBlockLen);
			pP2pProtocolAddBlockReq->uiBlockCount = totalLocalBlockNum;
			pP2pProtocolAddBlockReq->tType.ucType = P2P_PROTOCOL_ADD_BLOCK_REQ;
			struct timeval timeTemp;
			GETTIMEOFDAY(&timeTemp);
			pP2pProtocolAddBlockReq->tType.uiTimeStamp = timeTemp.tv_sec;
			pP2pProtocolAddBlockReq->uiSendTimes = HYPERBLOCK_SYNC_TIMES;
			pP2pProtocolAddBlockReq->uiBlockNum = (*itr).tBlockBaseInfo.uiID;

			T_PHYPERBLOCKSEND pHyperBlock;
			pHyperBlock = (T_PHYPERBLOCKSEND)(pP2pProtocolAddBlockReq + 1);
			pHyperBlock->tBlockBaseInfo = (*itr).tBlockBaseInfo;
			pHyperBlock->tHashAll = (*itr).tHashAll;

			T_PLOCALBLOCK pSendLocalBlock = NULL;
			pSendLocalBlock = (T_PLOCALBLOCK)(pHyperBlock + 1);
			int i = 0;
			uint8 chainNum = 0;
		
			list<LIST_T_LOCALBLOCK>::iterator hyperItr = itr->listPayLoad.begin();
			for (; hyperItr != itr->listPayLoad.end(); hyperItr++)
			{
				chainNum++;
				list<T_LOCALBLOCK>::iterator localItr = hyperItr->begin();
				for (; localItr != hyperItr->end(); localItr++)
				{
					pSendLocalBlock[i].tBlockBaseInfo = (*localItr).tBlockBaseInfo;
					pSendLocalBlock[i].tHHash = (*localItr).tHHash;
					pSendLocalBlock[i].tPayLoad = (*localItr).tPayLoad;
					pSendLocalBlock[i].uiAtChainNum = chainNum;
					i++;
				}
			}

			m_UdpSocket.Send(uiIP, usPort, (char*)pP2pProtocolAddBlockReq, iP2pProtocolAddBlockLen);

			char pszPeerIP[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			free(pP2pProtocolAddBlockReq);
			iP2pProtocolAddBlockLen = NULL;
		}
	}

}
void CHChainP2PManager::SendGetChainStateReq()
{
	T_P2PPROTOCOLGETCHAINSTATEREQ tGetStateReq;
	tGetStateReq.tType.ucType = P2P_PROTOCOL_GET_BLOCK_STATE_REQ;
	tGetStateReq.uiMaxBlockNum = g_tP2pManagerStatus.uiMaxBlockNum;
	struct timeval timeTemp;
	GETTIMEOFDAY(&timeTemp);
	tGetStateReq.tType.uiTimeStamp = timeTemp.tv_sec;

	int index = 0;
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	int tempSendPeerNum = g_tP2pManagerStatus.usBuddyPeerCount;
	int peerNum = m_PeerInfoList.size();
	if (peerNum < g_tP2pManagerStatus.usBuddyPeerCount)
	{
		tempSendPeerNum = peerNum;
	}
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (index < tempSendPeerNum)
		{
			char pszPeerIP[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr)->tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));
		
			m_UdpSocket.Send((*itr)->tPeerInfoByOther.uiIP, (*itr)->tPeerInfoByOther.uiPort,(char*)&tGetStateReq, sizeof(tGetStateReq));
			index++;
		}
		else
			break;
	}
}

uint64 CHChainP2PManager::GetCurBlockNumOfMyself()
{

	return 0;
}

uint64 CHChainP2PManager::GetCurBlockNumOfAllNode()
{
	return g_tP2pManagerStatus.uiMaxBlockNum;
}
VEC_T_BLOCKINFO CHChainP2PManager::GetBlockInfoByIndex(uint64 start, uint64 end)
{
	VEC_T_BLOCKINFO blockInfoVec;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		{
			list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
			for (; subItr != itr->listPayLoad.end(); subItr++)
			{
				list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
				for (; ssubItr != (*subItr).end(); ssubItr++)
				{
					T_PLOCALBLOCK localBlock = &(*ssubItr);

					T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
					T_PFILEINFO fileInfo = &((privateBlock->tPayLoad));

					P_TBLOCKINFO pBlockInfo = new TBLOCKINFO;

		
					pBlockInfo->iBlockNo = (*itr).tBlockBaseInfo.uiID;
					pBlockInfo->tPoeRecordInfo.cCustomInfo = fileInfo->customInfo;
					char strBuf[MAX_BUF_LEN];
					memset(strBuf, 0, MAX_BUF_LEN);
					CCommonStruct::Hash512ToStr(strBuf, &fileInfo->tFileHash);
					pBlockInfo->tPoeRecordInfo.cFileHash = strBuf;
					pBlockInfo->tPoeRecordInfo.cFileName = fileInfo->fileName;

					pBlockInfo->tPoeRecordInfo.cRightOwner = fileInfo->tFileOwner;
					pBlockInfo->tPoeRecordInfo.iFileSize = fileInfo->uiFileSize;
					pBlockInfo->tPoeRecordInfo.tRegisTime = privateBlock->tBlockBaseInfo.uiTime;
					pBlockInfo->iCreatTime = localBlock->tBlockBaseInfo.uiTime;
					blockInfoVec.push_back(pBlockInfo);
				}
			}
		}
	}
	return blockInfoVec;
}



uint64 CHChainP2PManager::GetLatestHyperBlockNo()
{
	return g_tP2pManagerStatus.uiMaxBlockNum;
}

uint32 CHChainP2PManager::GetSendRegisReqNum()
{

	muLock.lock();
	uint64 recvNum = g_tP2pManagerStatus.uiSendRegisReqNum;
	muLock.unlock();
	return recvNum;
}

uint32 CHChainP2PManager::GetSendConfirmingRegisReqNum()
{
	muLock.lock();
	uint64 recvNum = g_tP2pManagerStatus.uiSendConfirmingRegisReqNum;
	muLock.unlock();
	return recvNum;
}

uint32 CHChainP2PManager::GetRecvRegisReqNum()
{

	muLock.lock();
	uint64 recvNum = g_tP2pManagerStatus.uiRecvRegisReqNum;
	muLock.unlock();
	return recvNum;
}
uint32 CHChainP2PManager::GetRecvConfirmingRegisReqNum()
{
	muLock.lock();
	uint64 recvNum = g_tP2pManagerStatus.uiRecvConfirmingRegisReqNum;
	muLock.unlock();
	return recvNum;
}
bool CHChainP2PManager::MatchFile(string &checkFileHash, P_TEVIDENCEINFO pCheckInfo)
{
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
		for (; subItr != itr->listPayLoad.end(); subItr++)
		{
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);

				T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
				T_PFILEINFO fileInfo = &(privateBlock->tPayLoad);

				if (0 == pCheckInfo->cFileName.compare(fileInfo->fileName))
				{
					char strBuf[DEF_SHA512_LEN * 2];
					memset(strBuf, 0, DEF_SHA512_LEN * 2);
					CCommonStruct::Hash512ToStr(strBuf, &fileInfo->tFileHash);
					if (0 == checkFileHash.compare(strBuf))
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

string ChangeStorageUnit(uint64 data, char uint)
{
	string buf("");
	if (data >= BIN_UINT)
	{
		data = data / BIN_UINT;
		if (uint == 'B')
		{
			buf = ChangeStorageUnit(data, 'K').c_str();
		}
		else if (uint == 'K')
		{
			buf = ChangeStorageUnit(data, 'M').c_str();
		}
		else if (uint == 'M')
		{
			buf = ChangeStorageUnit(data, 'T').c_str();
		}
	}
	else
	{
		char testBuf[TEMP_BUF_LEN];
		memset(testBuf, 0, TEMP_BUF_LEN);
		sprintf(testBuf, "%d", data);
		buf = testBuf;
		if (uint == 'B')
		{
			buf += "Byte";
		}
		else
		{
			buf += uint;
			buf += "Byte";
		}
	}

	return buf;
}
string CHChainP2PManager::GetChainData()
{
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	uint64 iCatch = 0;
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		uint64 totalLocalBlockNum = 0;
		list<LIST_T_LOCALBLOCK>::iterator itrSub = itr->listPayLoad.begin();
		for (; itrSub != itr->listPayLoad.end(); itrSub++)
		{
			totalLocalBlockNum += (*itrSub).size();
		}
		uint64 hyperBlockSize = ((totalLocalBlockNum)* sizeof(T_LOCALBLOCK)) + sizeof(T_HYPERBLOCKSEND);
		iCatch += hyperBlockSize;
	}
	
	string strRet = ChangeStorageUnit(iCatch, 'B');
	return strRet;
}

uint64 CHChainP2PManager::GetAllNodesInTheNet()
{
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	uint64 ret = m_PeerInfoList.size();
	return ret;
}

void CHChainP2PManager::GetNodeDescription(string &info, string &ip, uint16 &uiport)
{
	info = m_MyPeerInfo.strName;
	uiport = m_MyPeerInfo.tPeerInfoByMyself.uiPort;

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = m_MyPeerInfo.tPeerInfoByMyself.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	ip = pszPeerIP;
}
uint32 CHChainP2PManager::GetNodeNum(uint32 uiStart, uint32 uiEnd)
{
	int iNum = g_tP2pManagerStatus.usBuddyPeerCount;
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	uint64 ret = m_PeerInfoList.size();
	if (ret <= iNum)
		iNum = ret;

	int i = 0;
	int sum = 0;
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();  
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (i < iNum)
		{
			if ( (*itr)->uiTime >= uiStart && (*itr)->uiTime <= uiEnd)
				sum++;
		}
		i++;
	}

	return sum;
}
uint32 CHChainP2PManager::GetStrongHyperBlockNo()
{
	return GetNodeNum(0, FIVE_MIN);
}
uint32 CHChainP2PManager::GetAverageHyperBlockNo()
{
	return GetNodeNum(FIVE_MIN + 1, HALF_AN_HOUR);
}
uint32 CHChainP2PManager::GetWeakHyperBlockNo()
{
	return GetNodeNum(ONE_HOUR, ONE_HOUR * 12);
}
uint32 CHChainP2PManager::GetOfflineHyperBlockNo()
{
	return GetNodeNum(ONE_HOUR * 12, ONE_HOUR * 24);
}

void CHChainP2PManager::GetSendingRate(string &sendRate)
{
	char strBuf[TEMP_BUF_LEN];
	memset(strBuf, 0, TEMP_BUF_LEN);
	sprintf(strBuf, "%dB/s", m_UdpSocket.GetSendRate());
	sendRate = strBuf;
}

void CHChainP2PManager::GetSentSize(string &allSendSize)
{
	allSendSize = m_UdpSocket.GetSendSize();
}

void CHChainP2PManager::GetRecvingRate(string &recvRate)
{
	char strBuf[TEMP_BUF_LEN];
	memset(strBuf, 0, TEMP_BUF_LEN);
	sprintf(strBuf, "%dB/s", m_UdpSocket.GetRecvRate());
	recvRate = strBuf;
}

void CHChainP2PManager::GetRecvedSize(string &allRecvSize)
{
	allRecvSize = m_UdpSocket.GetRecvSize();
}
int64 CHChainP2PManager::GetOnChainState(string queueId)
{
	int64 retNum = -3;

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistOnChainReq);
	LIST_T_LOCALCONSENSUS::iterator itrOnChain = g_tP2pManagerStatus.listOnChainReq.begin();
	for (itrOnChain; itrOnChain != g_tP2pManagerStatus.listOnChainReq.end(); itrOnChain++)
	{
		if (0 == queueId.compare((*itrOnChain).strID))
		{
			retNum = -2;
			return retNum;
		}
	}
	
	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	LIST_T_LOCALCONSENSUS::iterator itrOnChaining = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
	for (itrOnChaining; itrOnChaining != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itrOnChaining++)
	{
		if (0 == queueId.compare((*itrOnChaining).strID))
		{
			retNum = -1;
			return retNum;
		}
	}

	CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxMapSearchOnChain);
	ITR_MAP_T_SEARCHONCHAIN itr = g_tP2pManagerStatus.mapSearchOnChain.find(queueId);
	if(itr != g_tP2pManagerStatus.mapSearchOnChain.end())
	{
		retNum = (*itr).second.uiHyperID;
		return retNum;
	}

	return retNum;
}

VEC_T_BROWSERSHOWINFO CHChainP2PManager::GetBlockInfoByHash(string &hash)
{
	VEC_T_BROWSERSHOWINFO vecBrowserInfo;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
		for (; subItr != itr->listPayLoad.end(); subItr++)
		{
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);
				T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
				T_PFILEINFO fileInfo = &((privateBlock->tPayLoad));

				T_SHA512 tFileHash;
				char strBuf[DEF_SHA512_LEN];
				memset(strBuf, 0, DEF_SHA512_LEN);
				strncpy(strBuf, hash.c_str(), hash.length());
				CCommonStruct::StrToHash512(tFileHash.pID, strBuf);
				if (0 == strncmp(reinterpret_cast<const char*>(tFileHash.pID), reinterpret_cast<const char*>(fileInfo->tFileHash.pID), DEF_SHA512_LEN))
				{
					TBROWSERSHOWINFO tempBlockInfo;
					tempBlockInfo.iBlockNo = itr->tBlockBaseInfo.uiID;
					tempBlockInfo.iLocalChainNum = (*itr).listPayLoad.size();
					tempBlockInfo.iLocalBlockNum = 1;
					tempBlockInfo.iJoinedNodeNum = 1;
					tempBlockInfo.tPoeRecordInfo.cCustomInfo = fileInfo->customInfo;

					char strBuf[MAX_BUF_LEN];
					memset(strBuf, 0, MAX_BUF_LEN);
					CCommonStruct::Hash512ToStr(strBuf, &fileInfo->tFileHash);
					tempBlockInfo.tPoeRecordInfo.cFileHash = strBuf;

					tempBlockInfo.tPoeRecordInfo.cRightOwner = fileInfo->tFileOwner;
					tempBlockInfo.tPoeRecordInfo.cFileName = fileInfo->fileName;
					tempBlockInfo.tPoeRecordInfo.iFileSize = fileInfo->uiFileSize;
					tempBlockInfo.tTimeStamp = privateBlock->tBlockBaseInfo.uiTime;

					vecBrowserInfo.push_back(tempBlockInfo);
				}
			}
		}
	}
	return vecBrowserInfo;

}

VEC_T_BROWSERSHOWINFO CHChainP2PManager::GetBlockInfoByTime(string &time)
{
	VEC_T_BROWSERSHOWINFO vecBrowserInfo;
	return vecBrowserInfo;
}
VEC_T_BROWSERSHOWINFO CHChainP2PManager::GetBlockInfoByBlockNum(string &blockNum)
{
	VEC_T_BROWSERSHOWINFO vecBrowserInfo;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
		for (; subItr != itr->listPayLoad.end(); subItr++)
		{
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);

				T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
				T_PFILEINFO fileInfo = &((privateBlock->tPayLoad));

				uint64 bNum = atoi(blockNum.c_str());
				if (bNum == itr->tBlockBaseInfo.uiID)
				{
					TBROWSERSHOWINFO tempBlockInfo;
					tempBlockInfo.iBlockNo = itr->tBlockBaseInfo.uiID;
					tempBlockInfo.iLocalChainNum = (*itr).listPayLoad.size();
					tempBlockInfo.iLocalBlockNum = 1;
					tempBlockInfo.iJoinedNodeNum = 1;
					tempBlockInfo.tPoeRecordInfo.cCustomInfo = fileInfo->customInfo;

					char strBuf[MAX_BUF_LEN];
					memset(strBuf, 0, MAX_BUF_LEN);
					CCommonStruct::Hash512ToStr(strBuf, &fileInfo->tFileHash);
					tempBlockInfo.tPoeRecordInfo.cFileHash = strBuf;

					tempBlockInfo.tPoeRecordInfo.cRightOwner = fileInfo->tFileOwner;
					tempBlockInfo.tPoeRecordInfo.cFileName = fileInfo->fileName;
					tempBlockInfo.tPoeRecordInfo.iFileSize = fileInfo->uiFileSize;
					tempBlockInfo.tTimeStamp = privateBlock->tBlockBaseInfo.uiTime;

					vecBrowserInfo.push_back(tempBlockInfo);
				}
			}
		}
	}
	return vecBrowserInfo;

}
VEC_T_BROWSERSHOWINFO CHChainP2PManager::GetBlockInfoByCustomInfo(string &cusInfo)
{
	VEC_T_BROWSERSHOWINFO vecBrowserInfo;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
		for (; subItr != itr->listPayLoad.end(); subItr++)
		{
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);

				T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
				T_PFILEINFO fileInfo = &((privateBlock->tPayLoad));
				
				string strTemp = fileInfo->customInfo;
				if (string::npos != strTemp.find(cusInfo.c_str()))
				{
					TBROWSERSHOWINFO tempBlockInfo;
					tempBlockInfo.iBlockNo = itr->tBlockBaseInfo.uiID;
					tempBlockInfo.iLocalChainNum = (*itr).listPayLoad.size();
					tempBlockInfo.iLocalBlockNum = 1;
					tempBlockInfo.iJoinedNodeNum = 1;
					tempBlockInfo.tPoeRecordInfo.cCustomInfo = fileInfo->customInfo;

					char strBuf[MAX_BUF_LEN];
					memset(strBuf, 0, MAX_BUF_LEN);
					CCommonStruct::Hash512ToStr(strBuf, &fileInfo->tFileHash);
					tempBlockInfo.tPoeRecordInfo.cFileHash = strBuf;

					tempBlockInfo.tPoeRecordInfo.cRightOwner = fileInfo->tFileOwner;
					tempBlockInfo.tPoeRecordInfo.cFileName = fileInfo->fileName;
					tempBlockInfo.tPoeRecordInfo.iFileSize = fileInfo->uiFileSize;
					tempBlockInfo.tTimeStamp = privateBlock->tBlockBaseInfo.uiTime;

					vecBrowserInfo.push_back(tempBlockInfo);
				}
			}
		}
	}
	return vecBrowserInfo;
}

VEC_T_NODEINFO CHChainP2PManager::GetOtherLocalChain(uint16 chainNum)
{
	VEC_T_NODEINFO vecNodeInfo;

	int index = 0;
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (index % 2 == 1)
		{
			TNODEINFO nodeInfo;

			char pszPeerIP[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr)->tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));
			nodeInfo.strNodeIp = pszPeerIP;
			nodeInfo.uiNodeState = (*itr)->uiNodeState;

			vecNodeInfo.push_back(nodeInfo);
		}
		index++;
	}

	return vecNodeInfo;
}

VEC_T_NODEINFO CHChainP2PManager::GetMyLocalChain()
{
	VEC_T_NODEINFO vecNodeInfo;

	int index = 0;
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (index % 2 == 0)
		{
			TNODEINFO nodeInfo;

			char pszPeerIP[MAX_IP_LEN] = "";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr)->tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));
			nodeInfo.strNodeIp = pszPeerIP;
			nodeInfo.uiNodeState = (*itr)->uiNodeState;
			
			vecNodeInfo.push_back(nodeInfo);
		}
		index++;
	}

	return vecNodeInfo;
}


uint64 CHChainP2PManager::GetElaspedTimeOfCurrentConsensus()
{
	if (g_tP2pManagerStatus.uiNodeState == CONFIRMED)
	{
		return g_tP2pManagerStatus.uiTimeOfConsensus;
	}
	else if (g_tP2pManagerStatus.uiNodeState == CONFIRMING)
	{
		return (time(NULL) - g_tP2pManagerStatus.uiStartTimeOfConsensus);
	}
}
uint64 CHChainP2PManager::GetStartTimeOfConsensus()
{
	return g_tP2pManagerStatus.uiStartTimeOfConsensus;
}

VEC_T_HYPERBLOCKDBINFO CHChainP2PManager::ChainDataPersist()
{
	VEC_T_HYPERBLOCKDBINFO vec;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		T_HYPERBLOCKDBINFO tempHyperBlockDbInfo;
		tempHyperBlockDbInfo.ucBlockType = HYPER_BLOCK;
		tempHyperBlockDbInfo.uiBlockId = itr->tBlockBaseInfo.uiID;
		if (itr->tBlockBaseInfo.uiID == 1)
			tempHyperBlockDbInfo.uiReferHyperBlockId = 0;
		else
			tempHyperBlockDbInfo.uiReferHyperBlockId = itr->tBlockBaseInfo.uiID - 1;
		tempHyperBlockDbInfo.uiBlockTimeStamp = itr->tBlockBaseInfo.uiTime;
		tempHyperBlockDbInfo.strScript = itr->tBlockBaseInfo.strScript;
		tempHyperBlockDbInfo.strAuth = itr->tBlockBaseInfo.strAuth;

		
		memcpy((char*)tempHyperBlockDbInfo.strHashAll, (char*)itr->tHashAll.pID, DEF_SHA256_LEN);
		
		memcpy((char*)tempHyperBlockDbInfo.strHashSelf, (char*)itr->tBlockBaseInfo.tHashSelf.pID, DEF_SHA256_LEN);
		
		memcpy((char*)tempHyperBlockDbInfo.strPreHash, (char*)itr->tBlockBaseInfo.tPreHash.pID, DEF_SHA256_LEN);

		tempHyperBlockDbInfo.strPayload = "";
		vec.push_back(tempHyperBlockDbInfo);

		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
		for (; subItr != itr->listPayLoad.end(); subItr++)
		{
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);

				T_PPRIVATEBLOCK privateBlock = &(localBlock->tPayLoad);
				T_PFILEINFO fileInfo = &((privateBlock->tPayLoad));

				T_HYPERBLOCKDBINFO tempHyperBlockDbInfoLocal;
				tempHyperBlockDbInfoLocal.ucBlockType = LOCAL_BLOCK;
				tempHyperBlockDbInfoLocal.uiBlockId = localBlock->tBlockBaseInfo.uiID;
				tempHyperBlockDbInfoLocal.uiReferHyperBlockId = itr->tBlockBaseInfo.uiID;
				tempHyperBlockDbInfoLocal.uiBlockTimeStamp = localBlock->tBlockBaseInfo.uiTime;
				tempHyperBlockDbInfoLocal.strAuth = localBlock->tBlockBaseInfo.strAuth;
				tempHyperBlockDbInfoLocal.strScript = localBlock->tBlockBaseInfo.strScript;
				
				char strBuf[MAX_BUF_LEN];
				
				memset(tempHyperBlockDbInfoLocal.strPreHash, 0, DEF_SHA256_LEN);
				
				memcpy((char*)tempHyperBlockDbInfoLocal.strHashSelf, (char*)localBlock->tBlockBaseInfo.tHashSelf.pID, DEF_SHA256_LEN);

				memcpy((char*)tempHyperBlockDbInfoLocal.strHyperBlockHash, (char*)localBlock->tHHash.pID, DEF_SHA256_LEN);
				
				tempHyperBlockDbInfoLocal.uiLocalChainId = localBlock->uiAtChainNum;
				string strPayLoad = "";
			
				memset(strBuf, 0, MAX_BUF_LEN);
				CCommonStruct::Hash512ToStr(strBuf, &fileInfo->tFileHash);
				char buf[TEMP_BUF_LEN*2] = { 0 };
				memcpy(buf, strBuf, TEMP_BUF_LEN*2);

				tempHyperBlockDbInfoLocal.strPayload = buf;
				tempHyperBlockDbInfoLocal.strPayload[TEMP_BUF_LEN*2] = '\0';
				vec.push_back(tempHyperBlockDbInfoLocal);
			}
		}
	}
	return vec;
}

uint16 CHChainP2PManager::GetPoeRecordListNum()
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistOnChainReq);
	return g_tP2pManagerStatus.listOnChainReq.size();
}