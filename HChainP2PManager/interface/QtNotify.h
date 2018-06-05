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
#pragma once
#include "../headers/inter_public.h"
#include "../headers/commonstruct.h"
#include <QObject>

enum _enodestate
{
	IDLE =0,
	LOCAL_BUDDY,
	GLOBAL_BUDDY
};

class QtNotify : public QObject
{
	Q_OBJECT
public:
	QtNotify();
	~QtNotify();

	void SetHyperBlock(string hash, time_t time, uint64 blocknumber);
	void SetNodeStatus(uint16 status);
	void SetGlobleBuddyChainNum(uint16 number);
	void SetLocalBuddyChainInfo(LIST_T_LOCALCONSENSUS chaininfo);
	void SetConnectNodeUpdate(uint32 betternum, uint32 normalnum, uint32 badnum, uint32 downnum);
	void SetSendPoeNum(uint32 number);
	void SetReceivePoeNum(uint32 number);
	void SetBuddyStartTime(time_t stime, uint64 blocknumber);
	void SetNodeInfo(string info, string ip, uint16 port);
	void SetServerInfo(VEC_T_PPEERCONF info);
	void SetBuddyStop();
	void SetHyperBlockNumFromLocal(list<uint64> HyperBlockNum);
	void SetBuddyFailed(string hash, time_t time);
	void SetStatusMes(string msg);

signals:
	void SIG_BuddyStartTime(time_t stime, uint64 blocknumber);
	void SIG_LocalBuddyChainInfo(LIST_T_LOCALCONSENSUS chaininfo);
	void SIG_HyperBlock(string hash, time_t time, uint64 blocknumber);
	void SIG_NodeStatus(uint16 status);
	void SIG_GlobleBuddyChainNum(uint16 number);
	void SIG_ConnectNodeUpdate(uint32 betternum, uint32 normalnum, uint32 badnum, uint32 downnum);
	void SIG_SendPoeNum(uint32 number);
	void SIG_ReceivePoeNum(uint32 number);
	void SIG_NodeInfo(string info, string ip, uint16 port);
	void SIG_ServerInfo(VEC_T_PPEERCONF info);
	void SIG_BuddyStop();
	void SIG_HyperBlockNumFromLocal(list<uint64> HyperBlockNum);
	void SIG_BuddyFailed(string hash, time_t time);
	void SIG_StatusMes(string msg);

};

