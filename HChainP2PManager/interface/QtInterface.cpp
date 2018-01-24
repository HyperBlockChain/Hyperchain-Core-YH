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

#include "stdio.h"
#include "QtInterface.h"
#include "../HChainP2PManager.h"
#include <thread>
#include "../headers/includeComm.h"
#include "../HChainP2PManager.h"
#include "../headers/commonstruct.h"
#include "../headers/inter_public.h"

#include "../headers/UUFile.h"
#include "../db/dbmgr.h"
#ifdef WIN32
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#endif

#include <QCryptographicHash>
#define MAX_TEMP_BUF_LEN (512)

uint64 gStartTime = time(NULL);
CHChainP2PManager gP2PManager;

UUFile uufileTest;
uint16 g_csvfileNum;

void analysisCsvTest(string ip)
{
	uint64 blockNum = 0;
	while (1)
	{
		for (int i = 0; i <= 10; i++)
		{
			uint16 listNum = gP2PManager.GetPoeRecordListNum();
			if (listNum > 10)
			{
				SLEEP(1.5 * 60 * 1000);
				continue;
			}

			char line[MAX_TEMP_BUF_LEN] = { 0 };
			time_t tempTime = time(NULL);
			struct tm * t;
			t = localtime(&tempTime);
			sprintf(line, "%d-%d-%d %d:%d:%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

			QCryptographicHash h(QCryptographicHash::Sha512);
			h.addData(line, strlen(line));

			QString sha512_ = h.result().toHex();
			QString s1 = sha512_.mid(0, 32);
			QString s2 = sha512_.mid(32, 32);
			QString s3 = sha512_.mid(64, 32);
			QString s4 = sha512_.mid(96, 32);

			QString str512 = QString("%1%2%3%4").arg(s1).arg(s2).arg(s3).arg(s4);
			QByteArray ba = str512.toLatin1();
			char *testBuf = ba.data();

			char tempbuf[64] = { 0 };
			strncpy(tempbuf, testBuf, 8);

			TEVIDENCEINFO FileInfo;
			FileInfo.cFileName = ip.c_str();
			FileInfo.cFileName += "_";
			char num[64] = { 0 };
			sprintf(num, "%d", blockNum + 1);
			FileInfo.cFileName += num;
			FileInfo.cFileName += ".txt";
			FileInfo.cFileHash = testBuf;
			FileInfo.cCustomInfo = tempbuf;
			FileInfo.cRightOwner = tempbuf;

			FileInfo.tRegisTime = time(NULL);
			FileInfo.iFileState = 0;
			FileInfo.iFileSize = 1024;

			std::thread thr(SetFilePoeRecord, &FileInfo);
			thr.detach();
			blockNum++;
			SLEEP(2 * 60 * 1000);
		}
	}
}

uint16 getCsvFileNum()
{
	return g_csvfileNum;
}

void getHyperBlockFromLocal()
{
	gP2PManager.GetHyperBlockInfoFromLocal();
}

string Upqueue(string hash)
{
	int randNum = rand() % 150;
	{
		{
			TEVIDENCEINFO FileInfo;
			FileInfo.cFileName = "uyoo";
			FileInfo.cFileHash = hash.c_str();
			FileInfo.cCustomInfo = "";
			FileInfo.cRightOwner = "";

			FileInfo.tRegisTime = time(NULL);
			FileInfo.iFileState = 0;
			FileInfo.iFileSize = 1024;

			char line[MAX_TEMP_BUF_LEN] = { 0 };
			time_t tempTime = time(NULL);
			struct tm * t;
			t = localtime(&tempTime);
			sprintf(line, "%d-%d-%d-%d:%d:%d--%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, randNum);

			gP2PManager.AddNewBlock(FileInfo, line);
			return line;
		}
	}
}
void runP2P(int argc, char *argv[])
{
	if (!CCommonStruct::ReadConfig())
		return ;

	string strLogFile = g_confFile.strLogDir;
	strLogFile += "hyperchain_p2p.log";
	g_pLogHelper = open_logfile(strLogFile.c_str());

	
	string ipTemp = CCommonStruct::GetLocalIp();

	gP2PManager.Init();
	gP2PManager.Start();
}

uint64 GetCurBlockNumOfMyself()
{
	return gP2PManager.GetCurBlockNumOfMyself();
}

uint64 GetCurBlockNumOfAllNode()
{
	return gP2PManager.GetCurBlockNumOfAllNode();
}

VEC_T_BLOCKINFO GetBlockInfo(uint64 start, uint64 end)
{
	return gP2PManager.GetBlockInfoByIndex(start, end);
}

uint64 GetBaseBlockNum()
{
	return gP2PManager.GetLatestHyperBlockNo();
}

uint16 GetAllChainNum()
{
	return 2;
}

uint64 GetOnChainState(string queueId)
{
	return gP2PManager.GetOnChainState(queueId);
}

uint16 GetHaveConfirmChainNum()
{
	return (GetSendRegisReqNum(CONFIRMED) + GetRecvRegisRegNum(CONFIRMED));
}

uint64 GetTimeOfConsensus()
{
	return gP2PManager.GetElaspedTimeOfCurrentConsensus();
}
uint64 GetStartTimeOfCurrentConsensus()
{
	return gP2PManager.GetStartTimeOfConsensus();
}

VEC_T_NODEINFO  GetMyLocalChain()
{
	return gP2PManager.GetMyLocalChain();
}

VEC_T_NODEINFO GetOtherLocalChain(uint16 chainNum)
{
	return gP2PManager.GetOtherLocalChain(chainNum);
}

uint32 GetBetterNodeNum()
{
	return gP2PManager.GetStrongHyperBlockNo();
}

uint32 GetNormalNodeNum()
{
	return gP2PManager.GetAverageHyperBlockNo();
}

uint32 GetBadNodeNum()
{
	return gP2PManager.GetWeakHyperBlockNo();
}

uint32 GetDownNodeNum()
{
	return gP2PManager.GetOfflineHyperBlockNo();
}


uint32 GetSendRegisReqNum(uint16 regisReq)
{
	uint32 index;
	switch (regisReq)
	{
	case SEND:
		index = gP2PManager.GetSendRegisReqNum();
		break;
	case CONFIRMING:
		index = gP2PManager.GetSendConfirmingRegisReqNum();
		break;
	case CONFIRMED:
		index = gP2PManager.GetSendRegisReqNum() - gP2PManager.GetSendConfirmingRegisReqNum();
		break;
	default:
		break;
	}
	return index;   
}

uint32 GetRecvRegisRegNum(uint16 regisReq)
{
	uint32 index;
	switch (regisReq)
	{
	case RECV:
		index = gP2PManager.GetRecvRegisReqNum();
		break;
	case CONFIRMING:
		index = gP2PManager.GetRecvConfirmingRegisReqNum();
		break;
	case CONFIRMED:
		index = gP2PManager.GetRecvRegisReqNum() - gP2PManager.GetRecvConfirmingRegisReqNum();
		break;
	default:
		break;
	}
	return index;
}


void AddNewBlock(P_TEVIDENCEINFO pSetInfo, string queueId)
{
	TEVIDENCEINFO FileInfo;
	FileInfo.cFileName = pSetInfo->cFileName;
	FileInfo.cFileHash = pSetInfo->cFileHash;
	FileInfo.cCustomInfo = pSetInfo->cCustomInfo;
	FileInfo.cRightOwner = pSetInfo->cRightOwner;

	FileInfo.tRegisTime = time(NULL);
	FileInfo.iFileState = pSetInfo->iFileState;
	FileInfo.iFileSize = pSetInfo->iFileSize;

	gP2PManager.AddNewBlock(FileInfo, queueId.c_str());
}

void SetFilePoeRecord(P_TEVIDENCEINFO pSetInfo)
{
	std::thread t(AddNewBlock, pSetInfo, "");
	t.detach();
}

bool VerifyPoeRecord(string &checkFileHash, P_TEVIDENCEINFO pCheckInfo)
{
	return gP2PManager.MatchFile(checkFileHash, pCheckInfo);
}

uint64 GetConnNodesNum()
{
	return GetAllConnectedNodes();
}

uint64 GetNodeRunningTime()
{
	return time(NULL); 
}

uint32 GetAllPoeReqNum()
{
	return 45;
}

void GetSendingRate(string &sendRate)
{	
	gP2PManager.GetSendingRate(sendRate);
}
 
void GetSentSize(string &allSendSize)
{
	gP2PManager.GetSentSize(allSendSize);
}

void GetRecvingRate(string &recvRate)
{
	gP2PManager.GetRecvingRate(recvRate);
}

void GetRecvedSize(string &allRecvSize)
{
	gP2PManager.GetRecvedSize(allRecvSize);
}

void GetChainSize(string &buffSize)
{
	buffSize = "25MB";
}


bool isNum(string searchInfo)
{
	bool index = true;
	int i = 0;
	while (i < searchInfo.length())
	{
		int iTestNum = searchInfo[i] - '0';
		if ((0 < iTestNum) && (iTestNum < 9))
		{
			i++;
			continue;
		}
		else
		{
			index = false;
			break;
		}
	}
	return index;
}

VEC_T_BROWSERSHOWINFO Query(string &searchInfo)
{
	if (searchInfo.length() == DEF_SHA512_LEN)
	{
		return gP2PManager.GetBlockInfoByHash(searchInfo);
	}
	else if (string::npos != searchInfo.find("-"))
	{
		return gP2PManager.GetBlockInfoByTime(searchInfo);
	}
	else if (isNum(searchInfo))
	{
		return gP2PManager.GetBlockInfoByBlockNum(searchInfo);
	}
	else
	{
		return gP2PManager.GetBlockInfoByCustomInfo(searchInfo);
	}
}

void GetChainData(string &chainData)
{
    chainData = gP2PManager.GetChainData();
}

uint64 GetPoeReqTotalNum()
{
	return GetCurBlockNumOfAllNode();
}

uint64 GetAllConnectedNodes()
{    
	return gP2PManager.GetAllNodesInTheNet();
}

uint64 GetAllConfirmingPoeReqNum()
{
	return (GetRecvRegisRegNum(CONFIRMING) + GetSendRegisReqNum(CONFIRMING));
}

void GetNodeInfo(string &info, string &ip, uint16 &port)
{
	gP2PManager.GetNodeDescription(info, ip, port);
}


void ChainDataPersist()
{
}