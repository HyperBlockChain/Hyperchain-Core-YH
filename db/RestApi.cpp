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


#include "RestApi.h"

#include "HChainP2PManager/headers/commonstruct.h"
#include "HChainP2PManager/headers/inter_public.h"
#include "HChainP2PManager/HChainP2PManager.h"
#include "HChainP2PManager/interface/QtInterface.h"
#include "../HChainP2PManager/headers/UUFile.h"
#include "../HChainP2PManager/HttpUnit/HttpUnit.h"
#include "../wnd/common.h"
#include <cpprest/http_listener.h>
#include <cpprest/filestream.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <codecvt>
#endif


#include <string>
#include <locale>
#include <sstream>
using namespace std;

http_listener_config server_config;

utility::string_t address = U("http://*:8080");//U("http://0.0.0.0:8080");
web::uri_builder uri1(address);
auto addr = uri1.to_uri().to_string();
CommandHandler restHandler(addr, server_config);


string tstringToUtf8(const utility::string_t& str)
{
#ifdef _UTF16_STRINGS
	wstring_convert<codecvt_utf8<wchar_t> > strCnv;
	return strCnv.to_bytes(str);
#else
	return str;
#endif
}

utility::string_t stringToTstring(const string& str)
{
#ifdef _UTF16_STRINGS
	wstring_convert<codecvt_utf8<wchar_t> > strCnv;
	return strCnv.from_bytes(str);
#else
	return str;
#endif
}

CommandHandler::CommandHandler(utility::string_t url, http_listener_config server_config) : m_listener(url, server_config)
{
	m_listener.support(methods::GET, std::bind(&CommandHandler::handle_get, this, std::placeholders::_1));
	m_listener.support(methods::POST, std::bind(&CommandHandler::handle_post, this, std::placeholders::_1));
	m_listener.support(methods::PUT, std::bind(&CommandHandler::handle_put, this, std::placeholders::_1));
	m_listener.support(methods::DEL, std::bind(&CommandHandler::handle_del, this, std::placeholders::_1));
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
	if (string::npos != uri.find(_XPLATSTR("html")))
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
			concurrency::streams::fstream::open_istream(stringToTstring(confPath.c_str()), std::ios::in).then([=](concurrency::streams::istream is)
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
			auto filename = query.find(U("filename"));
			auto filehash = query.find(U("filehash"));
			auto customInfo = query.find(U("custominfo"));
			auto rightowner = query.find(U("rightowner"));

			if (filename != query.end() && !filename->second.empty() && filehash != query.end() && !filehash->second.empty()
				&& customInfo != query.end() && !customInfo->second.empty() && rightowner != query.end() && !rightowner->second.empty())
			{
				string strfilename = tstringToUtf8(filename->second);
				string strfilehash = tstringToUtf8(filehash->second);
				string strcustomInfo = tstringToUtf8(customInfo->second);
				string strrightowner = tstringToUtf8(rightowner->second);

				RestApi* api = new RestApi;
				vRet = api->MakeRegistration(strfilename, strfilehash, strcustomInfo, strrightowner);
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

			uint64_t nHyperBlockId = atoi(tstringToUtf8(sId).c_str());
			uint64_t nNum = atoi(tstringToUtf8(sNum).c_str());

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
			auto id = query.find(U("queue_id"));
			auto localhash = query.find(U("queue_hash"));
			auto ctime = query.find(U("queue_time"));

			if (id != query.end() && !id->second.empty() && localhash != query.end() && !localhash->second.empty() 
				&& ctime != query.end() && !ctime->second.empty())
			{			
				string strid = tstringToUtf8(id->second);
				string strhash = tstringToUtf8(localhash->second);
				string strtime = tstringToUtf8(ctime->second);

				stringstream strIn;
				uint64 utime;
				strIn << strtime;			
				strIn >> utime;

				RestApi* api = new RestApi;
				vRet = api->getOnchainStateEx(strid, strhash, utime);

				delete api;
				api = NULL;

			}
		}
		else if (path[0] == U("GetHyperBlockHead"))
		{
			auto cntEntryId = query.find(U("key"));
			utility::string_t sId = cntEntryId->second;	
			uint64_t nHyperBlockId = atoi(tstringToUtf8(sId).c_str());	

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
				char strBuf[MAX_BUF_LEN] = {0};				
				CCommonStruct::Hash512ToStr(strBuf, &(*itr).tLocalBlock.tPayLoad.tPayLoad.tFileHash);
			
		

				string_t oss;
				char num[8];
				memset(num, 0, sizeof(num));
				sprintf(num, "%d", i);
				oss = _XPLATSTR("readyOnChainHash[");
				oss += stringToTstring(num);
				oss += _XPLATSTR("]");
				vRet[oss] = json::value::string(stringToTstring(strBuf));

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

			vRet[_XPLATSTR("laststHyperBlockNo")] = json::value::number(num);

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

			vRet[_XPLATSTR("version")] = json::value::string(stringToTstring(version).c_str());
			vRet[_XPLATSTR("netType")] = json::value::string(stringToTstring(netType).c_str());
			vRet[_XPLATSTR("protocolVersion")] = json::value::string(stringToTstring(pVersion).c_str());
			vRet[_XPLATSTR("ip")] = json::value::string(stringToTstring(ip).c_str());
			vRet[_XPLATSTR("port")] = json::value::number(port);
			vRet[_XPLATSTR("name")] = json::value::string(stringToTstring(name).c_str());

		}
		else if (path[0] == U("GetStateOfCurrentConsensus"))
		{
			uint64_t blockNo;
			uint16 blockNum = 0;
			uint16 chainNum = 0;
			uint16 uiState = GetStateOfCurrentConsensus(blockNo, blockNum, chainNum);
			vRet[_XPLATSTR("curBuddyNo")] = json::value::number(blockNo);
			if (uiState == IDLE)
			{
				vRet[_XPLATSTR("consensusState")] = json::value::string(_XPLATSTR("idle"));
			}
			else if (uiState == LOCAL_BUDDY)
			{
				vRet[_XPLATSTR("consensusState")] = json::value::string(_XPLATSTR("localbuddy"));
			}
			else if (uiState == GLOBAL_BUDDY)
			{
				vRet[_XPLATSTR("consensusState")] = json::value::string(_XPLATSTR("globalbuddy"));
			}
		}
		else if (path[0] == U("GetDataOfCurrentConsensus"))
		{
			uint64_t blockNo;
			uint16 blockNum = 0;
			uint16 chainNum = 0;
			uint16 uiState = GetStateOfCurrentConsensus(blockNo, blockNum, chainNum);
			vRet[_XPLATSTR("curBuddyNo")] = json::value::number(blockNo);
			if (uiState == IDLE)
			{
				vRet[_XPLATSTR("consensusState")] = json::value::string(_XPLATSTR("idle"));
			}
			else if (uiState == LOCAL_BUDDY)
			{
				vRet[_XPLATSTR("consensusState")] = json::value::string(_XPLATSTR("localbuddy"));
				vRet[_XPLATSTR("blockNum")] = json::value::number(blockNum);
			}
			else if (uiState == GLOBAL_BUDDY)
			{
				vRet[_XPLATSTR("consensusState")] = json::value::string(_XPLATSTR("globalbuddy"));
				vRet[_XPLATSTR("chainNum")] = json::value::number(chainNum);
			}
		}
		else if (path[0] == U("GetConnectedNodesNum"))
		{
			uint32 retNum = GetConnectedNodesNum();
			vRet[_XPLATSTR("connectedNodesNum")] = json::value::number(retNum);
		}
		else if (path[0] == U("CreatCustomerizeConsensusScript"))
		{
			auto cntEntryType = query.find(U("Type"));
			auto cntEntryScript = query.find(U("Script"));

			if (cntEntryType != query.end() && !cntEntryType->second.empty() && cntEntryScript != query.end() && !cntEntryScript->second.empty())
			{

				utility::string_t sType = cntEntryType->second;
				utility::string_t sScript = cntEntryScript->second;
				
				string strType = tstringToUtf8(sType);
				if (0 != strType.compare("xml") || (sScript.length() > (1024 * 2)))
				{
					vRet[_XPLATSTR("returnValue")] = json::value::string(_XPLATSTR("type is wrong"));
				}
				else
				{			
					string strScript = tstringToUtf8(sScript);
					SetScriptPoeRecord(strScript.c_str());

					vRet[_XPLATSTR("returnValue")] = json::value::string(_XPLATSTR("success"));
				}
			}
		}
		http_response response(status_codes::OK);
		response.set_body(vRet);
		response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
		message.reply(response);
		return;
	}
	message.reply(status_codes::OK, json::value(_XPLATSTR("OK")));
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
	message.reply(status_codes::OK, "DE_XPLATSTR(");
}


json::value RestApi::blockHeadToJsonValue(T_HYPERBLOCKDBINFO blockDBInfo)
{
	json::value valHyperBlock;

	valHyperBlock[_XPLATSTR("hyperBlockId")] = json::value::number(blockDBInfo.uiBlockId);
	valHyperBlock[_XPLATSTR("hyperBlockHash")] = json::value::string(stringToTstring(DBmgr::instance()->hash256tostring(blockDBInfo.strHashSelf).c_str()));
	valHyperBlock[_XPLATSTR("hyperCreateTime")] = json::value::number(blockDBInfo.uiBlockTimeStamp);
	valHyperBlock[_XPLATSTR("buddyScript")] = json::value::string(_XPLATSTR(""));
	valHyperBlock[_XPLATSTR("publicKey")] = json::value::string(_XPLATSTR(""));
	valHyperBlock[_XPLATSTR("perHyperBlockHash")] = json::value::string(stringToTstring(DBmgr::instance()->hash256tostring(blockDBInfo.strPreHash).c_str()));
	
	return valHyperBlock;
}


json::value RestApi::blockToJsonValue(T_HYPERBLOCKDBINFO blockDBInfo)
{
	json::value valHyperBlock;

	valHyperBlock[_XPLATSTR("id")] = json::value::number(blockDBInfo.uiBlockId);
	valHyperBlock[_XPLATSTR("hash")] = json::value::string(stringToTstring(DBmgr::instance()->hash256tostring(blockDBInfo.strHashSelf)));
	valHyperBlock[_XPLATSTR("ctime")] = json::value::number(blockDBInfo.uiBlockTimeStamp);

	valHyperBlock[_XPLATSTR("hash_prev")] = json::value::string(stringToTstring(DBmgr::instance()->hash256tostring(blockDBInfo.strPreHash)));

	valHyperBlock[_XPLATSTR("type")] = json::value::number(blockDBInfo.ucBlockType);
	valHyperBlock[_XPLATSTR("hid")] = json::value::number(blockDBInfo.uiReferHyperBlockId);
	valHyperBlock[_XPLATSTR("hhash")] = json::value::string(stringToTstring(DBmgr::instance()->hash256tostring(blockDBInfo.strHyperBlockHash)));

	valHyperBlock[_XPLATSTR("payload")] = json::value::string(stringToTstring(blockDBInfo.strPayload));

	valHyperBlock[_XPLATSTR("chain_num")] = json::value::number(blockDBInfo.uiLocalChainId);
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

			ostringstream_t oss;  
			oss << info.uiReferHyperBlockId;
			string_t sKey=oss.str(); 

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

			ostringstream_t oss;
			oss << info.uiReferHyperBlockId;
			string_t sKey = oss.str();

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
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("onchaing"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(-1);
	}
	else if (nRet == -3)
	{
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("unknown"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(-1);
	}
	else if (nRet == -2)
	{
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("waiting"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(-1);
	}
	else
	{
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("success"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(nRet);
	}

	return vHyperBlocks;
}

json::value RestApi::getOnchainStateEx(string queueId, string localhash, uint64 utime)
{
	json::value vHyperBlocks;
	int64 nRet = GetOnChainState(queueId);

	
	if (nRet == -3) //find in db
	{
		nRet = DBmgr::instance()->getOnChainStateFromHashTime(localhash, utime);
		if (nRet == -1)
			nRet = -3;
	}


	if (nRet == -1)
	{
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("onchaing"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(-1);
	}
	else if (nRet == -3)
	{
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("unknown"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(-1);
	}
	else if (nRet == -2)
	{
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("waiting"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(-1);
	}
	else
	{
		vHyperBlocks[_XPLATSTR("queueId")] = json::value::string(stringToTstring(queueId));
		vHyperBlocks[_XPLATSTR("onChainState")] = json::value::string(stringToTstring("success"));
		vHyperBlocks[_XPLATSTR("hyperBlockId")] = json::value::number(nRet);
	}

	return vHyperBlocks;
}

json::value RestApi::MakeRegistration(string strfilename, string strfilehash, string strcustomInfo, string strrightowner)
{
	vector<string> vcdata;
	vcdata.clear();

	UpqueueEx(strfilename, strfilehash, strcustomInfo, strrightowner, vcdata);
	json::value valQueueID;

	if (vcdata.size() != 3)
	{
		valQueueID[_XPLATSTR("queue_id")] = json::value::string(stringToTstring(""));
		valQueueID[_XPLATSTR("queue_hash")] = json::value::string(stringToTstring(""));
		valQueueID[_XPLATSTR("queue_time")] = json::value::string(stringToTstring(""));
		valQueueID[_XPLATSTR("state")] = json::value::string(stringToTstring("unknown"));
	}
	else
	{
		valQueueID[_XPLATSTR("queue_id")] = json::value::string(stringToTstring(vcdata[0].c_str()));
		valQueueID[_XPLATSTR("queue_hash")] = json::value::string(stringToTstring(vcdata[1].c_str()));
		valQueueID[_XPLATSTR("queue_time")] = json::value::string(stringToTstring(vcdata[2].c_str()));
		valQueueID[_XPLATSTR("state")] = json::value::string(stringToTstring("waiting"));
	}
	
	return valQueueID;
}

#include <QMessageBox>
int RestApi::startRest()
{
	qDebug("start Rest Server");
	try
	{
		restHandler.open().wait();

	}
	catch (std::exception& ex)
	{
		QMessageBox::warning(nullptr, "RestApi Exception", ex.what());
		restHandler.close().wait();
		qDebug(ex.what());
	}

	return 0;
}

int RestApi::stopRest()
{
	qDebug("stop Rest Server");
	try
	{
		restHandler.close().wait();
	}
	catch (std::exception& ex)
	{
		qDebug(ex.what());
	}

	return 0;
}