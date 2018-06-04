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
#ifndef _SWITCHSOCK_H
#define _SWITCHSOCK_H

#include "sock.h"
#include "mutex.h"

#define BOOL int
#define HTTP_SWITCH_PORT 80


class SwitchSock
{
public:
	SwitchSock();
	~SwitchSock();


	void SetUseHttpSwitch(BOOL bUse=true);
	void SetAsHttpSwitch(BOOL bAs=true);
	bool Create(bool bUDP=false);


	virtual bool Connect(const string& host,unsigned short port);
	virtual bool isConnected();
	virtual bool isUDP();
	virtual bool GetPeerName(string& strIP,unsigned short &nPort);
	virtual bool GetPeerName(unsigned long& nIP,unsigned short &nPort);
	virtual bool GetLocalName(string& strIP,unsigned short &nPort);
	bool Bind(unsigned short nPort);
	bool Accept(SwitchSock& client);
	virtual void Close();

	virtual long Send(const char* buf,long buflen);
	virtual long Recv(char* buf,long buflen, int timeout = SOCKET_TIMEOUT);
	virtual long SendTo(const char* buf,int len,const struct sockaddr_in* toaddr,int tolen);
	virtual long SendTo(const char* buf,int len,const string& strHost,unsigned short nPort);
	virtual long RecvFrom(char* buf,int len,struct sockaddr_in* fromaddr,int* fromlen);
	virtual long RecvFrom(char* buf,int len,string& strHost,unsigned short & nPort);
	virtual long SendHttp(const char* buf,long buflen, bool bXml = false);
	SOCKET GetHandle();
private:
	BaseSock m_sock;
	bool _SendHttpPacket(const string& param,const string& data);
	bool _RecvHttpPacket(string& param,string& data);
	void _ParseParam(const string& param, map<string,string>& paramlist);
private:
	BOOL m_bUseHttpSwitch;
	BOOL m_bAsHttpSwitch;

	string m_strCurSwitch;

	char* m_pHttpRecvBuffer;
	int m_nHttpRecvLen;

	string m_strPeerHost;
	unsigned short m_nPeerPort;
	friend class CHttpSwitch;

	MMutex m_SendRecvLock;

};


#endif
