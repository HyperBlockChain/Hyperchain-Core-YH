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
#include "QtNotify.h"

QtNotify::QtNotify()
{
}

QtNotify::~QtNotify()
{
}

void QtNotify::SetHyperBlock(string hash, time_t time, uint64 blocknumber)
{
	emit SIG_HyperBlock(hash, time, blocknumber);
}

void QtNotify::SetNodeStatus(uint16 status)
{
	emit SIG_NodeStatus(status);
}

void QtNotify::SetBuddyStartTime(time_t stime, uint64 blocknumber)
{
	emit SIG_BuddyStartTime(stime, blocknumber);
}

void QtNotify::SetGlobleBuddyChainNum(uint16 number)
{
	emit SIG_GlobleBuddyChainNum(number);
}

void QtNotify::SetLocalBuddyChainInfo(LIST_T_LOCALCONSENSUS chaininfo)
{
	emit SIG_LocalBuddyChainInfo(chaininfo);
}

void QtNotify::SetConnectNodeUpdate(uint32 betternum, uint32 normalnum, uint32 badnum, uint32 downnum)
{
	emit SIG_ConnectNodeUpdate(betternum, normalnum, badnum, downnum);
}

void QtNotify::SetSendPoeNum(uint32 number)
{
	emit SIG_SendPoeNum(number);
}

void QtNotify::SetReceivePoeNum(uint32 number)
{
	emit SIG_ReceivePoeNum(number);
}

void  QtNotify::SetNodeInfo(string info, string ip, uint16 port)
{
	emit SIG_NodeInfo(info, ip, port);
}

void  QtNotify::SetServerInfo(VEC_T_PPEERCONF info)
{
	emit SIG_ServerInfo(info);
}

void QtNotify::SetBuddyStop()
{
	emit SIG_BuddyStop();
}

void QtNotify::SetHyperBlockNumFromLocal(list<uint64> HyperBlockNum)
{
	emit SIG_HyperBlockNumFromLocal(HyperBlockNum);
}

void QtNotify::SetBuddyFailed(string hash, time_t time)
{
	emit SIG_BuddyFailed(hash, time);
}

void QtNotify::SetStatusMes(string msg)
{
	emit SIG_StatusMes(msg);
}