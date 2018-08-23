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

#ifndef _SOCK_H
#define _SOCK_H
//---------------------------------------------------------------------------
#ifdef WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#define SOCKET int
#endif

#pragma warning (disable: 4786)

#include <string>
#include <vector>
#include <map>
using namespace std;

#define SOCKET_TIMEOUT 4
#define MAX_RECV_BUFFERSIZE 65530

class SwitchSock;
class CHttpSwitch;

class BaseSock
{
public:
	SOCKET m_sock;

public:
	BaseSock();
	virtual ~BaseSock();

	bool Create(bool bUDP=false);
	int isIp(const char *ip);	

	virtual bool Connect(const string& host,unsigned short port);
	virtual bool isConnected();
	virtual bool isUDP();
	virtual bool Bind(unsigned short nPort);
	virtual bool Accept(BaseSock& client);
	virtual void Close();

	virtual long Send(const char* buf,long buflen);
	virtual long Recv(char* buf,long buflen, int timeout = SOCKET_TIMEOUT);
	virtual long SendTo(const char* buf,int len,const struct sockaddr_in* toaddr,int tolen);
	virtual long RecvFrom(char* buf,int len,struct sockaddr_in* fromaddr,int* fromlen);

	virtual bool GetPeerName(string& strIP,unsigned short &nPort);
	virtual bool GetPeerName(unsigned long &nIP, unsigned short &nPort);
	virtual bool GetLocalName(string& strIP,unsigned short &nPort);
	virtual bool GetPeerName(struct sockaddr_in* fromaddr,int* fromlen);
	virtual bool GetLocalName(struct sockaddr_in* fromaddr,int* fromlen);

	SOCKET GetHandle();
private:
	bool m_bUDP;
	bool m_bConnected;

	string m_strHost;
	unsigned short m_nPort;

	friend class CHttpSwitch;
	friend class SwitchSock;
};

extern	string IntToStr(unsigned long iValue);

#endif
