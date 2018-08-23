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


void CHChainP2PManager::ProcessOnChainConfirmFinMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINCONFIRM pP2pProtocolOnChainConfirmRecv = (T_PP2PPROTOCOLONCHAINCONFIRM)(pBuf);

	log_info(g_pLogHelper, "CHChainP2PManager::ProcessOnChainConfirmFinMsg confirmFin hash,%s", pP2pProtocolOnChainConfirmRecv->GetHash());

	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
	ITR_LIST_T_BUDDYINFOSTATE itrRsp = g_tP2pManagerStatus.listCurBuddyRsp.begin();
	for (itrRsp; itrRsp != g_tP2pManagerStatus.listCurBuddyRsp.end();)
	{
		if (0 == strncmp((*itrRsp).strBuddyHash, pP2pProtocolOnChainConfirmRecv->GetHash(), DEF_STR_HASH256_LEN))
		{
			log_info(g_pLogHelper, "CHChainP2PManager::ProcessOnChainConfirmFinMsg recv confirmFin[%s], delete [%s] from listCurBuddyRsp111111", pszIP, (*itrRsp).strBuddyHash);
			itrRsp = g_tP2pManagerStatus.listCurBuddyRsp.erase(itrRsp);
		}
		else
		{
			string ipString = (*itrRsp).GetPeerAddrOut().GetIPString();
			SendRefuseReq((char*)ipString.c_str(), (*itrRsp).GetPeerAddrOut().GetPort(), (*itrRsp).strBuddyHash, RECV_REQ);
			log_info(g_pLogHelper, "CHChainP2PManager::ProcessOnChainConfirmFinMsg recv confirmFin[%s], delete [%s] from listCurBuddyRsp222222222", pszIP, (*itrRsp).strBuddyHash);
			itrRsp = g_tP2pManagerStatus.listCurBuddyRsp.erase(itrRsp);
		}
	}

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyReq);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyReq.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyReq.end();)
	{


		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainConfirmRecv->GetHash(), DEF_STR_HASH256_LEN))
		{
			log_info(g_pLogHelper, "CHChainP2PManager::ProcessOnChainConfirmFinMsg recv confirmFin from ip[%s], interrupt timer[%s]", pszIP, (*itr).strBuddyHash);


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

					char localHash[FILESIZES] = { 0 };
					CCommonStruct::Hash256ToStr(localHash, &(*itrSub).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

					log_info(g_pLogHelper, "CHChainP2PManager::ProcessOnChainConfirmFinMsg recv confirmFin from ip[%s], the localblock[%s][%s] not in my localBoddyList, insert", pszIP, localHash, (*itrSub).tLocalBlock.tPayLoad.tPayLoad.fileName);
					g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back((*itrSub));
					g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
					ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
					m_qtnotify->SetLocalBuddyChainInfo(g_tP2pManagerStatus.listLocalBuddyChainInfo);
					g_tP2pManagerStatus.tBuddyInfo.usBlockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();

					if ((*itrSub).GetPeer().GetPeerAddr().GetIP() != m_MyPeerInfo.GetPeerInfoByMyself().GetIP())
					{
						g_tP2pManagerStatus.SetRecvPoeNum(g_tP2pManagerStatus.GetRecvPoeNum()+1);
						m_qtnotify->SetReceivePoeNum(g_tP2pManagerStatus.GetRecvPoeNum());
					}
					SendCopyLocalBlock((*itrSub));

					g_tP2pManagerStatus.SetRecvRegisReqNum(g_tP2pManagerStatus.GetRecvRegisReqNum()+1);
					g_tP2pManagerStatus.SetRecvConfirmingRegisReqNum( g_tP2pManagerStatus.GetRecvConfirmingRegisReqNum()+ 1);
				}

			}
			log_info(g_pLogHelper, "CHChainP2PManager::ProcessOnChainConfirmFinMsg delete[%s] from listCurBuddyReq111111", (*itr).strBuddyHash);
			itr = g_tP2pManagerStatus.listCurBuddyReq.erase(itr);
		}
		else
		{
			string ipString = (*itr).GetPeerAddrOut().GetIPString();

			SendRefuseReq((char*)ipString.c_str(), (*itr).GetPeerAddrOut().GetPort(), (*itr).strBuddyHash, RECV_RSP);

			log_info(g_pLogHelper, "CHChainP2PManager::ProcessOnChainConfirmFinMsg delete[%s] from listCurBuddyReq12222222", (*itr).strBuddyHash);
			itr = g_tP2pManagerStatus.listCurBuddyReq.erase(itr);

		}

	}

	CAutoMutexLock muxAuto3(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
	g_tP2pManagerStatus.listRecvLocalBuddyRsp.clear();

	CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
	g_tP2pManagerStatus.listRecvLocalBuddyReq.clear();

}

void CHChainP2PManager::ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLPINGREQ pP2pProtocolPingReq = (T_PP2PPROTOCOLPINGREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingReqMsg recv login request from IP=%s PORT=%d", pszIP, usPort);

	T_P2PPROTOCOLPINGRSP pP2pPingRsp;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);

	pP2pPingRsp.SetResult(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_PING_RSP, timeTemp.tv_sec), P2P_PROTOCOL_SUCCESS));
	pP2pPingRsp.SetMaxBlockNum(g_tP2pManagerStatus.GetMaxBlockNum());
	pP2pPingRsp.SetPeeroutAddr(_tpeeraddress(inet_addr(pszIP), usPort));

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)&pP2pPingRsp, sizeof(pP2pPingRsp));
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingReqMsg send login rsp to IP=%s PORT=%d", pszIP, usPort);

	SearchPeerList(pP2pProtocolPingReq->GetName(), pP2pProtocolPingReq->tPeerAddr.GetIP(), pP2pProtocolPingReq->GetPeerAddr().GetPort(),
		inet_addr(pszIP), usPort, true, pP2pProtocolPingReq->GetType().GetTimeStamp(), pP2pProtocolPingReq->GetnodeState());

	SLEEP(1 * ONE_SECOND);
	SendPeerList(inet_addr(pszIP), usPort);

	if (pP2pProtocolPingReq->GetMaxBlockNum() < g_tP2pManagerStatus.GetMaxBlockNum())
	{
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingReqMsg loginNodeMaxBlockNum < MyMaxBlockNum, sync hyperBlock...");

		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPingReq->GetMaxBlockNum());
	}

	CCommonStruct::gettimeofday_update(&tvEnd);
	tvSpac.tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
}

void CHChainP2PManager::ProcessPingRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLPINGRSP pP2pProtocolPingRsp = (T_PP2PPROTOCOLPINGRSP)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingRspMsg recv login rsp from IP=%s PORT=%d", pszIP, usPort);

	m_MyPeerInfo.SetPeerInfoByOther(_tpeeraddress(pP2pProtocolPingRsp->GetPeeroutAddr().GetIP(), pP2pProtocolPingRsp->GetPeeroutAddr().GetPort()));

	string ipString = pP2pProtocolPingRsp->GetPeeroutAddr().GetIPString();

	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingRspMsg update my OUTIP=%s OUTPORT=%d", (char*)ipString.c_str(), pP2pProtocolPingRsp->GetPeeroutAddr().GetPort());

	if (pP2pProtocolPingRsp->GetMaxBlockNum() > g_tP2pManagerStatus.GetMaxBlockNum())
	{
		g_tP2pManagerStatus.SetMaxBlockNum(pP2pProtocolPingRsp->GetMaxBlockNum());
		m_qtnotify->SetHyperBlock("", 0, g_tP2pManagerStatus.GetMaxBlockNum());
	}
}

void CHChainP2PManager::ProcessWantNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLWANTNATTRAVERSALREQ pP2pProtocolWantNatTraversalReq = (T_PP2PPROTOCOLWANTNATTRAVERSALREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessWantNatTraversalReqMsg recv wantNatTraversalReq from IP=%s PORT=%d", pszIP, usPort);

	T_P2PPROTOCOLSOMEONEWANTCALLYOUREQ tWantNatTraversalReq;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);

	tWantNatTraversalReq.SetType(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_SOME_ONE_NAT_TRAVERSAL_TO_YOU_REQ, timeTemp.tv_sec));
	tWantNatTraversalReq.SetPeerWantNatTraversalAddr(pP2pProtocolWantNatTraversalReq->GetPeerOutAddr());

	string ipString = pP2pProtocolWantNatTraversalReq->GetPeerBeNatTraversaldAddr().GetIPString();

	log_info(g_pLogHelper, "CHChainP2PManager::ProcessWantNatTraversalReqMsg send someOneWantCallYouReq to IP=%s PORT=%d", (char*)ipString.data(), 
		pP2pProtocolWantNatTraversalReq->GetPeerBeNatTraversaldAddr().GetPort());
	m_UdpSocket.Send(pP2pProtocolWantNatTraversalReq->GetPeerBeNatTraversaldAddr().GetIP(), pP2pProtocolWantNatTraversalReq->GetPeerBeNatTraversaldAddr().GetPort(),
		(char*)&tWantNatTraversalReq, sizeof(tWantNatTraversalReq));
}

void CHChainP2PManager::ProcessSomeNodeWantToConnectYouReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{

	T_PP2PPROTOCOLSOMEONEWANTCALLYOUREQ pP2pProtocolSomeNodeWantConnectYouReq = (T_PP2PPROTOCOLSOMEONEWANTCALLYOUREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessSomeNodeWantToConnectYouReqMsg recv someNodeWantConnectYouReq from IP=%s PORT=%d", pszIP, usPort);

	T_P2PPROTOCOLNATTRAVERSALASKREQ pP2pProtocolNatTraversalAskReq;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);

	pP2pProtocolNatTraversalAskReq.SetType(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_NAT_TRAVERSAL_REQ, timeTemp.tv_sec));
	pP2pProtocolNatTraversalAskReq.SetPeerAddr(_tpeeraddress(g_confFile.GetLocalIP(), g_confFile.GetLocalPort()));

	string ipString = pP2pProtocolSomeNodeWantConnectYouReq->GetPeerWantNatTraversalAddr().GetIPString();

	m_UdpSocket.Send(pP2pProtocolSomeNodeWantConnectYouReq->GetPeerWantNatTraversalAddr().GetIP(), pP2pProtocolSomeNodeWantConnectYouReq->GetPeerWantNatTraversalAddr().GetPort(), 
		(char*)&pP2pProtocolNatTraversalAskReq, sizeof(pP2pProtocolNatTraversalAskReq));
}

void CHChainP2PManager::ProcessNatTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLNATTRAVERSALASKREQ pP2pProtocolNatTraversalAskReq = (T_PP2PPROTOCOLNATTRAVERSALASKREQ)(pBuf);

	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if ((pP2pProtocolNatTraversalAskReq->GetPeerAddr().GetIP() == (*itr)->GetPeerInfoByMyself().GetIP())
			&& (pP2pProtocolNatTraversalAskReq->GetPeerAddr().GetPort() == (*itr)->GetPeerInfoByMyself().GetPort())
			&& (inet_addr(pszIP) == (*itr)->tPeerInfoByOther.GetIP())
			&& (usPort == (*itr)->GetPeerInfoByOther().GetPort()))
		{
			(*itr)->SetTime(time(NULL));
			(*itr)->SetNatTraversalState(GOOD);

			log_info(g_pLogHelper, "CHChainP2PManager::ProcessNatTraversalReqMsg NatTraversal success");
			break;
		}
	}

}

void RecvBlockFun(void *param)
{
	CHChainP2PManager *pManager = (CHChainP2PManager *)param;
	int randNum = rand() % RANDTIME;
	SLEEP(randNum * ONE_SECOND);

	g_tP2pManagerStatus.SetRecvConfirmingRegisReqNum(g_tP2pManagerStatus.GetRecvConfirmingRegisReqNum() - 1);
}

void CHChainP2PManager::ProcessAddBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLADDBLOCKREQ pP2pProtocolAddBlockReqRecv = (T_PP2PPROTOCOLADDBLOCKREQ)(pBuf);

	T_HYPERBLOCK blockInfos;
	T_PHYPERBLOCKSEND pHyperBlockInfosTemp;
	pHyperBlockInfosTemp = (T_PHYPERBLOCKSEND)(pP2pProtocolAddBlockReqRecv + 1);

	blockInfos.SetBlockBaseInfo(pHyperBlockInfosTemp->GetBlockBaseInfo());
	blockInfos.SetHashAll(pHyperBlockInfosTemp->GetHashAll());

	uint16 uiRetNum = 0;
	uiRetNum = HyperBlockInListOrNot(pP2pProtocolAddBlockReqRecv->GetBlockNum(), pP2pProtocolAddBlockReqRecv->GetBlockCount(), blockInfos.GetBlockBaseInfo().GetHashSelf());
	if (uiRetNum == ERROR_EXIST)
	{
		return;
	}

	T_PLOCALBLOCK pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALBLOCK)(pHyperBlockInfosTemp + 1);

	int chainNumTemp = 1;
	LIST_T_LOCALBLOCK listLocakBlockTemp;
	for (int i = 0; i<pP2pProtocolAddBlockReqRecv->GetBlockCount(); i++)
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
			chainNumTemp = pLocalTemp.GetAtChainNum();
			listLocakBlockTemp.clear();
			listLocakBlockTemp.push_back(pLocalTemp);
		}

		if (i == pP2pProtocolAddBlockReqRecv->GetBlockCount() - 1)
		{
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
		}
	}

	m_HchainBlockList.push_back(blockInfos);
	m_HchainBlockListNew.push_back(T_HYPERBLOCKNEW(blockInfos));

	string ipString = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();

	{
		SaveHyperBlockToLocal(blockInfos);
		SaveLocalBlockToLocal(blockInfos);
	}

	WriteBlockLog(blockInfos);


	log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg insert hyperBlock[%d] to my blocklist", pP2pProtocolAddBlockReqRecv->GetBlockNum());

	if (g_tP2pManagerStatus.GetMaxBlockNum() < pHyperBlockInfosTemp->GetBlockBaseInfo().GetID())
	{
		g_tP2pManagerStatus.SetMaxBlockNum(pHyperBlockInfosTemp->GetBlockBaseInfo().GetID());
		m_qtnotify->SetHyperBlock("", 0, g_tP2pManagerStatus.GetMaxBlockNum());
		g_tP2pManagerStatus.SetPreHyperBlock(blockInfos);
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg update the newest hyperBlock[%d]", g_tP2pManagerStatus.GetMaxBlockNum());
	}

	if (uiRetNum == 1)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg update g_tP2pManagerStatus.tPreHyperBlock");
		g_tP2pManagerStatus.SetPreHyperBlock(blockInfos);
	}

	T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;

	pBlockStateAddr->SetPeerAddr(m_MyPeerInfo.GetPeerInfoByMyself());
	pBlockStateAddr->SetPeerAddrOut(m_MyPeerInfo.GetPeerInfoByOther());

	LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
	listPblockStateAddr.push_back(pBlockStateAddr);

	m_BlockStateMap[pP2pProtocolAddBlockReqRecv->GetBlockNum()] = listPblockStateAddr;

	string ipStringIn = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();
	string ipStringOut = m_MyPeerInfo.GetPeerInfoByOther().GetIPString();

	
	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		return;
	}


	if (pP2pProtocolAddBlockReqRecv->GetSendTime() == 0)
		return;

	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolAddBlockReqRecv->tType.uiTimeStamp = timeTemp.tv_sec;

	pP2pProtocolAddBlockReqRecv->SetSendTime(pP2pProtocolAddBlockReqRecv->GetSendTime() - 1);

	SendDataToPeer(pBuf, uiBufLen);
}

void CHChainP2PManager::ProcessCopyBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLCOPYBLOCKREQ pP2pProtocolCopyBlockReqRecv = (T_PP2PPROTOCOLCOPYBLOCKREQ)(pBuf);

	T_PLOCALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALCONSENSUS)(pP2pProtocolCopyBlockReqRecv + 1);

	char localHash[FILESIZES] = { 0 };
	CCommonStruct::Hash256ToStr(localHash, &pLocalBlockTemp->GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

	bool index;

	index = false;
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	ITR_LIST_T_LOCALCONSENSUS itrList = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
	for (; itrList != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itrList++)
	{
		if (((*itrList).GetLocalBlock().GetBlockBaseInfo().GetHashSelf() == pLocalBlockTemp->GetLocalBlock().GetBlockBaseInfo().GetHashSelf()))
		{
			index = true;
				pszIP, usPort, (*itrList).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName());
			break;
		}
	}

	if (index)
		return;

		pszIP, usPort, pLocalBlockTemp->GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName());
	g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back(*pLocalBlockTemp);
	g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
	ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
	m_qtnotify->SetLocalBuddyChainInfo(g_tP2pManagerStatus.listLocalBuddyChainInfo);
	g_tP2pManagerStatus.tBuddyInfo.usBlockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();

	g_tP2pManagerStatus.SetRecvRegisReqNum( g_tP2pManagerStatus.GetRecvRegisReqNum()+1);
	g_tP2pManagerStatus.SetRecvConfirmingRegisReqNum(g_tP2pManagerStatus.GetRecvConfirmingRegisReqNum()+1);
}

void CHChainP2PManager::ProcessCopyHyperBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLCOPYHYPERBLOCKREQ pP2pProtocolCopyHyperBlockReqRecv = (T_PP2PPROTOCOLCOPYHYPERBLOCKREQ)(pBuf);


	T_HYPERBLOCK blockInfos;
	T_PHYPERBLOCKSEND pHyperBlockInfosTemp;
	pHyperBlockInfosTemp = (T_PHYPERBLOCKSEND)(pP2pProtocolCopyHyperBlockReqRecv + 1);
	blockInfos.tBlockBaseInfo = pHyperBlockInfosTemp->GetBlockBaseInfo();
	blockInfos.tHashAll = pHyperBlockInfosTemp->GetHashAll();


	uint16 uiRetNum = 0;
	uiRetNum = HyperBlockInListOrNot(pP2pProtocolCopyHyperBlockReqRecv->GetBlockNum(), 
		pP2pProtocolCopyHyperBlockReqRecv->GetBlockCount(), blockInfos.GetBlockBaseInfo().GetHashSelf());
	if (uiRetNum == ERROR_EXIST)
	{
		return;
	}

	T_PLOCALBLOCK pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALBLOCK)(pHyperBlockInfosTemp + 1);

	uint64 chainNumTemp = 1;
	LIST_T_LOCALBLOCK listLocakBlockTemp;
	for (int i = 0; i < pP2pProtocolCopyHyperBlockReqRecv->GetBlockCount(); i++)
	{
		T_LOCALBLOCK pLocalTemp;
		pLocalTemp = *(pLocalBlockTemp + i);

		if (pLocalTemp.GetAtChainNum() == chainNumTemp)
		{
			listLocakBlockTemp.push_back(pLocalTemp);
		}
		else
		{
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
			chainNumTemp = pLocalTemp.GetAtChainNum();
			listLocakBlockTemp.clear();
			listLocakBlockTemp.push_back(pLocalTemp);
		}

		if (i == pP2pProtocolCopyHyperBlockReqRecv->GetBlockCount() - 1)
		{
			listLocakBlockTemp.sort(CmpareOnChainLocal());
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
		}
	}

	m_HchainBlockList.push_back(blockInfos);
	m_HchainBlockListNew.push_back(T_HYPERBLOCKNEW(blockInfos));


	string ipString = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();
	{
		SaveHyperBlockToLocal(blockInfos);
		SaveLocalBlockToLocal(blockInfos);
	}

	WriteBlockLog(blockInfos);

	g_tP2pManagerStatus.SetHaveRecvCopyHyperBlock(true);


	if (g_tP2pManagerStatus.GetMaxBlockNum() < pHyperBlockInfosTemp->GetBlockBaseInfo().GetID())
	{
		g_tP2pManagerStatus.SetMaxBlockNum(pHyperBlockInfosTemp->GetBlockBaseInfo().GetID());
		g_tP2pManagerStatus.SetPreHyperBlock(blockInfos);
	}

	if (uiRetNum == 1)
	{
		g_tP2pManagerStatus.SetPreHyperBlock(blockInfos);
	}
	T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;

	pBlockStateAddr->SetPeerAddr(m_MyPeerInfo.GetPeerInfoByMyself());
	pBlockStateAddr->SetPeerAddrOut(m_MyPeerInfo.GetPeerInfoByOther());

	LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
	listPblockStateAddr.push_back(pBlockStateAddr);
	m_BlockStateMap[pP2pProtocolCopyHyperBlockReqRecv->GetBlockNum()] = listPblockStateAddr;

	string ipStringIn = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();
	string ipStringOut = m_MyPeerInfo.GetPeerInfoByOther().GetIPString();


	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		return;
	}

	if (pP2pProtocolCopyHyperBlockReqRecv->GetSendTimes() == 0)
		return;

	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolCopyHyperBlockReqRecv->tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolCopyHyperBlockReqRecv->SetSendTimes(pP2pProtocolCopyHyperBlockReqRecv->GetSendTimes() - 1);
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
		if (0 != g_tP2pManagerStatus.tOnChainHashInfo.strHash.compare(""))
		{
			ReOnChainFun();
		}
		else
		{
			GetOnChainInfo();
			m_qtnotify->SetBuddyStartTime(0, 0);
		}

		m_qtnotify->SetHyperBlock("", 0, g_tP2pManagerStatus.GetMaxBlockNum());
	}
	else
	{

		m_qtnotify->SetHyperBlock(g_tP2pManagerStatus.tOnChainHashInfo.strHash, g_tP2pManagerStatus.tOnChainHashInfo.uiTime, g_tP2pManagerStatus.GetMaxBlockNum());

	}
	CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
	g_tP2pManagerStatus.listGlobalBuddyChainInfo.clear();
	g_tP2pManagerStatus.SetHaveOnChainReq(false);
	g_tP2pManagerStatus.SetLocalBuddyChainState(false);
	
	g_tP2pManagerStatus.uiNextStartTimeNewest = pHyperBlockInfosTemp->tBlockBaseInfo.GetTime();
	
	g_tP2pManagerStatus.SetStartGlobalFlag(false);
}

void CHChainP2PManager::ProcessGetBlockNodeMapReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	T_PP2PPROTOCOLGETCHAINSTATERSP pP2pProtocolGetChainStateRsp = NULL;

	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		return;
	}

	int blockNum = m_BlockStateMap.size();

	int ipP2pProtocolGetChainStateRspLen = sizeof(T_P2PPROTOCOLGETCHAINSTATERSP) + blockNum * sizeof(T_CHAINSTATEINFO);
	pP2pProtocolGetChainStateRsp = (T_PP2PPROTOCOLGETCHAINSTATERSP)malloc(ipP2pProtocolGetChainStateRspLen);

	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);

	pP2pProtocolGetChainStateRsp->SetResult(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_GET_BLOCK_STATE_RSP, timeTemp.tv_sec), P2P_PROTOCOL_SUCCESS));
	pP2pProtocolGetChainStateRsp->SetPeerAddr(m_MyPeerInfo.GetPeerInfoByMyself());
	pP2pProtocolGetChainStateRsp->SetCountPeerAddr(blockNum);

	T_PCHAINSTATEINFO pPeerInfos;
	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
		pPeerInfos = NULL;
	else
		pPeerInfos = (T_PCHAINSTATEINFO)(pP2pProtocolGetChainStateRsp + 1);

	int i = 0;
	ITR_MAP_BLOCK_STATE itr = m_BlockStateMap.begin();
	for (; itr != m_BlockStateMap.end(); itr++)
	{
		pPeerInfos[i].SetBlockNum((*itr).first);
		i++;
	}

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolGetChainStateRsp, ipP2pProtocolGetChainStateRspLen);

	free(pP2pProtocolGetChainStateRsp);
	pP2pProtocolGetChainStateRsp = NULL;
}

void CHChainP2PManager::ProcessGetBlockNodeMapRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLGETCHAINSTATERSP pP2pProtocolGetChainStateRsp = (T_PP2PPROTOCOLGETCHAINSTATERSP)(pBuf);
	if (P2P_PROTOCOL_SUCCESS != pP2pProtocolGetChainStateRsp->GetResult().GetResult())
	{
		return;
	}

	T_PCHAINSTATEINFO	pPeerInfo = NULL;
	T_PCHAINSTATEINFO pPeerInfoTemp = NULL;
	pPeerInfo = (T_PCHAINSTATEINFO)(pP2pProtocolGetChainStateRsp + 1);

	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	for (int i = 0; i<pP2pProtocolGetChainStateRsp->GetCount(); i++)
	{
		bool bIndex = false;
		pPeerInfoTemp = pPeerInfo + i;
		ITR_MAP_BLOCK_STATE itr = m_BlockStateMap.find(pPeerInfoTemp->GetBlockNum());
		if (itr != m_BlockStateMap.end())
		{
			ITR_LIST_T_PBLOCKSTATEADDR itrSub = (*itr).second.begin();
			for (; itrSub != (*itr).second.end(); itrSub++)
			{
				if (((*itrSub)->GetPeerAddr().GetIP() == pP2pProtocolGetChainStateRsp->GetPeerAddr().GetIP())
					&& ((*itrSub)->GetPeerAddr().GetPort() == pP2pProtocolGetChainStateRsp->GetPeerAddr().GetPort())
					&& (*itrSub)->GetPeerAddrOut().GetIP() == inet_addr(pszIP) && ((*itrSub)->GetPeerAddrOut().GetPort() == usPort))
				{
					bIndex = true;
					break;
				}
			}

			if (!bIndex)
			{
				T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR;

				pBlockStateAddr->SetPeerAddr(pP2pProtocolGetChainStateRsp->GetPeerAddr());
				pBlockStateAddr->SetPeerAddrOut(_tpeeraddress(inet_addr(pszIP), usPort));

				(*itr).second.push_back(pBlockStateAddr);

				string ipStringIn = pP2pProtocolGetChainStateRsp->GetPeerAddr().GetIPString();
				string ipStringOut = m_MyPeerInfo.GetPeerInfoByOther().GetIPString();

			}

		}

	}
}

void CHChainP2PManager::ProcessGlobalBuddyRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLGLOBALBUDDYRSP pP2pProtocolGlobalBuddyRsp = (T_PP2PPROTOCOLGLOBALBUDDYRSP)(pBuf);

	if (P2P_PROTOCOL_SUCCESS != pP2pProtocolGlobalBuddyRsp->GetResult().GetResult())
	{
		return;
	}


	T_PGLOBALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyRsp + 1);

	bool index;
	LIST_T_LOCALCONSENSUS listLocalConsensusInfo;
	uint64 uiChainCountNum = 0;

	for (int i = 0; i < pP2pProtocolGlobalBuddyRsp->GetBlockCount(); i++)
	{
		T_GLOBALCONSENSUS  localBlockInfo;
		localBlockInfo.tLocalBlock = (*(pLocalBlockTemp + i)).GetLocalBlock();
		localBlockInfo.tPeer = (*(pLocalBlockTemp + i)).GetPeer();
		localBlockInfo.uiAtChainNum = (*(pLocalBlockTemp + i)).GetChainNo();

		T_LOCALCONSENSUS  blockInfo;
		blockInfo.SetLocalBlock(localBlockInfo.GetLocalBlock());
		blockInfo.SetPeer(localBlockInfo.GetPeer());

		if (uiChainCountNum != localBlockInfo.GetChainNo())
		{
			uiChainCountNum = localBlockInfo.GetChainNo();
			if (listLocalConsensusInfo.size() != 0)
			{
				index = JudgExistAtGlobalBuddy(listLocalConsensusInfo);
				listLocalConsensusInfo.clear();
			}
		}


		listLocalConsensusInfo.push_back(blockInfo);
		char localHash[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &localBlockInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

		if (i == pP2pProtocolGlobalBuddyRsp->GetBlockCount() - 1)
		{
			index = JudgExistAtGlobalBuddy(listLocalConsensusInfo);
			listLocalConsensusInfo.clear();
		}
	}
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

	if (nodeSize == ONE_LOCAL_BLOCK || pP2pProtocolOnChainRspRecv->GetBlockCount() == ONE_LOCAL_BLOCK)
	{
		index = true;
	}

	if (!index)
	{
		return;
	}

	if (pP2pProtocolOnChainRspRecv->GetHyperBlockNum() != g_tP2pManagerStatus.GetMaxBlockNum() + 1)
	{
		SendRefuseReq(pszIP, usPort, pP2pProtocolOnChainRspRecv->GetHash(), RECV_REQ);
		return;
	}

	CAutoMutexLock muxAuto3(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyRsp.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyRsp.end(); itr++)
	{
		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainRspRecv->GetHash(), DEF_STR_HASH256_LEN))
		{

		}
	}

	T_BUDDYINFOSTATE buddyInfo;

	CAutoMutexLock muxAuto5(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	copyLocalBuddyList(buddyInfo.localList, g_tP2pManagerStatus.listLocalBuddyChainInfo);

	T_PLOCALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALCONSENSUS)(pP2pProtocolOnChainRspRecv + 1);

	g_tP2pManagerStatus.bLocalBuddyChainState = true;

	index = false;
	for (int i = 0; i < pP2pProtocolOnChainRspRecv->GetBlockCount(); i++)
	{
		T_LOCALCONSENSUS  LocalBlockInfo;
		LocalBlockInfo = *(pLocalBlockTemp + i);

		char localHash[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &LocalBlockInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

		index = JudgExistAtLocalBuddy(buddyInfo.GetList(), LocalBlockInfo);

		if (index)
			continue;

		buddyInfo.LocalListPushBack(LocalBlockInfo);
		buddyInfo.LocalListSort();
		ChangeLocalBlockPreHash(buddyInfo.GetLocalConsensus());

	}

	buddyInfo.Set(pP2pProtocolOnChainRspRecv->GetHash(), RECV_ON_CHAIN_RSP, _tpeeraddress(inet_addr(pszIP), usPort));

	g_tP2pManagerStatus.listCurBuddyRsp.push_back(buddyInfo);

	SendConfirmReq(pszIP, usPort, pP2pProtocolOnChainRspRecv->GetHash(), P2P_PROTOCOL_SUCCESS);

}

void CHChainP2PManager::ProcessGetHyperBlockByNoReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLGETHYPERBLOCKBYNOREQ pP2pProtocolGetHyperBlockByNoReq = (T_PP2PPROTOCOLGETHYPERBLOCKBYNOREQ)(pBuf);

	SendOneHyperBlockByNo(inet_addr(pszIP), usPort, pP2pProtocolGetHyperBlockByNoReq->GetBlockNum());
}

void CHChainP2PManager::ProcessOnChainReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINREQ pP2pProtocolOnChainReqRecv = (T_PP2PPROTOCOLONCHAINREQ)(pBuf);

	SendOnChainRsp(pszIP, usPort, pBuf, uiBufLen);
}

//wls todo out?
void CHChainP2PManager::ProcessGlobalBuddyReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLGLOBALBUDDYREQ pP2pProtocolGlobalBuddyReqRecv = (T_PP2PPROTOCOLGLOBALBUDDYREQ)(pBuf);


	if (!g_tP2pManagerStatus.StartGlobalFlag())
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
	if ((*itr).GetPeer().GetPeerAddr().GetIP() == m_MyPeerInfo.GetPeerInfoByMyself().GetIP()
		&& (*itr).GetPeer().GetPeerAddr().GetPort() == m_MyPeerInfo.GetPeerInfoByMyself().GetPort()
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

		for (int i = 0; i < pP2pProtocolGlobalBuddyReqRecv->GetBlockCount(); i++)
		{
			T_GLOBALCONSENSUS  localBlockInfo;
			localBlockInfo.SetLocalBlock((*(pLocalBlockTemp + i)).GetLocalBlock());
			localBlockInfo.SetPeer((*(pLocalBlockTemp + i)).GetPeer());
			localBlockInfo.SetChainNo((*(pLocalBlockTemp + i)).GetChainNo());

			T_LOCALCONSENSUS localInfo;
			localInfo.tLocalBlock = localBlockInfo.GetLocalBlock();
			localInfo.tPeer = localBlockInfo.GetPeer();

			if (uiChainCountNum != localBlockInfo.GetChainNo())
			{
				uiChainCountNum = localBlockInfo.GetChainNo();
				if (listLocalConsensusInfo.size() != 0)
				{
					index = JudgExistAtGlobalBuddy(listLocalConsensusInfo);
					listLocalConsensusInfo.clear();
				}
			}

			char localHash[FILESIZES] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());


			listLocalConsensusInfo.push_back(localInfo);

			if (i == pP2pProtocolGlobalBuddyReqRecv->GetBlockCount() - 1)
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


		struct timeval timeTemp;
		CCommonStruct::gettimeofday_update(&timeTemp);

		uint64 tempNum = g_tP2pManagerStatus.listGlobalBuddyChainInfo.size();

		pP2pProtocolGlobalBuddyRsp->SetResult(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_GLOBAL_BUDDY_RSP, timeTemp.tv_sec), P2P_PROTOCOL_SUCCESS));
		pP2pProtocolGlobalBuddyRsp->SetBlockCount(blockNum);
		pP2pProtocolGlobalBuddyRsp->SetPeerAddr(m_MyPeerInfo.GetPeerInfoByMyself());
		pP2pProtocolGlobalBuddyRsp->SetChainCount(tempNum);

		m_qtnotify->SetGlobleBuddyChainNum(tempNum);

		T_PGLOBALCONSENSUS pPeerInfos;
		if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
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

				pPeerInfos[i].SetLocalBlock((*subItr).GetLocalBlock());
				pPeerInfos[i].SetPeer((*subItr).GetPeer());
				pPeerInfos[i].SetChainNo(chainNum);
				i++;

				char localHash[FILESIZES] = { 0 };
				CCommonStruct::Hash256ToStr(localHash, &(*subItr).tLocalBlock.GetBlockBaseInfo().GetHashSelf());

			}
		}

		string ipString = pP2pProtocolGlobalBuddyReqRecv->GetPeerAddr().GetIPString();

		if (pP2pProtocolGlobalBuddyReqRecv->GetPeerAddr().GetIP() != m_MyPeerInfo.GetPeerInfoByOther().GetIP())
		{
			m_UdpSocket.Send(pP2pProtocolGlobalBuddyReqRecv->GetPeerAddr().GetIP(), pP2pProtocolGlobalBuddyReqRecv->GetPeerAddr().GetPort(),
				(char*)pP2pProtocolGlobalBuddyRsp, ipP2pProtocolGlobalBuddyRspLen);
		}
		free(pP2pProtocolGlobalBuddyRsp);
		pP2pProtocolGlobalBuddyRsp = NULL;
	}
	else
	{
		ITR_LIST_T_LOCALCONSENSUS endItr = g_tP2pManagerStatus.listLocalBuddyChainInfo.end();
		endItr--;

		string ipStringIn = endItr->GetPeer().GetPeerAddr().GetIPString();
		string ipStringOut = endItr->GetPeer().GetPeerAddrOut().GetIPString();

		m_UdpSocket.Send(endItr->GetPeer().GetPeerAddr().GetIP(), endItr->GetPeer().GetPeerAddr().GetPort(), pBuf, uiBufLen);
	}
}

void CHChainP2PManager::ProcessOnChainConfirmMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINCONFIRM pP2pProtocolOnChainConfirmRecv = (T_PP2PPROTOCOLONCHAINCONFIRM)(pBuf);

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyReq);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyReq.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyReq.end(); itr++)
	{

		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainConfirmRecv->GetHash(), DEF_STR_HASH256_LEN))
		{

			if (pP2pProtocolOnChainConfirmRecv->GetResult().GetResult() == P2P_PROTOCOL_SUCCESS)
			{
				SendConfirmRsp(pszIP, usPort, (*itr).strBuddyHash);
			}
		}
	}
}

void CHChainP2PManager::ProcessOnChainConfirmRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINCONFIRMRSP pP2pProtocolOnChainConfirmRspRecv = (T_PP2PPROTOCOLONCHAINCONFIRMRSP)(pBuf);

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
	ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyRsp.begin();
	for (itr; itr != g_tP2pManagerStatus.listCurBuddyRsp.end();)
	{
		if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolOnChainConfirmRspRecv->GetHash(), DEF_STR_HASH256_LEN))
		{
			if (pP2pProtocolOnChainConfirmRspRecv->GetResult().GetResult() == P2P_PROTOCOL_SUCCESS)
			{

				bool index = false;
				ITR_LIST_T_LOCALCONSENSUS itrSub = (*itr).localList.begin();
				for (; itrSub != (*itr).localList.end(); itrSub++)
				{
					index = JudgExistAtLocalBuddy(g_tP2pManagerStatus.listLocalBuddyChainInfo, (*itrSub));
					if (index)
						continue;

					char localHash[FILESIZES] = { 0 };
					CCommonStruct::Hash256ToStr(localHash, &(*itrSub).tLocalBlock.GetBlockBaseInfo().GetHashSelf());


					CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
					g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back((*itrSub));

					g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
					ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
					m_qtnotify->SetLocalBuddyChainInfo(g_tP2pManagerStatus.listLocalBuddyChainInfo);
					g_tP2pManagerStatus.tBuddyInfo.usBlockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();

					if ((*itrSub).tPeer.tPeerAddr.GetIP() != m_MyPeerInfo.tPeerInfoByMyself.GetIP())
					{
						g_tP2pManagerStatus.SetRecvPoeNum(g_tP2pManagerStatus.GetRecvPoeNum()+1);
						m_qtnotify->SetReceivePoeNum(g_tP2pManagerStatus.GetRecvPoeNum());
					}

					SendCopyLocalBlock((*itrSub));

					g_tP2pManagerStatus.SetRecvRegisReqNum(g_tP2pManagerStatus.GetRecvRegisReqNum()+1);
					g_tP2pManagerStatus.SetRecvConfirmingRegisReqNum(g_tP2pManagerStatus.GetRecvConfirmingRegisReqNum()+1);
				}
			}

			SendConfirmFin(pszIP, usPort, (*itr).strBuddyHash);
			itr = g_tP2pManagerStatus.listCurBuddyRsp.erase(itr);
		}
		else
		{
			string ipString = (*itr).GetPeerAddrOut().GetIPString();

			SendRefuseReq((char*)ipString.c_str(), (*itr).GetPeerAddrOut().GetPort(), (*itr).strBuddyHash, RECV_REQ);

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
			string ipString = (*itr).GetPeerAddrOut().GetIPString();

			SendRefuseReq((char*)ipString.c_str(), (*itrReq).GetPeerAddrOut().GetPort(), (*itrReq).strBuddyHash, RECV_RSP);

			itrReq = g_tP2pManagerStatus.listCurBuddyReq.erase(itrReq);
		}
	}

	CAutoMutexLock muxAuto3(g_tP2pManagerStatus.MuxlistRecvLocalBuddyRsp);
	g_tP2pManagerStatus.listRecvLocalBuddyRsp.clear();

	CAutoMutexLock muxAuto2(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
	g_tP2pManagerStatus.listRecvLocalBuddyReq.clear();
}

void CHChainP2PManager::ProcessPeerListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLPEERLISTREQ pP2pProtocolPeerListReq = (T_PP2PPROTOCOLPEERLISTREQ)(pBuf);

	SearchPeerList(pP2pProtocolPeerListReq->GetName(), pP2pProtocolPeerListReq->GetPeerAddr().GetIP(), pP2pProtocolPeerListReq->GetPeerAddr().GetPort(),
		inet_addr(pszIP), usPort, true, pP2pProtocolPeerListReq->GetType().GetTimeStamp(), pP2pProtocolPeerListReq->GetNodeState());

	SendPeerList(inet_addr(pszIP), usPort);

	if (pP2pProtocolPeerListReq->GetMaxBlockNum() < g_tP2pManagerStatus.GetMaxBlockNum())
	{
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPeerListReq->GetMaxBlockNum());
	}
}

void CHChainP2PManager::ProcessPeerListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLPEERLISTRSP pP2pProtocolPeerListRsp = (T_PP2PPROTOCOLPEERLISTRSP)(pBuf);
	if (P2P_PROTOCOL_SUCCESS != pP2pProtocolPeerListRsp->GetResult().GetResult())
	{
		return;
	}
	T_PPEERINFO	pPeerInfo = NULL;
	T_PPEERINFO pPeerInfoTemp = NULL;
	pPeerInfo = (T_PPEERINFO)(pP2pProtocolPeerListRsp + 1);

	SearchPeerList(pP2pProtocolPeerListRsp->GetName(), pP2pProtocolPeerListRsp->GetPeerAddr().GetIP(), pP2pProtocolPeerListRsp->GetPeerAddr().GetPort(), 
		inet_addr(pszIP), usPort, true, pP2pProtocolPeerListRsp->GetResult().GetType().GetTimeStamp(), pP2pProtocolPeerListRsp->GetnodeState());
	for (int i = 0; i<pP2pProtocolPeerListRsp->GetCount(); i++)
	{
		pPeerInfoTemp = pPeerInfo + i;

		uint64 temp = 0;
		SearchPeerList(pPeerInfoTemp->GetName(), pPeerInfoTemp->GetPeerInfoByMyself().GetIP(), pPeerInfoTemp->GetPeerInfoByMyself().GetPort(), 
			pPeerInfoTemp->GetPeerInfoByOther().GetIP(), pPeerInfoTemp->GetPeerInfoByOther().GetPort(), false, temp, pPeerInfoTemp->GetNodeState());
	}

	if (pP2pProtocolPeerListRsp->GetMaxBlockNum() < g_tP2pManagerStatus.GetMaxBlockNum())
	{
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPeerListRsp->GetMaxBlockNum());
	}

	CCommonStruct::gettimeofday_update(&tvEnd);
	tvSpac.tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
}


void CHChainP2PManager::ProcessRefuseReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLREFUSEREQ pP2pProtocolRefuseReq = (T_PP2PPROTOCOLREFUSEREQ)(pBuf);

	if (pP2pProtocolRefuseReq->GetSubType() == RECV_RSP)
	{
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyRsp);
		ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyRsp.begin();
		for (itr; itr != g_tP2pManagerStatus.listCurBuddyRsp.end();)
		{

			if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolRefuseReq->GetHash(), DEF_STR_HASH256_LEN))
			{
				itr = g_tP2pManagerStatus.listCurBuddyRsp.erase(itr);
			}
			else
			{
				itr++;
			}
		}
	}
	else if (pP2pProtocolRefuseReq->GetSubType() == RECV_REQ)
	{
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistCurBuddyReq);
		ITR_LIST_T_BUDDYINFOSTATE itr = g_tP2pManagerStatus.listCurBuddyReq.begin();
		for (itr; itr != g_tP2pManagerStatus.listCurBuddyReq.end();)
		{

			if (0 == strncmp((*itr).strBuddyHash, pP2pProtocolRefuseReq->GetHash(), DEF_STR_HASH256_LEN))
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


void CHChainP2PManager::SendConfirmReq(char* pszIP, unsigned short usPort, string hash, uint8 state)
{
	T_PP2PPROTOCOLONCHAINCONFIRM pP2pProtocolOnChainConfirm = NULL;

	int ipP2pProtocolOnChainConfirmLen = sizeof(T_P2PPROTOCOLONCHAINCONFIRM);
	pP2pProtocolOnChainConfirm = (T_PP2PPROTOCOLONCHAINCONFIRM)malloc(ipP2pProtocolOnChainConfirmLen);
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolOnChainConfirm->SetInitHash(0);
	pP2pProtocolOnChainConfirm->SetP2pprotocolonchainconfirm(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_ON_CHAIN_CONFIRM, timeTemp.tv_sec), state), const_cast<char*> (hash.c_str()));

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolOnChainConfirm, ipP2pProtocolOnChainConfirmLen);

	g_tP2pManagerStatus.bLocalBuddyChainState = false;

	free(pP2pProtocolOnChainConfirm);
	pP2pProtocolOnChainConfirm = NULL;
}

void CHChainP2PManager::SendConfirmRsp(char* pszIP, unsigned short usPort, string hash)
{
	T_PP2PPROTOCOLONCHAINCONFIRMRSP pP2pProtocolOnChainConfirmRsp = NULL;

	int ipP2pProtocolOnChainConfirmRspLen = sizeof(T_P2PPROTOCOLONCHAINCONFIRMRSP);
	pP2pProtocolOnChainConfirmRsp = (T_PP2PPROTOCOLONCHAINCONFIRMRSP)malloc(ipP2pProtocolOnChainConfirmRspLen);
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);

	pP2pProtocolOnChainConfirmRsp->SetInitHash(0);
	pP2pProtocolOnChainConfirmRsp->SetP2pprotocolonchainconfirmrsp(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_ON_CHAIN_CONFIRM_RSP, timeTemp.tv_sec), P2P_PROTOCOL_SUCCESS), const_cast<char*>(hash.c_str()));
	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolOnChainConfirmRsp, ipP2pProtocolOnChainConfirmRspLen);

	g_tP2pManagerStatus.bLocalBuddyChainState = false;

	free(pP2pProtocolOnChainConfirmRsp);
	pP2pProtocolOnChainConfirmRsp = NULL;
}

void CHChainP2PManager::SendRefuseReq(char* pszIP, unsigned short usPort, string hash, uint8 type)
{
	T_PP2PPROTOCOLREFUSEREQ pP2pProtocolRefuseReq = NULL;
	int ipP2pProtocolRefuseReqLen = sizeof(T_P2PPROTOCOLREFUSEREQ);
	pP2pProtocolRefuseReq = (T_PP2PPROTOCOLREFUSEREQ)malloc(ipP2pProtocolRefuseReqLen);
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolRefuseReq->SetP2pprotocolrefusereq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_REFUSE_REQ, timeTemp.tv_sec), (char*)hash.c_str(), type);

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolRefuseReq, ipP2pProtocolRefuseReqLen);

	g_tP2pManagerStatus.SetLocalBuddyChainState(false);

	free(pP2pProtocolRefuseReq);
	ipP2pProtocolRefuseReqLen = NULL;
}

void CHChainP2PManager::SendOnChainRsp(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLONCHAINREQ pP2pProtocolOnChainReqRecv = (T_PP2PPROTOCOLONCHAINREQ)(pBuf);

	if (pP2pProtocolOnChainReqRecv->GetHyperBlockNum() != g_tP2pManagerStatus.GetMaxBlockNum() + 1)
	{
		return;
	}

	g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo.Lock();
	uint8 nodeSize = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
	g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo.UnLock();
	if (nodeSize == 0)
	{
		return;
	}

	bool index = false;

	if (nodeSize == ONE_LOCAL_BLOCK || pP2pProtocolOnChainReqRecv->GetBlockCount() == ONE_LOCAL_BLOCK)
	{
		index = true;
	}

	if (!index)
	{
		return;
	}

	bool sendCopyIndex = false;
	if (nodeSize > 1 && pP2pProtocolOnChainReqRecv->GetBlockCount() == ONE_LOCAL_BLOCK)
	{
		sendCopyIndex = true;
	}

	g_tP2pManagerStatus.SetLocalBuddyChainState(true);

	T_BUDDYINFOSTATE buddyInfo;

	g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo.Lock();
	copyLocalBuddyList(buddyInfo.localList, g_tP2pManagerStatus.listLocalBuddyChainInfo);
	g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo.UnLock();

	T_PLOCALCONSENSUS  pLocalBlockTemp;
	pLocalBlockTemp = (T_PLOCALCONSENSUS)(pP2pProtocolOnChainReqRecv + 1);

	for (int i = 0; i < pP2pProtocolOnChainReqRecv->GetBlockCount(); i++)
	{
		T_LOCALCONSENSUS  LocalBlockInfo;
		LocalBlockInfo = *(pLocalBlockTemp + i);

		char localHash[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &LocalBlockInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());


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
	buddyInfo.SetPeerAddrOut(T_PEERADDRESS(inet_addr(pszIP), usPort));
	buddyInfo.SetBuddyState(SEND_ON_CHAIN_RSP);


	T_SHA256 tempHash(0);
	GetSHA256(tempHash.pID, (const char*)(pPeerInfosTemp), pPeerInfosTempLen);//TODO:hash 可能有误0

	char strLocalHashTemp[FILESIZES] = { 0 };
	memset(strLocalHashTemp, 0, FILESIZES);
	CCommonStruct::Hash256ToStr(strLocalHashTemp, &tempHash);

	buddyInfo.SetBuddyHashInit(0);
	buddyInfo.SetBuddyHash(strLocalHashTemp);

	g_tP2pManagerStatus.MuxlistCurBuddyReq.UnLock();
	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistCurBuddyReq);
	ITR_LIST_T_BUDDYINFOSTATE itrReq = g_tP2pManagerStatus.listCurBuddyReq.begin();
	for (itrReq; itrReq != g_tP2pManagerStatus.listCurBuddyReq.end(); itrReq++)
	{
		if (0 == memcmp((*itrReq).GetBuddyHashEx(), buddyInfo.GetBuddyHashEx(), DEF_STR_HASH256_LEN))
		{
			return;
		}
	}
	g_tP2pManagerStatus.listCurBuddyReq.push_back(buddyInfo);

	T_PP2PPROTOCOLONCHAINRSP pP2pProtocolOnChainRsp = NULL;
	uint8 blockNum = buddyInfo.localList.size();
	int ipP2pProtocolOnChainRspLen = sizeof(T_P2PPROTOCOLONCHAINRSP) + blockNum * sizeof(T_LOCALCONSENSUS);
	pP2pProtocolOnChainRsp = (T_PP2PPROTOCOLONCHAINRSP)malloc(ipP2pProtocolOnChainRspLen);
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolOnChainRsp->SetInitHash(0);
	pP2pProtocolOnChainRsp->SetP2pprotocolonchainrsp(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_ON_CHAIN_RSP, timeTemp.tv_sec), P2P_PROTOCOL_SUCCESS),
		g_tP2pManagerStatus.GetMaxBlockNum() + 1, blockNum, strLocalHashTemp);


	T_PLOCALCONSENSUS pPeerInfos;
	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
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

void CHChainP2PManager::SendConfirmFin(char* pszIP, unsigned short usPort, string hash)
{

	T_PP2PPROTOCOLONCHAINCONFIRMFIN pP2pProtocolOnChainConfirmFin = NULL;

	int ipP2pProtocolOnChainConfirmFinLen = sizeof(T_P2PPROTOCOLONCHAINCONFIRMFIN);
	pP2pProtocolOnChainConfirmFin = (T_PP2PPROTOCOLONCHAINCONFIRMFIN)malloc(ipP2pProtocolOnChainConfirmFinLen);
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);

	pP2pProtocolOnChainConfirmFin->SetInitHash(0);
	pP2pProtocolOnChainConfirmFin->SetP2pprotocolonchainconfirmfin(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_ON_CHAIN_CONFIRM_FIN, timeTemp.tv_sec), P2P_PROTOCOL_SUCCESS), const_cast<char*>(hash.c_str()));

	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolOnChainConfirmFin, ipP2pProtocolOnChainConfirmFinLen);

	g_tP2pManagerStatus.SetLocalBuddyChainState(false);

	free(pP2pProtocolOnChainConfirmFin);
	pP2pProtocolOnChainConfirmFin = NULL;
}

void CHChainP2PManager::SendToOutPeerWantNatTraversalReq(T_PPEERINFO tPpeerInfo)
{
	T_P2PPROTOCOLWANTNATTRAVERSALREQ tNatTraversalReq;

	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	tNatTraversalReq.SetType(_tp2pprotocoltype(P2P_PROTOCOL_WANT_NAT_TRAVERSAL_REQ, timeTemp.tv_sec));
	tNatTraversalReq.SetPeerOutAddr(m_MyPeerInfo.GetPeerInfoByOther());
	tNatTraversalReq.SetPeerBeNatTraversaldAddr(tPpeerInfo->GetPeerInfoByOther());

	uint32 uiIp = 0;
	uint16 usPort = 0;
	int index = 0;
	while (index < MAX_SEND_NAT_TRAVERSAL_NODE_NUM)
	{
		ITR_VEC_T_PPEERCONF itrConfPeer = g_confFile.vecPeerConf.begin();
		for (; itrConfPeer != g_confFile.vecPeerConf.end(); itrConfPeer++)
		{
			if ((*itrConfPeer)->GetPeerState() == OUTNET)
			{
				uiIp = (*itrConfPeer)->GetInternetAddress().GetIP();
				usPort = (*itrConfPeer)->GetInternetAddress().GetPort();
				index++;
				break;
			}
		}

		if (uiIp == 0 && usPort == 0)
		{
			break;
		}

		string ipString = GetIpString(uiIp);

		m_UdpSocket.Send(uiIp, usPort, (char*)&tNatTraversalReq, sizeof(tNatTraversalReq));
	}
}

void CHChainP2PManager::ReOnChain(T_LOCALCONSENSUS localInfo)
{
	if (g_tP2pManagerStatus.HaveOnChainReq())
	{
		return;
	}

	g_tP2pManagerStatus.SetHaveOnChainReq(true);

	if ((time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus()) >= LOCALBUDDYTIME)
	{
		return;
	}

	g_tP2pManagerStatus.SetTimeOfConsensus(time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus());

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back(localInfo);
	g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
	ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
	m_qtnotify->SetLocalBuddyChainInfo(g_tP2pManagerStatus.listLocalBuddyChainInfo);
	g_tP2pManagerStatus.tBuddyInfo.usBlockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
}

void CHChainP2PManager::SearchPeerList(char *strName, uint32 uiIPIN, uint16 usPortIN, uint32 uiIPOUT, uint16 usPortOUT, bool bFlag, uint64 uiTime, uint16 uiNodeState)
{
	string ipStringIn = GetIpString(uiIPIN);
	string ipStringOut = GetIpString(uiIPOUT);

	bool bIndex = false;
	if ((uiIPOUT == m_MyPeerInfo.GetPeerInfoByOther().GetIP())
		&& (usPortOUT == m_MyPeerInfo.GetPeerInfoByOther().GetPort())
		&& (uiIPIN == m_MyPeerInfo.GetPeerInfoByMyself().GetIP())
		&& (usPortIN == m_MyPeerInfo.GetPeerInfoByMyself().GetPort()))
	{
		bIndex = true;
		return;
	}

	if (!bIndex)
	{
		if (0 == strncmp(m_MyPeerInfo.GetName(), strName, MAX_NODE_NAME_LEN)
			&& (uiIPIN == m_MyPeerInfo.GetPeerInfoByMyself().GetIP())
			&& (usPortIN == m_MyPeerInfo.GetPeerInfoByMyself().GetPort()))
		{

			m_MyPeerInfo.SetPeerInfoByOther(T_PEERADDRESS(uiIPOUT, usPortOUT));

			if ((uiIPIN == uiIPOUT) && (usPortIN == usPortOUT))
			{
				m_MyPeerInfo.SetState(OUTNET);
			}
			else
			{
				m_MyPeerInfo.SetState(INNERNET);
			}

			ipStringIn = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();
			ipStringOut = m_MyPeerInfo.GetPeerInfoByOther().GetIPString();

			return;
		}
	}

	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	bool iIndex = false;
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (0 == strncmp((*itr)->GetName(), strName, MAX_NODE_NAME_LEN)
			&& (uiIPIN == (*itr)->GetPeerInfoByMyself().GetIP())
			&& (usPortIN == (*itr)->GetPeerInfoByMyself().GetPort())
			&& (uiIPOUT == (*itr)->GetPeerInfoByOther().GetIP())
			&& (usPortOUT == (*itr)->GetPeerInfoByOther().GetPort()))
		{

			uint64 timeTemp = time(NULL);
			if (bFlag)
			{
				(*itr)->uiTime = (timeTemp - uiTime)/* * 1000 + (timeTemp.tv_usec - uiTime.tv_sec) / 1000*/;
			}

			(*itr)->SetNodeState(uiNodeState);
			iIndex = true;
		}
	}

	if (!iIndex)
	{
		T_PPEERINFO pPeerInfo = new T_PEERINFO;

		pPeerInfo->SetName(strName);
		pPeerInfo->SetPeerInfoByMyself(T_PEERADDRESS(uiIPIN, usPortIN));
		pPeerInfo->SetPeerInfoByOther(T_PEERADDRESS(uiIPOUT, usPortOUT));

		if ((pPeerInfo->GetPeerInfoByMyself().GetIP() == pPeerInfo->GetPeerInfoByOther().GetIP())
			&& (pPeerInfo->GetPeerInfoByMyself().GetPort() == pPeerInfo->GetPeerInfoByOther().GetPort()))
		{
			pPeerInfo->SetState(OUTNET);
			pPeerInfo->SetNatTraversalState(GOOD);
		}
		else
		{
			pPeerInfo->SetState(OUTNET);
			pPeerInfo->SetNatTraversalState(GOOD);
		}
		if (bFlag)
		{
			uint64 timeTemp = time(NULL);

			pPeerInfo->SetTime(timeTemp - uiTime);
		}
		else
		{
			pPeerInfo->SetTime(INIT_TIME);
		}
		pPeerInfo->SetNodeState(uiNodeState);

		string ipString = pPeerInfo->GetPeerInfoByMyself().GetIPString();

		string strForbIp1("0.0.0.0");

		if (0 != strForbIp1.compare(ipString.c_str()))
		{
			m_PeerInfoList.push_back(pPeerInfo);

			m_PeerInfoList.sort(Cmpare());

			m_qtnotify->SetConnectNodeUpdate(m_PeerInfoList.size(), 0, 0, 0);
		}
	}
}

void CHChainP2PManager::SendPeerList(uint32 uiIP, uint16 usPort)
{
;
	string ipString = GetIpString(uiIP);

	CAutoMutexLock muxAuto(m_MuxPeerInfoList);

	T_PP2PPROTOCOLPEERLISTRSP pP2pProtocolPeerlistRsp = NULL;

	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		return;
	}

	int tempCount = 0;
	int nodeNum = m_PeerInfoList.size();
	if (g_tP2pManagerStatus.GetBuddyPeerCount() >= nodeNum)
	{
		tempCount = nodeNum;
	}
	else
	{
		tempCount = g_tP2pManagerStatus.GetBuddyPeerCount();
	}

	int iP2pProtocolPeerlistRspLen = sizeof(T_P2PPROTOCOLPEERLISTRSP) + tempCount * sizeof(T_PEERINFO);
	pP2pProtocolPeerlistRsp = (T_PP2PPROTOCOLPEERLISTRSP)malloc(iP2pProtocolPeerlistRspLen);
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolPeerlistRsp->SetP2pprotocolpeerlistrsp(T_P2PPROTOCOLRSP(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_PEERLIST_RSP, timeTemp.tv_sec), P2P_PROTOCOL_SUCCESS),
		T_PEERADDRESS(g_confFile.GetLocalIP(), g_confFile.GetLocalPort()), const_cast<char*>(g_confFile.GetLocalNodeName().c_str()), tempCount, g_tP2pManagerStatus.GetMaxBlockNum(), g_tP2pManagerStatus.GetNodeState());

	T_PPEERINFO pPeerInfos;
	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
		pPeerInfos = NULL;
	else
		pPeerInfos = (T_PPEERINFO)(pP2pProtocolPeerlistRsp + 1);

	int i = 0;
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (i < tempCount)
		{
			pPeerInfos[i].SetPeerinfo((*itr)->GetPeerInfoByMyself(),
				(*itr)->GetPeerInfoByOther(), (*itr)->GetState(), (*itr)->GetNatTraversalState(), (*itr)->GetTime(), (*itr)->GetNodeState(), (*itr)->GetName());

			string ipStringIn = pPeerInfos[i].GetPeerInfoByMyself().GetIPString();
			string ipStringOut = pPeerInfos[i].GetPeerInfoByOther().GetIPString();

			ipStringIn = GetIpString((*itr)->GetPeerInfoByMyself().GetIP());
			ipStringOut = GetIpString((*itr)->GetPeerInfoByOther().GetIP());

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

void CHChainP2PManager::SendBlockNodeMapReq()
{
	T_P2PPROTOCOLGETCHAINSTATEREQ tGetStateReq;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	tGetStateReq.SetP2pprotocolgetchainstatereq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_GET_BLOCK_STATE_REQ, timeTemp.tv_sec), g_tP2pManagerStatus.GetMaxBlockNum());

	int index = 0;
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	int tempSendPeerNum = g_tP2pManagerStatus.GetBuddyPeerCount();
	int peerNum = m_PeerInfoList.size();
	if (peerNum < g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		tempSendPeerNum = peerNum;
	}
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if (index < tempSendPeerNum)
		{
			string ipString = GetIpString((*itr)->GetPeerInfoByOther().GetIP());
			m_UdpSocket.Send((*itr)->GetPeerInfoByOther().GetIP(), (*itr)->GetPeerInfoByOther().GetPort(), (char*)&tGetStateReq, sizeof(tGetStateReq));
			index++;
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
	if ((*itrLocal).GetPeer().GetPeerAddr().GetIP() == m_MyPeerInfo.GetPeerInfoByMyself().GetIP()
		&& (*itrLocal).GetPeer().GetPeerAddr().GetPort() == m_MyPeerInfo.GetPeerInfoByMyself().GetPort()
	{
		isEndNode = true;
	}

	if (!isEndNode)
	{
		char pszPeerIP[FILESIZES] = { 0 };
		struct in_addr addPeerIP;
		addPeerIP.s_addr = (*itrLocal).GetPeer().GetPeerAddr().GetIP();
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
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolGlobalBuddyReq->SetP2pprotocolglobalconsensusreq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_GLOBAL_BUDDY_REQ, timeTemp.tv_sec),
		m_MyPeerInfo.GetPeerInfoByMyself(), blockNum, g_tP2pManagerStatus.listGlobalBuddyChainInfo.size());

	T_PGLOBALCONSENSUS pPeerInfos;
	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
		pPeerInfos = NULL;
	else
		pPeerInfos = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyReq + 1);

	uint8 i = 0;
	uint8 chainNum = 0;
	ITR_LIST_LIST_GLOBALBUDDYINFO itrSend = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
	for (; itrSend != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itrSend++)
	{
		//	i = 0;
		chainNum++;
		ITR_LIST_T_LOCALCONSENSUS subItr = itrSend->begin();
		for (; subItr != itrSend->end(); subItr++)
		{
			pPeerInfos[i].SetGlobalconsenus((*subItr).GetPeer(), (*subItr).GetLocalBlock(), chainNum);
			i++;

			char localHash[FILESIZES] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &(*subItr).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
		}
	}
	SendDataToPeer((char*)pP2pProtocolGlobalBuddyReq, ipP2pProtocolGlobalBuddyReqLen);

	free(pP2pProtocolGlobalBuddyReq);
	pP2pProtocolGlobalBuddyReq = NULL;
}