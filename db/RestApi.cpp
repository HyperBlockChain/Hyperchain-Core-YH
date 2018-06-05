/*copyright 2016-2018 hyperchain.net (Hyperchain)
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
#include "RestApi.h"

#include "HChainP2PManager/headers/commonstruct.h"
#include "HChainP2PManager/headers/inter_public.h"
#include "HChainP2PManager/HChainP2PManager.h"
#include "HChainP2PManager/interface/QtInterface.h"
#include "../HChainP2PManager/headers/UUFile.h"
#include "../HChainP2PManager/HttpUnit/HttpUnit.h"

#include <cpprest\http_listener.h>
#include <cpprest\filestream.h>
#include <io.h>
#include <fcntl.h>
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "httpapi.lib")

http_listener_config server_config;

utility::string_t address = U("http://*:8080");
web::uri_builder uri1(address);
auto addr = uri1.to_uri().to_string();
CommandHandler restHandler(addr, server_config);

CommandHandler::CommandHandler(utility::string_t url, http_listener_config server_config) : m_listener(url, server_config)
{
	m_listener.support(methods::GET, std::bind(&CommandHandler::handle_get, this, std::placeholders::_1));
	m_listener.support(methods::POST, std::bind(&CommandHandler::handle_post, this, std::placeholders::_1));
	m_listener.support(methods::PUT, std::bind(&CommandHandler::handle_put, this, std::placeholders::_1));
	m_listener.support(methods::DEL, std::bind(&CommandHandler::handle_del, this, std::placeholders::_1));
}

string ws2s(const wstring& ws)
{
	string curLocale = setlocale(LC_ALL, NULL);

	setlocale(LC_ALL, "chs");

	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	string result = _Dest;
	delete[]_Dest;

	setlocale(LC_ALL, curLocale.c_str());

	return result;
}

wstring s2ws(const string& s)
{
    setlocale(LC_ALL, "chs");

    const char* _Source = s.c_str();
    size_t _Dsize = s.size() + 1;
    wchar_t *_Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    wstring result = _Dest;
    delete []_Dest;

    setlocale(LC_ALL, "C");

    return result;
}

std::vector<utility::string_t> requestPath(const http_request & message) {
	auto relativePath = uri::decode(message.relative_uri().path());
	return uri::split_path(relativePath);
}

UUFile			m_uufiletest;

static void handle_error(pplx::task<void>& t);
utility::string_t resource_type(const utility::string_t& strSuffix)
{
	std::map<utility::string_t, utility::string_t> oVals;
	oVals[U(".html")] = U("text/html");
	oVals[U(".js")] = U("application/javascript");
	oVals[U(".css")] = U("text/css");
	oVals[U(".png")] = U("application/octet-stream");
	oVals[U(".jpg")] = U("application/octet-stream");

	auto pIt = oVals.find(strSuffix);
	if (pIt != oVals.end())
		return pIt->second;
	return U("application/octet-stream");
}

void CommandHandler::handle_get(http_request message)
{
	qDebug("get");

	utility::string_t hash;

	auto uri = message.relative_uri().to_string();
	if (string::npos != uri.find(L"html"))
	{
		string localPath = m_uufiletest.GetAppPath();
		string confPath = localPath + "index.html";

		char *strBody = NULL;
		unsigned int uiRecvLen = 0;

		int ret = HttpDownloadF("http://192.168.0.55/hyperchain/index.html", &strBody, uiRecvLen);
		if (200 == ret)
		{
			message.reply(status_codes::OK, strBody, ::utility::conversions::to_utf8string("text/html; charset=utf-8"));
		}
		else
		{
			concurrency::streams::fstream::open_istream(s2ws(confPath.c_str()), std::ios::in).then([=](concurrency::streams::istream is)
			{

				message.reply(status_codes::OK, is, U("text/html"));

			});
		}

		if (strBody != NULL)
		{
			delete strBody;
			strBody = NULL;
		}
		return;
	}

	auto path = requestPath(message);
	if (!path.empty() && path.size() == 1) {

		std::map<utility::string_t, utility::string_t> query = uri::split_query(uri::decode(message.request_uri().query()));

		json::value vRet;

		if (path[0] == U("SubmitRegistration"))
		{
			auto cntEntry = query.find(U("hash"));

			if (cntEntry != query.end() && !cntEntry->second.empty())
			{
				hash = cntEntry->second;
				string sUserData = ws2s(hash);
				RestApi* api = new RestApi;

				vRet = api->MakeRegistration(sUserData);
				delete api;
				api = NULL;

			}
		}
		else if (path[0] == U("GetHyperblocks"))
		{
			auto cntEntryId = query.find(U("start_id"));
			auto cntEntryNum = query.find(U("num"));
			utility::string_t sId = cntEntryId->second;
			utility::string_t sNum = cntEntryNum->second;

			uint64_t nHyperBlockId = atoi(ws2s(sId).c_str());
			uint64_t nNum = atoi(ws2s(sNum).c_str());

			RestApi* api = new RestApi;
			vRet = api->getHyperblocks(nHyperBlockId, nNum);
			if (vRet.is_null())
			{
				GetHyperBlockInfoFromP2P(nHyperBlockId, nNum);
			}

			if (api != NULL)
			{
				delete api;
				api = NULL;
			}

		}
		else if (path[0] == U("GetOnchainState"))
		{
			auto cntEntry = query.find(U("id"));

			if (cntEntry != query.end() && !cntEntry->second.empty())
			{
				hash = cntEntry->second;
				string sUserData = ws2s(hash);
				RestApi* api = new RestApi;

				vRet = api->getOnchainState(sUserData);

				delete api;
				api = NULL;

			}
		}
		else if (path[0] == U("GetHyperBlockHead"))
		{
			auto cntEntryId = query.find(U("key"));

			utility::string_t sId = cntEntryId->second;

			uint64_t nHyperBlockId = atoi(ws2s(sId).c_str());

			RestApi* api = new RestApi;
			vRet = api->getHyperblocksHead(nHyperBlockId);
			if (api != NULL)
			{
				delete api;
				api = NULL;
			}

		}
		else if (path[0] == U("GetRegWaitingList"))
		{

			LIST_T_LOCALCONSENSUS listInfo = GetPoeRecordList();
			ITR_LIST_T_LOCALCONSENSUS itr = listInfo.begin();
			int i = 0;
			for (itr; itr != listInfo.end(); itr++)
			{
				char strBuf[MAX_BUF_LEN];
				memset(strBuf, 0, MAX_BUF_LEN);
				CCommonStruct::Hash512ToStr(strBuf, &(*itr).tLocalBlock.tPayLoad.tPayLoad.tFileHash);

				wstring oss;
				char num[8];
				memset(num, 0, sizeof(num));
				sprintf(num, "%d", i);
				oss = L"readyOnChainHash[";
				oss += s2ws(num);
				oss += L"]";
				vRet[oss] = json::value::string(s2ws(strBuf));

				i += 1;
			}

		}
		else if (path[0] == U("GetLatestHyperBlockNo"))
		{

			RestApi* api = new RestApi;
			uint64_t num = api->getLatestHyperBlockNo();
			if (api != NULL)
			{
				delete api;
				api = NULL;
			}

			vRet[L"laststHyperBlockNo"] = json::value::number(num);

		}
		else if (path[0] == U("GetLatestHyperBlock"))
		{

			RestApi* api = new RestApi;
			uint64_t num = api->getLatestHyperBlockNo();
			vRet = api->getHyperblocks(num, 1);

			if (api != NULL)
			{
				delete api;
				api = NULL;
			}

		}
		else if (path[0] == U("GetNodeRuntimeEnv"))
		{

			string version;
			string ip;
			string netType;
			string pVersion;
			string name;
			uint16 port;
			GetNodeRunTimeEnv(version, netType, pVersion, ip, port, name);

			vRet[L"version"] = json::value::string(s2ws(version).c_str());
			vRet[L"netType"] = json::value::string(s2ws(netType).c_str());
			vRet[L"protocolVersion"] = json::value::string(s2ws(pVersion).c_str());
			vRet[L"ip"] = json::value::string(s2ws(ip).c_str());
			vRet[L"port"] = json::value::number(port);
			vRet[L"name"] = json::value::string(s2ws(name).c_str());

		}
		else if (path[0] == U("GetStateOfCurrentConsensus"))
		{

			uint64 blockNo;
			uint16 blockNum = 0;
			uint16 chainNum = 0;
			uint16 uiState = GetStateOfCurrentConsensus(blockNo, blockNum, chainNum);
			vRet[L"curBuddyNo"] = json::value::number(blockNo);
			if (uiState == IDLE)
			{
				vRet[L"consensusState"] = json::value::string(L"idle");
			}
			else if (uiState == LOCAL_BUDDY)
			{
				vRet[L"consensusState"] = json::value::string(L"localbuddy");
			}
			else if (uiState == GLOBAL_BUDDY)
			{
				vRet[L"consensusState"] = json::value::string(L"globalbuddy");
			}

		}
		else if (path[0] == U("GetDataOfCurrentConsensus"))
		{

			uint64 blockNo;
			uint16 blockNum = 0;
			uint16 chainNum = 0;
			uint16 uiState = GetStateOfCurrentConsensus(blockNo, blockNum, chainNum);
			vRet[L"curBuddyNo"] = json::value::number(blockNo);
			if (uiState == IDLE)
			{
				vRet[L"consensusState"] = json::value::string(L"idle");
			}
			else if (uiState == LOCAL_BUDDY)
			{
				vRet[L"consensusState"] = json::value::string(L"localbuddy");
				vRet[L"blockNum"] = json::value::number(blockNum);
			}
			else if (uiState == GLOBAL_BUDDY)
			{
				vRet[L"consensusState"] = json::value::string(L"globalbuddy");
				vRet[L"chainNum"] = json::value::number(chainNum);
			}

		}
		else if (path[0] == U("GetConnectedNodesNum"))
		{

			uint32 retNum = GetConnectedNodesNum();
			vRet[L"connectedNodesNum"] = json::value::number(retNum);

		}
		else if (path[0] == U("CreatCustomerizeConsensusScript"))
		{

			auto cntEntryType = query.find(U("type"));
			auto cntEntryScript = query.find(U("script"));
			utility::string_t sType = cntEntryType->second;
			utility::string_t sScript = cntEntryScript->second;

			string strType = ws2s(sType);
			if (0 != strType.compare("xml") || (sScript.length() > (1024 * 2)))
			{
				vRet[L"returnValue"] = json::value::string(L"type is wrong");

			}
			else
			{
				string strScript = ws2s(sScript);
				SetScriptPoeRecord(strScript.c_str());

				vRet[L"returnValue"] = json::value::string(L"success");

			}
		}
		http_response response(status_codes::OK);
		response.set_body(vRet);
		response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
		message.reply(response);
		return;
	}
	message.reply(status_codes::OK, json::value(L"OK"));
}

void CommandHandler::handle_post(http_request message)
{

	message.reply(status_codes::OK, "POST");
}

void CommandHandler::handle_put(http_request message)
{

	message.reply(status_codes::OK, "PUT");
}

void CommandHandler::handle_del(http_request message)
{

	message.reply(status_codes::OK, "DEL");
}

json::value RestApi::blockHeadToJsonValue(T_HYPERBLOCKDBINFO blockDBInfo)
{
	json::value valHyperBlock;

	valHyperBlock[L"hyperBlockId"] = json::value::number(blockDBInfo.uiBlockId);
	valHyperBlock[L"hyperBlockHash"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strHashSelf).c_str()));
	valHyperBlock[L"hyperCreateTime"] = json::value::number(blockDBInfo.uiBlockTimeStamp);
	valHyperBlock[L"buddyScript"] = json::value::string(L"");
	valHyperBlock[L"publicKey"] = json::value::string(L"");
	valHyperBlock[L"perHyperBlockHash"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strPreHash).c_str()));

	return valHyperBlock;
}
json::value RestApi::blockToJsonValue(T_HYPERBLOCKDBINFO blockDBInfo)
{
	json::value valHyperBlock;

	valHyperBlock[L"id"] = json::value::number(blockDBInfo.uiBlockId);
	valHyperBlock[L"hash"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strHashSelf).c_str()));
	valHyperBlock[L"ctime"] = json::value::number(blockDBInfo.uiBlockTimeStamp);

	valHyperBlock[L"per_hash"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strPreHash).c_str()));

	valHyperBlock[L"type"] = json::value::number(blockDBInfo.ucBlockType);
	valHyperBlock[L"hid"] = json::value::number(blockDBInfo.uiReferHyperBlockId);
	valHyperBlock[L"hhash"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strHyperBlockHash).c_str()));

	valHyperBlock[L"payload"] = json::value::string(s2ws(blockDBInfo.strPayload));

	valHyperBlock[L"chain_num"] = json::value::number(blockDBInfo.uiLocalChainId);
	return valHyperBlock;
}

json::value RestApi::getHyperblocks(uint64_t nStartId, uint64_t nNum)
{
	json::value vHyperBlocks;
	QList<T_HYPERBLOCKDBINFO> queue;
	int nEndId = nStartId + nNum - 1;
	int nRet = DBmgr::instance()->getHyperblocks(queue, nStartId, nEndId);
	if (nRet == 0)
	{
		int i = 0;
		for (; i != queue.size(); ++i) {
			T_HYPERBLOCKDBINFO info = queue.at(i);

			wostringstream oss;
			oss << info.uiReferHyperBlockId;
			wstring sKey=oss.str();
			wostringstream chain_num;
			chain_num << info.uiLocalChainId;
			wstring sChainNum = chain_num.str();

			if (info.ucBlockType == 1) {
				vHyperBlocks[sKey][0] = blockToJsonValue(info);
			}
			else if (info.ucBlockType == 2) {
				vHyperBlocks[sKey][i] = blockToJsonValue(info);
			}
        }
	}

	return vHyperBlocks;
}

json::value RestApi::getHyperblocksHead(uint64_t nStartId)
{
	json::value vHyperBlocks;
	QList<T_HYPERBLOCKDBINFO> queue;
	int nRet = DBmgr::instance()->getHyperblockshead(queue, nStartId);
	if (nRet == 0)
	{
		int i = 0;
		for (; i != queue.size(); ++i) {
			T_HYPERBLOCKDBINFO info = queue.at(i);

			wostringstream oss;
			oss << info.uiReferHyperBlockId;
			wstring sKey = oss.str();
			wostringstream chain_num;
			chain_num << info.uiLocalChainId;
			wstring sChainNum = chain_num.str();

			if (info.ucBlockType == 1) {
				vHyperBlocks[sKey][0] = blockHeadToJsonValue(info);
			}
			else if (info.ucBlockType == 2) {
				vHyperBlocks[sKey][i] = blockHeadToJsonValue(info);
			}
		}
	}
	return vHyperBlocks;
}

int RestApi::getLatestHyperBlockNo()
{
	return DBmgr::instance()->getLatestHyperBlockNo();
}
json::value RestApi::getOnchainState(string queueId)
{
	json::value vHyperBlocks;
	int64 nRet = GetOnChainState(queueId);

	if (nRet == -1)
	{
		vHyperBlocks[L"queueId"] = json::value::string(s2ws(queueId.c_str()));
		vHyperBlocks[L"onChainState"] = json::value::string(s2ws("onchaing"));
		vHyperBlocks[L"hyperBlockId"] = json::value::number(-1);
	}
	else if (nRet == -3)
	{
		vHyperBlocks[L"queueId"] = json::value::string(s2ws(queueId.c_str()));
		vHyperBlocks[L"onChainState"] = json::value::string(s2ws("unknown"));
		vHyperBlocks[L"hyperBlockId"] = json::value::number(-1);
	}
	else if (nRet == -2)
	{
		vHyperBlocks[L"queueId"] = json::value::string(s2ws(queueId.c_str()));
		vHyperBlocks[L"onChainState"] = json::value::string(s2ws("waiting"));
		vHyperBlocks[L"hyperBlockId"] = json::value::number(-1);
	}
	else
	{
		vHyperBlocks[L"queueId"] = json::value::string(s2ws(queueId.c_str()));
		vHyperBlocks[L"onChainState"] = json::value::string(s2ws("success"));
		vHyperBlocks[L"hyperBlockId"] = json::value::number(nRet);
	}

	return vHyperBlocks;
}

json::value RestApi::MakeRegistration(string strUserData)
{
	string queueId = Upqueue(strUserData);
	json::value valQueueID;
	valQueueID[L"queue_id"] = json::value::string(s2ws(queueId.c_str()));
	valQueueID[L"state"] = json::value::string(s2ws("waiting"));
	return valQueueID;

}

int RestApi::startRest()
{
	qDebug("start Rest Server");

	try
	{

		restHandler.open().wait();

	}
	catch (std::exception& ex)
	{
		restHandler.close().wait();
		qDebug(ex.what());
	}

	return 0;
}
