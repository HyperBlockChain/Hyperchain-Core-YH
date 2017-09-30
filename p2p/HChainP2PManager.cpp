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
#include <QJsonDocument>  
#include <QDebug> 
#include <QJsonObject>
//#include "/headers/commonstruct.h"

#define MAX_SEND_NAT_TRAVERSAL_NODE_NUM (2)
#define MAX_SEND_PEERLIST_RSP_PERIOD	(5*60)
#define MAX_SAVE_PEERLIST_PERIOD		(30*60)
#define MAX_SEND_CHAIN_STATE_RSP_PERIOD	(5*60)
#define MAX_RECV_UDP_BUF_LEN			(30*1024)
#define	MAX_NATTRAVERSAL_PERIOD			(10*60)
#define MAX_BUF_LEN						(256)
#define MAX_NUM_LEN						(16)
#define RANDTIME						(60)
#define BUDDYSCRIPT						("buddy_script")
#define AUTHKEY							("auth_key")
#define ONE_SECOND						(1000)
#define ONE_MINITE						(60*ONE_SECOND)
#define INIT_TIME						(10)
#define ONE_KILO						(1024)
#define FIVE_MINI						(5*60)  
#define ONE_HOUR						(60*60)
#define DIVIDEND_NUM					(2)

extern UUFile m_uufile;
extern T_CONFFILE	g_confFile;
extern void SHA256(unsigned char* sha256, const char* str, long long length);
T_P2PMANAGERSTATUS g_tP2pManagerStatus;
std::mutex muLock;
bool g_bWriteStatusIsSet = true;

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

bool CHChainP2PManager::Init()
{
	log_info(g_pLogHelper, "CHChainP2PManager::Init IN...");
	strncpy(m_MyPeerInfo.strName, g_confFile.strLocalNodeName.c_str(), MAX_NODE_NAME_LEN);
	m_MyPeerInfo.tPeerInfoByMyself.uiIP = g_confFile.uiLocalIP;
	m_MyPeerInfo.tPeerInfoByMyself.uiPort = g_confFile.uiLocalPort;
	m_MyPeerInfo.tPeerInfoByOther.uiIP = 0;
	m_MyPeerInfo.tPeerInfoByOther.uiPort = 0;
	m_MyPeerInfo.uiTime = INIT_TIME;
	m_MyPeerInfo.uiState = DEFAULT_NET;
	g_tP2pManagerStatus.usBuddyPeerCount = g_confFile.uiSaveNodeNum;
	g_tP2pManagerStatus.uiNodeState = DEFAULT_REGISREQ_STATE;

	GreateGenesisBlock();
	
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
			pPeerInfo->uiNATTraversalState = GOOD;
		}
		else
		{
			pPeerInfo->uiState = INNERNET;
			pPeerInfo->uiNATTraversalState = DEFAULT_NATTRAVERSAL_STATE;
			SendToOutPeerWantNATTraversalReq(pPeerInfo);
		}
		pPeerInfo->uiNodeState = DEFAULT_REGISREQ_STATE;
		pPeerInfo->uiTime = 10;
		strncpy(pPeerInfo->strName, (*itr)->strName, MAX_NODE_NAME_LEN);
		m_PeerInfoList.push_back(pPeerInfo);

		m_PeerInfoList.sort(Cmpare());
	}
	
	m_funcUdpProcess.Set(UdpProcessEntry, this);
	m_funcUpdateDataProcess.Set(UpdateDataProcessEntry, this);
	m_funcWriteStatusProcess.Set(WriteStatusProcessEntry, this);

	char* pszLocalIP = NULL;

	struct in_addr addPeerIP;
	addPeerIP.s_addr = g_confFile.uiLocalIP;
	
	if (1 != m_UdpSocket.Init(inet_ntoa(addPeerIP), g_confFile.uiLocalPort))
		return false;
	m_UdpSocket.SetUsed(true);

	SLEEP(1*1000);
	uint16 uiIndex = 0;
	ITR_VEC_T_PPEERCONF itrConf = g_confFile.vecPeerConf.begin();
	for (; itrConf != g_confFile.vecPeerConf.end(); itrConf++)
	{
		if (uiIndex > MAX_SEND_NAT_TRAVERSAL_NODE_NUM)
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

	log_info(g_pLogHelper, "CHChainP2PManager::UpdateDataProcessImp Thread is running.");

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
			SavePeerList();
		}
		
		if (MAX_SEND_CHAIN_STATE_RSP_PERIOD < (tmNow - tmSendChainStateRsp))
		{
			tmSendChainStateRsp = tmNow;
			SendBlockNodeMapReq();
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

		SLEEP(10);
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
	log_info(g_pLogHelper, "CHChainP2PManager::WriteStatusProcessImp Thread is running.");
	time_t tmNATTraversalTime;
	time(&tmNATTraversalTime);
	
	SLEEP(ONE_MINITE);

	while (!g_bWriteStatusIsSet)
	{
		SLEEP(ONE_SECOND);
	}
	

	string strStatuFile = g_confFile.strLogDir;
	strStatuFile += "p2p_status.log";

	FILE* fp = fopen(strStatuFile.c_str(), "a");
	if (NULL == fp)
		return;

	while (1)
	{
		SLEEP(5 * ONE_SECOND);
		string strTxt("");

		strTxt = PrintAllPeerNodeList();
		fprintf(fp, "%s\n", strTxt.c_str());
		fflush(fp);

		strTxt = PrintAllLocalBlocks();
		fprintf(fp, "%s\n", strTxt.c_str());
		fflush(fp);

		strTxt = PrintBlockNodeMap();
		fprintf(fp, "%s\n", strTxt.c_str());
		fflush(fp);
	}
	fclose(fp);
}

string CHChainP2PManager::PrintAllPeerNodeList()
{
	//return "";
	string retData = "";
	char buf[MAX_BUF_LEN] = { 0 };
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
		memset(buf, 0, MAX_BUF_LEN);
		sprintf(buf, "%d", (*itr)->tPeerInfoByMyself.uiPort);
		retData += buf;

		retData += "  IPOUT=";
		addPeerIP.s_addr = (*itr)->tPeerInfoByOther.uiIP;
		retData += inet_ntoa(addPeerIP);

		retData += "  PORTOUT=";
		memset(buf, 0, MAX_BUF_LEN);
		sprintf(buf, "%d", (*itr)->tPeerInfoByOther.uiPort);
		retData += buf;

		retData += "  NETSTATE=";
		if ((*itr)->uiState == OUTNET)
		{
			retData += "out_net";
		}
		else if ((*itr)->uiState == INNERNET)
		{
			retData += "inner_net|";
			if ((*itr)->uiNATTraversalState == GOOD)
			{
				retData += "good";
			}
			else if ((*itr)->uiNATTraversalState == BAD)
			{
				retData += "bad";
			}
			else
			{
				retData += "default";
			}
		}
		else if ((*itr)->uiState == DEFAULT_NET)
		{
			retData += "default_net";
		}	
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

string CHChainP2PManager::PrintAllLocalBlocks()
{
	string retData = "";
	char buf[MAX_BUF_LEN] = { 0 };
	retData += "==================BLOCKINFO==================\n";

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

				retData += "BlockNum=";
				memset(buf, 0, MAX_BUF_LEN);
				sprintf(buf, "%d\n", itr->tBlockBaseInfo.uiID);
				retData += buf;

				retData += "FileName=";
				retData += fileInfo->fileName;
				retData += "\n";

				retData += "CustomInfo=";
				retData += fileInfo->customInfo;
				retData += "\n";


				retData += "FileHash=";
				memset(buf, 0, MAX_BUF_LEN);
				CCommonStruct::Hash512ToStr(buf, &fileInfo->tFileHash);
				retData += buf;
				retData += "\n";

				retData += "FileOwner=";
				retData += fileInfo->tFileOwner;
				retData += "\n";

				retData += "FileSize=";
				memset(buf, 0, MAX_BUF_LEN);
				sprintf(buf, "%d\n", fileInfo->uiFileSize);
				retData += buf;

				retData += "\n";

			}
		}

	}

	retData += "================================================\n";
	return retData;
}

string CHChainP2PManager::PrintBlockNodeMap()
{
	string retData = "";
	char buf[MAX_BUF_LEN] = { 0 };
	retData += "==================BLOCKSTATEINFO==================\n";

	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	ITR_MAP_BLOCK_STATE itr = m_BlockStateMap.begin();
	for (; itr != m_BlockStateMap.end(); itr++)
	{
		retData += "BlockNum=";
		memset(buf, 0, MAX_BUF_LEN);
		sprintf(buf, "%d", (*itr).first);
		retData += buf;
		
		retData += "  NodeNum=";
		memset(buf, 0, MAX_BUF_LEN);
		sprintf(buf, "%d", (*itr).second.size());
		retData += buf;
		retData += "\n";

		ITR_LIST_T_PBLOCKSTATEADDR itrSub = (*itr).second.begin();
		for (; itrSub != (*itr).second.end(); itrSub++)
		{
			retData += "          IPIN=";
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itrSub)->tPeerAddr.uiIP;
			retData += inet_ntoa(addPeerIP);

			retData += "  IPOUT=";
			addPeerIP.s_addr = (*itrSub)->tPeerAddrOut.uiIP;
			memset(buf, 0, MAX_BUF_LEN);
			strcpy(buf, inet_ntoa(addPeerIP));
			retData += buf;
			retData += "\n";
		}
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

	log_info(g_pLogHelper, "CHChainP2PManager::UdpProcessImp Thread is running." );
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
			ProcessWantNATTraversalReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_SOME_ONE_NAT_TRAVERSAL_TO_YOU_REQ:
			ProcessSomeNodeWantToConnectYouReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_NAT_TRAVERSAL_REQ:
			ProcessNATTraversalReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_ADD_BLOCK_REQ:
			ProcessAddBlockReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_ON_CHAIN_REQ:
			break;
		case P2P_PROTOCOL_ON_CHAIN_RSP:
			break;
		case P2P_PROTOCOL_ON_CHAIN_CONFIRM:
			break;
		case P2P_PROTOCOL_COPY_BLOCK_REQ:
			break;
		case P2P_PROTOCOL_GLOBAL_BUDDY_REQ:
			break;
		case P2P_PROTOCOL_GLOBAL_BUDDY_RSP:
			break;
		case P2P_PROTOCOL_COPY_HYPER_BLOCK_REQ:
			break;
		case P2P_PROTOCOL_COPY_HYPER_BLOCK_RSP:
			break;
		case P2P_PROTOCOL_GET_BLOCK_STATE_REQ:
			ProcessGetBlockNodeMapReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		case P2P_PROTOCOL_GET_BLOCK_STATE_RSP:
			ProcessGetBlockNodeMapRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen);
			break;
		default:
			break;

		}

	}
	delete[] pRecvBuf;
	pRecvBuf = NULL;
}

void CHChainP2PManager::SendToOutPeerWantNATTraversalReq(T_PPEERINFO tPpeerInfo)
{
	T_P2PPROTOCOLSTARTNATTRAVERSALREQ tNATTraversalReq;

	tNATTraversalReq.tType.ucType = P2P_PROTOCOL_WANT_NAT_TRAVERSAL_REQ;
	tNATTraversalReq.tPeerOutAddr.uiIP = m_MyPeerInfo.tPeerInfoByOther.uiIP;
	tNATTraversalReq.tPeerOutAddr.uiPort = m_MyPeerInfo.tPeerInfoByOther.uiPort;
	tNATTraversalReq.tPeerBeNATTraversaldAddr.uiIP= tPpeerInfo->tPeerInfoByOther.uiIP;
	tNATTraversalReq.tPeerBeNATTraversaldAddr.uiPort = tPpeerInfo->tPeerInfoByOther.uiPort;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	tNATTraversalReq.tType.uiTimeStamp = timeTemp.tv_sec;

	uint32 uiIp = 0;
	uint16 usPort = 0;
	int index = 0;
	while (index < MAX_SEND_NAT_TRAVERSAL_NODE_NUM)
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
		log_info(g_pLogHelper, "CHChainP2PManager::SendToOutPeerWantNATTraversalReq send NATTraversalReq to IP=%s PORT=%d.", pszPeerIP, usPort);
		
		m_UdpSocket.Send(uiIp, usPort, (char*)&tNATTraversalReq, sizeof(tNATTraversalReq));
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
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolPeerlistReq.tType.uiTimeStamp = timeTemp.tv_sec;
	strncpy(pP2pProtocolPeerlistReq.strName, g_confFile.strLocalNodeName.c_str(), MAX_NODE_NAME_LEN);

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	log_info(g_pLogHelper, "CHChainP2PManager::SendGetPeerListReq send Peerlistreq to nodename=%s IP=%s PORT=%d ", strName, pszPeerIP, usPort);

	m_UdpSocket.Send(uiIP, usPort, (char*)&pP2pProtocolPeerlistReq, sizeof(pP2pProtocolPeerlistReq));
}
void CHChainP2PManager::SavePeerList()
{
	log_info(g_pLogHelper, "CHChainP2PManager::SavePeerList to peerlist.xml");

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
			char tempBuf[MAX_NUM_LEN];
			memset(tempBuf, 0, MAX_NUM_LEN);

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
			memset(tempBuf, 0, MAX_NUM_LEN);
			sprintf(tempBuf, "%d", (*itr)->tPeerInfoByOther.uiPort);
			strBuf += tempBuf;
			strBuf += "</outserverport>\n";

			strBuf += "      <nodestate>";
			memset(tempBuf, 0, MAX_NUM_LEN);
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
	CCommonStruct::gettimeofday_update(&timePtr);
	tPingReq.tType.uiTimeStamp = timePtr.tv_sec;
	
	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	log_info(g_pLogHelper, "CHChainP2PManager::SendLoginReq TO ip:%s port:%d", pszPeerIP, usPort);

	m_UdpSocket.Send(uiIP, usPort, (char*)&tPingReq, sizeof(tPingReq));
}

void CHChainP2PManager::ProcessPeerListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLPEERLISTRSP pP2pProtocolPeerListRsp = (T_PP2PPROTOCOLPEERLISTRSP)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPeerListRspMsg recv PeerlistRsp from IP=%s PORT=%d NAME=%s", pszIP, usPort, pP2pProtocolPeerListRsp->strName);
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
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessPeerListRspMsg RecvNodeMaxBlockNum < MyMaxBlockNum, sync hyperBlock...");
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPeerListRsp->uiMaxBlockNum);
	}

	CCommonStruct::gettimeofday_update(&tvEnd);
	tvSpac.tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
}
void CHChainP2PManager::ProcessPeerListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLPEERLISTREQ pP2pProtocolPeerListReq = (T_PP2PPROTOCOLPEERLISTREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPeerListReqMsg recv Peerlist request from IP=%s PORT=%d", pszIP, usPort);

	SearchPeerList(pP2pProtocolPeerListReq->strName, pP2pProtocolPeerListReq->tPeerAddr.uiIP, pP2pProtocolPeerListReq->tPeerAddr.uiPort, inet_addr(pszIP), usPort, true, pP2pProtocolPeerListReq->tType.uiTimeStamp, pP2pProtocolPeerListReq->uiNodeState);
	
	SendPeerList(inet_addr(pszIP), usPort);

	if (pP2pProtocolPeerListReq->uiMaxBlockNum < g_tP2pManagerStatus.uiMaxBlockNum)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessPeerListReqMsg RecvNodeMaxBlockNum < MyMaxBlockNum, sync hyperBlock...");
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPeerListReq->uiMaxBlockNum);
	}
}
void CHChainP2PManager::ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLPINGREQ pP2pProtocolPingReq = (T_PP2PPROTOCOLPINGREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingReqMsg recv login request from IP=%s PORT=%d", pszIP, usPort);

	T_P2PPROTOCOLPINGRSP pP2pPingRsp;
	pP2pPingRsp.tResult.tType.ucType = P2P_PROTOCOL_PING_RSP;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pPingRsp.tResult.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pPingRsp.tResult.iResult = P2P_PROTOCOL_SUCCESS;
	pP2pPingRsp.tPeerOutAddr.uiIP = inet_addr(pszIP);
	pP2pPingRsp.tPeerOutAddr.uiPort = usPort;
	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)&pP2pPingRsp, sizeof(pP2pPingRsp));
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingReqMsg send login rsp to IP=%s PORT=%d", pszIP, usPort);

	SearchPeerList(pP2pProtocolPingReq->strName, pP2pProtocolPingReq->tPeerAddr.uiIP, pP2pProtocolPingReq->tPeerAddr.uiPort, inet_addr(pszIP), usPort, true, pP2pProtocolPingReq->tType.uiTimeStamp, pP2pProtocolPingReq->uiNodeState);
	
	SLEEP(1*1000);
	SendPeerList(inet_addr(pszIP), usPort);

	if (pP2pProtocolPingReq->uiMaxBlockNum < g_tP2pManagerStatus.uiMaxBlockNum)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingReqMsg loginNodeMaxBlockNum < MyMaxBlockNum, sync hyperBlock...");
		SendBlockToPeer(inet_addr(pszIP), usPort, pP2pProtocolPingReq->uiMaxBlockNum);
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
	m_MyPeerInfo.tPeerInfoByOther.uiIP = pP2pProtocolPingRsp->tPeerOutAddr.uiIP;
	m_MyPeerInfo.tPeerInfoByOther.uiPort = pP2pProtocolPingRsp->tPeerOutAddr.uiPort;

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = pP2pProtocolPingRsp->tPeerOutAddr.uiIP;;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessPingRspMsg update my OUTIP=%s OUTPORT=%d", pszPeerIP, pP2pProtocolPingRsp->tPeerOutAddr.uiPort);

}
void CHChainP2PManager::ProcessWantNATTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLSTARTNATTRAVERSALREQ pP2pProtocolWantNATTraversalReq = (T_PP2PPROTOCOLSTARTNATTRAVERSALREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessWantNATTraversalReqMsg recv wantNATTraversalReq from IP=%s PORT=%d", pszIP, usPort);

	T_P2PPROTOCOLSOMEPEERWANTCONNECTYOUREQ tWantNATTraversalReq;
	tWantNATTraversalReq.tType.ucType = P2P_PROTOCOL_SOME_ONE_NAT_TRAVERSAL_TO_YOU_REQ;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	tWantNATTraversalReq.tType.uiTimeStamp = timeTemp.tv_sec;
	tWantNATTraversalReq.tPeerWantNATTraversalAddr.uiIP = pP2pProtocolWantNATTraversalReq->tPeerOutAddr.uiIP;
	tWantNATTraversalReq.tPeerWantNATTraversalAddr.uiPort = pP2pProtocolWantNATTraversalReq->tPeerOutAddr.uiPort;

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = pP2pProtocolWantNATTraversalReq->tPeerBeNATTraversaldAddr.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessWantNATTraversalReqMsg send someOneWantCallYouReq to IP=%s PORT=%d", pszPeerIP, pP2pProtocolWantNATTraversalReq->tPeerBeNATTraversaldAddr.uiPort);
	m_UdpSocket.Send(pP2pProtocolWantNATTraversalReq->tPeerBeNATTraversaldAddr.uiIP, pP2pProtocolWantNATTraversalReq->tPeerBeNATTraversaldAddr.uiPort, (char*)&tWantNATTraversalReq, sizeof(tWantNATTraversalReq));
}
void CHChainP2PManager::ProcessSomeNodeWantToConnectYouReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLSOMEPEERWANTCONNECTYOUREQ pP2pProtocolSomeNodeWantConnYouReq = (T_PP2PPROTOCOLSOMEPEERWANTCONNECTYOUREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessSomeNodeWantToConnectYouReqMsg recv pP2pProtocolSomeNodeWantConnYouReq from IP=%s PORT=%d", pszIP, usPort);

	T_P2PPROTOCOLNATTRAVERSALASKREQ pP2pProtocolHoleAskReq;
	pP2pProtocolHoleAskReq.tType.ucType = P2P_PROTOCOL_NAT_TRAVERSAL_REQ;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolHoleAskReq.tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolHoleAskReq.tPeerAddr.uiIP = g_confFile.uiLocalIP;
	pP2pProtocolHoleAskReq.tPeerAddr.uiPort = g_confFile.uiLocalPort;

	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = pP2pProtocolSomeNodeWantConnYouReq->tPeerWantNATTraversalAddr.uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessSomeNodeWantToConnectYouReqMsg send holeAskReq TO IP=%s PORT=%d", pszPeerIP, pP2pProtocolSomeNodeWantConnYouReq->tPeerWantNATTraversalAddr.uiPort);

	m_UdpSocket.Send(pP2pProtocolSomeNodeWantConnYouReq->tPeerWantNATTraversalAddr.uiIP, pP2pProtocolSomeNodeWantConnYouReq->tPeerWantNATTraversalAddr.uiPort, (char*)&pP2pProtocolHoleAskReq, sizeof(pP2pProtocolHoleAskReq));
}
void CHChainP2PManager::ProcessNATTraversalReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLNATTRAVERSALASKREQ pP2pProtocolNATTraversalAskReq = (T_PP2PPROTOCOLNATTRAVERSALASKREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessNATTraversalReqMsg recv NATTraversalReq from IP=%s PORT=%d", pszIP, usPort);

	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	ITR_LIST_T_PPEERINFO itr = m_PeerInfoList.begin();
	for (; itr != m_PeerInfoList.end(); itr++)
	{
		if ((pP2pProtocolNATTraversalAskReq->tPeerAddr.uiIP == (*itr)->tPeerInfoByMyself.uiIP)
			&& (pP2pProtocolNATTraversalAskReq->tPeerAddr.uiPort == (*itr)->tPeerInfoByMyself.uiPort)
			&& (inet_addr(pszIP) == (*itr)->tPeerInfoByOther.uiIP)
			&& (usPort == (*itr)->tPeerInfoByOther.uiPort))
		{
			(*itr)->uiTime = time(NULL);
			(*itr)->uiNATTraversalState = GOOD;
			log_info(g_pLogHelper, "CHChainP2PManager::ProcessNATTraversalReqMsg NATTraversal success");
			break;
		}
	}

}

void CHChainP2PManager::ProcessAddBlockReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PP2PPROTOCOLADDBLOCKREQ pP2pProtocolAddBlockReqRecv = (T_PP2PPROTOCOLADDBLOCKREQ)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg recv addBlockReq[%d] from IP=%s PORT=%d", pP2pProtocolAddBlockReqRecv->uiBlockNum, pszIP, usPort);

	bool index = false;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	CAutoMutexLock muxAutoMap(m_MuxBlockStateMap);
	ITR_LIST_T_HYPERBLOCK itrList = m_HchainBlockList.begin();
	for (; itrList != m_HchainBlockList.end(); itrList++)
	{
		if ((*itrList).tBlockBaseInfo.uiID == pP2pProtocolAddBlockReqRecv->uiBlockNum)
		{
			index = true;
			log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg hyperBlock[%d] exist...", pP2pProtocolAddBlockReqRecv->uiBlockNum);
			break;
		}
		else
		{
			continue;
		}
	}

	if (index)
		return;

	T_HYPERBLOCK blockInfos;
	T_PHYPERBLOCKSEND pHyperBlockInfosTemp;
	pHyperBlockInfosTemp = (T_PHYPERBLOCKSEND)(pP2pProtocolAddBlockReqRecv + 1);
	blockInfos.tBlockBaseInfo = pHyperBlockInfosTemp->tBlockBaseInfo;
	blockInfos.tHashAll = pHyperBlockInfosTemp->tHashAll;

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
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
			chainNumTemp += pLocalTemp.uiAtChainNum;
			listLocakBlockTemp.clear();
			listLocakBlockTemp.push_back(pLocalTemp);
		}

		if (i == pP2pProtocolAddBlockReqRecv->uiBlockCount - 1)
		{
			blockInfos.listPayLoad.push_back(listLocakBlockTemp);
		}
	}
	
	m_HchainBlockList.push_back(blockInfos);

	log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg insert hyperBlock[%d] to my blocklist", pP2pProtocolAddBlockReqRecv->uiBlockNum);
	g_tP2pManagerStatus.tPreHyperBlock = blockInfos;

	if (g_tP2pManagerStatus.uiMaxBlockNum < pHyperBlockInfosTemp->tBlockBaseInfo.uiID)
	{
		g_tP2pManagerStatus.uiMaxBlockNum = pHyperBlockInfosTemp->tBlockBaseInfo.uiID;
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg update the newest hyperBlock[%d]", g_tP2pManagerStatus.uiMaxBlockNum);
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
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessAddBlockReqMsg ADD (INIP=%s INPORT=%d | OUTIP=%s OUTPORT=%d) to hyperBlockState[%d]", pszPeerIP, m_MyPeerInfo.tPeerInfoByMyself.uiPort, pszPeerIPOut, m_MyPeerInfo.tPeerInfoByOther.uiPort, pP2pProtocolAddBlockReqRecv->uiBlockNum);

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		return;
	}

	if (pP2pProtocolAddBlockReqRecv->uiSendTimes == 0)
		return;

	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
	pP2pProtocolAddBlockReqRecv->tType.uiTimeStamp = timeTemp.tv_sec;
	pP2pProtocolAddBlockReqRecv->uiSendTimes = pP2pProtocolAddBlockReqRecv->uiSendTimes - 1;

	SendDataToPeer(pBuf, uiBufLen);
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
			char pszPeerIP[MAX_NUM_LEN] = { 0 };
			struct in_addr addPeerIP;
			addPeerIP.s_addr = (*itrAdd)->tPeerInfoByOther.uiIP;
			strcpy(pszPeerIP, inet_ntoa(addPeerIP));

			log_info(g_pLogHelper, "CHChainP2PManager::SendDataToPeer send buddy req to IP=%s PORT=%d", pszPeerIP, (*itrAdd)->tPeerInfoByOther.uiPort);
			m_UdpSocket.Send((*itrAdd)->tPeerInfoByOther.uiIP, (*itrAdd)->tPeerInfoByOther.uiPort, buf, bufLen);
			index1++;
		}
		else
			break;
	}
}

  
void CHChainP2PManager::ProcessGetBlockNodeMapReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{ 
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessGetBlockNodeMapReqMsg recv ChainStateReq from IP=%s PORT=%d", pszIP, usPort);
	CAutoMutexLock muxAuto(m_MuxBlockStateMap);
	T_PP2PPROTOCOLGETCHAINSTATERSP pP2pProtocolGetChainStateRsp = NULL;

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::ProcessGetBlockNodeMapReqMsg usFriendPeerCount is 0");
		return;
	}

	int blockNum = m_BlockStateMap.size();

	int ipP2pProtocolGetChainStateRspLen = sizeof(T_P2PPROTOCOLGETCHAINSTATERSP) + blockNum * sizeof(T_CHAINSTATEINFO);
	pP2pProtocolGetChainStateRsp = (T_PP2PPROTOCOLGETCHAINSTATERSP)malloc(ipP2pProtocolGetChainStateRspLen);

	pP2pProtocolGetChainStateRsp->tResult.tType.ucType = P2P_PROTOCOL_GET_BLOCK_STATE_RSP;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
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

	log_info(g_pLogHelper, "CHChainP2PManager::ProcessGetBlockNodeMapReqMsg send ChainStateRsp to IP=%s PORT=%d", pszIP, usPort);
	m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pP2pProtocolGetChainStateRsp, ipP2pProtocolGetChainStateRspLen);

	free(pP2pProtocolGetChainStateRsp);
	pP2pProtocolGetChainStateRsp = NULL;
}
void CHChainP2PManager::ProcessGetBlockNodeMapRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	struct timeval tvStart, tvEnd, tvSpac;
	CCommonStruct::gettimeofday_update(&tvStart);

	T_PP2PPROTOCOLGETCHAINSTATERSP pP2pProtocolGetChainStateRsp = (T_PP2PPROTOCOLGETCHAINSTATERSP)(pBuf);
	log_info(g_pLogHelper, "CHChainP2PManager::ProcessGetBlockNodeMapRspMsg recv ChainStateRsp from IP=%s PORT=%d", pszIP, usPort);
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
				log_info(g_pLogHelper, "CHChainP2PManager::ProcessGetBlockNodeMapRspMsg addblockstate INIP=%s INPORT=%d | OUTIP=%s OUTPORT=%d", pszPeerIP, pP2pProtocolGetChainStateRsp->tPeerAddr.uiPort, pszIP, usPort);

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
	log_info(g_pLogHelper, "CHChainP2PManager::SearchPeerList INIP=%s INPORT=%d | OUTIP=%s OUTPORT=%d in or not...", pszPeerIP, usPortIN, pszPeerIPOut, usPortOUT);
	bool bIndex = false;
	if ((uiIPOUT == m_MyPeerInfo.tPeerInfoByOther.uiIP)
		&& (usPortOUT == m_MyPeerInfo.tPeerInfoByOther.uiPort)
		&& (uiIPIN == m_MyPeerInfo.tPeerInfoByMyself.uiIP)
		&& (usPortIN == m_MyPeerInfo.tPeerInfoByMyself.uiPort))
	{
		log_info(g_pLogHelper, "CHChainP2PManager::SearchPeerList Myself");
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
			log_info(g_pLogHelper, "CHChainP2PManager::SearchPeerList Myself...update my outip and outport, INIP=%s INPORT=%d | OUTIP=%s OUTPORT=%d", pszPeerIP, m_MyPeerInfo.tPeerInfoByMyself.uiPort, pszPeerIPOut, m_MyPeerInfo.tPeerInfoByOther.uiPort);

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
			log_info(g_pLogHelper, "CHChainP2PManager::SearchPeerList FIND in my Peerlist");
		
			uint64 timeTemp = time(NULL);
			if(bFlag)
			{
				(*itr)->uiTime = (timeTemp - uiTime)/* * 1000 + (timeTemp.tv_usec - uiTime.tv_sec) / 1000*/;
			}
			
			(*itr)->uiNodeState = uiNodeState;
			iIndex = true;
		}
	}

	if (!iIndex)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::SearchPeerList not find, ADD ip to my Peerlist");
		T_PPEERINFO pPeerInfo = new T_PEERINFO;
		
		strncpy(pPeerInfo->strName ,strName, MAX_NODE_NAME_LEN);
		pPeerInfo->tPeerInfoByMyself.uiIP = uiIPIN;
		pPeerInfo->tPeerInfoByMyself.uiPort = usPortIN;
		pPeerInfo->tPeerInfoByOther.uiIP = uiIPOUT;
		pPeerInfo->tPeerInfoByOther.uiPort = usPortOUT;
		if ((pPeerInfo->tPeerInfoByMyself.uiIP == pPeerInfo->tPeerInfoByOther.uiIP)
			&& (pPeerInfo->tPeerInfoByMyself.uiPort == pPeerInfo->tPeerInfoByOther.uiPort))
		{
			pPeerInfo->uiState = OUTNET;
			pPeerInfo->uiNATTraversalState = GOOD;
		}
		else
		{
			pPeerInfo->uiState = INNERNET;
			pPeerInfo->uiNATTraversalState = DEFAULT_NATTRAVERSAL_STATE;
			SendToOutPeerWantNATTraversalReq(pPeerInfo);
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
		string strForbIp1("0.0.0.0");
		if (0 != strForbIp1.compare(pszPeerIP))
		{
			m_PeerInfoList.push_back(pPeerInfo);

			m_PeerInfoList.sort(Cmpare());
		}
	}
}
void CHChainP2PManager::SendPeerList(uint32 uiIP, uint16 usPort)
{
	char pszPeerIP[MAX_IP_LEN] = "";
	struct in_addr addPeerIP;
	addPeerIP.s_addr = uiIP;
	strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);

	T_PP2PPROTOCOLPEERLISTRSP pP2pProtocolPeerlistRsp = NULL;

	if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::SendPeerList usFriendPeerCount is 0");
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
	CCommonStruct::gettimeofday_update(&timeTemp);
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
			pPeerInfos[i].uiNATTraversalState = (*itr)->uiNATTraversalState;
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

			log_info(g_pLogHelper, "CHChainP2PManager::SendPeerList to NAME=%s INIP=%s INPORT=%d |OUTIP=%s OUTPORT=%d",(*itr)->strName, pszPeerIP, (*itr)->tPeerInfoByMyself.uiPort, pszPeerIPOut, (*itr)->tPeerInfoByOther.uiPort);

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
void CHChainP2PManager::GreateGenesisBlock()
{
	log_info(g_pLogHelper, "CHChainP2PManager::GreateGenesisBlock create Originate Hyper Block");

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	CAutoMutexLock muxAuto1(m_MuxBlockStateMap);

	T_FILEINFO fileInfo;
	strncpy(fileInfo.fileName, "filename_start", MAX_FILE_NAME_LEN);
	strncpy(fileInfo.customInfo, "custominfo_start", MAX_CUSTOM_INFO_LEN);
	strncpy(fileInfo.tFileOwner, "fileowner_start", MAX_CUSTOM_INFO_LEN);
	fileInfo.uiFileSize = 0;
	memset(fileInfo.tFileHash.pID, 0, DEF_SHA512_LEN);

	T_PRIVATEBLOCK tPrivateBlock;
	tPrivateBlock.tBlockBaseInfo.uiID = 0;
	tPrivateBlock.tBlockBaseInfo.uiTime = 0;
	strncpy(tPrivateBlock.tBlockBaseInfo.strScript, BUDDYSCRIPT, MAX_SCRIPT_LEN);
	strncpy(tPrivateBlock.tBlockBaseInfo.strAuth, AUTHKEY, MAX_AUTH_LEN);
	tPrivateBlock.tPayLoad = fileInfo;
	memset(tPrivateBlock.tBlockBaseInfo.tPreHash.pID, 1, DEF_SHA256_LEN);
	memset(tPrivateBlock.tHHash.pID, 1, DEF_SHA256_LEN);
	memset(tPrivateBlock.tBlockBaseInfo.tHashSelf.pID, 1, DEF_SHA256_LEN);

	T_LOCALBLOCK tLocalBlock;
	tLocalBlock.tBlockBaseInfo.uiID = 0;
	tLocalBlock.tBlockBaseInfo.uiTime = 0;
	strncpy(tLocalBlock.tBlockBaseInfo.strScript, BUDDYSCRIPT, MAX_SCRIPT_LEN);
	strncpy(tLocalBlock.tBlockBaseInfo.strAuth, AUTHKEY, MAX_AUTH_LEN);
	memset(tLocalBlock.tBlockBaseInfo.tPreHash.pID, 1, DEF_SHA256_LEN);
	memset(tLocalBlock.tHHash.pID, 1, DEF_SHA256_LEN);
	tLocalBlock.tPayLoad = tPrivateBlock;
	memset(tLocalBlock.tBlockBaseInfo.tHashSelf.pID, 1, DEF_SHA256_LEN);

	T_HYPERBLOCK tHyperChainBlock;
	tHyperChainBlock.tBlockBaseInfo.uiID = 0;
	tHyperChainBlock.tBlockBaseInfo.uiTime = time(NULL);
	strncpy(tHyperChainBlock.tBlockBaseInfo.strScript, BUDDYSCRIPT, MAX_SCRIPT_LEN);
	strncpy(tHyperChainBlock.tBlockBaseInfo.strAuth, AUTHKEY, MAX_AUTH_LEN);
	memset(tHyperChainBlock.tBlockBaseInfo.tPreHash.pID, 1, DEF_SHA256_LEN);
	LIST_T_LOCALBLOCK ListLocalBlock;
	ListLocalBlock.push_back(tLocalBlock);
	tHyperChainBlock.listPayLoad.push_back(ListLocalBlock);
	memset(tHyperChainBlock.tHashAll.pID, 1, DEF_SHA256_LEN);
	memset(tHyperChainBlock.tBlockBaseInfo.tHashSelf.pID, 1, DEF_SHA256_LEN);

	m_HchainBlockList.push_back(tHyperChainBlock);

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
	log_info(g_pLogHelper, "CHChainP2PManager::GreateGenesisBlock add (INIP=%s INPORT=%d | OUTIP=%s OUTPORT=%d) to OriginateHyperBlock[0] ", pszPeerIP, m_MyPeerInfo.tPeerInfoByMyself.uiPort, pszPeerIPOut, m_MyPeerInfo.tPeerInfoByOther.uiPort);

	g_tP2pManagerStatus.tPreHyperBlock = tHyperChainBlock;
	g_tP2pManagerStatus.uiStartTimeOfConsensus = tHyperChainBlock.tBlockBaseInfo.uiTime;
}


void CHChainP2PManager::AddNewBlock(TEVIDENCEINFO tFileInfo)
{
	g_tP2pManagerStatus.uiSendRegisReqNum += 1;
	g_tP2pManagerStatus.uiSendConfirmingRegisReqNum += 1;
	
	log_info(g_pLogHelper, "CHChainP2PManager::AddNewBlock in...");
	
	T_FILEINFO pFileInfo;
	strncpy(pFileInfo.fileName, tFileInfo.cFileName.c_str(), MAX_FILE_NAME_LEN);
	strncpy(pFileInfo.customInfo, tFileInfo.cCustomInfo.c_str(), MAX_CUSTOM_INFO_LEN);
	strncpy(pFileInfo.tFileOwner, tFileInfo.cRightOwner.c_str(), MAX_CUSTOM_INFO_LEN);
	pFileInfo.uiFileSize = tFileInfo.iFileSize;
	char tempBuf[DEF_SHA512_LEN];
	memset(tempBuf, 0, DEF_SHA512_LEN);
	strncpy(tempBuf, tFileInfo.cFileHash.c_str(), DEF_SHA512_LEN);
	CCommonStruct::StrToHash512(pFileInfo.tFileHash.pID, tempBuf);

	T_PRIVATEBLOCK tPrivateBlock;
	tPrivateBlock.tBlockBaseInfo.uiID = 1;
	tPrivateBlock.tBlockBaseInfo.uiTime = time(NULL);
	strncpy(tPrivateBlock.tBlockBaseInfo.strScript, BUDDYSCRIPT, MAX_SCRIPT_LEN);
	strncpy(tPrivateBlock.tBlockBaseInfo.strAuth, AUTHKEY, MAX_AUTH_LEN);
	tPrivateBlock.tPayLoad = pFileInfo;
	memset(tPrivateBlock.tBlockBaseInfo.tPreHash.pID, 0, DEF_SHA256_LEN);
	tPrivateBlock.tHHash = g_tP2pManagerStatus.tPreHyperBlock.tBlockBaseInfo.tHashSelf;
	GetSHA256(tPrivateBlock.tBlockBaseInfo.tHashSelf.pID, (const char*)(&tPrivateBlock), sizeof(tPrivateBlock));

	T_LOCALBLOCK tLocalBlock;
	tLocalBlock.tBlockBaseInfo.uiID = 1;
	tLocalBlock.uiAtChainNum = 1;
	tLocalBlock.tBlockBaseInfo.uiTime = time(NULL);
	strncpy(tLocalBlock.tBlockBaseInfo.strScript, BUDDYSCRIPT, MAX_SCRIPT_LEN);
	strncpy(tLocalBlock.tBlockBaseInfo.strAuth, AUTHKEY, MAX_AUTH_LEN);
	memset(tLocalBlock.tBlockBaseInfo.tPreHash.pID, 0, DEF_SHA256_LEN);
	tLocalBlock.tHHash = g_tP2pManagerStatus.tPreHyperBlock.tBlockBaseInfo.tHashSelf;
	tLocalBlock.tPayLoad = tPrivateBlock;
	GetSHA256(tLocalBlock.tBlockBaseInfo.tHashSelf.pID, (const char*)(&tLocalBlock), sizeof(tLocalBlock));

	T_LOCALCONSENSUS localConsensusInfo;
	localConsensusInfo.tPeer.tPeerAddr = m_MyPeerInfo.tPeerInfoByMyself;
	localConsensusInfo.tPeer.tPeerAddrOut = m_MyPeerInfo.tPeerInfoByOther;
	localConsensusInfo.tLocalBlock = tLocalBlock;
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
			CCommonStruct::gettimeofday_update(&timeTemp);
			pP2pProtocolAddBlockReq->tType.uiTimeStamp = timeTemp.tv_sec;
			pP2pProtocolAddBlockReq->uiSendTimes = 3;
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
			log_info(g_pLogHelper, "CHChainP2PManager::SendBlockToPeer send hyperBlock[%d] TO IP=%s PORT=%d", pP2pProtocolAddBlockReq->uiBlockNum, pszPeerIP, usPort);

			free(pP2pProtocolAddBlockReq);
			iP2pProtocolAddBlockLen = NULL;
		}
	}

}
void CHChainP2PManager::SendBlockNodeMapReq()
{
	T_P2PPROTOCOLGETCHAINSTATEREQ tGetStateReq;

	tGetStateReq.tType.ucType = P2P_PROTOCOL_GET_BLOCK_STATE_REQ;
	tGetStateReq.uiMaxBlockNum = g_tP2pManagerStatus.uiMaxBlockNum;
	struct timeval timeTemp;
	CCommonStruct::gettimeofday_update(&timeTemp);
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
			log_info(g_pLogHelper, "CHChainP2PManager::SendBlockNodeMapReq To IP=%s PORT=%d", pszPeerIP, (*itr)->tPeerInfoByOther.uiPort);
		
			m_UdpSocket.Send((*itr)->tPeerInfoByOther.uiIP, (*itr)->tPeerInfoByOther.uiPort,(char*)&tGetStateReq, sizeof(tGetStateReq));
			index++;
		}
		else
			break;
	}
}

uint64 CHChainP2PManager::GetLocalLatestBlockNo()
{
	return 0;
}

uint64 CHChainP2PManager::GetLatestHyperBlockNo()
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
bool CHChainP2PManager::VerifyPoeRecord(string &checkFileHash, P_TEVIDENCEINFO pCheckInfo)
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
					char strBuf[DEF_SHA512_LEN];
					memset(strBuf, 0, DEF_SHA512_LEN);
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
	if (data >= ONE_KILO)
	{
		data = data / ONE_KILO;
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
		char testBuf[MAX_BUF_LEN];
		memset(testBuf, 0, MAX_BUF_LEN);
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
uint32 CHChainP2PManager::GetStrongNodeNum()
{
	return GetNodeNum(0, FIVE_MINI);
}
uint32 CHChainP2PManager::GetAverageNodeNum()
{
	return GetNodeNum(FIVE_MINI + 1, ONE_HOUR/2);
}
uint32 CHChainP2PManager::GetWeakNodeNum()
{
	return GetNodeNum(ONE_HOUR, ONE_HOUR*12);
}
uint32 CHChainP2PManager::GetOfflineNodeNum()
{
	return GetNodeNum(ONE_HOUR*12, ONE_HOUR*24);
}

void CHChainP2PManager::GetSendingRate(string &sendRate)
{
	char strBuf[MAX_NUM_LEN];
	memset(strBuf, 0, MAX_NUM_LEN);
	sprintf(strBuf, "%dB/s", m_UdpSocket.GetSendRate());
	sendRate = strBuf;
}

void CHChainP2PManager::GetSentSize(string &allSendSize)
{
	allSendSize = m_UdpSocket.GetSendSize();
}

void CHChainP2PManager::GetRecvingRate(string &recvRate)
{
	char strBuf[MAX_NUM_LEN];
	memset(strBuf, 0, MAX_NUM_LEN);
	sprintf(strBuf, "%dB/s", m_UdpSocket.GetRecvRate());
	recvRate = strBuf;
}

void CHChainP2PManager::GetRecvedSize(string &allRecvSize)
{
	allRecvSize = m_UdpSocket.GetRecvSize();
}
VEC_T_QUERYSHOWINFO CHChainP2PManager::GetBlockInfoByHash(string &hash)
{
	VEC_T_QUERYSHOWINFO vecBrowserInfo;
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
					TQUERYSHOWINFO tempBlockInfo;
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

VEC_T_QUERYSHOWINFO CHChainP2PManager::GetBlockInfoByTime(string &time)
{
	VEC_T_QUERYSHOWINFO vecBrowserInfo;
	return vecBrowserInfo;
}
VEC_T_QUERYSHOWINFO CHChainP2PManager::GetBlockInfoByBlockNum(string &blockNum)
{
	VEC_T_QUERYSHOWINFO vecBrowserInfo;
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
					TQUERYSHOWINFO tempBlockInfo;
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
VEC_T_QUERYSHOWINFO CHChainP2PManager::GetBlockInfoByCustomInfo(string &cusInfo)
{
	VEC_T_QUERYSHOWINFO vecBrowserInfo;
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
					TQUERYSHOWINFO tempBlockInfo;
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
		if (index % DIVIDEND_NUM == 1)
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
		if (index % DIVIDEND_NUM == 0)
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
		tempHyperBlockDbInfo.ucBlockType = 1;
		tempHyperBlockDbInfo.uiBlockId = itr->tBlockBaseInfo.uiID;
		if (itr->tBlockBaseInfo.uiID == 1)
			tempHyperBlockDbInfo.uiReferHyperBlockId = 0;
		else
			tempHyperBlockDbInfo.uiReferHyperBlockId = itr->tBlockBaseInfo.uiID - 1;
		tempHyperBlockDbInfo.uiBlockTimeStamp = itr->tBlockBaseInfo.uiTime;
		tempHyperBlockDbInfo.strScript = itr->tBlockBaseInfo.strScript;
		tempHyperBlockDbInfo.strAuth = itr->tBlockBaseInfo.strAuth;

		char strBuf[MAX_BUF_LEN];
		memset(strBuf, 0, MAX_BUF_LEN);
		CCommonStruct::Hash256ToStr(strBuf, &itr->tHashAll);
		tempHyperBlockDbInfo.strHashAll = strBuf;

		memset(strBuf, 0, MAX_BUF_LEN);
		CCommonStruct::Hash256ToStr(strBuf, &itr->tBlockBaseInfo.tHashSelf);
		tempHyperBlockDbInfo.strHashSelf = strBuf;

		memset(strBuf, 0, MAX_BUF_LEN);
		CCommonStruct::Hash256ToStr(strBuf, &itr->tBlockBaseInfo.tPreHash);
		tempHyperBlockDbInfo.strPreHash = strBuf;

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
				tempHyperBlockDbInfoLocal.ucBlockType = 2;
				tempHyperBlockDbInfoLocal.uiBlockId = localBlock->tBlockBaseInfo.uiID;
				tempHyperBlockDbInfoLocal.uiReferHyperBlockId = itr->tBlockBaseInfo.uiID;
				tempHyperBlockDbInfoLocal.uiBlockTimeStamp = localBlock->tBlockBaseInfo.uiTime;
				tempHyperBlockDbInfoLocal.strAuth = localBlock->tBlockBaseInfo.strAuth;
				tempHyperBlockDbInfoLocal.strScript = localBlock->tBlockBaseInfo.strScript;
				
				tempHyperBlockDbInfoLocal.strPreHash = "";

				memset(strBuf, 0, MAX_BUF_LEN);
				CCommonStruct::Hash256ToStr(strBuf, &localBlock->tBlockBaseInfo.tHashSelf);
				tempHyperBlockDbInfoLocal.strHashSelf = strBuf;

				memset(strBuf, 0, MAX_BUF_LEN);
				CCommonStruct::Hash256ToStr(strBuf, &localBlock->tHHash);
				tempHyperBlockDbInfoLocal.strHyperBlockHash = strBuf;
				tempHyperBlockDbInfoLocal.uiLocalChainId = localBlock->uiAtChainNum;
				string strPayLoad = "";
			
				memset(strBuf, 0, MAX_BUF_LEN);
				CCommonStruct::Hash512ToStr(strBuf, &fileInfo->tFileHash);
				char buf[MAX_BUF_LEN] = { 0 };
				strncpy(buf, strBuf, MAX_BUF_LEN);

				tempHyperBlockDbInfoLocal.strPayload = buf;
				tempHyperBlockDbInfoLocal.strPayload[MAX_BUF_LEN] = '\0';
				vec.push_back(tempHyperBlockDbInfoLocal);
			}
		}
	}
	return vec;
}