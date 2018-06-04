﻿/*copyright 2016-2018 hyperchain.net (Hyperchain)
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

#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

#include "../headers/platform.h"
#include "../headers/commonstruct.h"
#include "../utility/MutexObj.h"
#include "../debug/Log.h"
#include "../crypto/crc32.h"
#ifdef WIN32
#include <winsock2.h>
#endif

#define MAX_RECV_BUF_SIZE (1024*16)
#define MAX_BUF_LEN (1024 * 32)
#define MAX_RECV_LIST_COUNT (20000)
#define MAX_INTER_FACES (16)
#define MAX_CRC_LEN (16)
#define CURRENT_VERSION ('1')
#define MAX_LIST_NUM (4294967295)
#define UDP_INIT_FLAG (123456)
#define UDP_INIT_PAKTYPE ('1')
#define UDP_ACK_PAKTYPE  ('2')

enum _esendtype
{
	SYN_TYPE = 1,
	ACK_TYPE = 2
};
enum _erecvflag
{
	DEFAULT = 0,
	ACK_FALG
};

#pragma pack(1)
typedef struct _ttestdata
{
    unsigned short    usInsertSendListNum;
    unsigned short usRetryTimes;
    unsigned short usActualSendReqNum;
	volatile unsigned short usRecvReqAckNum;
    unsigned short usRecvRspNum;
    unsigned short usSendRspAckNum;
	unsigned short usSendFailed;
	unsigned int   usSendListCount;
	unsigned short usFastTime;
	unsigned short usSlowTime;
	unsigned short usRecvListNum;

	_ttestdata()
	{
		usInsertSendListNum = 0;
		usRetryTimes 		= 0;
		usActualSendReqNum 	= 0;
		usRecvReqAckNum 	= 0;
		usRecvRspNum 		= 0;
		usSendRspAckNum 	= 0;
		usSendFailed 		= 0;
		usSendListCount     = 0;
		usFastTime			= 100;
		usSlowTime			= 0;
		usRecvListNum		= 0;
	}

	_ttestdata& operator = (const _ttestdata& param)
	{
		if(this != &param)
		{
			this->usInsertSendListNum = param.usInsertSendListNum;
			this->usRetryTimes        = param.usRetryTimes;
			this->usActualSendReqNum  = param.usActualSendReqNum;
			this->usRecvReqAckNum     = param.usRecvReqAckNum;
			this->usRecvRspNum		  = param.usRecvRspNum;
			this->usSendRspAckNum     = param.usSendRspAckNum;
			this->usSendFailed		  = param.usSendFailed;
			this->usSendListCount     = param.usSendListCount;
			this->usFastTime		  = param.usFastTime;
			this->usSlowTime		  = param.usSlowTime;
			this->usRecvListNum 	  = param.usRecvListNum;
		}
		return *this;
	}
}T_TESTDATA, *T_PTESTDATA;

typedef struct _tsendnode
{
	 unsigned short				usPort;
	 unsigned int 				uiIp;
	 unsigned int 				uiSendLen;
	 unsigned int 				uiRetryTimes;
	 char 						*sendBuf;

	 uint64						uiLastSendTime;
	 unsigned short				usFlag;
}T_SENDNODE, *T_PSENDNODE;

typedef struct _tuuseehead
{
	 unsigned int	usPackNum;
	 unsigned int   uiUuseeFlag;
	 unsigned int   uiSendBufCrc;
     unsigned int  	uiBufLen;
	 char		  	PackType;
	 char			Version;
}T_UUSEEHEAD, *T_PUUSEEHEAD;

#pragma pack()

typedef list<T_SENDNODE>			 LIST_T_SENDNODE;
typedef LIST_T_SENDNODE::iterator    ITR_LIST_T_SENDNODE;

typedef list<T_PSENDNODE>            LIST_T_PSENDNODE;
typedef LIST_T_PSENDNODE::iterator   ITR_LIST_T_PSENDNODE;

typedef map<unsigned int, T_PSENDNODE>		MAP_T_PSENDNODE;
typedef MAP_T_PSENDNODE::iterator    ITR_MAP_T_PSENDNODE;

class CUdpSocket
{
public:

	CUdpSocket();
	virtual ~CUdpSocket();

	enum _esendresult
	{
		LOCAL_IP = -2,
		SEND_FAILED = -1,
		SEND_SUCCESS
	};

	enum _erecvresult
	{
		RECV_LIST_EMPTY = -1,
		RECV_SUCCESS = 1,
		RECV_BUF_NOT_ENOUGH
	};

public:
	void SetUsed(bool bFlag) { m_bUsed = bFlag; }
	bool IsUsed(){ return m_bUsed; }
	int Init(const char* localIp, unsigned int localPort);
	int Send(unsigned int ip, unsigned short port, const char* buf, unsigned int len);
	int Recv(unsigned int &ip, unsigned short &port, char* buf, unsigned int &len);
	unsigned int GetSendListNum();
	string GetStatus();
	uint64 GetRecvRate();
	string GetRecvSize();
	uint64 GetSendRate();
	string GetSendSize();

private:
	void RecvData();
	void SendAgain();
	static void THREAD_API RecvDataEntry(void* pParam);
	static void THREAD_API SendAgainEntry(void* pParam);

	string			GetLocalIp(int fd, const string& eth);
	void 			OupPutTestData();
	string			SetNetNum(string netSize, uint64 recvNum);
private:
	bool					m_bUsed;
    unsigned int			m_packetNum;
	unsigned int			m_localPort;
	const char				*m_localIp;
#ifdef WIN32
	SOCKET					m_listenFd;
#else
	int						m_listenFd;
#endif
	LIST_T_SENDNODE			m_recvList;
	LIST_T_PSENDNODE		m_sendList;
	MAP_T_PSENDNODE			m_sendMap;
	CMutexObj				m_recvListLock;
	CMutexObj				m_sendMapLock;

	int						m_networkCardNum;
	T_TESTDATA				m_testData;
	T_TESTDATA				m_GetData;

	string					m_netRecv;
	uint64					m_netRecvTemp;
	uint64					m_netRateRecv;

	string					m_netSend;
	uint64					m_netSendTemp;
	uint64					m_netRateSend;
};

#endif
