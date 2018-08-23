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
#include "interface/QtNotify.h"
#include "utility/ElapsedTime.h"
#include "utility/OSFunctions.h"
#include "headers/commonstruct.h"
#include "headers/lambda.h"
#include "headers/convert2.h"
#include "utility/netutil.h"
#include <iostream>
#include <sstream>

using namespace std;


extern UUFile m_uufile;
extern T_CONFFILE	g_confFile;
extern void SHA256(unsigned char* sha256, const char* str, long long length);

CMutexObj g_MuxP2pManagerStatus;
T_P2PMANAGERSTATUS g_tP2pManagerStatus;

std::mutex muLock;
bool g_bWriteStatusIsSet = true;


uint64 _tBuddyInfo::GetCurBuddyNo()const
{
	return uiCurBuddyNo;
}

uint16 _tBuddyInfo::GetBlockNum()const
{
	return usBlockNum;
}

uint16 _tBuddyInfo::GetChainNum()const
{
	return usChainNum;
}

uint8 _tBuddyInfo::GetBuddyState()const
{
	return eBuddyState;
}

void _tBuddyInfo::SetBlockNum(uint16 num)
{
	usBlockNum = num;
}

uint64 _tOnChainHashInfo::GetTime()const
{
	return uiTime;
}

string _tOnChainHashInfo::GetHash()const
{
	return strHash;
}

void _tOnChainHashInfo::Set(uint64 t, string h)
{
	uiTime = t;
	strHash = h;
}

void _tp2pmanagerstatus::clearStatus()
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
	tOnChainHashInfo.strHash = "";
	tOnChainHashInfo.uiTime = 0;
	uiSendPoeNum = 0;
	uiRecivePoeNum = 0;
	tBuddyInfo.uiCurBuddyNo = 0;
	tBuddyInfo.eBuddyState = IDLE;
	tBuddyInfo.usBlockNum = 0;
	tBuddyInfo.usChainNum = 0;
}    


bool _tp2pmanagerstatus::StartGlobalFlag()const
{
	return bStartGlobalFlag;
}

bool _tp2pmanagerstatus::HaveOnChainReq()const
{
	return bHaveOnChainReq;
}

bool _tp2pmanagerstatus::HaveRecvCopyHyperBlock()const
{
	return bHaveRecvCopyHyperBlock;
}

int32 _tp2pmanagerstatus::GetMaxBlockNum()const
{
	return uiMaxBlockNum;
}

uint16 _tp2pmanagerstatus::GetBuddyPeerCount()const
{
	return usBuddyPeerCount;
}

uint16 _tp2pmanagerstatus::GetNodeState()const
{
	return uiNodeState;	
}

uint64 _tp2pmanagerstatus::GetSendRegisReqNum()const
{
	return uiSendRegisReqNum;
}

uint64 _tp2pmanagerstatus::GetRecvRegisReqNum()const
{
	return uiRecvRegisReqNum;
}

uint64 _tp2pmanagerstatus::GetSendConfirmingRegisReqNum()const
{
	return uiSendConfirmingRegisReqNum;
}

uint64 _tp2pmanagerstatus::GetRecvConfirmingRegisReqNum()const
{
	return uiRecvConfirmingRegisReqNum;
}

uint64 _tp2pmanagerstatus::GetStartTimeOfConsensus()const
{
	return uiStartTimeOfConsensus;
}

uint64 _tp2pmanagerstatus::GetNextStartTimeNewest()const
{
	return uiNextStartTimeNewest;
}

uint64 _tp2pmanagerstatus::GetTimeOfConsensus()const
{
	return uiTimeOfConsensus;
}

uint16 _tp2pmanagerstatus::GetGlobalBuddyChainCount()const
{
	return usGlobalBuddyChainCount;
}

bool _tp2pmanagerstatus::LocalBuddyChainState()const
{
	return bLocalBuddyChainState;
}

T_PEERADDRESS _tp2pmanagerstatus::GetLocalBuddyAddr()const
{
	return tLocalBuddyAddr;
}

T_HYPERBLOCK _tp2pmanagerstatus::GetPreHyperBlock()const
{
	return tPreHyperBlock;
}

LIST_T_LOCALCONSENSUS& _tp2pmanagerstatus::GetListOnChainReq()
{
	return listOnChainReq;
}

void _tp2pmanagerstatus::SetBuddyPeerCount(uint16 count)
{
	usBuddyPeerCount = count;
}

void _tp2pmanagerstatus::SetGlobalBuddyAddr(T_PEERADDRESS addr)
{
	tLocalBuddyAddr = addr;;
}

void _tp2pmanagerstatus::SetTimeOfConsensus(uint64 t)
{
	uiTimeOfConsensus = t;
}

void _tp2pmanagerstatus::SetGlobalBuddyChainCount(uint16 count)
{
	usGlobalBuddyChainCount = count;
}

void _tp2pmanagerstatus::SetStartGlobalFlag(bool flag)
{
	bStartGlobalFlag = flag;
}

void _tp2pmanagerstatus::SetHaveOnChainReq(bool haveOnChainReq)
{
	bHaveOnChainReq = haveOnChainReq;
}

void _tp2pmanagerstatus::SetHaveRecvCopyHyperBlock(bool haveRecvCopyHyperBlock)
{
	bHaveRecvCopyHyperBlock = haveRecvCopyHyperBlock;
}

void _tp2pmanagerstatus::SetLocalBuddyChainState(bool state)
{
	bLocalBuddyChainState = state;
}

void _tp2pmanagerstatus::SetNextStartTimeNewest(uint64 t)
{
	uiNextStartTimeNewest = t;
}

void _tp2pmanagerstatus::SetRecvRegisReqNum(uint64 num)
{
	uiRecvRegisReqNum = num;
}

uint64 _tp2pmanagerstatus::GetSendPoeNum()const
{
	return uiSendPoeNum;
}

void _tp2pmanagerstatus::SetSendPoeNum(uint64 num)
{
	uiSendPoeNum = num;
}

void _tp2pmanagerstatus::SetStartTimeOfConsensus(uint64 t)
{
	uiStartTimeOfConsensus = t;
}

void _tp2pmanagerstatus::SetNodeState(uint16 state)
{
	uiNodeState = state;
}

void _tp2pmanagerstatus::SetP2pmanagerstatus(T_HYPERBLOCK PreHyperBlock, int32 MaxBlockNum, bool ClearlistGlobalBuddyChainInfo, bool ClearlistLocalBuddyChainInfo, 
	bool HaveOnChainReq, bool LocalBuddyChainState, uint64 NextStartTimeNewest, uint64 TimeOfConsensus, uint64 SendConfirmingRegisReqNum, uint16 NodeState)
{
	tPreHyperBlock = PreHyperBlock;
	uiMaxBlockNum = MaxBlockNum;
	if (ClearlistGlobalBuddyChainInfo)
	{
		listGlobalBuddyChainInfo.clear();
	}
	if (ClearlistLocalBuddyChainInfo)
	{
		listLocalBuddyChainInfo.clear();
	}
	bHaveOnChainReq = HaveOnChainReq;
	bLocalBuddyChainState = LocalBuddyChainState;
	uiNextStartTimeNewest = NextStartTimeNewest;
	uiTimeOfConsensus = TimeOfConsensus;
	uiSendConfirmingRegisReqNum = SendConfirmingRegisReqNum;
	uiNodeState = NodeState;
}

void _tp2pmanagerstatus::SetSendRegisReqNum(uint64 num)
{
	uiSendRegisReqNum = num;
}

void _tp2pmanagerstatus::SetSendConfirmingRegisReqNum(uint64 num)
{
	uiSendConfirmingRegisReqNum = num;
}

void _tp2pmanagerstatus::SetRecvConfirmingRegisReqNum(uint64 num)
{
	uiRecvConfirmingRegisReqNum = num;
}

void _tp2pmanagerstatus::SetMaxBlockNum(int32 num)
{
	uiMaxBlockNum = num;
}

void _tp2pmanagerstatus::SetPreHyperBlock(const T_HYPERBLOCK&block)
{
	tPreHyperBlock = block;
}

uint64 _tp2pmanagerstatus::GetRecvPoeNum()const
{
	return uiRecivePoeNum;
}

void _tp2pmanagerstatus::SetRecvPoeNum(uint64 num)
{
	uiRecivePoeNum = num;
}

T_STRUCTBUDDYINFO _tp2pmanagerstatus::GetBuddyInfo()const
{
	return tBuddyInfo;
}

void _tp2pmanagerstatus::SetBuddyInfo(T_STRUCTBUDDYINFO info)
{
	tBuddyInfo = info;
}

CHChainP2PManager::CHChainP2PManager(void)
{
}
CHChainP2PManager::~CHChainP2PManager(void)
{
}



void CHChainP2PManager::GetHyperBlockNumInfoFromLocal()
{
	std::list<uint64> HyperBlockNum;
	CHyperchainDB::GetHyperBlockNumInfo(HyperBlockNum);
	m_qtnotify->SetHyperBlockNumFromLocal(HyperBlockNum);
}

T_FILEINFO CHChainP2PManager::GetFileInfoFormPayload(string str)
{
	T_FILEINFO tPayLoad;
	int nPos = str.find("fileName=");
	int nPos1 = str.find("fileHash=");
	string fileHash = str.substr(nPos1 + 9, nPos - (nPos1 + 9));


	T_SHA512 tFileHash(0);
	memcpy(tFileHash.pID, fileHash.c_str(), DEF_SHA512_LEN);

	nPos1 = str.find("fileCustom=");
	string fileName = str.substr(nPos + 9, nPos1 - (nPos + 9));

	nPos = str.find("fileOwner=");
	string fileCustom = str.substr(nPos1 + 11, nPos - (nPos1 + 11));

	nPos1 = str.find("fileSize=");
	string fileOwner = str.substr(nPos + 10, nPos1 - (nPos + 10));

	string filesize = str.substr(nPos1 + 9, str.length() - (nPos1 + 9));

	uint64 nfilesize = 0;
	stringstream ss;
	ss << filesize;
	ss >> nfilesize;

	tPayLoad.SetFileInfo(nfilesize, fileName.c_str(), fileCustom.c_str(), fileOwner.c_str());
	tPayLoad.SetFileHash(tFileHash);

	return tPayLoad;
}

void CHChainP2PManager::GetHyperBlockInfoFromLocal()
{
	HyperBlockDB hyperchainDB;
	CHyperchainDB::GetLatestHyperBlock(hyperchainDB);


	const T_HYPERBLOCKDBINFO& hbi = hyperchainDB.GetHyperBlock();
	if(hbi.GetBlockId()!=0)
	{
		
		T_HYPERBLOCK hyperBlock;
		_tblockbaseinfo tBBI;
		tBBI.SetBlockBaseInfo(hbi.GetBlockId(),hbi.GetBlockTimeStamp(),(int8*)hbi.GetAuth().c_str(),(int8*)hbi.GetScript().c_str(),
			hbi.GetHashSelf(),hbi.GetPreHash());
		hyperBlock.SetBlockBaseInfo(tBBI);
		
		uint64 chainNumTemp  = 1;
		bool index = false;
		LIST_T_LOCALBLOCK listLocakBlockTemp;
		LocalChainDB::iterator itrMap = hyperchainDB.GetMapLocalChain().begin();
		for (itrMap; itrMap != hyperchainDB.GetMapLocalChain().end(); itrMap++)
		{
			index = true;
			LIST_T_LOCALBLOCK listLocalBlockTemp;
			LocalBlockDB::iterator itrMapBlock = (*itrMap).second.begin();
			for (itrMapBlock; itrMapBlock != (*itrMap).second.end(); itrMapBlock++)
			{
				const _tBlockPersistStru& lbd = (*itrMapBlock).second;

				T_LOCALBLOCK pLocalTemp(T_BLOCKBASEINFO(lbd.GetBlockId(), lbd.GetBlockTimeStamp(), (int8*)lbd.GetAuth().c_str(), (int8*)lbd.GetScript().c_str(),lbd.GetHashSelf(), lbd.GetPreHash()), 
					lbd.GetHyperBlockHash(), lbd.GetLocalChainId());		
				pLocalTemp.GetPayLoad().SetPayLoad(GetFileInfoFormPayload((*itrMapBlock).second.GetPayload().c_str()));	

				listLocalBlockTemp.push_back(pLocalTemp);
			}
			listLocakBlockTemp.sort(CmpareOnChainLocal());		
			hyperBlock.PushBack(listLocalBlockTemp);
		}
		
		if (index)
		{
			m_HchainBlockList.push_back(hyperBlock);
			m_HchainBlockListNew.push_back(T_HYPERBLOCKNEW(hyperBlock));
			WriteBlockLog(hyperBlock);
		}

		if (g_tP2pManagerStatus.GetMaxBlockNum() < hyperBlock.GetBlockBaseInfo().GetID())
		{
			g_tP2pManagerStatus.SetMaxBlockNum(hyperBlock.GetBlockBaseInfo().GetID());
			g_tP2pManagerStatus.SetPreHyperBlock(hyperBlock);

			m_qtnotify->SetHyperBlock("", 0,g_tP2pManagerStatus.GetMaxBlockNum());	
		}
		
	}
}
bool CHChainP2PManager::Init()
{
	m_MyPeerInfo.SetNodeState(DEFAULT_NET);
	m_MyPeerInfo.SetPeerInfoByMyself(T_PEERADDRESS(g_confFile.GetLocalIP(), g_confFile.GetLocalPort()));
	m_MyPeerInfo.SetPeerInfoByOther(T_PEERADDRESS(0,0));
	m_MyPeerInfo.SetName((int8*)g_confFile.GetLocalNodeName().c_str());
	m_MyPeerInfo.SetTime(INIT_TIME);

	g_tP2pManagerStatus.SetBuddyPeerCount(g_confFile.GetSaveNodeNum());
	g_tP2pManagerStatus.SetNodeState(DEFAULT_REGISREQ_STATE);

	if (g_tP2pManagerStatus.GetMaxBlockNum() == 0)
	{
		GreateGenesisBlock();
		m_qtnotify->SetHyperBlock("", 0, 0);
	}
	else
	{
		g_tP2pManagerStatus.SetStartTimeOfConsensus(time(NULL));		
	}

	ITR_VEC_T_PPEERCONF itr = g_confFile.vecPeerConf.begin();
	for (; itr != g_confFile.vecPeerConf.end(); itr++)
	{
		T_PPEERINFO pPeerInfo = new T_PEERINFO;
		const T_PPEERCONF&pc = *itr;


		pPeerInfo->SetPeerInfoByMyself(T_PEERADDRESS(pc->GetIntranetAddress().GetIP(),pc->GetIntranetAddress().GetPort()));
		pPeerInfo->SetPeerInfoByOther(T_PEERADDRESS(pc->GetInternetAddress().GetIP(), pc->GetInternetAddress().GetPort()));
		if (pPeerInfo->GetPeerInfoByMyself().GetIP() == pPeerInfo->GetPeerInfoByOther().GetIP())
		{
			pPeerInfo->SetState(OUTNET);
			pPeerInfo->SetNatTraversalState(GOOD);
		}
		else
		{
			pPeerInfo->SetState(INNERNET);
			pPeerInfo->SetNatTraversalState(DEFAULT_NATTRAVERSAL_STATE);
		}
		pPeerInfo->SetNodeState(DEFAULT_REGISREQ_STATE);
		pPeerInfo->SetTime(INIT_TIME);
		pPeerInfo->SetName(pc->GetNodeName());

		m_PeerInfoList.push_back(pPeerInfo);

		m_PeerInfoList.sort(Cmpare());
		m_qtnotify->SetConnectNodeUpdate(m_PeerInfoList.size(), 0, 0, 0);
	}
	
	m_funcUdpProcess.Set(UdpProcessEntry, this);
	m_funcUpdateDataProcess.Set(UpdateDataProcessEntry, this);
	m_funcWriteStatusProcess.Set(WriteStatusProcessEntry, this);
	m_funcMemCacheProcess.Set(MemCacheProcessEntry, this);
	
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
	addPeerIP.s_addr = g_confFile.GetLocalIP();
	
	string strIp = inet_ntoa(addPeerIP);
	m_qtnotify->SetNodeInfo(g_confFile.GetLocalNodeName(), strIp.c_str(), g_confFile.GetLocalPort());

	if (1 != m_UdpSocket.Init(inet_ntoa(addPeerIP), g_confFile.GetLocalPort()))
		return false;
	m_UdpSocket.SetUsed(true);

	SLEEP(1 * ONE_SECOND);
	uint16 uiIndex = 0;
	ITR_VEC_T_PPEERCONF itrConf = g_confFile.vecPeerConf.begin();
	for (; itrConf != g_confFile.vecPeerConf.end(); itrConf++)
	{
		if (uiIndex > MAX_SEND_NAT_TRAVERSAL_NODE_NUM)
		{
			break;
		}

		if ((*itrConf)->GetPeerState() == OUTNET)
		{
			SendLoginReq((*itrConf)->GetIntranetAddress().GetIP(), (*itrConf)->GetIntranetAddress().GetPort());
			uiIndex++;
		}
	}

	g_tP2pManagerStatus.tBuddyInfo.uiCurBuddyNo = g_tP2pManagerStatus.GetMaxBlockNum() + 1;
	g_tP2pManagerStatus.tBuddyInfo.eBuddyState = IDLE;

	m_qtnotify->SetServerInfo(g_confFile.vecPeerConf);
	return true;
}

void CHChainP2PManager::GetNewHyperInfoAndReOnChain()
{
	uint i = 1;
	while (i<2)
	{
		if (g_tP2pManagerStatus.HaveOnChainReq())
		{

			GetHyperBlockByNo(g_tP2pManagerStatus.GetMaxBlockNum() + 1);
			
			char logBuf[BUFFERLEN] = { 0 };
			sprintf(logBuf, "now downloading Hyperblock [%] from Hyperchain network.", g_tP2pManagerStatus.GetMaxBlockNum() + 1);
			m_qtnotify->SetStatusMes(logBuf);
			i++;
			SLEEP(1 * ONE_SECOND);
		}
		else
		{
			break;
		}
	}

}

void CHChainP2PManager::WriteBlockLog(T_HYPERBLOCK hyperBlock)
{
	//open status file
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


string CHChainP2PManager::getBlockInfo(T_HYPERBLOCK hyperBlock)
{

	string retData = "";
	char buf[BUFFERLEN] = { 0 };
	retData += "==================BLOCKINFO==================\n";

	{
		retData += "Block No:	";
		memset(buf, 0, BUFFERLEN);
		sprintf(buf, "%d\n", hyperBlock.GetBlockBaseInfo().GetID());
		retData += buf;

		retData += "Block Hash:	";
		memset(buf, 0, BUFFERLEN);
		retData += "0x";
		CCommonStruct::Hash256ToStr(buf, &hyperBlock.GetBlockBaseInfo().GetHashSelf());
		retData += buf;
		retData += "\n";

		retData += "Time:		";
		memset(buf, 0, BUFFERLEN);
		struct tm * t;	
		uint64 time = hyperBlock.GetBlockBaseInfo().GetTime();
		t = localtime((time_t*)&(time));
		sprintf(buf, "%d-%d-%d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
		retData += buf;

		retData += "Previous Hash:	";
		retData += "0x";
		memset(buf, 0, BUFFERLEN);
		CCommonStruct::Hash256ToStr(buf, &hyperBlock.GetBlockBaseInfo().GetPreHash());
		retData += buf;
		retData += "\n";
		retData += "Extra:		";
		retData += "Payload:\n";
		retData += "\n";

		uint16 chainNum = 0;
		
		list<LIST_T_LOCALBLOCK>::iterator subItr = hyperBlock.GetlistPayLoad().begin();
		for (; subItr != hyperBlock.GetlistPayLoad().end(); subItr++)		
		{
			chainNum += 1;
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);

				T_PPRIVATEBLOCK privateBlock = &(localBlock->GetPayLoad());
				T_FILEINFO &fileInfo = privateBlock->GetPayLoad();

				retData += "HyperBlockHash:	";
				memset(buf, 0, BUFFERLEN);
				retData += "0x";
				CCommonStruct::Hash256ToStr(buf, &hyperBlock.GetBlockBaseInfo().GetHashSelf());
				retData += buf;
				retData += "";

				retData += "AtChainNum:";
				memset(buf, 0, BUFFERLEN);
				sprintf(buf, "%d ", chainNum);
				retData += buf;

				retData += "LocalBlockHash:";
				memset(buf, 0, BUFFERLEN);
				retData += "0x";
				CCommonStruct::Hash256ToStr(buf, &localBlock->GetBlockBaseInfo().GetHashSelf());
				retData += buf;
				retData += "";

				retData += "FileName=";
				retData += fileInfo.GetFileName();
				retData += "";

				retData += "CustomInfo=";
				retData += fileInfo.GetCustomInfo();
				retData += "";


				retData += "FileHash=";
				memset(buf, 0, BUFFERLEN);
				CCommonStruct::Hash512ToStr(buf, &fileInfo.GetFileHash());
				retData += buf;
				retData += "";

				retData += "FileOwner=";
				retData += fileInfo.GetFileOwner();
				retData += "";

				retData += "FileSize=";
				memset(buf, 0, BUFFERLEN);
				sprintf(buf, "%d\n", fileInfo.GetFileSize());
				retData += buf;

				retData += "\n";

			}
		}

	}

	retData += "================================================\n";
	return retData;
}

void CHChainP2PManager::SendLoginReq(uint32 uiIP, uint16 usPort)
{
	T_P2PPROTOCOLPINGREQ tPingReq;

	struct timeval timePtr;
	CCommonStruct::gettimeofday_update(&timePtr);
	tPingReq.SetP2pprotocolpingreq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_PING_REQ, timePtr.tv_sec), T_PEERADDRESS(g_confFile.GetLocalIP(), g_confFile.GetLocalPort()),
		m_MyPeerInfo.GetName(), g_tP2pManagerStatus.GetMaxBlockNum(), g_tP2pManagerStatus.GetNodeState());

	string ipString = GetIpString(uiIP);

	log_info(g_pLogHelper, "CHChainP2PManager::SendLoginReq TO ip:%s port:%d", (char*)ipString.data(), usPort);

	m_UdpSocket.Send(uiIP, usPort, (char*)&tPingReq, sizeof(tPingReq));
}

void CHChainP2PManager::GetHyperBlockByNo(uint64 blockNum)
{
	struct timeval timePtr;
	CCommonStruct::gettimeofday_update(&timePtr);
	T_P2PPROTOCOLGETHYPERBLOCKBYNOREQ tGetHyperBlockByNoReq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_GET_HYPERBLOCK_BY_NO_REQ, timePtr.tv_sec), blockNum);
	SendDataToPeer((char*)&tGetHyperBlockByNoReq, sizeof(tGetHyperBlockByNoReq));
}


void CHChainP2PManager::copyLocalBuddyList(LIST_T_LOCALCONSENSUS &endList, LIST_T_LOCALCONSENSUS fromList)
{
	ITR_LIST_T_LOCALCONSENSUS itrList = fromList.begin();
	for (; itrList != fromList.end(); itrList++)
	{
		T_LOCALCONSENSUS tempBlock;
		tempBlock.SetLoaclConsensus((*itrList).GetPeer(), (*itrList).GetLocalBlock());
		endList.push_back(tempBlock);
	}
}

void CHChainP2PManager::ChangeLocalBlockPreHash(LIST_T_LOCALCONSENSUS &localList)
{
	int localSize = localList.size();
	T_PLOCALCONSENSUS localInfo;
	ITR_LIST_T_LOCALCONSENSUS itr = localList.begin();
	ITR_LIST_T_LOCALCONSENSUS itrNext = itr++;
	itrNext->GetLocalBlock().GetBlockBaseInfo().GetPreHash().SetInit(0);

	uint16 num = 1;
	while (num < localSize)
	{
		(*itr).GetLocalBlock().GetBlockBaseInfo().SetPreHash((*itrNext).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
		itr++;
		itrNext++;
		num += 1;
	}
}

void CHChainP2PManager::GetOnChainInfo()
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	{
		if (g_tP2pManagerStatus.listOnChainReq.empty())
		{
			m_qtnotify->SetNodeStatus(IDLE);
			g_tP2pManagerStatus.tBuddyInfo.eBuddyState = IDLE;

			return;
		}

		T_LOCALCONSENSUS onChainInfo;
		while (1)
		{
			onChainInfo = g_tP2pManagerStatus.listOnChainReq.front();
			g_tP2pManagerStatus.listOnChainReq.pop_front();

			if ((onChainInfo.uiRetryTime >= 3))
			{
				m_qtnotify->SetBuddyFailed(onChainInfo.GetFileHash(), onChainInfo.GetLocalBlock().GetBlockBaseInfo().GetTime());
				if (g_tP2pManagerStatus.listOnChainReq.empty())
				{
					m_qtnotify->SetNodeStatus(IDLE);
					g_tP2pManagerStatus.tBuddyInfo.eBuddyState = IDLE;
					return;
				}
			}
			else
			{
				break;
			}
		}

		g_tP2pManagerStatus.listLocalBuddyChainInfo.push_back(onChainInfo);
		g_tP2pManagerStatus.listLocalBuddyChainInfo.sort(CmpareOnChain());
		ChangeLocalBlockPreHash(g_tP2pManagerStatus.listLocalBuddyChainInfo);
		m_qtnotify->SetLocalBuddyChainInfo(g_tP2pManagerStatus.listLocalBuddyChainInfo);
		g_tP2pManagerStatus.tBuddyInfo.usBlockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();

		g_tP2pManagerStatus.tBuddyInfo.uiCurBuddyNo = g_tP2pManagerStatus.GetMaxBlockNum() + 1;
		g_tP2pManagerStatus.tBuddyInfo.eBuddyState = LOCAL_BUDDY;
		g_tP2pManagerStatus.uiNodeState = CONFIRMING;

		g_tP2pManagerStatus.curBuddyBlock = onChainInfo;

		char localHash[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &onChainInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

		char logBuf[BUFFERLEN] = { 0 };
		sprintf(logBuf, "[%s],[%s],[%s], now trying register to Blockchain", onChainInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName(),
			onChainInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileOwner(), localHash);
		m_qtnotify->SetStatusMes(logBuf);

		g_tP2pManagerStatus.tOnChainHashInfo.strHash = onChainInfo.GetFileHash();
		g_tP2pManagerStatus.tOnChainHashInfo.uiTime = onChainInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileCreateTime();
	}

	g_tP2pManagerStatus.bHaveOnChainReq = true;

	return;
}

void CHChainP2PManager::SendOneHyperBlockByNo(uint32 uiIP, uint16 usPort, uint64 uiBlockNum)
{

	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		return;
	}

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		if ((*itr).GetBlockBaseInfo().GetID() == uiBlockNum)
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

			struct timeval timeTemp;
			CCommonStruct::gettimeofday_update(&timeTemp);

			pP2pProtocolCopyHyperBlockReq->SetType(_tp2pprotocoltype(P2P_PROTOCOL_COPY_HYPER_BLOCK_REQ, timeTemp.tv_sec));
			pP2pProtocolCopyHyperBlockReq->SetBlockCount(blockNum);
			pP2pProtocolCopyHyperBlockReq->SetBlockNum((*itr).GetBlockBaseInfo().GetID());
			pP2pProtocolCopyHyperBlockReq->SetChainCount((*itr).listPayLoad.size());
			pP2pProtocolCopyHyperBlockReq->SetPeerAddr(m_MyPeerInfo.GetPeerInfoByMyself());

			T_PHYPERBLOCKSEND pHyperBlockSend;
			pHyperBlockSend = (T_PHYPERBLOCKSEND)(pP2pProtocolCopyHyperBlockReq + 1);
			pHyperBlockSend->SetBlockBaseInfo((*itr).GetBlockBaseInfo());
			pHyperBlockSend->SetHashAll((*itr).GetHashAll());

			T_PLOCALBLOCK pPeerInfos;
			if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
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
					pPeerInfos[i].SetBlockBaseInfo((*subItrH).GetBlockBaseInfo());
					pPeerInfos[i].SetHHash((*subItrH).GetHHash());
					pPeerInfos[i].SetAtChainNum(chainNum);
					pPeerInfos[i].SetPayLoad((*subItrH).GetPayLoad());
					i++;
				}
			}

			m_UdpSocket.Send(uiIP, usPort, (char*)pP2pProtocolCopyHyperBlockReq, ipP2pProtocolCopyHyperBlockReqLen);

			free(pP2pProtocolCopyHyperBlockReq);
			pP2pProtocolCopyHyperBlockReq = NULL;
		}
	}
}


void CHChainP2PManager::SendDataToPeer(char *buf, uint32 bufLen)
{
	int index1 = 0;
	CAutoMutexLock muxAutoPeerList(m_MuxPeerInfoList);
	int tempSendPeerNum = g_tP2pManagerStatus.GetBuddyPeerCount();
	int peerNum = m_PeerInfoList.size();
	if (peerNum < g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		tempSendPeerNum = peerNum;
	}

	ITR_LIST_T_PPEERINFO itrAdd = m_PeerInfoList.begin();
	for (; itrAdd != m_PeerInfoList.end(); itrAdd++)
	{
		if (index1 < tempSendPeerNum)
		{

			string ipString = (*itrAdd)->GetPeerInfoByOther().GetIPString();

			if ((*itrAdd)->GetPeerInfoByOther().GetIP() != m_MyPeerInfo.GetPeerInfoByOther().GetIP())
			{
				m_UdpSocket.Send((*itrAdd)->GetPeerInfoByOther().GetIP(), (*itrAdd)->GetPeerInfoByOther().GetPort(), buf, bufLen);
			}
			index1++;
		}
		else
			break;
	}
}

void CHChainP2PManager::StartGlobalBuddy()
{
	g_tP2pManagerStatus.SetStartGlobalFlag(true);
	g_tP2pManagerStatus.SetGlobalBuddyChainCount(1);
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistRecvLocalBuddyReq);
	g_tP2pManagerStatus.listRecvLocalBuddyReq.clear();

	CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	if (g_tP2pManagerStatus.listLocalBuddyChainInfo.size() == 0 || g_tP2pManagerStatus.listLocalBuddyChainInfo.size() == 1)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::StartGlobalBuddy listLocalBuddyChainInfo.size=0 || listLocalBuddyChainInfo.size=1, not start globalBuddy");
		return;
	}
	LIST_T_LOCALCONSENSUS::iterator itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.end();
	itr--;
	if (((*itr).GetPeer().GetPeerAddr().GetIP() == m_MyPeerInfo.GetPeerInfoByMyself().GetIP()) && ((*itr).GetPeer().GetPeerAddr().GetPort() == m_MyPeerInfo.GetPeerInfoByMyself().GetPort()))
	{
		T_PP2PPROTOCOLGLOBALBUDDYREQ pP2pProtocolGlobalBuddyReq = NULL;

		uint8 blockNum = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
		int ipP2pProtocolGlobalBuddyReqLen = sizeof(T_P2PPROTOCOLGLOBALBUDDYREQ) + blockNum * sizeof(T_GLOBALCONSENSUS);
		pP2pProtocolGlobalBuddyReq = (T_PP2PPROTOCOLGLOBALBUDDYREQ)malloc(ipP2pProtocolGlobalBuddyReqLen);	
		struct timeval timeTemp;
		CCommonStruct::gettimeofday_update(&timeTemp);
		pP2pProtocolGlobalBuddyReq->SetP2pprotocolglobalconsensusreq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_GLOBAL_BUDDY_REQ, timeTemp.tv_sec), m_MyPeerInfo.GetPeerInfoByMyself(),
			blockNum, g_tP2pManagerStatus.GetGlobalBuddyChainCount());

		T_PGLOBALCONSENSUS pPeerInfos;
		if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
			pPeerInfos = NULL;
		else
			pPeerInfos = (T_PGLOBALCONSENSUS)(pP2pProtocolGlobalBuddyReq + 1);

		int i = 0;
		ITR_LIST_T_LOCALCONSENSUS itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
		for (; itr != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itr++)
		{
			pPeerInfos[i].SetGlobalconsenus((*itr).GetPeer(), (*itr).GetLocalBlock(), 1);
			i++;

			char localHash[FILESIZES] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &(*itr).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
		}

		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
		g_tP2pManagerStatus.listGlobalBuddyChainInfo.push_back(g_tP2pManagerStatus.listLocalBuddyChainInfo);
		g_tP2pManagerStatus.tBuddyInfo.usChainNum = g_tP2pManagerStatus.listGlobalBuddyChainInfo.size();

		SendDataToPeer((char*)pP2pProtocolGlobalBuddyReq, ipP2pProtocolGlobalBuddyReqLen);


		free(pP2pProtocolGlobalBuddyReq);
		pP2pProtocolGlobalBuddyReq = NULL;
	}
}




void CHChainP2PManager::SendCopyLocalBlock(T_LOCALCONSENSUS localBlock)
{

	log_info(g_pLogHelper, "CHChainP2PManager::SendCopyLocalBlock ..........");
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistLocalBuddyChainInfo);
	uint8 nodeSize = g_tP2pManagerStatus.listLocalBuddyChainInfo.size();
	if (nodeSize > NOT_START_BUDDY_NUM)
	{
		T_PP2PPROTOCOLCOPYBLOCKREQ pP2pProtocolCopyBlockReq = NULL;
		int ipP2pProtocolCopyBlockReqLen = sizeof(T_P2PPROTOCOLCOPYBLOCKREQ) + sizeof(T_LOCALCONSENSUS);
		pP2pProtocolCopyBlockReq = (T_PP2PPROTOCOLCOPYBLOCKREQ)malloc(ipP2pProtocolCopyBlockReqLen);
		struct timeval timeTemp;
		CCommonStruct::gettimeofday_update(&timeTemp);
		pP2pProtocolCopyBlockReq->SetType(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_COPY_BLOCK_REQ, timeTemp.tv_sec));


		T_PLOCALCONSENSUS pLocalBlockInfo = (T_PLOCALCONSENSUS)(pP2pProtocolCopyBlockReq + 1);
		pLocalBlockInfo->SetLocalBlock(localBlock.GetLocalBlock());
		pLocalBlockInfo->SetPeer(localBlock.GetPeer());


		ITR_LIST_T_LOCALCONSENSUS itr = g_tP2pManagerStatus.listLocalBuddyChainInfo.begin();
		for (; itr != g_tP2pManagerStatus.listLocalBuddyChainInfo.end(); itr++)
		{
			if ((*itr).tLocalBlock.GetBlockBaseInfo().GetHashSelf() == localBlock.GetLocalBlock().GetBlockBaseInfo().GetHashSelf())
			{
				continue;
			}

			string ipString = (*itr).GetPeer().GetPeerAddr().GetIPString();

			char localHash[FILESIZES] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localBlock.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

			log_info(g_pLogHelper, "CHChainP2PManager::SendCopyLocalBlock send newLocalBlock[%s][%s] to IP=%s PORT=%d", localBlock.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName(),
				localHash, (char*)ipString.c_str(), (*itr).GetPeer().GetPeerAddr().GetPort());

			m_UdpSocket.Send((*itr).GetPeer().GetPeerAddr().GetIP(), (*itr).GetPeer().GetPeerAddr().GetPort(), (char*)pP2pProtocolCopyBlockReq, ipP2pProtocolCopyBlockReqLen);
		}

		free(pP2pProtocolCopyBlockReq);
		pP2pProtocolCopyBlockReq = NULL;
	}
}



bool CHChainP2PManager::JudgExistAtLocalBuddy(LIST_T_LOCALCONSENSUS localList, T_LOCALCONSENSUS localBlockInfo)
{
	bool index = false;
	ITR_LIST_T_LOCALCONSENSUS itrList = localList.begin();
	for (; itrList != localList.end(); itrList++)
	{
		if (((*itrList).GetPeer().GetPeerAddr().GetIP() == localBlockInfo.GetPeer().GetPeerAddr().GetIP())
			&& ((*itrList).GetPeer().GetPeerAddrOut().GetIP() == localBlockInfo.GetPeer().GetPeerAddrOut().GetIP())
			&& (*itrList).GetLocalBlock().GetBlockBaseInfo().GetHashSelf() == localBlockInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf())
		{
			char localHash[FILESIZES] = { 0 };
			CCommonStruct::Hash256ToStr(localHash, &localBlockInfo.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());

			index = true;
			break;
		}
	}

	return index;
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
		char startHash[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(startHash, &(*subItrStart).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
		char endHash[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(endHash, &(*subItrEnd).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
			num, startHash, (*subItrStart).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName(), endHash, (*subItrEnd).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName());

		char startHashtemp[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(startHashtemp, &(*localStart).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
		char endHashtemp[FILESIZES] = { 0 };
		CCommonStruct::Hash256ToStr(endHashtemp, &(*localEnd).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
			startHashtemp, (*localStart).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName(), endHashtemp, (*localEnd).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName());

		if ((localStart->GetLocalBlock().GetBlockBaseInfo().GetHashSelf() == subItrStart->GetLocalBlock().GetBlockBaseInfo().GetHashSelf())
			&& (localEnd->tLocalBlock.GetBlockBaseInfo().GetHashSelf() == subItrEnd->GetLocalBlock().GetBlockBaseInfo().GetHashSelf()))
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
			g_tP2pManagerStatus.tBuddyInfo.usChainNum = g_tP2pManagerStatus.listGlobalBuddyChainInfo.size();
			g_tP2pManagerStatus.listGlobalBuddyChainInfo.sort(CmpareGlobalBuddy());
		}
	}

	return index;
}



void CHChainP2PManager::SaveHyperBlockToLocal(T_HYPERBLOCK tHyperBlock)
{
	T_HYPERBLOCKDBINFO hyperBlockInfo(tHyperBlock.GetHashAll(), tHyperBlock.GetBlockBaseInfo().GetHashSelf(), tHyperBlock.GetBlockBaseInfo().GetHashSelf(), tHyperBlock.GetBlockBaseInfo().GetPreHash(), "", tHyperBlock.GetBlockBaseInfo().GetScript(), 
		tHyperBlock.GetBlockBaseInfo().GetAuth(), HYPER_BLOCK, tHyperBlock.GetBlockBaseInfo().GetID(), tHyperBlock.GetBlockBaseInfo().GetID(), tHyperBlock.GetBlockBaseInfo().GetTime(), 0);

	CHyperchainDB::saveHyperBlockToDB(hyperBlockInfo);
}

void CHChainP2PManager::SaveLocalBlockToLocal(T_HYPERBLOCK tHyperBlock)
{
	uint64 blockNum = 0;
	list<LIST_T_LOCALBLOCK>::iterator subItr = tHyperBlock.GetlistPayLoad().begin();
	for (; subItr != tHyperBlock.GetlistPayLoad().end(); subItr++)
	{
		
		list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
		for (; ssubItr != (*subItr).end(); ssubItr++)
		{
			blockNum += 1;
			string spayload;
			string fileTest = (*ssubItr).GetPayLoad().GetPayLoad().GetFileName();

			if (0 == fileTest.compare("test_server"))
			{
				spayload = (*ssubItr).GetPayLoad().GetPayLoad().GetCustomInfo();		
			}
			else
			{
				char strBuf[MAX_BUF_LEN];
				memset(strBuf, 0, MAX_BUF_LEN);
				CCommonStruct::Hash512ToStr(strBuf, &(*ssubItr).GetPayLoad().GetPayLoad().GetFileHash());
				char buf[MAX_BUF_LEN];
				memset(buf, 0, MAX_BUF_LEN);;
				memcpy(buf, strBuf, MAX_BUF_LEN);
				string tempBuf = buf;
				spayload += "fileHash=";
				spayload += tempBuf;
				spayload += "fileName=";
				spayload += (*ssubItr).tPayLoad.GetPayLoad().GetFileName();
				spayload += "fileCustom=";
				spayload += (*ssubItr).tPayLoad.GetPayLoad().GetCustomInfo();				
				spayload += "fileOwner=";
				spayload += (*ssubItr).tPayLoad.GetPayLoad().GetFileOwner();
				spayload += "fileSize=";			
				sprintf(buf, "%d", (*ssubItr).GetPayLoad().GetPayLoad().GetFileSize());
				spayload += buf;		
			
			}

			T_HYPERBLOCKDBINFO hyperBlockInfo(T_SHA256(0), tHyperBlock.GetBlockBaseInfo().GetHashSelf(), (*ssubItr).GetBlockBaseInfo().GetHashSelf(), (*ssubItr).GetBlockBaseInfo().GetPreHash(), spayload,
				(*ssubItr).GetBlockBaseInfo().GetScript(), (*ssubItr).GetBlockBaseInfo().GetAuth(), LOCAL_BLOCK, blockNum, tHyperBlock.GetBlockBaseInfo().GetID(), (*ssubItr).GetBlockBaseInfo().GetTime(), (*ssubItr).GetAtChainNum(), 0);

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
	if ((*itr).GetPeer().GetPeerAddr() == m_MyPeerInfo.GetPeerInfoByMyself())
	{
		log_info(g_pLogHelper, "CHChainP2PManager::CreatHyperBlock I am not the last node");
		isEndNode = true;
	}

	if (isEndNode)
	{
		log_info(g_pLogHelper, "CHChainP2PManager::CreatHyperBlock I am the last node,make the hyperblock, time>(3*60)");

		char logBuf[BUFFERLEN] = { 0 };
		m_qtnotify->SetStatusMes("node now has joined the new Hyperblock build session... ");

		T_HYPERBLOCK tHyperChainBlock;
		T_BLOCKBASEINFO BlockBaseInfo;
		BlockBaseInfo.SetPreHash(g_tP2pManagerStatus.tPreHyperBlock.GetBlockBaseInfo().GetHashSelf());
		BlockBaseInfo.SetBlockBaseInfo((g_tP2pManagerStatus.GetMaxBlockNum() + 1), (g_tP2pManagerStatus.GetStartTimeOfConsensus() + NEXTBUDDYTIME), (char*)AUTHKEY, (char*)BUDDYSCRIPT);
		tHyperChainBlock.SetBlockBaseInfo(BlockBaseInfo);

		uint16 blockNum = 0;
		LIST_T_LOCALBLOCK listLocalBlockInfo;
		list<LIST_T_LOCALBLOCK> listPayLoad;	
		CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistGlobalBuddyChainInfo);
		ITR_LIST_LIST_GLOBALBUDDYINFO itr = g_tP2pManagerStatus.listGlobalBuddyChainInfo.begin();
		for (; itr != g_tP2pManagerStatus.listGlobalBuddyChainInfo.end(); itr++)
		{
			ITR_LIST_T_LOCALCONSENSUS subItr = (*itr).begin();
			for (; subItr != (*itr).end(); subItr++)
			{
				blockNum += 1;
				listLocalBlockInfo.push_back((*subItr).tLocalBlock);	

				char localHash[FILESIZEL] = { 0 };
				CCommonStruct::Hash256ToStr(localHash, &(*subItr).GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
					tHyperChainBlock.GetBlockBaseInfo().GetID(), localHash, (*subItr).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileName());
			}

			listLocalBlockInfo.sort(CmpareOnChainLocal());
			listPayLoad.push_back(listLocalBlockInfo);			
			listLocalBlockInfo.clear();
		}
		tHyperChainBlock.SetlistPayLoad(listPayLoad);

		T_SHA256 HashAll(0);
		tHyperChainBlock.SetHashAll(HashAll);
		GetSHA256(tHyperChainBlock.GetBlockBaseInfo().GetHashSelf().pID, (const char*)(&tHyperChainBlock), sizeof(tHyperChainBlock));



		m_qtnotify->SetHyperBlock(g_tP2pManagerStatus.tOnChainHashInfo.strHash, g_tP2pManagerStatus.tOnChainHashInfo.uiTime, tHyperChainBlock.tBlockBaseInfo.GetID());

		CAutoMutexLock muxAuto1(m_MuxHchainBlockList);
		m_HchainBlockList.push_back(tHyperChainBlock);
		m_HchainBlockListNew.push_back(T_HYPERBLOCKNEW(tHyperChainBlock));
		char pszPeerIP1[MAX_IP_LEN] = { 0 };
		struct in_addr addPeerIP1;
		addPeerIP1.s_addr = m_MyPeerInfo.GetPeerInfoByMyself().GetIP();
		strcpy(pszPeerIP1, inet_ntoa(addPeerIP1));
		{
			SaveHyperBlockToLocal(tHyperChainBlock);
			SaveLocalBlockToLocal(tHyperChainBlock);
		}
		WriteBlockLog(tHyperChainBlock);

		if (0 != strcmp(g_tP2pManagerStatus.curBuddyBlock.GetID(), ""))
		{
			T_SEARCHINFO searchInfo(tHyperChainBlock.GetBlockBaseInfo().GetID(), time(NULL));	
			CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxMapSearchOnChain);
			g_tP2pManagerStatus.mapSearchOnChain[g_tP2pManagerStatus.curBuddyBlock.GetID()] = searchInfo;
		}

		char localHash[FILESIZEL] = { 0 };
		CCommonStruct::Hash256ToStr(localHash, &tHyperChainBlock.GetBlockBaseInfo().GetHashSelf());

		T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR(m_MyPeerInfo.GetPeerInfoByMyself(), m_MyPeerInfo.GetPeerInfoByOther());

		LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
		listPblockStateAddr.push_back(pBlockStateAddr);
		m_BlockStateMap[tHyperChainBlock.GetBlockBaseInfo().GetID()] = listPblockStateAddr;

		string ipStringIn = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();
		string ipStringOut = m_MyPeerInfo.GetPeerInfoByOther().GetIPString();

		log_info(g_pLogHelper, "CHChainP2PManager::CreatHyperBlock ADD (INIP=%s INPORT=%d | OUTIP=%s OUTPORT=%d) to hyperBlockState[%d]",
			(char*)ipStringIn.c_str(), m_MyPeerInfo.GetPeerInfoByMyself().GetPort(), (char*)ipStringOut.c_str(), m_MyPeerInfo.GetPeerInfoByOther().GetPort(), tHyperChainBlock.GetBlockBaseInfo().GetID());

		g_tP2pManagerStatus.SetP2pmanagerstatus(tHyperChainBlock, g_tP2pManagerStatus.GetMaxBlockNum() + 1, true, true, false, false, tHyperChainBlock.GetBlockBaseInfo().GetTime(),
			time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus(), g_tP2pManagerStatus.GetSendConfirmingRegisReqNum() - 1, CONFIRMED);

		T_PP2PPROTOCOLCOPYHYPERBLOCKREQ pP2pProtocolCopyHyperBlockReq = NULL;
		blockNum = 0;
		list<LIST_T_LOCALBLOCK>::iterator itrH = tHyperChainBlock.GetlistPayLoad().begin();
		for (; itrH != tHyperChainBlock.GetlistPayLoad().end(); itrH++)
		{
			{
				blockNum += (*itrH).size();
			}
		}

		int ipP2pProtocolCopyHyperBlockReqLen = sizeof(T_P2PPROTOCOLCOPYHYPERBLOCKREQ) + sizeof(T_HYPERBLOCKSEND) + blockNum * sizeof(T_LOCALBLOCK);
		pP2pProtocolCopyHyperBlockReq = (T_PP2PPROTOCOLCOPYHYPERBLOCKREQ)malloc(ipP2pProtocolCopyHyperBlockReqLen);
		T_P2PPROTOCOLTYPE pType;
		struct timeval timeTemp;
		CCommonStruct::gettimeofday_update(&timeTemp);
		pType.SetP2pprotocoltype(P2P_PROTOCOL_COPY_HYPER_BLOCK_REQ, timeTemp.tv_sec);
		T_PEERADDRESS pPeerAddr(m_MyPeerInfo.GetPeerInfoByMyself().GetIP(), m_MyPeerInfo.GetPeerInfoByMyself().GetPort());
		pP2pProtocolCopyHyperBlockReq->SetP2pprotocolcopyhyperblockreq(pType, pPeerAddr, tHyperChainBlock.GetBlockBaseInfo().GetID(), 3, blockNum, tHyperChainBlock.GetlistPayLoad().size());

		T_PHYPERBLOCKSEND pHyperBlockSend;
		pHyperBlockSend = (T_PHYPERBLOCKSEND)(pP2pProtocolCopyHyperBlockReq + 1);
		pHyperBlockSend->SetHyperBlockSend(tHyperChainBlock.GetBlockBaseInfo(), tHyperChainBlock.GetHashAll());

		T_PLOCALBLOCK pPeerInfos;
		if (0 == g_tP2pManagerStatus.usBuddyPeerCount)
			pPeerInfos = NULL;
		else
			pPeerInfos = (T_PLOCALBLOCK)(pHyperBlockSend + 1);

		uint8 chainNum = 0;
		int i = 0;
		itrH = tHyperChainBlock.listPayLoad.begin();
		for (; itrH != tHyperChainBlock.listPayLoad.end(); itrH++)
		{
			chainNum++;
			ITR_LIST_T_LOCALBLOCK subItrH = (*itrH).begin();
			for (; subItrH != (*itrH).end(); subItrH++)
			{
				pPeerInfos[i].SetLocalBlock((*subItrH).GetBlockBaseInfo(), (*subItrH).GetHHash(), chainNum, (*subItrH).GetPayLoad());
				i++;
			}
		}

		log_info(g_pLogHelper, "CHChainP2PManager::CreatHyperBlock sync hyperblock to other node");
		SendDataToPeer((char*)pP2pProtocolCopyHyperBlockReq, ipP2pProtocolCopyHyperBlockReqLen);

		free(pP2pProtocolCopyHyperBlockReq);
		pP2pProtocolCopyHyperBlockReq = NULL;
	}

	return isEndNode;
}


uint16 CHChainP2PManager::HyperBlockInListOrNot(uint64 blockNum, uint64 blockCount, T_SHA256 tHashSelf)
{
	uint16 retNum = DEFAULT_ERROR_NO;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	CAutoMutexLock muxAutoMap(m_MuxBlockStateMap);
	ITR_LIST_T_HYPERBLOCK itrList = m_HchainBlockList.begin();
	for (; itrList != m_HchainBlockList.end(); itrList++)
	{
		if ((*itrList).GetBlockBaseInfo().GetID() == blockNum)
		{
			{
				uint8 oldHyperLocalBlockNum = 0;
				list<LIST_T_LOCALBLOCK>::iterator itrH = (*itrList).GetlistPayLoad().begin();
				for (; itrH != (*itrList).GetlistPayLoad().end(); itrH++)
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

bool CHChainP2PManager::CurBuddyBlockInTheHyperBlock(T_HYPERBLOCK blockInfos)
{
	bool index = false;
	list<LIST_T_LOCALBLOCK>::iterator itr = blockInfos.GetlistPayLoad().begin();
	for (; itr != blockInfos.GetlistPayLoad().end(); itr++)
	{
		ITR_LIST_T_LOCALBLOCK subItr = itr->begin();
		for (subItr; subItr != itr->end(); subItr++)
		{
			if (subItr->GetBlockBaseInfo().GetHashSelf() == g_tP2pManagerStatus.curBuddyBlock.GetLocalBlock().GetBlockBaseInfo().GetHashSelf())
			{
				index = true;
				char localHash[FILESIZES] = { 0 };
				CCommonStruct::Hash256ToStr(localHash, &g_tP2pManagerStatus.curBuddyBlock.GetLocalBlock().GetBlockBaseInfo().GetHashSelf());
				if (0 != strcmp(g_tP2pManagerStatus.curBuddyBlock.GetID(), ""))
				{
					T_SEARCHINFO searchInfo;
					searchInfo.Set(blockInfos.GetBlockBaseInfo().GetID(), time(NULL));
	
					CAutoMutexLock muxAuto1(g_tP2pManagerStatus.MuxMapSearchOnChain);
					g_tP2pManagerStatus.mapSearchOnChain[g_tP2pManagerStatus.curBuddyBlock.GetID()] = searchInfo;
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




void CHChainP2PManager::GreateGenesisBlock()
{
	log_info(g_pLogHelper, "CHChainP2PManager::Create Originate Hyper Block");

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	CAutoMutexLock muxAuto1(m_MuxBlockStateMap);

	T_FILEINFO fileInfo;
	fileInfo.SetFileInfo(0, "filename_start", "custominfo_start", "fileowner_start", T_SHA512(1), 0);

	T_BLOCKBASEINFO PBaseInfo(0, 0, (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(1), T_SHA256(1));
	T_PRIVATEBLOCK tPrivateBlock(PBaseInfo, T_SHA256(1), fileInfo);

	T_BLOCKBASEINFO LBaseInfo(0, 0, (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(1), T_SHA256(1));
	T_LOCALBLOCK tLocalBlock(LBaseInfo, T_SHA256(1), tPrivateBlock);
	

	LIST_T_LOCALBLOCK ListLocalBlock;
	ListLocalBlock.push_back(tLocalBlock);

	T_HYPERBLOCK tHyperChainBlock;
	T_BLOCKBASEINFO HBaseInfo(0, time(NULL), (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(1), T_SHA256(1));
	tHyperChainBlock.SetBlockBaseInfo(HBaseInfo);
	tHyperChainBlock.SetHashAll(T_SHA256(1));
	tHyperChainBlock.PushBack(ListLocalBlock);

	m_HchainBlockList.push_back(tHyperChainBlock);
	m_HchainBlockListNew.push_back(T_HYPERBLOCKNEW(tHyperChainBlock));
	SaveHyperBlockToLocal(tHyperChainBlock);
	WriteBlockLog(tHyperChainBlock);

	T_PBLOCKSTATEADDR pBlockStateAddr = new T_BLOCKSTATEADDR(m_MyPeerInfo.GetPeerInfoByMyself(), m_MyPeerInfo.GetPeerInfoByOther());

	LIST_T_PBLOCKSTATEADDR listPblockStateAddr;
	listPblockStateAddr.push_back(pBlockStateAddr);
	m_BlockStateMap[tLocalBlock.GetBlockBaseInfo().GetID()] = listPblockStateAddr;


	string ipStringIn = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();
	string ipStringOut = m_MyPeerInfo.GetPeerInfoByOther().GetIPString();

	g_tP2pManagerStatus.SetPreHyperBlock(tHyperChainBlock);
	g_tP2pManagerStatus.SetStartTimeOfConsensus(tHyperChainBlock.GetBlockBaseInfo().GetTime());
	g_tP2pManagerStatus.SetNextStartTimeNewest(g_tP2pManagerStatus.GetStartTimeOfConsensus());
	g_tP2pManagerStatus.SetHaveOnChainReq(false);

}

void CHChainP2PManager::AddNewBlock(TEVIDENCEINFO tFileInfo, const char *rand)
{
	g_tP2pManagerStatus.SetSendRegisReqNum(g_tP2pManagerStatus.GetSendRegisReqNum() + 1);
	g_tP2pManagerStatus.SetSendConfirmingRegisReqNum(g_tP2pManagerStatus.GetSendConfirmingRegisReqNum() + 1);

	T_FILEINFO pFileInfo;
	pFileInfo.SetFileInfo(tFileInfo.GetFileSize(), tFileInfo.GetFileName().c_str(), tFileInfo.GetRightOwner().c_str(), tFileInfo.GetRegisTime());
	


	if (0 == strcmp(pFileInfo.GetFileName(), "Proxy_Sever"))
	{			
		pFileInfo.SetCustomInfo(tFileInfo.GetFileHash().c_str());
	}
	else
	{	
		pFileInfo.SetCustomInfo(tFileInfo.GetCustomInfo().c_str());
		char tempBuf[DEF_SHA512_LEN] = {0};
		strncpy(tempBuf, tFileInfo.GetFileHash().c_str(), DEF_SHA512_LEN);
		CCommonStruct::StrToHash512(pFileInfo.GetFileHash().pID, tempBuf);
	}

	T_BLOCKBASEINFO pBaseInfo(1, time(NULL), (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(0));
	T_PRIVATEBLOCK tPrivateBlock(pBaseInfo, g_tP2pManagerStatus.GetPreHyperBlock().GetBlockBaseInfo().GetHashSelf(), pFileInfo);
	GetSHA256(tPrivateBlock.GetBlockBaseInfo().GetHashSelf().pID, (const char*)(&tPrivateBlock), sizeof(tPrivateBlock));


	T_BLOCKBASEINFO LBaseInfo(1, time(NULL), (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(0));
	T_LOCALBLOCK tLocalBlock(LBaseInfo, g_tP2pManagerStatus.GetPreHyperBlock().GetBlockBaseInfo().GetHashSelf(), 1, tPrivateBlock);
	GetSHA256(tLocalBlock.GetBlockBaseInfo().GetHashSelf().pID, (const char*)(&tLocalBlock), sizeof(tLocalBlock));

	T_LOCALCONSENSUS LocalConsensusInfo(T_BLOCKSTATEADDR(m_MyPeerInfo.GetPeerInfoByMyself(), m_MyPeerInfo.GetPeerInfoByOther()), tLocalBlock, rand, 0, tFileInfo.GetFileHash().c_str());

	ITR_LIST_T_LOCALCONSENSUS itrFind = g_tP2pManagerStatus.GetListOnChainReq().begin();
	for (itrFind; itrFind != g_tP2pManagerStatus.GetListOnChainReq().end(); itrFind++)
	{
		if (((*itrFind).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileHash() == LocalConsensusInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileHash())
			&& ((*itrFind).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileCreateTime() == LocalConsensusInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileCreateTime()))
		{
			(*itrFind).SetRetryTime(0);
		}
	}
	g_tP2pManagerStatus.GetListOnChainReq().push_back(LocalConsensusInfo);	
	g_tP2pManagerStatus.SetSendPoeNum(g_tP2pManagerStatus.GetSendPoeNum() + 1);
	m_qtnotify->SetSendPoeNum(g_tP2pManagerStatus.GetSendPoeNum());

}

void CHChainP2PManager::AddNewBlockEx(TEVIDENCEINFO tFileInfo, const char *rand, string& localhash, string& strtime)
{
	g_tP2pManagerStatus.SetSendRegisReqNum(g_tP2pManagerStatus.GetSendRegisReqNum() + 1);
	g_tP2pManagerStatus.SetSendConfirmingRegisReqNum(g_tP2pManagerStatus.GetSendConfirmingRegisReqNum() + 1);

	log_info(g_pLogHelper, "CHChainP2PManager::AddNewBlock in...");

	T_FILEINFO pFileInfo;
	pFileInfo.SetFileInfo(tFileInfo.GetFileSize(), tFileInfo.GetFileName().c_str(), tFileInfo.GetRightOwner().c_str(), tFileInfo.GetRegisTime());

	if (0 == strcmp(pFileInfo.GetFileName(), "Proxy_Sever"))
	{
		pFileInfo.SetCustomInfo(tFileInfo.GetFileHash().c_str());
	}
	else
	{
		pFileInfo.SetCustomInfo(tFileInfo.GetCustomInfo().c_str());
		char tempBuf[DEF_SHA512_LEN] = { 0 };
		strncpy(tempBuf, tFileInfo.GetFileHash().c_str(), DEF_SHA512_LEN);
		CCommonStruct::StrToHash512(pFileInfo.GetFileHash().pID, tempBuf);
	}

	T_BLOCKBASEINFO pBaseInfo(1, time(NULL), (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(0));
	T_PRIVATEBLOCK tPrivateBlock(pBaseInfo, g_tP2pManagerStatus.GetPreHyperBlock().GetBlockBaseInfo().GetHashSelf(), pFileInfo);
	GetSHA256(tPrivateBlock.GetBlockBaseInfo().GetHashSelf().pID, (const char*)(&tPrivateBlock), sizeof(tPrivateBlock));
	
	T_BLOCKBASEINFO LBaseInfo(1, time(NULL), (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(0));
	T_LOCALBLOCK tLocalBlock(LBaseInfo, g_tP2pManagerStatus.GetPreHyperBlock().GetBlockBaseInfo().GetHashSelf(), 1, tPrivateBlock);
	GetSHA256(tLocalBlock.GetBlockBaseInfo().GetHashSelf().pID, (const char*)(&tLocalBlock), sizeof(tLocalBlock));
	localhash = DBmgr::instance()->hash256tostring(tLocalBlock.GetBlockBaseInfo().GetHashSelf().pID).c_str();
	strtime = to_string(tLocalBlock.GetBlockBaseInfo().GetTime());

	T_LOCALCONSENSUS LocalConsensusInfo(T_BLOCKSTATEADDR(m_MyPeerInfo.GetPeerInfoByMyself(), m_MyPeerInfo.GetPeerInfoByOther()), tLocalBlock, rand, 0, tFileInfo.GetFileHash().c_str());

	ITR_LIST_T_LOCALCONSENSUS itrFind = g_tP2pManagerStatus.GetListOnChainReq().begin();
	for (itrFind; itrFind != g_tP2pManagerStatus.GetListOnChainReq().end(); itrFind++)
	{
		if (((*itrFind).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileHash() == LocalConsensusInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileHash())
			&& ((*itrFind).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileCreateTime() == LocalConsensusInfo.GetLocalBlock().GetPayLoad().GetPayLoad().GetFileCreateTime()))
		{
			(*itrFind).SetRetryTime(0);
		}
	}
	g_tP2pManagerStatus.GetListOnChainReq().push_back(LocalConsensusInfo);
	g_tP2pManagerStatus.SetSendPoeNum(g_tP2pManagerStatus.GetSendPoeNum() + 1);
	m_qtnotify->SetSendPoeNum(g_tP2pManagerStatus.GetSendPoeNum());
}

void CHChainP2PManager::SendOneBlockToPeer(uint32 uiIP, uint16 usPort, uint64 uiBlockNum)
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
		if ((*itr).tBlockBaseInfo.uiID = uiBlockNum)
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
			struct timeval timeTemp;
			CCommonStruct::gettimeofday_update(&timeTemp);
			pP2pProtocolAddBlockReq->SetP2pprotocoladdblockreq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_ADD_BLOCK_REQ, timeTemp.tv_sec), (*itr).GetBlockBaseInfo().GetID(), 3, totalLocalBlockNum);


			T_PHYPERBLOCKSEND pHyperBlock;
			pHyperBlock = (T_PHYPERBLOCKSEND)(pP2pProtocolAddBlockReq + 1);	
			pHyperBlock->SetHyperBlockSend((*itr).GetBlockBaseInfo(), (*itr).GetHashAll());

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
					pSendLocalBlock[i].SetLocalBlock((*localItr).GetBlockBaseInfo(), (*localItr).GetHHash(), chainNum, (*localItr).GetPayLoad());
					i++;
				}
			}

			m_UdpSocket.Send(uiIP, usPort, (char*)pP2pProtocolAddBlockReq, iP2pProtocolAddBlockLen);

			string ipString = GetIpString(uiIP);

			log_info(g_pLogHelper, "CHChainP2PManager::SendOneBlockToPeer send hyperBlock[%d] TO IP=%s PORT=%d", pP2pProtocolAddBlockReq->GetBlockNum(), (char*)ipString.c_str(), usPort);

			free(pP2pProtocolAddBlockReq);
			iP2pProtocolAddBlockLen = NULL;
		}
	}

}

void CHChainP2PManager::SendBlockToPeer(uint32 uiIP, uint16 usPort, uint64 uiBlockNum)
{
	T_PP2PPROTOCOLADDBLOCKREQ pP2pProtocolAddBlockReq = NULL;

	if (0 == g_tP2pManagerStatus.GetBuddyPeerCount())
	{
		return;
	}

	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		if ((*itr).GetBlockBaseInfo().GetID() > uiBlockNum)
		{
			uint64 totalLocalBlockNum = 0;
			list<LIST_T_LOCALBLOCK>::iterator itrSub = itr->GetlistPayLoad().begin();
			for (; itrSub != itr->GetlistPayLoad().end(); itrSub++)
			{
				totalLocalBlockNum += (*itrSub).size();
			}
			uint64 hyperBlockSize = ((totalLocalBlockNum)* sizeof(T_LOCALBLOCK)) + sizeof(T_HYPERBLOCKSEND);

			int iP2pProtocolAddBlockLen = sizeof(T_P2PPROTOCOLADDBLOCKREQ) + hyperBlockSize;

			pP2pProtocolAddBlockReq = (T_PP2PPROTOCOLADDBLOCKREQ)malloc(iP2pProtocolAddBlockLen);
			struct timeval timeTemp;
			CCommonStruct::gettimeofday_update(&timeTemp);
			pP2pProtocolAddBlockReq->SetP2pprotocoladdblockreq(T_P2PPROTOCOLTYPE(P2P_PROTOCOL_ADD_BLOCK_REQ, timeTemp.tv_sec), (*itr).GetBlockBaseInfo().GetID(), 3, totalLocalBlockNum);


			T_PHYPERBLOCKSEND pHyperBlock;
			pHyperBlock = (T_PHYPERBLOCKSEND)(pP2pProtocolAddBlockReq + 1);
			pHyperBlock->SetHyperBlockSend((*itr).GetBlockBaseInfo(), (*itr).GetHashAll());
	

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
					pSendLocalBlock[i].SetLocalBlock((*localItr).GetBlockBaseInfo(), (*localItr).GetHHash(), chainNum, (*localItr).GetPayLoad());
					i++;
				}
			}

			m_UdpSocket.Send(uiIP, usPort, (char*)pP2pProtocolAddBlockReq, iP2pProtocolAddBlockLen);

			string ipString = GetIpString(uiIP);

			free(pP2pProtocolAddBlockReq);
			iP2pProtocolAddBlockLen = NULL;
		}
	}

}


uint64 CHChainP2PManager::GetLocalLatestBlockNo()
{
	return 0;
}

uint64 CHChainP2PManager::GetLatestHyperBlockNo()
{
	return g_tP2pManagerStatus.GetMaxBlockNum();
}

uint32 CHChainP2PManager::GetSendRegisReqNum()
{

	muLock.lock();
	uint64 recvNum = g_tP2pManagerStatus.GetSendRegisReqNum();
	muLock.unlock();
	return recvNum;
}

uint32 CHChainP2PManager::GetSendConfirmingRegisReqNum()
{
	muLock.lock();
	uint64 recvNum = g_tP2pManagerStatus.GetSendConfirmingRegisReqNum();
	muLock.unlock();
	return recvNum;
}

uint32 CHChainP2PManager::GetRecvRegisReqNum()
{

	muLock.lock();
	uint64 recvNum = g_tP2pManagerStatus.GetRecvRegisReqNum();
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

				T_PPRIVATEBLOCK privateBlock = &(localBlock->GetPayLoad());
				T_PFILEINFO fileInfo = &(privateBlock->GetPayLoad());

				if (0 == pCheckInfo->GetFileName().compare(fileInfo->GetFileName()))
				{
					char strBuf[DEF_SHA512_LEN * 2] = {0};
					CCommonStruct::Hash512ToStr(strBuf, &fileInfo->GetFileHash());						
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
	info = m_MyPeerInfo.GetName();
	uiport = m_MyPeerInfo.GetPeerInfoByMyself().GetPort();

	ip = m_MyPeerInfo.GetPeerInfoByMyself().GetIPString();
}

uint16 CHChainP2PManager::GetStateOfCurrentConsensus(uint64 &blockNo, uint16 &blockNum, uint16 &chainNum)
{
	blockNo = g_tP2pManagerStatus.GetBuddyInfo().GetCurBuddyNo();
	blockNum = g_tP2pManagerStatus.GetBuddyInfo().GetBlockNum();
	chainNum = g_tP2pManagerStatus.GetBuddyInfo().GetChainNum();

	return g_tP2pManagerStatus.GetBuddyInfo().GetBuddyState();
}

uint32 CHChainP2PManager::GetConnectedNodesNum()
{
	CAutoMutexLock muxAuto(m_MuxPeerInfoList);
	uint64 ret = m_PeerInfoList.size();
	return ret;
}

VEC_T_PPEERCONF CHChainP2PManager::GetPeerInfo()
{
	return g_confFile.vecPeerConf;
}

uint32 CHChainP2PManager::GetNodeNum(uint32 uiStart, uint32 uiEnd)
{
	int iNum = g_tP2pManagerStatus.GetBuddyPeerCount();
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
		retNum = (*itr).second.GetHyperID();
		return retNum;
	}

	return retNum;

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
	
			string ipString = (*itr)->GetPeerInfoByOther().GetIPString();

			TNODEINFO nodeInfo((*itr)->GetNodeState(), (char*)ipString.c_str());
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
			string ipString = (*itr)->GetPeerInfoByOther().GetIPString();

			TNODEINFO nodeInfo((*itr)->uiNodeState, (char*)ipString.c_str());
			vecNodeInfo.push_back(nodeInfo);
		}
		index++;
	}

	return vecNodeInfo;
}


uint64 CHChainP2PManager::GetElaspedTimeOfCurrentConsensus()
{
	if (g_tP2pManagerStatus.GetNodeState() == CONFIRMED)
	{
		return g_tP2pManagerStatus.GetTimeOfConsensus();
	}
	else if (g_tP2pManagerStatus.GetNodeState() == CONFIRMING)
	{
		return (time(NULL) - g_tP2pManagerStatus.GetStartTimeOfConsensus());
	}

}

uint64 CHChainP2PManager::GetStartTimeOfConsensus()
{
	return g_tP2pManagerStatus.GetStartTimeOfConsensus();
}


VEC_T_HYPERBLOCKDBINFO CHChainP2PManager::ChainDataPersist()
{
	VEC_T_HYPERBLOCKDBINFO vec;
	CAutoMutexLock muxAuto(m_MuxHchainBlockList);
	ITR_LIST_T_HYPERBLOCK itr = m_HchainBlockList.begin();
	for (; itr != m_HchainBlockList.end(); itr++)
	{
		T_HYPERBLOCKDBINFO tempHyperBlockDbInfo;
		uint64 ReferHyperBlockId = 0;
		if (itr->GetBlockBaseInfo().GetID() != 1)
		{
			ReferHyperBlockId = itr->GetBlockBaseInfo().GetID() - 1;
		}			
		tempHyperBlockDbInfo.Set(itr->GetHashAll(), itr->GetBlockBaseInfo().GetHashSelf(), itr->GetBlockBaseInfo().GetPreHash(),
			"", itr->GetBlockBaseInfo().GetScript(), itr->GetBlockBaseInfo().GetAuth(), HYPER_BLOCK, itr->GetBlockBaseInfo().GetID() ,ReferHyperBlockId, itr->GetBlockBaseInfo().GetTime());
		vec.push_back(tempHyperBlockDbInfo);

		list<LIST_T_LOCALBLOCK>::iterator subItr = itr->listPayLoad.begin();
		for (; subItr != itr->listPayLoad.end(); subItr++)
		{
			list<T_LOCALBLOCK>::iterator ssubItr = (*subItr).begin();
			for (; ssubItr != (*subItr).end(); ssubItr++)
			{
				T_PLOCALBLOCK localBlock = &(*ssubItr);
				T_PPRIVATEBLOCK privateBlock = &(localBlock->GetPayLoad());
				T_PFILEINFO fileInfo = &((privateBlock->GetPayLoad()));
				
				char strBuf[MAX_BUF_LEN + 1] = { 0 };					
				CCommonStruct::Hash512ToStr(strBuf, &fileInfo->GetFileHash());			
				strBuf[MAX_BUF_LEN] = '\0';
				T_SHA256 preHash;
				preHash.SetInit(0);

				T_HYPERBLOCKDBINFO tempHyperBlockDbInfoLocal;
				tempHyperBlockDbInfoLocal.Set(localBlock->GetHHash(), localBlock->GetBlockBaseInfo().GetHashSelf(), preHash, strBuf, localBlock->GetBlockBaseInfo().GetScript(),
					localBlock->GetBlockBaseInfo().GetAuth(), LOCAL_BLOCK, localBlock->GetBlockBaseInfo().GetID(), itr->GetBlockBaseInfo().GetID(), 
					localBlock->GetBlockBaseInfo().GetTime(), localBlock->GetAtChainNum());

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

LIST_T_LOCALCONSENSUS CHChainP2PManager::GetPoeRecordList()
{
	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistOnChainReq);
	return g_tP2pManagerStatus.listOnChainReq;
}

void CHChainP2PManager::GetHyperBlockInfoFromP2P(uint64 start, uint64 num)
{
	if (num > 100)
		num = 100;

	for (int i = 0; i < num; i++)
	{
		GetHyperBlockByNo(start + i);
	}
}
bool CHChainP2PManager::SetQtNotify(QtNotify *pnotify)
{
	m_qtnotify = pnotify;
	return m_qtnotify;
}
void CHChainP2PManager::AddNewScriptBlock(string script,const char *rand)
{
	g_tP2pManagerStatus.SetSendRegisReqNum(g_tP2pManagerStatus.GetSendRegisReqNum() + 1);
	g_tP2pManagerStatus.SetSendConfirmingRegisReqNum(g_tP2pManagerStatus.GetSendConfirmingRegisReqNum() + 1);

	T_FILEINFO pFileInfo;
	pFileInfo.SetFileInfo(BUFFERLEN, "", "", "");

	T_BLOCKBASEINFO pBaseInfo(1, time(NULL), (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(0));
	T_PRIVATEBLOCK tPrivateBlock(pBaseInfo, g_tP2pManagerStatus.GetPreHyperBlock().GetBlockBaseInfo().GetHashSelf(), pFileInfo);
	GetSHA256(tPrivateBlock.GetBlockBaseInfo().GetHashSelf().pID, (const char*)(&tPrivateBlock), sizeof(tPrivateBlock));

	T_LOCALBLOCK tLocalBlock(T_BLOCKBASEINFO(1, time(NULL), (char*)AUTHKEY, (char*)BUDDYSCRIPT, T_SHA256(0)), g_tP2pManagerStatus.GetPreHyperBlock().GetBlockBaseInfo().GetHashSelf(), 1, tPrivateBlock);
	GetSHA256(tLocalBlock.GetBlockBaseInfo().GetHashSelf().pID, (const char*)(&tLocalBlock), sizeof(tLocalBlock));

	T_LOCALCONSENSUS LocalConsensusInfo(T_BLOCKSTATEADDR(m_MyPeerInfo.GetPeerInfoByMyself(), m_MyPeerInfo.GetPeerInfoByOther()), tLocalBlock, rand, 0);
	g_tP2pManagerStatus.GetListOnChainReq().push_back(LocalConsensusInfo);	
	g_tP2pManagerStatus.SetSendPoeNum(g_tP2pManagerStatus.GetSendPoeNum() + 1);
	m_qtnotify->SetSendPoeNum(g_tP2pManagerStatus.GetSendPoeNum());
}

void CHChainP2PManager::DeleteFailedPoe(string script, uint64 time)
{
	T_SHA512 tFileHash;
	char tempBuf[DEF_SHA512_LEN];
	memset(tempBuf, 0, DEF_SHA512_LEN);
	strncpy(tempBuf, script.c_str(), DEF_SHA512_LEN);
	CCommonStruct::StrToHash512(tFileHash.pID, tempBuf);

	CAutoMutexLock muxAuto(g_tP2pManagerStatus.MuxlistOnChainReq);
	ITR_LIST_T_LOCALCONSENSUS itrFind = g_tP2pManagerStatus.listOnChainReq.begin();
	for (itrFind; itrFind != g_tP2pManagerStatus.listOnChainReq.end(); itrFind++)
	{
		if (((*itrFind).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileHash() == tFileHash)
			&& ((*itrFind).GetLocalBlock().GetPayLoad().GetPayLoad().GetFileCreateTime() == time))
		{
			itrFind = g_tP2pManagerStatus.listOnChainReq.erase(itrFind);
			break;
		}
	}
}



