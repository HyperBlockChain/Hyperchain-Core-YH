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
#include "HChainP2PManager.h"
#include "network/p2pprotocol.h"
#include "headers/UUFile.h"
#include "headers/inter_public.h"
#include "interface/QtInterface.h"
#include "HttpUnit/HttpUnit.h"
#include <QJsonDocument>  
#include <QDebug> 
#include <QJsonObject>
#include "db/HyperchainDB.h"
#include "interface/QtNotify.h"
#include "utility/ElapsedTime.h"
#include "utility/OSFunctions.h"
#include "headers/commonstruct.h"
#include "../db/dbmgr.h"
#include "headers/lambda.h"
#include "headers/convert2.h"
#include "utility/netutil.h"
#include "crypto/sha2.h"

extern UUFile m_uufile;
extern T_CONFFILE	g_confFile;
extern void SHA256(unsigned char* sha256, const char* str, long long length);

extern CMutexObj g_MuxP2pManagerStatus;
extern T_P2PMANAGERSTATUS g_tP2pManagerStatus;

extern std::mutex muLock;
extern bool g_bWriteStatusIsSet;

bool CHChainP2PManager::Start()
{
	if (0 != m_threadUdpProcess.Start(&m_funcUdpProcess))
		return false;

	if (0 != m_threadUpdateDataProcess.Start(&m_funcUpdateDataProcess))
		return false;

	if (0 != m_threadWriteStatusProcess.Start(&m_funcWriteStatusProcess))
		return false;

	if (0 != m_threadMemCacheProsess.Start(&m_funcMemCacheProcess))
		return false;

	return true;
}
void CHChainP2PManager::Stop()
{
	m_threadUdpProcess.Kill();
	m_threadUpdateDataProcess.Kill();
	m_threadWriteStatusProcess.Kill();
	m_threadMemCacheProsess.Kill();
}
void CHChainP2PManager::Join()
{
	m_threadUdpProcess.Join();
	m_threadUpdateDataProcess.Join();
	m_threadWriteStatusProcess.Join();
	m_threadMemCacheProsess.Join();
}
void CHChainP2PManager::Teardown()
{

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
		string ipString = GetIpString(uiPeerIP);

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
			
			T_PEERADDRESS peerAddrOut(inet_addr(pszPeerIP), usPeerPort);
			localBuddyInfo.Set(RECV_REQ, uiRecvBufLen, pRecvBuf, peerAddrOut);
			

			bool index = false;
			CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
			LIST_T_BUDDYINFO::iterator itr = g_tP2pManagerStatus.listRecvLocalBuddyReq.begin();
			for (itr; itr != g_tP2pManagerStatus.listRecvLocalBuddyReq.end(); itr++)
			{
				if ((*itr).GetRequestAddress().GetIP() == localBuddyInfo.GetRequestAddress().GetIP())
				{
					index = true;
					break;
				}
			}

			if (!index)
			{
				g_tP2pManagerStatus.listRecvLocalBuddyReq.push_back(localBuddyInfo);
				g_tP2pManagerStatus.SetRecvRegisReqNum(g_tP2pManagerStatus.GetRecvRegisReqNum() + 1);
				g_tP2pManagerStatus.SetRecvConfirmingRegisReqNum(g_tP2pManagerStatus.GetRecvConfirmingRegisReqNum() + 1);
			}
			break;
		}
		case P2P_PROTOCOL_ON_CHAIN_RSP:
		{
			T_BUDDYINFO localBuddyInfo;
	
			T_PEERADDRESS peerAddrOut(inet_addr(pszPeerIP), usPeerPort);
			localBuddyInfo.Set(RECV_RSP, uiRecvBufLen, pRecvBuf, peerAddrOut);


			bool index = false;
			CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
			LIST_T_BUDDYINFO::iterator itr = g_tP2pManagerStatus.listRecvLocalBuddyRsp.begin();
			for (itr; itr != g_tP2pManagerStatus.listRecvLocalBuddyRsp.end(); itr++)
			{
				if ((*itr).GetRequestAddress().GetIP() == localBuddyInfo.GetRequestAddress().GetIP())
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
			ProcessGetBlockNodeMapReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_GET_BLOCK_STATE_RSP:
			ProcessGetBlockNodeMapRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
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
				if (i < g_tP2pManagerStatus.GetBuddyPeerCount())
				{
					SendGetPeerListReq((*itr)->GetName(), (*itr)->GetPeerInfoByOther().GetIP(), (*itr)->GetPeerInfoByOther().GetPort());
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

	SLEEP(ONE_MIN);

	while (!g_bWriteStatusIsSet)
	{
		SLEEP(ONE_SECOND);
	}


	//open status file
	string strStatuFile = g_confFile.strLogDir;
	strStatuFile += "p2p_status_all.log";

	FILE* fp = fopen(strStatuFile.c_str(), "a");
	if (NULL == fp)
		return;

	while (1)
	{
		SLEEP(5 * ONE_SECOND);
		string strTxt("");

		strTxt = PrintAllLocalBlocks();
		fprintf(fp, "%s\n", strTxt.c_str());
		fflush(fp);

	}
	fclose(fp);
}

int CHChainP2PManager::GetChainUsedMemory()
{
	return OSFunctions::GetUsedMemoryHC();
}

void CHChainP2PManager::ClearHalfMemoryCache()
{
	if (m_HchainBlockList.size() <= 1) return;
	auto halfOuter = m_HchainBlockList.begin();
	std::advance(halfOuter, (m_HchainBlockList.size() + 1) / 2);
	ITR_LIST_T_HYPERBLOCK itrOuter = halfOuter;
	for (; itrOuter != m_HchainBlockList.end(); itrOuter++)
	{
		list<LIST_T_LOCALBLOCK>::iterator itrInner = itrOuter->GetlistPayLoad().begin();
		for (; itrInner != itrOuter->GetlistPayLoad().end(); itrInner++)
		{
			(*itrInner).clear();
		}
	}

	m_HchainBlockList.erase(halfOuter, m_HchainBlockList.end());

	auto itrNew = m_HchainBlockListNew.begin();
	for (; itrNew != m_HchainBlockListNew.end(); itrNew++)
		itrNew->Clear();
	m_HchainBlockListNew.clear();
}


void* CHChainP2PManager::MemCacheProcessEntry(void*pParam)
{
	CHChainP2PManager*pThis = static_cast<CHChainP2PManager*>(pParam);
	if (NULL != pThis)
		pThis->MemCacheProcessImp();

	return NULL;
}

void CHChainP2PManager::MemCacheProcessImp()
{
	m_threadMemCacheProsess.ThreadStarted();
	ElapsedTime et;
	et.Start();

	while (1)
	{
		et.End();
		int days = et.GetDays();
		int freeMems = OSFunctions::GetCurrentFreeMemoryOS();
		int totalMems = OSFunctions::GetTotalMemoryOS();
		int idlePercentage = OSFunctions::GetCurrentCPUIdlePercentageOS();
		int usedMemory = GetChainUsedMemory();

		bool isClean = false;
		float memPercentage = (float)freeMems / (float)totalMems;
		if (memPercentage < 1.0 / 3.0) isClean = true;
		if (usedMemory>1.3 * BUFFERLEN) isClean = true;
		if (days >= 1 && !isClean && usedMemory>1024) isClean = true;
		if (isClean && idlePercentage < 10.0)
		{
			CAutoMutexLock muxAuto(m_MuxHchainBlockList);
			ClearHalfMemoryCache();

			et.Start();
			isClean = false;
		}

		SLEEP(60 * 1000 * 60 * 3);
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
		if ((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) > LOCALBUDDYTIME)
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
			g_tP2pManagerStatus.tLocalBuddyAddr = localInfo.GetRequestAddress();

			string ipString = localInfo.GetRequestAddress().GetIPString();

			ProcessOnChainReqMsg((char*)ipString.c_str(), localInfo.GetRequestAddress().GetPort(), const_cast<char*>(localInfo.GetBuffer()), localInfo.GetBufferLength());
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
		if ((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) > LOCALBUDDYTIME)
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
			g_tP2pManagerStatus.tLocalBuddyAddr = localInfo.GetRequestAddress();

			string ipString = localInfo.GetRequestAddress().GetIPString();

			log_info(g_pLogHelper, "333CHChainP2PManager::RecvLocalBuddyPackRspThreadEntryImp GET a local buddy RSP from pool");

			ProcessOnChainRspMsg((char*)ipString.data(), localInfo.GetRequestAddress().GetPort(), const_cast<char*>(localInfo.GetBuffer()), localInfo.GetBufferLength());
		}
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
		if ((*itrList).GetPeer().GetPeerAddr().GetIP() == m_MyPeerInfo.GetPeerInfoByMyself().GetIP()
			&& (*itrList).GetPeer().GetPeerAddr().GetPort() == m_MyPeerInfo.GetPeerInfoByMyself().GetPort()
		{
			localInfo = (*itrList);
			char localHash[FILESIZES] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

			char logBuf[BUFFERLEN] = { 0 };
			char hashBuf[BUFFERLEN] = { 0 };
			memset(hashBuf, 0, BUFFERLEN);
			CCommonStruct::Hash512ToStr(hashBuf, &localInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileHash());
			sprintf(logBuf, "[%s][%s][%s], on chain, now trying register to Blockchain, tried [%d] times",
				localInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName(),
				localInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileOwner(),
				hashBuf,
				localInfo.GetRetryTime());
			m_qtnotify->SetStatusMes(logBuf);

			localInfo.uiRetryTime += 1;
			g_tP2pManagerStatus.listLocalBuddyChainInfo.clear();
			g_tP2pManagerStatus.listOnChainReq.push_front(localInfo);
			break;
		}
	}
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
				if (timeNow - (*itr).second.uiTime > 60 * 10)
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

void CHChainP2PManager::LocalBuddyThreadEntryImp()
{
	bool indexGlobal = true;
	bool bIndexCreate = true;
	bool bFirstTimes = true;
	bool bCreatBlock = false;
	while (1)
	{
		if ((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) <= LOCALBUDDYTIME)
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


			if (!g_tP2pManagerStatus.HaveOnChainReq())
			{
				GetOnChainInfo();
				m_qtnotify->SetBuddyStartTime(0, 0);
			}

			uint64 tempNum2 = 0;
			{
				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
				tempNum2 = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
			if (tempNum2 == 0)
			{
				SLEEP(2 * ONE_SECOND);
				continue;
			}


			char logBuf[BUFFERLEN] = { 0 };
			m_qtnotify->SetStatusMes("broadcast Local Consensus Request");

			m_qtnotify->SetNodeStatus(LOCAL_BUDDY);
			g_tP2pManagerStatus.tBuddyInfo.eBuddyState = LOCAL_BUDDY;
			SendLocalBuddyReq();

			SLEEP(2 * ONE_SECOND);
		}
		else if (((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) > LOCALBUDDYTIME) && ((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) <= GLOBALBUDDYTIME))
		{
			uint64 tempNum = 0;
			{
				CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
				tempNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
			if (tempNum == 0 || tempNum == 1)
			{
				SLEEP(2 * ONE_SECOND);
				continue;
			}

			if (!indexGlobal)
			{
				indexGlobal = true;
				m_qtnotify->SetNodeStatus(GLOBAL_BUDDY);
				g_tP2pManagerStatus.tBuddyInfo.eBuddyState = GLOBAL_BUDDY;
				StartGlobalBuddy();
			}
			else
			{
				SendGlobalBuddyReq();

				char logBuf[BUFFERLEN] = { 0 };
				m_qtnotify->SetStatusMes("broadcast Global Consensus Request");
			}
			SLEEP(2 * ONE_SECOND);
			continue;
		}
		else if (((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) > GLOBALBUDDYTIME) && ((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) <= NEXTBUDDYTIME))
		{
			g_tP2pManagerStatus.SetStartGlobalFlag(false);
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
				if (g_tP2pManagerStatus.HaveOnChainReq())
				{
					CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
					if (0 != g_tP2pManagerStatus.tOnChainHashInfo.strHash.compare(""))
					{
						ReOnChainFun();
					}

					CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
					g_tP2pManagerStatus.listGlobalBuddyChainInfo.clear();
					if (g_tP2pManagerStatus.HaveOnChainReq())
					{
						g_tP2pManagerStatus.SetStartTimeOfConsensus(g_tP2pManagerStatus.GetStartTimeOfConsensus() + (NEXTBUDDYTIME));
					}
					g_tP2pManagerStatus.SetHaveOnChainReq(false);
					g_tP2pManagerStatus.SetLocalBuddyChainState(false);

					log_info(g_pLogHelper, "CHChainP2PManager::LocalBuddyThreadEntryImp, time>(2*60)......,not have new hyper block ,update time");

				}
				else
				{
					{
						if (g_tP2pManagerStatus.GetStartTimeOfConsensus() == g_tP2pManagerStatus.GetNextStartTimeNewest())
						{

							g_tP2pManagerStatus.SetNextStartTimeNewest(g_tP2pManagerStatus.GetNextStartTimeNewest() + (NEXTBUDDYTIME));
							g_tP2pManagerStatus.SetStartTimeOfConsensus(g_tP2pManagerStatus.GetNextStartTimeNewest());
						}
						else
						{

							if ((g_tP2pManagerStatus.GetNextStartTimeNewest() - g_tP2pManagerStatus.GetStartTimeOfConsensus()) > NEXTBUDDYTIME)
							{
								g_tP2pManagerStatus.SetStartTimeOfConsensus(g_tP2pManagerStatus.GetStartTimeOfConsensus() + NEXTBUDDYTIME);
								g_tP2pManagerStatus.SetNextStartTimeNewest(g_tP2pManagerStatus.GetStartTimeOfConsensus());
							}
							else
							{
								g_tP2pManagerStatus.SetStartTimeOfConsensus(g_tP2pManagerStatus.GetNextStartTimeNewest());
							}

						}
						m_qtnotify->SetBuddyStop();
					}


				}
			}
			SLEEP(2 * ONE_SECOND);
		}
	}
}

void CHChainP2PManager::SendGetPeerListReq(int8 *strName, uint32 uiIP, uint16 usPort)
{
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);

	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		return;
	}

	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);

	T_P2PPROTOCOLPEERLISTREQ pP2pProtocolPeerlistReq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_PEERLIST_REQ, timeTemp.tv_sec), T_PEERADDRESS(g_confFile.GetLocalIP(), g_confFile.GetLocalPort()),
		g_tP2pManagerStatus.GetMaxBlockNum(), const_cast<char*>(g_confFile.GetLocalNodeName().c_str()), g_tP2pManagerStatus.GetNodeState());

	string ipString = GetIpString(uiIP);

	m_UdpSocket.Send(uiIP, usPort, (char*)&pP2pProtocolPeerlistReq, sizeof(pP2pProtocolPeerlistReq));
}

void CHChainP2PManager::SendLocalBuddyReq()
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	T_PP2PPROTOCOLONCHAINREQ pP2pProtocolOnChainReq = NULL;
	uint8 blockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
	int ipP2pProtocolOnChainReqLen = sizeof(T_P2PPROTOCOLONCHAINREQ) + blockNum*sizeof(T_LOCALCONSENSUS);
	pP2pProtocolOnChainReq = (T_PP2PPROTOCOLONCHAINREQ)malloc(ipP2pProtocolOnChainReqLen);
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolOnChainReq->SetP2pprotocolonchainreq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_ON_CHAIN_REQ, timeTemp.tv_sec), g_tP2pManagerStatus.GetMaxBlockNum() + 1, blockNum);

	T_PLOCALCONSENSUS pPeerInfos = NULL;
	pPeerInfos = (T_PLOCALCONSENSUS)(pP2pProtocolOnChainReq + 1);

	uint8 i = 0;
	ITR_LIST_T_LOCALCONSENSUS itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
	for (; itr != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itr++)
	{
		pPeerInfos[i].SetLoaclConsensus((*itr).GetPeer(), (*itr).GetLocalBlock());
		i++;

		char localHash[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &(*itr).tLocalBlock.GetBlockBaseInfo().GetHashSelf());
	}

	SendDataToPeer((char*)pP2pProtocolOnChainReq, ipP2pProtocolOnChainReqLen);

	free(pP2pProtocolOnChainReq);
	pP2pProtocolOnChainReq = NULL;
}

string CHChainP2PManager::PrintAllPeerNodeList()
{
	//return "";
	string retData = "";
	char buf[FILESIZEL] = { 0 };
	retData += "==================PEERLISTINFO==================\n";
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);

	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		retData += "NAME=";
		retData += (*itr)->GetName();

		retData += "  IPIN=";
		struct in_addr addPeerIP;
		addPeerIP.s_addr = (*itr)->GetPeerInfoByMyself().GetIP();
		retData += inet_ntoa(addPeerIP);

		retData += "  PORTIN=";
		memset(buf, 0, FILESIZEL);
		sprintf(buf, "%d", (*itr)->GetPeerInfoByMyself().GetPort());
		retData += buf;

		retData += "  IPOUT=";
		addPeerIP.s_addr = (*itr)->GetPeerInfoByOther().GetIP();
		retData += inet_ntoa(addPeerIP);

		retData += "  PORTOUT=";
		memset(buf, 0, FILESIZEL);
		sprintf(buf, "%d", (*itr)->GetPeerInfoByOther().GetIP());
		retData += buf;

		retData += "  NETSTATE=";
		if ((*itr)->GetState() == OUTNET)
		{
			retData += "out_net";
		}
		else if ((*itr)->GetState() == INNERNET)
		{
			retData += "inner_net|";
			if ((*itr)->GetNatTraversalState() == GOOD)
			{
				retData += "good";
			}
			else if ((*itr)->GetNatTraversalState() == BAD)
			{
				retData += "bad";
			}
			else
			{
				retData += "default";
			}
		}
		else if ((*itr)->GetState() == DEFAULT_NET)
		{
			retData += "default_net";
		}
		retData += "\n";
	}

	retData += "================================================\n";
	return retData;
}

string CHChainP2PManager::PrintBlockNodeMap()
{
	string retData = "";
	char buf[FILESIZEL] = { 0 };
	retData += "==================BLOCKSTATEINFO==================\n";

	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	ITR_MAP_BLOCK_STATE itr = m_BlockStateMap.begin();
	for (; itr != m_BlockStateMap.end(); itr++)
	{
		retData += "BlockNum=";
		memset(buf, 0, FILESIZEL);
		sprintf(buf, "%d", (*itr).first);
		retData += buf;

		retData += "  NodeNum=";
		memset(buf, 0, FILESIZEL);
		sprintf(buf, "%d", (*itr).second.size());
		retData += buf;
		retData += "\n";

		ITR_LIST_T_PBLOCKSTATEADDR itrSub = (*itr).second.begin();
		for (; itrSub != (*itr).second.end(); itrSub++)
		{
			retData += "          IPIN=";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itrSub)->GetPeerAddr().GetIP();
			retData += inet_ntoa(addPeerIP);


			retData += "  IPOUT=";
			addPeerIP.s_addr = (*itrSub)->GetPeerAddrOut().GetIP();
			memset(buf, 0, FILESIZEL);
			strcpy(buf, inet_ntoa(addPeerIP));
			retData += buf;
			retData += "\n";
		}
	}

	retData += "================================================\n";
	return retData;
}

void CHChainP2PManager::SavePeerList()
{
	string strBuf = "<?xml version=\"1.0\" encoding=\"utf - 8\"?>\n";
	strBuf += "<peerinfo>\n";

	int i = 0;

	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (i < g_tP2pManagerStatus.GetBuddyPeerCount())
		{
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itr)->GetPeerInfoByMyself().GetIP();
			char tempBuf[MAX_NUM_LEN];
			memset(tempBuf, 0, MAX_NUM_LEN);

			strBuf += "   <nodeinfo>\n";
			strBuf += "      <serverip>";
			strBuf += inet_ntoa(addPeerIP);
			strBuf += "</serverip>\n";

			strBuf += "      <serverport>";
			sprintf(tempBuf, "%d", (*itr)->GetPeerInfoByMyself().GetPort());
			strBuf += tempBuf;
			strBuf += "</serverport>\n";

			addPeerIP.s_addr = (*itr)->GetPeerInfoByOther().GetIP();
			strBuf += "      <outserverip>";
			strBuf += inet_ntoa(addPeerIP);
			strBuf += "</outserverip>\n";

			strBuf += "      <outserverport>";
			memset(tempBuf, 0, MAX_NUM_LEN);
			sprintf(tempBuf, "%d", (*itr)->GetPeerInfoByOther().GetPort());
			strBuf += tempBuf;
			strBuf += "</outserverport>\n";

			strBuf += "      <nodestate>";
			memset(tempBuf, 0, MAX_NUM_LEN);
			sprintf(tempBuf, "%d", (*itr)->GetState());
			strBuf += tempBuf;
			strBuf += "</nodestate>\n";

			strBuf += "      <nodename>";
			strBuf += (*itr)->GetName();
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


string CHChainP2PManager::PrintAllLocalBlocks()
{
	string retData = "";
	char buf[BUFFERLEN] = { 0 };
	retData += "==================BLOCKINFO==================\n";

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		retData += "HyperBlockNo=";
		memset(buf, 0, BUFFERLEN);
		sprintf(buf, "%d\n", itr->GetBlockBaseInfo().GetID());
		retData += buf;

		retData += "ChainNum=";
		memset(buf, 0, BUFFERLEN);
		sprintf(buf, "%d\n", itr->GetlistPayLoad().size());
		retData += buf;

		retData += "HyperBlockHash=";
		memset(buf, 0, BUFFERLEN);
		CCommonStruct::Hash256ToStr(buf, &itr->GetBlockBaseInfo().GetHashSelf());
		retData += buf;
		retData += "\n";

		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->GetlistPayLoad().begin();
		for (; subItr != itr->GetlistPayLoad().end(); subItr++)
		{
			uint16 chainNum = 0;
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				chainNum += 1;
				T_PLOCALBLOCK localBlock = &(*ssubItr);


				T_PPRIVATEBLOCK privateBlock = &(localBlock->GetPayLoad());
				T_PFILEINFO fileInfo = &(privateBlock->GetPayLoad());

				retData += "chainNo=";
				memset(buf, 0, BUFFERLEN);
				sprintf(buf, "%d   ", chainNum);
				retData += buf;

				retData += "localBlockHash=";
				memset(buf, 0, BUFFERLEN);
				CCommonStruct::Hash256ToStr(buf, &ssubItr->GetBlockBaseInfo().GetHashSelf());
				retData += buf;
				retData += "   ";

				retData += "PreBlockHash=";
				memset(buf, 0, BUFFERLEN);
				CCommonStruct::Hash256ToStr(buf, &ssubItr->GetBlockBaseInfo().GetPreHash());
				retData += buf;
				retData += "   ";

				retData += "fileHash=";
				memset(buf, 0, BUFFERLEN);
				CCommonStruct::Hash512ToStr(buf, &fileInfo->GetFileHash());
				retData += buf;
				retData += "\n";

			}
			retData += "\n";
		}

	}

	retData += "================================================\n";
	return retData;
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

void CHChainP2PManager::SendNodeState(uint8 nodeState)
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);

	char strSendState[MAX_BUF_LEN] = "";

	char pszPeerIP[MAX_BUF_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = m_MyPeerInfo.GetPeerInfoByOther().GetIP();
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));

	QJsonObject json;
	json.insert("ip", pszPeerIP);
	json.insert("port", PORT);
	json.insert("state", nodeState);
	json.insert("reqNum", 0);
	json.insert("nextHyperBlockTime", QJsonValue((qint64)g_tP2pManagerStatus.uiStartTimeOfConsensus + 3 * 60));
	json.insert("hyperHeight", QJsonValue((qint64)g_tP2pManagerStatus.tPreHyperBlock.tBlockBaseInfo.uiID + 1));
	json.insert("localBlockNum", QJsonValue((qint64)g_tP2pManagerStatus.listLocalBuddyChainInfo.size()));

	T_SHA256 tHashSelf;
	uint64 hashLen = g_tP2pManagerStatus.listLocalBuddyChainInfo.size()*sizeof(T_LOCALCONSENSUS);
	char localHash[FILESIZES] = { 0 };
	CCommonStruct::Hash256ToStr(localHash, &tHashSelf);

	json.insert("localChainHash", QJsonValue(localHash));
	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
	json.insert("chainNum", QJsonValue((qint64)g_tP2pManagerStatus.listGlobalBuddyChainInfo.size()));

	QJsonDocument document;
	document.setObject(json);
	QByteArray byteArray = document.toJson(QJsonDocument::Compact);
	QString strJson(byteArray);

	string innerServer = TEST_SERVER1;
	innerServer += strJson.toStdString();

	string outServer = TEST_SERVER2;
	outServer += strJson.toStdString();

	char *strBody = NULL;
	unsigned int uiRecvLen = 0;
	int ret = HttpDownloadT(innerServer, &strBody, uiRecvLen);
	if (200 == ret)
	{

	}

	char *strBodyOut = NULL;
	unsigned int uiRecvLenOut = 0;
	ret = HttpDownloadT(outServer, &strBodyOut, uiRecvLenOut);
	if (200 == ret)
	{

	}

}
