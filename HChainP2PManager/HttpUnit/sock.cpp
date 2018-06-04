/*copyright 2016-2018 hyperchain.net (Hyperchain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT。
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
#ifdef WIN32
//#include "stdafx.h"
#endif

#include <stdio.h>
#include <errno.h>

#ifndef WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "sock.h"
//#include "../log.h"

#define WriteLog() MangerLog::Instance()

#ifndef WIN32
#define strnicmp strncasecmp
#endif

#pragma comment(lib, "Ws2_32.lib")


BaseSock::BaseSock()
{
		m_bUDP = false;
        m_sock = -1;
		m_nPort = 0;
		m_bConnected = false;

#ifdef WIN32
    WSADATA lwsaData;
    WSAStartup(MAKEWORD(2,2), &lwsaData);
#endif
}

 BaseSock::~BaseSock()
{
        Close();
}

 SOCKET BaseSock::GetHandle()
 {
	 return m_sock;
 }

void BaseSock::Close()
{
        if(m_sock!=-1)
        {
#ifdef WIN32
			shutdown(m_sock,SD_BOTH);
			closesocket(m_sock);
	//		WSACleanup();
#else
			shutdown(m_sock, SHUT_RDWR);
            close(m_sock);
#endif
            m_sock=-1;
        }
		m_bConnected = false;
}

bool BaseSock::Create(bool bUDP)
{
	m_bUDP = bUDP;

	if(!m_bUDP)
		m_sock = socket(AF_INET,SOCK_STREAM,0);
	else
	{
		m_sock = socket(AF_INET,SOCK_DGRAM,0);
	}

	int bufsize = 102400;
	setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(bufsize));

	return (m_sock!=-1);
}


int BaseSock::isIp(const char *ip) 
{ 
	if(strlen(ip) <7||strlen(ip)> 15)
		return -1;

	char	*bpos,*epos; 
	char   	temp[16]; 
	strcpy(temp,ip); 
	bpos = epos = temp; 
	int   	i=0; 
	while(*epos) 
	{ 
		if(*epos== '.') 
		{ 
			i++; 
			*epos=0; 
			if(atoi(bpos)> 255||atoi(bpos) <0)
				return -1;
			bpos = epos+1; 
		} 
		else if(!isdigit(*epos))
			return -1; 
		epos++; 
	} 

	if(i!=3)
		return -1;

	if(atoi(bpos)> 255||atoi(bpos) <0)
		return -1;

	return 0;
}
bool BaseSock::Connect(const string& host,unsigned short port)
{
        if(m_sock==-1)
		return false;

	m_strHost=host;
	m_nPort=port;

	bool bflag = false;
	struct hostent *he = NULL;
	if(isIp(host.c_str()) < 0)
	{
		bflag = true;
		he = gethostbyname(host.c_str());
		if(he == NULL)
		{
			Close();
			return false;
		}
	}	 		

    struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
    sin.sin_family=AF_INET;
	if(bflag)
	{
        	sin.sin_addr=*((struct   in_addr*)he->h_addr);
	}
	else
	{
		sin.sin_addr.s_addr = inet_addr(m_strHost.c_str());
	}

        sin.sin_port=htons(port);
        if(connect(m_sock ,(struct sockaddr *)&sin,sizeof(sin)))
        {
                Close();
                return false;
        }


		m_bConnected =  true;
        return true;
}

long BaseSock::Send(const char* buf,long buflen)
{

        if(m_sock==-1)
        {
          return -1;
        }

		int start=0;
		int sended=0;
		do
		{
			int len =send(m_sock,buf+sended,buflen-sended,0);
			if(len<0)
			{
				break;
			}
			sended+=len;
		}while(sended<buflen);

        return sended;
}

long BaseSock::Recv(char* buf,long buflen, int timeout)
{
   if(m_sock==-1)
   {
       return -1;
    }
	int i = 0;
	while(i < 5)
	{
        	fd_set fd;
        	FD_ZERO(&fd);
        	FD_SET(m_sock, &fd);
        	struct timeval  val = {timeout, 10};

        	int selret = select(m_sock+1,&fd,NULL,NULL,&val);
        	if(selret == 0)
		{
			return selret;
		}
		if(selret < 0)
		{
			if(errno == EINTR)
			{
				i ++;
				continue;
			}
			return selret;
		}
		
		int len = recv(m_sock, buf, buflen, 0);
		return len;
	}
	return -1;
}

bool BaseSock::GetPeerName(struct sockaddr_in* addr,int* fromlen)
{
#ifdef WIN32
	if(getpeername(m_sock,(struct sockaddr*)addr,fromlen)!=0)
#else
	if(getpeername(m_sock,(struct sockaddr*)addr,(socklen_t*)fromlen)!=0)
#endif
		return false;
	return true;
}
bool BaseSock::GetLocalName(struct sockaddr_in* addr,int* fromlen)
{
#ifdef WIN32
	if(getsockname(m_sock,(struct sockaddr*)addr,fromlen)!=0)
#else
	if(getsockname(m_sock,(struct sockaddr*)addr,(socklen_t*)fromlen)!=0)
#endif
		return false;
	return true;
}

bool BaseSock::GetPeerName(unsigned long &nIP, unsigned short &nPort)
{
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	int addrlen=sizeof(addr);
	
#ifdef WIN32
	if(getpeername(m_sock,(struct sockaddr*)&addr, &addrlen)!=0)
#else
	if(getpeername(m_sock,(struct sockaddr*)&addr,(socklen_t*)&addrlen)!=0)
#endif
		return false;

#ifdef WIN32
	nIP = addr.sin_addr.S_un.S_addr;
#else
	nIP = addr.sin_addr.s_addr;
#endif

	nPort = ntohs(addr.sin_port);
	return true;

}

bool BaseSock::GetPeerName(string& strIP,unsigned short &nPort)
{
    struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	int addrlen=sizeof(addr);

	if(!GetPeerName(&addr,&addrlen))
		return false;

	char szIP[64];
#ifdef WIN32
	sprintf(szIP,"%u.%u.%u.%u",addr.sin_addr.S_un.S_addr&0xFF,(addr.sin_addr.S_un.S_addr>>8)&0xFF,(addr.sin_addr.S_un.S_addr>>16)&0xFF,(addr.sin_addr.S_un.S_addr>>24)&0xFF);
#else
	sprintf(szIP,"%u.%u.%u.%u",addr.sin_addr.s_addr&0xFF,(addr.sin_addr.s_addr>>8)&0xFF,(addr.sin_addr.s_addr>>16)&0xFF,(addr.sin_addr.s_addr>>24)&0xFF);
#endif  

	strIP=szIP;
	nPort=ntohs(addr.sin_port);
	return true;
}

bool BaseSock::GetLocalName(string& strIP,unsigned short &nPort)
{
    struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	int addrlen=sizeof(addr);

	if(!GetLocalName(&addr,&addrlen))
		return false;

	char szIP[64];
#ifdef WIN32
	sprintf(szIP,"%u.%u.%u.%u",addr.sin_addr.S_un.S_addr&0xFF,(addr.sin_addr.S_un.S_addr>>8)&0xFF,(addr.sin_addr.S_un.S_addr>>16)&0xFF,(addr.sin_addr.S_un.S_addr>>24)&0xFF);
#else
	sprintf(szIP,"%u.%u.%u.%u",addr.sin_addr.s_addr&0xFF,(addr.sin_addr.s_addr>>8)&0xFF,(addr.sin_addr.s_addr>>16)&0xFF,(addr.sin_addr.s_addr>>24)&0xFF);
#endif
	strIP=szIP;
	nPort=ntohs(addr.sin_port);
	return true;
}

bool BaseSock::isConnected()
{
        return (m_sock!=-1)&&m_bConnected;
}

bool BaseSock::isUDP()
{
        return m_bUDP;
}

long BaseSock::SendTo(const char* buf,int len,const struct sockaddr_in* toaddr,int tolen)
{
        if(m_sock==-1)
        {
          return -1;
        }

		return sendto(m_sock,buf,len,0,(const struct sockaddr*)toaddr,tolen);
}

long BaseSock::RecvFrom(char* buf,int len,struct sockaddr_in* fromaddr,int* fromlen)
{
        if(m_sock==-1)
        {
          return -1;
        }
#ifdef WIN32
		return recvfrom(m_sock,buf,len,0,(struct sockaddr*)fromaddr,fromlen);
#else
		return recvfrom(m_sock,buf,len,0,(struct sockaddr*)fromaddr,(socklen_t*)fromlen);
#endif
}

bool BaseSock::Bind(unsigned short nPort)
{
    if(m_sock==-1)
    {
      return false;
    }

    char* localIP = const_cast<char*>("0.0.0.0");
    struct sockaddr_in sin;
    sin.sin_family=AF_INET;
#ifdef WIN32
    sin.sin_addr.S_un.S_addr=inet_addr(localIP);
#else
	sin.sin_addr.s_addr=inet_addr(localIP);
#endif
    memset(sin.sin_zero,0,8);
    sin.sin_port=htons(nPort);

	if(bind(m_sock,(sockaddr*)&sin,sizeof(sockaddr_in))!=0)
		return false;
	listen(m_sock,1024);

	m_bConnected = true;
	return true;
}
bool BaseSock::Accept(BaseSock& client)
{
    if(m_sock==-1)
    {
      return false;
    }

	client.m_sock = accept(m_sock,NULL,NULL);
	client.m_bConnected = true;

	return (client.m_sock != -1);
}
