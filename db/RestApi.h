/*Copyright 2017 hyperchain.net  (Hyperchain)
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
#ifndef __RESTAPI_H__
#define __RESTAPI_H__

#include <qdebug>
#include <thread>
#include <stdio.h>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include "HyperchainDB.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
using namespace web::http;
using namespace web::http::client;

class CommandHandler
{
public:
	CommandHandler() {}
	CommandHandler(utility::string_t url, http_listener_config server_config);
	pplx::task<void> open() { return m_listener.open(); }
	pplx::task<void> close() { return m_listener.close(); }
private:

	void handle_get(http_request message);
	void handle_post(http_request message);
	void handle_put(http_request message);
	void handle_del(http_request message);
	http_listener m_listener;
};

class RestApi
{
public:
	RestApi() {}
	~RestApi() {}
public:

	static int startRest();
public:
	json::value blockToJsonValue(T_HYPERBLOCKDBINFO blockDBInfo);
	json::value blockHeadToJsonValue(T_HYPERBLOCKDBINFO blockDBInfo);

	json::value MakeRegistration(string strUserData);

	json::value getHyperblocks(uint64_t nStartId, uint64_t nNum);

	json::value RestApi::getOnchainState(string queueId);

	json::value getHyperblocksHead(uint64_t nStartId);
	int RestApi::getLatestHyperBlockNo();
};

#endif