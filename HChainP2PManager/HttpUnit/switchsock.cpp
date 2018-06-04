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
#ifdef WIN32
//#include "stdafx.h"
#endif

#include "switchsock.h"
#include <map>
using namespace std;


static BOOL s_bUseSwitch=false;
static MMutex s_SocketListLock;
static map<SOCKET,SwitchSock*> s_SocketList;
static BOOL s_bUseProxy=false;
static string s_szHttpProxy = "";
static unsigned short s_nHttpProxyPort=0;


static vector< pair<string, unsigned short> > g_vecHttpSwitchHost;
const char c_strDefaultHttpSwitchHost[] = "114.30.47.10";//218.61.32.99


SwitchSock::SwitchSock()
{
	m_bUseHttpSwitch=s_bUseSwitch;

	m_bAsHttpSwitch=false;
	m_pHttpRecvBuffer = NULL;
	m_nHttpRecvLen = 0;
}

SwitchSock::~SwitchSock()
{
	if(m_pHttpRecvBuffer!=NULL)
		delete [] m_pHttpRecvBuffer;
	m_pHttpRecvBuffer = NULL;

}

SOCKET SwitchSock::GetHandle()
{
	return m_sock.GetHandle();
}

void SwitchSock::SetUseHttpSwitch(BOOL bUse)
{
	Close();

	m_bUseHttpSwitch=bUse;

	if(m_bUseHttpSwitch)
	{
		if(m_pHttpRecvBuffer == NULL)
		{
			m_pHttpRecvBuffer =  new char[MAX_RECV_BUFFERSIZE+10];
		}
	}else
	{
		if(m_pHttpRecvBuffer != NULL)
		{
			delete [] m_pHttpRecvBuffer;
			m_pHttpRecvBuffer = NULL;
		}
	}
}

void SwitchSock::SetAsHttpSwitch(BOOL bUse)
{
	m_bAsHttpSwitch=bUse;
	if(m_bAsHttpSwitch)
	{
		SetUseHttpSwitch(true);
	}
}

bool SwitchSock::Create(bool bUDP)
{
	Close();

	if(!m_bUseHttpSwitch)
		return m_sock.Create(bUDP);

	if(!m_bAsHttpSwitch)
	{

		pair<string, unsigned short> host;
		if(g_vecHttpSwitchHost.empty())
		{
			host.first = c_strDefaultHttpSwitchHost;
			host.second  = HTTP_SWITCH_PORT;
		}else
		{
			host = g_vecHttpSwitchHost[rand()%g_vecHttpSwitchHost.size()];
		}

		m_strCurSwitch = host.first;

		if(!m_sock.Create(false))
			return false;

		if(s_bUseProxy)
		{
			if(!m_sock.Connect(s_szHttpProxy,s_nHttpProxyPort))
			{
				return false;
			}
		}else
		{
			if(!m_sock.Connect(m_strCurSwitch,host.second ))
			{
				return false;
			}
		}

		if(!_SendHttpPacket("Cmd=Create&UDP="+IntToStr(bUDP),""))
		{
			return false;
		}

		return true;
	}

	return true;
}

bool SwitchSock::Bind(unsigned short nPort)
{
	return m_sock.Bind(nPort);
}
bool SwitchSock::Accept(SwitchSock& client)
{
	return m_sock.Accept(client.m_sock);
}

bool SwitchSock::Connect(const string& host,unsigned short port)
{
	if(!m_bUseHttpSwitch)
		return m_sock.Connect(host,port);

	m_strPeerHost = host;
	m_nPeerPort =port;

	string param,data;
	map<string,string> paramlist;

	if(!_SendHttpPacket("Cmd=Connect&Host="+host+"&Port="+IntToStr(port),""))
	{
		goto RETURN;
	}

	if(!_RecvHttpPacket(param,data))
	{
		goto RETURN;
	}

	_ParseParam(param,paramlist);

	if(paramlist["Cmd"]=="Connected")
		return true;

RETURN:
	m_sock.Close();
	return false;
}

bool SwitchSock::GetPeerName(unsigned long& nIP,unsigned short &nPort)
{
	return m_sock.GetPeerName(nIP, nPort);
}

bool SwitchSock::GetPeerName(string& strIP,unsigned short &nPort)
{
	if(!m_bUseHttpSwitch)
		return m_sock.GetPeerName(strIP,nPort);

	string param,data;
	map<string,string> paramlist;

	if(!_SendHttpPacket("Cmd=GetPeerName",""))
	{
		goto RETURN;
	}

	if(!_RecvHttpPacket(param,data))
	{
		goto RETURN;
	}

	_ParseParam(param,paramlist);

	strIP=paramlist["PeerIP"];
	nPort=atol(paramlist["PeerPort"].c_str());
	return (strIP!="");

RETURN:
	m_sock.Close();
	return false;
}

bool SwitchSock::GetLocalName(string& strIP,unsigned short &nPort)
{
	if(!m_bUseHttpSwitch)
		return m_sock.GetLocalName(strIP,nPort);

	string param,data;
	map<string,string> paramlist;

	if(!_SendHttpPacket("Cmd=GetLocalName",""))
	{
		goto RETURN;
	}

	if(!_RecvHttpPacket(param,data))
	{
		goto RETURN;
	}

	_ParseParam(param,paramlist);

	strIP=paramlist["LocalIP"];
	nPort=atol(paramlist["LocalPort"].c_str());
	return (strIP!="");

RETURN:
	m_sock.Close();
	return false;
}
bool SwitchSock::isConnected()
{
	return m_sock.isConnected();
}

bool SwitchSock::isUDP()
{
	return m_sock.isUDP();
}
void SwitchSock::Close()
{
	m_sock.Close();
}

long SwitchSock::Send(const char* buf,long buflen)
{
	if(!m_bUseHttpSwitch)
		return m_sock.Send(buf,buflen);

	if(!_SendHttpPacket("Cmd=Send",string(buf,buflen)))
		return -1;
	return buflen;
}

long SwitchSock::SendHttp(const char* buf,long buflen, bool bXml)
{
	string httphead;
	httphead="HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nP3P: CP=CAO PSA OUR\r\nnConnection: Close\r\nContent-Length: "+IntToStr(buflen)+"\r\n\r\n";	
	
	if (bXml)
	{
		httphead="HTTP/1.0 200 OK\r\nContent-Type: text/xml\r\nnConnection: Close\r\nContent-Length: "+IntToStr(buflen)+"\r\n\r\n";	
	}

	if(m_sock.Send(httphead.c_str(),httphead.length())!=httphead.length())
	{
		return false;
	}

	if(buflen!=0)
	{
		if(m_sock.Send(buf,buflen)!= buflen)
		{
			return false;
		}
	}
	return true;
}

long SwitchSock::Recv(char* buf,long buflen, int timeout)
{
	if(!m_bUseHttpSwitch)
		return m_sock.Recv(buf,buflen, timeout);

	m_SendRecvLock.Lock();
	string param,data;
	if(!_RecvHttpPacket(param,data))
	{
		m_SendRecvLock.Unlock();
		return -1;
	}

	if(buflen<data.length())
	{
		memcpy(buf,data.c_str(),buflen);
		m_SendRecvLock.Unlock();
		return buflen;
	}
	
	memcpy(buf,data.c_str(),data.length());
	m_SendRecvLock.Unlock();
	return data.length();
}

long SwitchSock::SendTo(const char* buf,int buflen,const struct sockaddr_in* toaddr,int tolen)
{
	if(!m_bUseHttpSwitch)
		return m_sock.SendTo(buf,buflen,toaddr,tolen);


#ifdef WIN32
	if(!_SendHttpPacket("Cmd=SendTo&IP="+IntToStr(toaddr->sin_addr.S_un.S_addr)+"&Port="+IntToStr(toaddr->sin_port),string(buf,buflen)))
#else
	if(!_SendHttpPacket("Cmd=SendTo&IP="+IntToStr(toaddr->sin_addr.s_addr)+"&Port="+IntToStr(toaddr->sin_port),string(buf,buflen)))
#endif
	{
		m_SendRecvLock.Unlock();
		return -1;
	}

	return buflen;
}

long SwitchSock::SendTo(const char* buf,int len,const string& strHost,unsigned short nPort)
{
        struct hostent * he=gethostbyname(strHost.c_str());
        if(he==NULL)
        {
                return -1;
        }
        struct sockaddr_in sin;
        sin.sin_family=AF_INET;
        sin.sin_addr=*((struct   in_addr*)he->h_addr);
        memset(sin.sin_zero,0,8);
        sin.sin_port=htons(nPort);
		return SendTo(buf,len,&sin,sizeof(sin));
}

long SwitchSock::RecvFrom(char* buf,int buflen,struct sockaddr_in* fromaddr,int* fromlen)
{
	if(!m_bUseHttpSwitch)
		return m_sock.RecvFrom(buf,buflen,fromaddr,fromlen);

	m_SendRecvLock.Lock();
	string param,data;
	if(!_RecvHttpPacket(param,data))
	{
		m_SendRecvLock.Unlock();
		return -1;
	}

	map<string,string> paramlist;
	size_t i=0;
	_ParseParam(param,paramlist);

	if(fromaddr !=NULL)
	{
#ifdef WIN32
		fromaddr->sin_addr.S_un.S_addr = atol(paramlist["IP"].c_str());
#else
		fromaddr->sin_addr.s_addr = atol(paramlist["IP"].c_str());
#endif
		fromaddr->sin_port = atol(paramlist["Port"].c_str());
	}

	if(buflen<data.length())
	{
		memcpy(buf,data.c_str(),buflen);
		m_SendRecvLock.Unlock();
		return buflen;
	}
	
	memcpy(buf,data.c_str(),data.length());
	m_SendRecvLock.Unlock();
	return data.length();
}

long SwitchSock::RecvFrom(char* buf,int len,string& strHost,unsigned short & nPort)
{
	struct sockaddr_in addr;
	int fromlen=sizeof(addr);                                                    
	long nRecvLen=RecvFrom(buf,len,&addr,&fromlen);

	char szIP[64];
#ifdef WIN32
	sprintf(szIP,"%u.%u.%u.%u",addr.sin_addr.S_un.S_addr&0xFF,(addr.sin_addr.S_un.S_addr>>8)&0xFF,(addr.sin_addr.S_un.S_addr>>16)&0xFF,(addr.sin_addr.S_un.S_addr>>24)&0xFF);
#else
	sprintf(szIP,"%u.%u.%u.%u",addr.sin_addr.s_addr&0xFF,(addr.sin_addr.s_addr>>8)&0xFF,(addr.sin_addr.s_addr>>16)&0xFF,(addr.sin_addr.s_addr>>24)&0xFF);
#endif
	strHost=szIP;
	nPort=ntohs(addr.sin_port);
	return nRecvLen;
}

bool SwitchSock::_SendHttpPacket(const string& param,const string& data)
{
	string httphead;
	if(!m_bAsHttpSwitch)
	{
		httphead="POST http://"+m_strCurSwitch+":"+IntToStr(HTTP_SWITCH_PORT)+"/?"+param+" HTTP/1.0\r\nConnection: Keep-Alive\r\nAccept: */*.\r\nContent-Length: "+IntToStr(data.length())+"\r\nHost: "+m_strCurSwitch+"\r\n\r\n";
	}else
	{
		httphead="HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nParam:"+param+"\r\nConnection: Keep-Alive\r\nContent-Length: "+IntToStr(data.length())+"\r\n\r\n";	
	}
	if(m_sock.Send(httphead.c_str(),httphead.length())!=httphead.length())
	{
		return false;
	}

	if(data.length()!=0)
	{
		if(m_sock.Send(data.c_str(),data.length())!=data.length())
		{
			return false;
		}
	}
	return true;
}

bool SwitchSock::_RecvHttpPacket(string& param,string& data)
{
	param="";
	data="";

	if(m_pHttpRecvBuffer == NULL)
	{
		return false;
	}

	char* pContent=NULL;
	m_pHttpRecvBuffer[m_nHttpRecvLen]='\0';
	pContent=strstr(m_pHttpRecvBuffer,"\r\n\r\n");

	int nErrorCount = 0;
	while(pContent==NULL)
	{
		int nRecvLen=m_sock.Recv(m_pHttpRecvBuffer+m_nHttpRecvLen,MAX_RECV_BUFFERSIZE);
		if(nRecvLen <= 0)
		{
			if(nRecvLen == 0)
			{
				if(nErrorCount++ < 100)
				{
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);
#endif
					continue;
				}
			}
			return false;
		}
		nErrorCount = 0;

		m_nHttpRecvLen+=nRecvLen;

		if(m_nHttpRecvLen>MAX_RECV_BUFFERSIZE)
			return false;

		m_pHttpRecvBuffer[m_nHttpRecvLen]='\0';

		pContent=strstr(m_pHttpRecvBuffer,"\r\n\r\n");
	}

	*pContent='\0';
	pContent+=4;

	int nCntLen=0;

	if(!m_bAsHttpSwitch)
	{
		char* pParam=strstr(m_pHttpRecvBuffer,"Content-Length");
		if(pParam == NULL)
			return false;
		pParam = strchr(pParam+strlen("Content-Length"),':');
		if(pParam == NULL)
			return false;
		pParam+=1;

		nCntLen=atol(pParam);

		pParam=strstr(m_pHttpRecvBuffer,"Param");
		if(pParam == NULL)
			return false;
		pParam = strchr(pParam+strlen("Param"),':');
		if(pParam == NULL)
			return false;
		pParam+=1;

		char* pEnd=strchr(pParam,'\r');
		if(pEnd!=NULL)
		{
			*pEnd='\0';
		}
		pEnd=strchr(pParam,'\n');
		if(pEnd!=NULL)
		{
			*pEnd='\0';
		}
		param = pParam;
	}else
	{
		char* pParam=strchr(m_pHttpRecvBuffer,'?');
		if(pParam==NULL)
			return false;
		pParam+=1;

		char* pEnd=strchr(pParam,' ');
		if(pEnd==NULL)
			return false;
		*pEnd='\0';
		param=pParam;

		pParam=strstr(pEnd+1,"Content-Length");
		if(pParam == NULL)
			return false;
		pParam = strchr(pParam+strlen("Content-Length"),':');
		if(pParam == NULL)
			return false;
		pParam+=1;

		nCntLen=atol(pParam);
	}

	data.resize(nCntLen);
	if(m_nHttpRecvLen-(pContent-m_pHttpRecvBuffer)<nCntLen)
	{
		int nRecved=0;
		nRecved = m_nHttpRecvLen-(pContent-m_pHttpRecvBuffer);
		memcpy((char*)data.c_str(),pContent,nRecved);
		m_nHttpRecvLen = 0;

		int nErrorCount = 0;
		do
		{
			int nRecvLen=m_sock.Recv((char*)data.c_str()+nRecved,nCntLen-nRecved);
			if(nRecvLen <= 0)
			{
				if(nRecvLen == 0)
				{
					if(nErrorCount++ < 100)
					{
#ifdef WIN32
						Sleep(50);
#else
						usleep(50);
#endif
						continue;
					}
				}
				return false;
			}
			nErrorCount = 0;

			nRecved+=nRecvLen;
		}while(nRecved<nCntLen);
	}else
	{
		memcpy((char*)data.c_str(),pContent,nCntLen);
		m_nHttpRecvLen = m_nHttpRecvLen-(pContent-m_pHttpRecvBuffer)-nCntLen;
		memcpy(m_pHttpRecvBuffer,pContent+nCntLen,m_nHttpRecvLen);
	}
	
	return true;
}

string IntToStr(unsigned long iValue)
{
        char szBuf[80];
        sprintf(szBuf, "%u", iValue);
        return szBuf;
}


void SwitchSock::_ParseParam(const string& param, map<string,string> &paramlist)
{
	paramlist.clear();
	int start=0;
	int endpos=string::npos;

	do
	{
		int valuepos=param.find('=',start);
		if(valuepos==string::npos)
			break;

		endpos=param.find('&',start);
		if(endpos==string::npos)
		{
			paramlist[param.substr(start,valuepos-start)]=param.substr(valuepos+1,param.length());
		}else
		{
			paramlist[param.substr(start,valuepos-start)]=param.substr(valuepos+1,endpos-valuepos-1);
			start=endpos+1;
		}

	}while(endpos!=string::npos);
}
