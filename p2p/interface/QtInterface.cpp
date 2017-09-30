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

uint64 gStartTime = time(NULL);
CHChainP2PManager gP2PManager;

UUFile uufileTest;
uint16 g_csvfileNum;
void analysisCsvTest()
{
	string localPath = uufileTest.GetAppPath();
	string csvPath = localPath + "uyoo_hash.csv";
	
	vector<string> vec;
	std::ifstream fin(csvPath.c_str(), std::ios::in);
	char line[512] = { 0 };
	while (fin.getline(line, sizeof(line)))
	{
		vec.push_back(line);
	}
	fin.clear();
	fin.close();

	g_csvfileNum = vec.size();

	vector<string>::iterator itr = vec.begin();
	for (; itr != vec.end(); itr++)
	{
		TEVIDENCEINFO FileInfo;
		FileInfo.cFileName = "";
		FileInfo.cFileHash = (*itr).c_str();
		FileInfo.cCustomInfo = "";
		FileInfo.cRightOwner = "";

		FileInfo.tRegisTime = time(NULL);
		FileInfo.iFileState = 0;
		FileInfo.iFileSize = 1024;
		
		std::thread t(SetFilePoeRecord, &FileInfo);
		t.detach();

		int randNum = rand() % 15;
		SLEEP(randNum * 1000);
	}
}
uint16 getCsvFileNum()
{
	return g_csvfileNum;
}

void runP2P(int argc, char *argv[])
{
	if (!CCommonStruct::ReadConfig())
		return ;

	string strLogFile = g_confFile.strLogDir;
	strLogFile += "hyperchain_p2p.log";
	g_pLogHelper = open_logfile(strLogFile.c_str());

	log_info(g_pLogHelper, "hyperBlock P2P start......");
	
	string ipTemp = CCommonStruct::GetLocalIp();
	

	gP2PManager.Init();
	gP2PManager.Start();

}

uint64 GetLocalLatestBlockNo()
{
	return gP2PManager.GetLocalLatestBlockNo();
}

uint64 GetLatestHyperBlockNo()
{
	return gP2PManager.GetLatestHyperBlockNo();
}

VEC_T_BLOCKINFO GetBlockInfoByIndex(uint64 start, uint64 end)
{
	return gP2PManager.GetBlockInfoByIndex(start, end);
}

uint16 GetLocalChainNum()
{
	return 2;
}

uint16 GetConfirmedChainNum()
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

uint32 GetStrongNodeNum()
{
	return gP2PManager.GetStrongNodeNum();
}

uint32 GetAverageNodeNum()
{
	return gP2PManager.GetAverageNodeNum();
}

uint32 GetWeakNodeNum()
{
	return gP2PManager.GetWeakNodeNum();
}

uint32 GetOfflineNodeNum()
{
	return gP2PManager.GetOfflineNodeNum();
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

void AddNewBlock(P_TEVIDENCEINFO pSetInfo)
{
	TEVIDENCEINFO FileInfo;
	FileInfo.cFileName = pSetInfo->cFileName;
	FileInfo.cFileHash = pSetInfo->cFileHash;
	FileInfo.cCustomInfo = pSetInfo->cCustomInfo;
	FileInfo.cRightOwner = pSetInfo->cRightOwner;

	FileInfo.tRegisTime = time(NULL);
	FileInfo.iFileState = pSetInfo->iFileState;
	FileInfo.iFileSize = pSetInfo->iFileSize;

	gP2PManager.AddNewBlock(FileInfo);
}

void SetFilePoeRecord(P_TEVIDENCEINFO pSetInfo)
{
	std::thread t(AddNewBlock, pSetInfo);
	t.detach();
}

bool VerifyPoeRecord(string &checkFileHash, P_TEVIDENCEINFO pCheckInfo)
{
	return gP2PManager.VerifyPoeRecord(checkFileHash, pCheckInfo);
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

VEC_T_QUERYSHOWINFO Query(string &searchInfo)
{
	VEC_T_QUERYSHOWINFO vec_info;
	return vec_info;
}

void GetChainData(string &chainData)
{
    chainData = gP2PManager.GetChainData();
}

uint64 GetPoeReqTotalNum()
{
	return GetLatestHyperBlockNo();
}

uint64 GetAllConnectedNodes()
{    
	return 0;
}

uint64 GetAllConfirmingPoeReqNum()
{
	return (GetRecvRegisRegNum(CONFIRMING) + GetSendRegisReqNum(CONFIRMING));
}

void GetNodeDescription(string &info, string &ip, uint16 &port)
{
	gP2PManager.GetNodeDescription(info, ip, port);
}

void ChainDataPersist()
{
}
uint64 GetElaspedTimeOfCurrentConsensus()
{
	return gP2PManager.GetElaspedTimeOfCurrentConsensus();
}