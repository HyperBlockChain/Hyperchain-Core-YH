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

#ifndef __QT_INTERFACE_H__
#define __QT_INTERFACE_H__

#include "../headers/inter_public.h"
#include "../HChainP2PManager.h"
#include "QtNotify.h"
#include "db/RestApi.h"
#ifdef __cplusplus
extern "C"
{
#endif
	CHChainP2PManager getManagerPtr();

	void runP2P(int argc, char *argv[]);

	uint64 GetLocalLatestBlockNo();

	uint64 GetLatestHyperBlockNo();

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

	void setNotify(QtNotify * qnotify);
	void ChainDataPersist();
	uint16 getCsvFileNum();
	void getHyperBlockFromLocal();
	void getAllHyperBlockFromLocal();
	uint64 GetOnChainState(string fileHash);
	string Upqueue(string strfilename, string strfilehash, string strcustomInfo, string strrightowner);
	void UpqueueEx(string strfilename, string strfilehash, string strcustomInfo, string strrightowner, vector<string>& out_vc);

	VEC_T_PPEERCONF GetPeerInfo();
	json::value QueryByWeb(uint64 blockNum);
	uint16 GetPoeRecordListNum();
	LIST_T_LOCALCONSENSUS GetPoeRecordList();
	void GetNodeRunTimeEnv(string &version, string &netType, string &protocolVersion, string &ip, uint16 &port, string &name);
	uint16 GetStateOfCurrentConsensus(uint64 &blockNo, uint16 &blockNum, uint16 &chainNum);
	uint32 GetConnectedNodesNum();
	void SetScriptPoeRecord(string script);
	void GetHyperBlockInfoFromP2P(uint64 start, uint64 end);

	void GetHyperBlockNumInfoFromLocal();
	void DeleteFailedPoe(string script, uint64 time);
#ifdef __cplusplus
}
#endif

#endif