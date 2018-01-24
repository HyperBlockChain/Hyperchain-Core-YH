/*Copyright 2017 hyperchain.net (Hyper Block Chain)
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
#include "udpsocket.h"
#define SEND_TIMES (5)

#ifdef WIN32
extern void win_gettimeofday(struct timeval *tp);
#define GETTIMEOFDAY(ptr)	win_gettimeofday(ptr)
#else
#define GETTIMEOFDAY(ptr)	gettimeofday(ptr, 0)
#endif

#define MILI_UINT					(10000000)
#define LEFT_SHIFT_32				(32)
#define DELI_TEN					(10)
#ifdef WIN32
void win_gettimeofday(struct timeval *tp)
{
	uint64_t  intervals;
	FILETIME  ft;

	GetSystemTimeAsFileTime(&ft);

	intervals = ((uint64_t)ft.dwHighDateTime << LEFT_SHIFT_32) | ft.dwLowDateTime;
	intervals -= 116444736000000000;

	tp->tv_sec = (long)(intervals / MILI_UINT);
	tp->tv_usec = (long)((intervals % MILI_UINT) / DELI_TEN);
}
#endif

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}
#endif

CUdpSocket::CUdpSocket()
{
	m_packetNum = 0;
	m_localIp = NULL;
	m_localPort = 0;   
	m_listenFd = -1;
	m_bUsed = false;

	
#ifdef WIN32
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);  

	if (WSAStartup(sockVersion, &wsaData) != 0)  
	{
		END_THREAD();
		THREAD_EXIT;
	}
#endif

}

CUdpSocket::~CUdpSocket()
{
	m_packetNum = 0;
	m_localIp = NULL;
	m_localPort = 0;
	m_listenFd = -1;
#ifdef WIN32
	WSACleanup(); 
#endif
}

int CUdpSocket::Init(const char* localIp, unsigned int localPort)
{
	m_localIp = localIp;
	m_localPort = localPort;

	DEFINE_THREAD(thread1);
	BEGIN_THREAD(thread1, SendAgainEntry, this);
	SLEEP(1*1000);
	BEGIN_THREAD(thread1, RecvDataEntry, this);
	
	return 1;	
}

void THREAD_API CUdpSocket::RecvDataEntry(void* pParam)
{        
	CUdpSocket* pThis = static_cast<CUdpSocket*>(pParam);
	if(NULL != pThis)
		pThis->RecvData();
} 

void THREAD_API CUdpSocket::SendAgainEntry(void* pParam)
{
	CUdpSocket* pThis = static_cast<CUdpSocket*>(pParam);
	if(NULL != pThis)
		pThis->SendAgain();
}

void CUdpSocket::RecvData()
{
#ifndef WIN32
	pthread_detach(pthread_self());
#endif

	char *recvBuf = new char[MAX_BUF_LEN];
	if(recvBuf == NULL)
	{
		log_err(g_pLogHelper, "(US::RecvData) new buf failed(%s)", strerror(errno));
	
		END_THREAD();
		THREAD_EXIT;
	}

	this->m_listenFd = socket(AF_INET, SOCK_DGRAM, 0);		
	
#ifdef WIN32
	if (this->m_listenFd == SOCKET_ERROR || this->m_listenFd == INVALID_SOCKET)
#else
	if (this->m_listenFd == -1)
#endif
	{
		log_err(g_pLogHelper, "(US::RecvData) recv socket create failed(%s)\n", strerror(errno));
		delete [] recvBuf;
		recvBuf = NULL;

		END_THREAD();
		THREAD_EXIT;
	}

	int bufsize = MAX_RECV_BUF_SIZE;
	int ret1 = setsockopt(this->m_listenFd, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(bufsize));

	struct sockaddr_in localAdd;
	memset(&localAdd, 0, sizeof(localAdd));
	localAdd.sin_family 	 = AF_INET;
	if(this->m_localIp == NULL)
	{
		localAdd.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		localAdd.sin_addr.s_addr = inet_addr(this->m_localIp);
	}

	localAdd.sin_port = htons(this->m_localPort);

	int ret = bind(this->m_listenFd, (struct sockaddr*)&localAdd, sizeof(localAdd));
	if(ret == -1)
	{
		log_err(g_pLogHelper, "(US::RecvData) bind failed(%s)\n", strerror(errno));
		this->m_listenFd = -1;
		delete [] recvBuf;
		recvBuf = NULL;

		END_THREAD();
		THREAD_EXIT;
	}

	struct timeval tvStart, tvEnd;
	GETTIMEOFDAY(&tvStart);
	while (1)
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(this->m_listenFd, &fd);
		struct timeval timeOut;
		timeOut.tv_sec = 10;
		timeOut.tv_usec = 0;

		int sleret = select(this->m_listenFd + 1, &fd, NULL, NULL, &timeOut);
		
		if (sleret == 0)
		{
			continue;
		}
		else if (sleret < 0)
		{
			delete[] recvBuf;
			recvBuf = NULL;
			END_THREAD();
			THREAD_EXIT;
		}

		{
			struct sockaddr_in fromAdd;
			memset(&fromAdd, 0, sizeof(sockaddr_in));
			int fromLen = sizeof(fromAdd);

			memset(recvBuf, 0, MAX_BUF_LEN);
			int recvNum;
#ifdef WIN32
			recvNum = recvfrom(this->m_listenFd, recvBuf, MAX_BUF_LEN, 0, (struct sockaddr*)&fromAdd, &fromLen);
#else
			recvNum = recvfrom(this->m_listenFd, recvBuf, MAX_BUF_LEN, 0, (struct sockaddr*)&fromAdd, (socklen_t*)&fromLen);
#endif

			if (recvNum == -1)
			{
				continue;
			}
			else if (recvNum == 0)
			{
				continue;
			}

		
			GETTIMEOFDAY(&tvEnd);
			m_netRecv = SetNetNum(m_netRecv, recvNum);
			m_netRecvTemp += recvNum;
			uint64 per = (tvEnd.tv_sec - tvStart.tv_sec)*1000 + (tvEnd.tv_usec - tvStart.tv_usec)/1000;
			if (per >= 1000*60)
			{
				GETTIMEOFDAY(&tvStart);
				m_netRateRecv = m_netRecvTemp / 60;
				m_netRecvTemp = 0;
			}
	
			if (((T_PUUSEEHEAD)recvBuf)->uiUuseeFlag != 123456)
			{
				continue;
			}

			if (((T_PUUSEEHEAD)recvBuf)->Version > CURRENT_VERSION)
			{
				continue;
			}

			if (((T_PUUSEEHEAD)recvBuf)->PackType == '1')
			{
				this->m_testData.usRecvRspNum++;
				this->m_recvListLock.Lock();
				unsigned short usRecvNum = this->m_recvList.size();
				this->m_testData.usRecvListNum = usRecvNum;
				if (usRecvNum > MAX_RECV_LIST_COUNT)
				{
					this->m_recvListLock.UnLock();
					continue;
				}
				this->m_recvListLock.UnLock();

				unsigned int uiCrc = crc32buf((recvBuf + sizeof(T_UUSEEHEAD)), ((T_PUUSEEHEAD)recvBuf)->uiBufLen);
				if (uiCrc == ((T_PUUSEEHEAD)recvBuf)->uiSendBufCrc)
				{
					unsigned int acklen = sizeof(T_UUSEEHEAD);
					char *ackBuf = (char*)malloc(sizeof(T_UUSEEHEAD));
					memset(ackBuf, 0, acklen);
					((T_PUUSEEHEAD)recvBuf)->PackType = '2';
					((T_PUUSEEHEAD)recvBuf)->uiSendBufCrc = 0;
					memcpy(ackBuf, (T_PUUSEEHEAD)recvBuf, acklen);

					int sendNum = sendto(this->m_listenFd, ackBuf, acklen, 0, (struct sockaddr*)&fromAdd, fromLen);
					if (sendNum == -1)
					{
					}
					else if (sendNum > 0)
					{
						this->m_testData.usSendRspAckNum++;
					}

					T_SENDNODE tSendNode;
					tSendNode.sendBuf = new char[((T_PUUSEEHEAD)recvBuf)->uiBufLen];

					tSendNode.uiIp = fromAdd.sin_addr.s_addr;
					tSendNode.usPort = ntohs(fromAdd.sin_port);
					tSendNode.uiSendLen = ((T_PUUSEEHEAD)recvBuf)->uiBufLen;
					memcpy(tSendNode.sendBuf, (char*)((T_UUSEEHEAD*)recvBuf + 1), tSendNode.uiSendLen);

					this->m_recvListLock.Lock();
					this->m_recvList.push_back(tSendNode);
					this->m_recvListLock.UnLock();

					continue;
				}
				else
				{
				}
			}
			else if (((T_PUUSEEHEAD)recvBuf)->PackType == '2')
			{
				this->m_sendMapLock.Lock();
				this->m_testData.usRecvReqAckNum++;

				ITR_MAP_T_PSENDNODE iter = this->m_sendMap.find(((T_PUUSEEHEAD)recvBuf)->usPackNum);
				if (iter != this->m_sendMap.end())
				{
					(iter->second)->usFlag = ACK_FALG;

					struct timeval tempTime;
					uint64 lastTime = (iter->second)->uiLastSendTime;
					gettimeofday(&tempTime, NULL);
					unsigned int useTime = ((tempTime.tv_sec * 1000) + (tempTime.tv_usec / 1000)) - lastTime;
					if (useTime < this->m_testData.usFastTime)
					{
						this->m_testData.usFastTime = useTime;
					}
					if (useTime > this->m_testData.usSlowTime)
					{
						this->m_testData.usSlowTime = useTime;
					}
				}

				this->m_sendMapLock.UnLock();

			}
		}
	}
	delete []recvBuf;
	recvBuf = NULL;	
}

string CUdpSocket::SetNetNum(string netSize, uint64 recvNum)
{
	string strTemp = netSize;
	int len = strTemp.length();
	float retNum = 0;

	char buf[16];
	memset(buf, 0, 16);
	retNum = atof(strTemp.substr(0, len - 1).c_str());

	if (strTemp[len] == 'B')
	{
	}
	else if (strTemp[len] == 'K')
	{
		retNum += (retNum * 1024);
	}
	else if (strTemp[len] == 'M')
	{
		retNum += (retNum * 1024 * 1024);
	}
	else if (strTemp[len] == 'G')
	{
		retNum += (retNum * 1024 * 1024 * 1024);
	}
	else if (strTemp[len] == 'T')
	{
		retNum += (retNum * 1024 * 1024 * 1024 * 1024);
	}

	retNum += recvNum;
	
	memset(buf, 0, 16);
	if (strTemp[len] == 'B')
	{
		sprintf(buf, "%.1fB", retNum);
	}
	else if (strTemp[len] == 'K')
	{
		sprintf(buf, "%.1fK", retNum / 1024);
	}
	else if (strTemp[len] == 'M')
	{
		sprintf(buf, "%.1fM", (retNum / 1024) / 1024);
	}
	else if (strTemp[len] == 'G')
	{
		sprintf(buf, "%.1fG", ((retNum / 1024) / 1024) / 1024);
	}
	else if (strTemp[len] == 'T')
	{
		sprintf(buf, "%.1fT", (((retNum / 1024) / 1024) / 1024) / 1024);
	}

	return buf;
}
string CUdpSocket::GetStatus()
{
	string retData = "";
	char buf[1024] = {0};
	sprintf(buf, "(CUdpSocket::GetStatues) insertSendListNum:%d RetryTimes:%d ActualSendReqNum:%d RecvReqAckNum:%d RecvRspNum:%d SendRspAckNum:%d SendFaied:%d \nsendListTaskNum:%u FastTime:%d SlowTime:%d RecvListNum:%d", m_GetData.usInsertSendListNum, m_GetData.usRetryTimes, m_GetData.usActualSendReqNum, m_GetData.usRecvReqAckNum, m_GetData.usRecvRspNum, m_GetData.usSendRspAckNum, m_GetData.usSendFailed, m_GetData.usSendListCount, m_GetData.usFastTime, m_GetData.usSlowTime, m_GetData.usRecvListNum);
	retData = buf;
	return retData;
}
void CUdpSocket::OupPutTestData()
{
}

void CUdpSocket::SendAgain()

{
#ifndef WIN32
	pthread_detach(pthread_self());
#endif

	struct timeval  tmOutPutControlTime;
	gettimeofday(&tmOutPutControlTime, NULL);

	unsigned int sendControlNum = 0;
	time_t       tmLastTime = 0;

	struct timeval timeBefore,timeEnd, tvStart, tvEnd;
	GETTIMEOFDAY(&tvStart);
	while(1)
	{
		this->m_sendMapLock.Lock();
		if(this->m_sendList.empty())
		{
			this->m_sendMapLock.UnLock();
			SLEEP(5);
			continue;
		}

		this->m_sendMapLock.UnLock();

		SLEEP(10);

		gettimeofday(&timeBefore, NULL);
		this->m_sendMapLock.Lock();
		ITR_LIST_T_PSENDNODE iter;
		struct timeval tmNow, tmTemp;
		int tempNum = 0;
		for(iter = this->m_sendList.begin(); iter != this->m_sendList.end(); )
		{
			if(tempNum >= 150)
			{
				break;
			}

			if ((*iter)->uiRetryTimes >= SEND_TIMES || (*iter)->usFlag == ACK_FALG)
			{
				struct in_addr testIp;
				testIp.s_addr = (*iter)->uiIp;
				unsigned int uiTempPack = ((T_PUUSEEHEAD)((*iter)->sendBuf))->usPackNum;

				bool bFlag = false;	
				if((*iter)->usFlag == ACK_FALG)
				{			
					bFlag = true;
				}

				if(!bFlag)
				{
					if ((*iter)->uiRetryTimes >= SEND_TIMES)
					{
						this->m_testData.usSendFailed ++;
					}
				}
				
				ITR_MAP_T_PSENDNODE iter_map;
				iter_map = this->m_sendMap.find(uiTempPack);
				if(iter_map != this->m_sendMap.end())
				{
					if(iter_map->second != NULL)
					{
						iter_map->second = NULL;
					}

					this->m_sendMap.erase(iter_map);
				}
				else
				{
				}
				
				if((*iter)->sendBuf != NULL)
				{
					delete [](*iter)->sendBuf;
					(*iter)->sendBuf = NULL;
				}	
				if((*iter) != NULL)
				{
					delete (*iter);
					(*iter) == NULL;
				}
				iter = this->m_sendList.erase(iter);


				continue; 
			}

			gettimeofday(&tmNow, NULL);
			unsigned int intervalTime = ((tmNow.tv_sec) * 1000 + (tmNow.tv_usec) / 1000) - (*iter)->uiLastSendTime;
			if(((*iter)->uiRetryTimes == 0) || (intervalTime > 200))
			{	
				this->m_testData.usActualSendReqNum ++;
				this->m_testData.usSendListCount = this->m_sendMap.size();
				if((*iter)->uiRetryTimes == 0)
				{

				}
				else if((*iter)->uiRetryTimes > 0)
				{
					this->m_testData.usRetryTimes ++;
				}

				struct sockaddr_in serverAdd;
				memset(&serverAdd, 0, sizeof(serverAdd));
				serverAdd.sin_family = AF_INET;
				serverAdd.sin_port = (*iter)->usPort;
				serverAdd.sin_addr.s_addr = (*iter)->uiIp;

				if(this->m_listenFd == -1)
				{
					iter ++;
					continue;
				}

				int sedNum = sendto(this->m_listenFd, (*iter)->sendBuf, (*iter)->uiSendLen, 0, (struct sockaddr*)&serverAdd, sizeof(serverAdd));
				if(sedNum < 0)
				{
					iter ++;
					continue;
				}

				GETTIMEOFDAY(&tvEnd);
				m_netSend = SetNetNum(m_netSend, sedNum);
				m_netSendTemp += sedNum;
				uint64 per = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
				if (per >= 1000 * 60)
				{
					GETTIMEOFDAY(&tvStart);
					m_netRateSend = m_netSendTemp / 60;
					m_netSendTemp = 0;
				}
				
				gettimeofday(&tmTemp, NULL);
				(*iter)->uiLastSendTime = tmTemp.tv_sec * 1000 + tmTemp.tv_usec/1000;
				(*iter)->uiRetryTimes ++;

				this->m_sendList.push_back(*iter);		
				iter = this->m_sendList.erase(iter);

				tempNum ++;
				sendControlNum ++;
			}
			else
			{
				break;
			}
		}
		gettimeofday(&timeEnd, NULL);
		int useTime = (timeEnd.tv_sec - timeBefore.tv_sec)*1000 + (timeEnd.tv_usec - timeBefore.tv_usec)/1000;

		struct timeval outPutTempTime;
		gettimeofday(&outPutTempTime, NULL);
		bool bOutPutFlag = false;
		if(((outPutTempTime.tv_sec - tmOutPutControlTime.tv_sec)*1000 + (outPutTempTime.tv_usec - tmOutPutControlTime.tv_usec)/1000) >= 1000)
		{
			this->m_GetData = this->m_testData;

			this->m_testData.usInsertSendListNum = 0;
			this->m_testData.usRetryTimes = 0;
			this->m_testData.usActualSendReqNum = 0;
			this->m_testData.usRecvReqAckNum = 0; 
			this->m_testData.usRecvRspNum = 0; 
			this->m_testData.usSendRspAckNum = 0;
			this->m_testData.usSendFailed = 0; 
			this->m_testData.usSendListCount = 0;
			this->m_testData.usFastTime = 100;
			this->m_testData.usSlowTime = 0;

			gettimeofday(&tmOutPutControlTime, NULL);
			bOutPutFlag = true;
		}
		this->m_sendMapLock.UnLock();

	}
}


int CUdpSocket::Send(unsigned int ip, unsigned short port, const char* buf, unsigned int len)
{
	m_sendMapLock.Lock();

	T_UUSEEHEAD tUuseeHead;
	tUuseeHead.uiUuseeFlag    = 123456;
	tUuseeHead.Version        = '1';
	tUuseeHead.PackType       = '1';
	tUuseeHead.usPackNum      = m_packetNum;
	tUuseeHead.uiBufLen       = len;
	tUuseeHead.uiSendBufCrc   = crc32buf((char*)buf, len);

	T_PSENDNODE tpSendNode = new T_SENDNODE;
	tpSendNode->sendBuf       = new char[sizeof(T_UUSEEHEAD) + len];

	memcpy(tpSendNode->sendBuf, &tUuseeHead, sizeof(T_UUSEEHEAD));
	memcpy(tpSendNode->sendBuf + sizeof(T_UUSEEHEAD), buf, len);

	tpSendNode->usFlag 		= DEFAULT;
	tpSendNode->uiIp	 		= ip;
	tpSendNode->usPort 		= htons(port);
	tpSendNode->uiSendLen		= sizeof(T_UUSEEHEAD) + len;
	tpSendNode->uiRetryTimes  = 0;
	struct timeval timeTemp;
	gettimeofday(&timeTemp, NULL);
	tpSendNode->uiLastSendTime = timeTemp.tv_sec * 1000 + timeTemp.tv_usec / 1000;
	m_sendList.push_front(tpSendNode);
	ITR_MAP_T_PSENDNODE iter = m_sendMap.find(m_packetNum);
	if(iter != m_sendMap.end())
	{
	}
	m_sendMap[m_packetNum] = tpSendNode;

	m_testData.usInsertSendListNum ++;
	m_packetNum ++;
	m_sendMapLock.UnLock();

	return SEND_SUCCESS;
}

int CUdpSocket::Recv(unsigned int &ip, unsigned short &port, char* buf, unsigned int &len)
{
	int nRet = RECV_LIST_EMPTY;

	m_recvListLock.Lock();
	if(m_recvList.empty())
	{   
		m_recvListLock.UnLock();    
		return nRet;
	}   

	T_SENDNODE sendNode;
	sendNode = m_recvList.front();
	ip = sendNode.uiIp;
	port = sendNode.usPort;
	if(len < sendNode.uiSendLen)
	{   
		nRet =  RECV_BUF_NOT_ENOUGH;
	}   
	else
	{   
		memcpy(buf, sendNode.sendBuf, sendNode.uiSendLen);
		nRet = RECV_SUCCESS;
	}   

	len = sendNode.uiSendLen;

	if(sendNode.sendBuf != NULL)
	{   
		delete [] sendNode.sendBuf;
		sendNode.sendBuf = NULL;
	}   
	m_recvList.pop_front();
	m_recvListLock.UnLock();

	return nRet;
}

unsigned int CUdpSocket::GetSendListNum()
{
	m_sendMapLock.Lock();
	unsigned int mapRet = m_sendMap.size();
	m_sendMapLock.UnLock();
	return mapRet;
}

uint64 CUdpSocket::GetRecvRate()
{
	return m_netRateRecv;
}

string CUdpSocket::GetRecvSize()
{
	return m_netRecv;
}
uint64 CUdpSocket::GetSendRate()
{
	return m_netRateSend;
}
string CUdpSocket::GetSendSize()
{
	return m_netSend;
}