/*Copyright 2017 hyperchain.net  (Hyper Block Chain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT.
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

void CommandHandler::handle_get(http_request message)
{

	qDebug("get");
	
	utility::string_t hash;

	auto path = requestPath(message);
	if (!path.empty() && path.size() == 1) {
		
		std::map<utility::string_t, utility::string_t> query = uri::split_query(uri::decode(message.request_uri().query()));

		if (path[0] == U("submitregistration")) 
		{
			auto cntEntry = query.find(U("hash"));

			if (cntEntry != query.end() && !cntEntry->second.empty())
			{
				hash = cntEntry->second;
				string sUserData = ws2s(hash);
				RestApi* api = new RestApi;

				json::value valQueueID = api->MakeRegistration(sUserData);
				delete api;
				api = NULL;
				message.reply(status_codes::OK, valQueueID);
				return;
			}
		}
		else if (path[0] == U("gethyperblocks"))
		{
			auto cntEntryId = query.find(U("start_id"));
			auto cntEntryNum = query.find(U("num"));
			utility::string_t sId = cntEntryId->second;
			utility::string_t sNum = cntEntryNum->second;

			uint64_t nHyperBlockId = atoi(ws2s(sId).c_str());
			uint64_t nNum = atoi(ws2s(sNum).c_str());

			RestApi* api = new RestApi;
			json::value vRet = api->getHyperblocks(nHyperBlockId, nNum);

			message.reply(status_codes::OK, vRet);

			return;
		}
		else if (path[0] == U("getonchainstate"))
		{
			auto cntEntry = query.find(U("id"));

			if (cntEntry != query.end() && !cntEntry->second.empty())
			{
				hash = cntEntry->second;
				string sUserData = ws2s(hash);
				RestApi* api = new RestApi;

				json::value vRet = api->getOnchainState(sUserData);

				delete api;
				api = NULL;

				message.reply(status_codes::OK, vRet);
				return;
			}
		}
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


json::value RestApi::blockToJsonValue(T_HYPERBLOCKDBINFO blockDBInfo)
{
	json::value valHyperBlock;

	valHyperBlock[L"hash"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strHashSelf).c_str()));
	valHyperBlock[L"id"] = json::value::number(blockDBInfo.uiBlockId);
	valHyperBlock[L"type"] = json::value::number(blockDBInfo.ucBlockType);
	valHyperBlock[L"hid"] = json::value::number(blockDBInfo.uiReferHyperBlockId);
	valHyperBlock[L"hhash"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strHyperBlockHash).c_str()));
	valHyperBlock[L"hash_prev"] = json::value::string(s2ws(DBmgr::instance()->hash256tostring(blockDBInfo.strPreHash).c_str()));
	valHyperBlock[L"payload"] = json::value::string(s2ws(blockDBInfo.strPayload));
	valHyperBlock[L"ctime"] = json::value::number(blockDBInfo.uiBlockTimeStamp);
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
