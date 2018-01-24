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

#ifndef __QT_INTERFACE_H__
#define __QT_INTERFACE_H__

#include "../headers/inter_public.h"
#include "../HChainP2PManager.h"

#ifdef __cplusplus
extern "C"
{
#endif
	CHChainP2PManager getManagerPtr();
	void runP2P(int argc, char *argv[]);
	uint64 GetCurBlockNumOfMyself();
	uint64 GetCurBlockNumOfAllNode();
	VEC_T_BLOCKINFO GetBlockInfo(uint64 start, uint64 end);
	uint64 GetBaseBlockNum();
	uint16 GetAllChainNum();
	VEC_T_NODEINFO GetMyLocalChain();
	VEC_T_NODEINFO GetOtherLocalChain(uint16 chainNum);
	uint16 GetHaveConfirmChainNum();
	uint64 GetTimeOfConsensus();
	uint32 GetBetterNodeNum();
	uint32 GetNormalNodeNum();
	uint32 GetBadNodeNum();
	uint32 GetDownNodeNum();
	uint32 GetSendRegisReqNum(uint16 regisReq);
	uint32 GetRecvRegisRegNum(uint16 regisReq);
	void SetFilePoeRecord(P_TEVIDENCEINFO pSetInfo);
	bool VerifyPoeRecord(string &checkFileHash, P_TEVIDENCEINFO pCheckInfo);
	uint64 GetConnNodesNum();
	uint64 GetNodeRunningTime();
	uint32 GetAllPoeReqNum();
	void GetSendingRate(string &sendRate);
	void GetSentSize(string &allSendSize);
	void GetRecvingRate(string &recvRate);
	void GetRecvedSize(string &allRecvSize);
	void GetChainSize(string &buffSize);
	VEC_T_BROWSERSHOWINFO Query(string &searchInfo);
	void GetChainData(string &chainData);
	uint64 GetPoeReqTotalNum();
	uint64 GetAllConnectedNodes();
	uint64 GetAllConfirmingPoeReqNum();
	uint64 GetStartTimeOfCurrentConsensus();
	void GetNodeInfo(string &info, string &ip, uint16& port);
	void ChainDataPersist();
	uint16 getCsvFileNum();
	void getHyperBlockFromLocal();
	uint64 GetOnChainState(string fileHash);
	string Upqueue(string hash);
#ifdef __cplusplus
}
#endif

#endif//__QT_INTERFACE_H__