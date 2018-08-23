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
#include "../headers/commonstruct.h"
#include "../headers/UUFile.h"
#include "../debug/Log.h"
#include <QDir>

#ifdef WIN32
#include <winsock2.h>
#endif
T_CONFFILE	g_confFile;
UUFile			m_uufile;

CCommonStruct::CCommonStruct()
{
}

CCommonStruct::~CCommonStruct()
{
}

#ifdef WIN32
void CCommonStruct::win_gettimeofday(struct timeval *tp)
{
	uint64_t  intervals;
	FILETIME  ft;

	GetSystemTimeAsFileTime(&ft);

	intervals = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	intervals -= 116444736000000000;

	tp->tv_sec = (long)(intervals / 10000000);
	tp->tv_usec = (long)((intervals % 10000000) / 10);
}
#endif

void CCommonStruct::gettimeofday_update(struct timeval *ptr)
{
#ifdef WIN32
	win_gettimeofday(ptr);
#else
	gettimeofday(ptr, 0);
#endif
}
int CCommonStruct::CompareHash(const T_SHA256& arhashFirst, const T_SHA256& arhashSecond)
{
	const unsigned char* pFirst = NULL;
	const unsigned char* pSecond = NULL;
	for(int i=0; i<DEF_SHA256_LEN; i++)
	{
		pFirst = arhashFirst.pID+i;
		pSecond = arhashSecond.pID+i;
		if(*pFirst>*pSecond)
			return 1;
		if(*pFirst<*pSecond)
			return -1;
	}
	return 0;
}

void CCommonStruct::Hash256ToStr(char* getStr, T_PSHA256 phash)
{
	char ucBuf[10] = {0};

	unsigned int uiNum = 0;
	for(uiNum; uiNum < 32; uiNum ++)
	{
		//sprintf(ucBuf, "%x", phash->pID[uiNum]&0x0ff);
		memset(ucBuf, 0, 10);
		sprintf(ucBuf, "%x", phash->pID[uiNum]);
		strcat(getStr, ucBuf);	
	}
	int i = 0;
}

void CCommonStruct::StrToHash256(unsigned char *des, char* start)
{
	char ucBuf[512] = { 0 };

	char *pStr = start;
	unsigned char *pDes = des;
	unsigned int uiNum = 0;
	for (uiNum; uiNum < 32; uiNum++)
	{
		//sprintf(ucBuf, "%x", phash->pID[uiNum]&0x0ff);
		sscanf(pStr, "%x", pDes);
		pStr++;
		pDes++;
		//strcat(phash->pID, ucBuf);
	}
}

void CCommonStruct::Hash512ToStr(char* getStr, T_PSHA512 phash)
{
	//char ucBuf[10] = { 0 };

	//unsigned int uiNum = 0;
	//for (uiNum; uiNum < 128; uiNum++)
	//{
	//	//sprintf(ucBuf, "%x", phash->pID[uiNum]&0x0ff);
	//	memset(ucBuf, 0, 10);
	//	sprintf(ucBuf, "%x", phash->pID[uiNum]);
	//	strcat(getStr, ucBuf);
	//}

	////int iLen = strlen(getStr);


	memcpy(getStr, phash->pID, DEF_SHA512_LEN);
}
void CCommonStruct::hash512tostring(char* out, const char* szHash)
{
	/*
		if (strlen(szHash) != DEF_SHA512_LEN * 2)
			return;
		int len = DEF_SHA512_LEN * 2;
		char str[DEF_SHA512_LEN * 2];
		memset(str, 0, len);
		memcpy(str, szHash, len);
		for (int i = 0; i < len; i += 2) {
			//小写转大写
			if (str[i] >= 'a' && str[i] <= 'f') str[i] = str[i] & ~0x20;
			if (str[i + 1] >= 'a' && str[i] <= 'f') str[i + 1] = str[i + 1] & ~0x20;
			//处理第前4位
			if (str[i] >= 'A' && str[i] <= 'F')
				out[i / 2] = (str[i] - 'A' + 10) << 4;
			else
				out[i / 2] = (str[i] & ~0x30) << 4;
			//处理后4位, 并组合起来
			if (str[i + 1] >= 'A' && str[i + 1] <= 'F')
				out[i / 2] |= (str[i + 1] - 'A' + 10);
			else
				out[i / 2] |= (str[i + 1] & ~0x30);
		}

	*/
}

void CCommonStruct::StrToHash512(unsigned char *out, char* szHash)
{
	
	memcpy(out, szHash, DEF_SHA512_LEN);

	//int len = DEF_SHA512_LEN * 2;
	//char str[DEF_SHA512_LEN * 2];
	//memset(str, 0, len);
	//memcpy(str, szHash, len);
	//for (int i = 0; i < len; i += 2) {
	//	//小写转大写
	//	if (str[i] >= 'a' && str[i] <= 'f') str[i] = str[i] & ~0x20;
	//	if (str[i + 1] >= 'a' && str[i] <= 'f') str[i + 1] = str[i + 1] & ~0x20;
	//	//处理第前4位
	//	if (str[i] >= 'A' && str[i] <= 'F')
	//		out[i / 2] = (str[i] - 'A' + 10) << 4;
	//	else
	//		out[i / 2] = (str[i] & ~0x30) << 4;
	//	//处理后4位, 并组合起来
	//	if (str[i + 1] >= 'A' && str[i + 1] <= 'F')
	//		out[i / 2] |= (str[i + 1] - 'A' + 10);
	//	else
	//		out[i / 2] |= (str[i + 1] & ~0x30);
	//}

}
T_SHA256 DistanceHash(const T_SHA256& arLeft, const T_SHA256& arRight)
{
	T_SHA256 guidDistance;
	T_SHA256 left			= arLeft;
	T_SHA256 right		= arRight;
	if(arLeft<arRight)
	{
		left = arRight;
		right =  arLeft;
	}

	for(int i=0; i<DEF_SHA256_LEN; i++)
	{
		guidDistance.pID[i] = left.pID[i] - right.pID[i];
	}

	return guidDistance;
}

void CCommonStruct::ReplaceAll(string& str,const string& old_value,const string& new_value)
{
     if (old_value.empty())
           return;

     int Pos = 0;
     while ((Pos = str.find(old_value, Pos)) != string::npos) {
	     str.erase(Pos, old_value.size());
     str.insert(Pos, new_value);
	 Pos += new_value.size();
	}
}

void CCommonStruct::ReparePath(string& astrPath)
{
#ifdef WIN32
     ReplaceAll(astrPath, "/", "\\");
#else
     ReplaceAll(astrPath, "\\", "/");
#endif
}
string CCommonStruct::GetLocalIp()
{
	char ipTempBuf[64] = { 0 };
	m_uufile.getlocalip(ipTempBuf);
	return ipTempBuf;
}
bool CCommonStruct::ReadConfig()
{
	string localPath = m_uufile.GetAppPath();
	string confPath = localPath +  "hychain_conf.xml";
	string peerListPath = localPath + "peerlist.xml";

	m_uufile.ReparePath(confPath);
	m_uufile.ReparePath(peerListPath);
	  
	//read conf file
	string strXmlFile = m_uufile.LoadFile(confPath).c_str();
	if (strXmlFile.empty())
	{
		log_err(g_pLogHelper, "load config file failed!!! %d", IERROR);
		return false;
	}

	string strTemp = m_uufile.GetItemData(strXmlFile, "savenodenum");
	g_confFile.uiSaveNodeNum = atoi(strTemp.c_str());

	//////////////////////////////////////////////
	//2018-07-03
	//g_confFile.uiSaveNodeNum += 2;
	//////////////////////////////////////////////

	strTemp = m_uufile.GetItemData(strXmlFile, "logfile");
	if (0 == strTemp.compare(""))
	{
		QString path = QDir::tempPath();
		g_confFile.strLogDir = path.toStdString();
		g_confFile.strLogDir += "hyperchainlog\\";
	}
	else
	{
		g_confFile.strLogDir = strTemp;
	}

	QString strLogPath = QString::fromStdString(g_confFile.strLogDir);
	QDir kDir;
	if (!kDir.exists(strLogPath)) {
		kDir.mkpath(strLogPath); 
	}

	
	strTemp = m_uufile.GetItemData(strXmlFile, "localip").c_str();
	if (0 == strTemp.compare(""))
	{
		char ipTempBuf[64] = { 0 };
		m_uufile.getlocalip(ipTempBuf);
		g_confFile.uiLocalIP = inet_addr(ipTempBuf);

	}
	else
	{
		g_confFile.uiLocalIP = inet_addr(strTemp.c_str());
	}
	
	strTemp = m_uufile.GetItemData(strXmlFile, "localport").c_str();
	if (0 == strTemp.compare(""))
	{
		g_confFile.uiLocalPort = LISTEN_PORT;
	}
	else
	{
		g_confFile.uiLocalPort = atoi(strTemp.c_str());
	}
	
	strTemp = m_uufile.GetItemData(strXmlFile, "localnodename").c_str();
	if (0 == strTemp.compare(""))
	{
		g_confFile.strLocalNodeName = "node";
		char ipTempBuf[64] = { 0 };
		m_uufile.getlocalip(ipTempBuf);
		g_confFile.strLocalNodeName += ipTempBuf;
	}
	else
	{
		g_confFile.strLocalNodeName = strTemp;
	}
	

	//read peerlist file
	strXmlFile = m_uufile.LoadFile(peerListPath).c_str();
	if (strXmlFile.empty())
	{
		log_err(g_pLogHelper, "load peerlist file failed!!! %d", IERROR);
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//2018-07-03
	//T_PPEERCONF pPeerConfADD1 = new T_PEERCONF;
	//pPeerConfADD1->tPeerAddr.uiIP = inet_addr("124.16.139.178");
	//pPeerConfADD1->tPeerAddr.uiPort = 8115;
	//pPeerConfADD1->tPeerAddrOut.uiIP = inet_addr("124.16.139.178");
	//pPeerConfADD1->tPeerAddrOut.uiPort = 8115;
	//pPeerConfADD1->uiPeerState = 1;
	//strncpy(pPeerConfADD1->strName, "124.16.139.178", MAX_NODE_NAME_LEN);
	//g_confFile.vecPeerConf.push_back(pPeerConfADD1);

	//T_PPEERCONF pPeerConfADD2 = new T_PEERCONF;
	//pPeerConfADD2->tPeerAddr.uiIP = inet_addr("124.16.139.173");
	//pPeerConfADD2->tPeerAddr.uiPort = 8115;
	//pPeerConfADD2->tPeerAddrOut.uiIP = inet_addr("124.16.139.173");
	//pPeerConfADD2->tPeerAddrOut.uiPort = 8115;
	//pPeerConfADD2->uiPeerState = 1;
	//strncpy(pPeerConfADD2->strName, "124.16.139.173", MAX_NODE_NAME_LEN);
	//g_confFile.vecPeerConf.push_back(pPeerConfADD2);
	/////////////////////////////////////////////////////////////////////////////////////


	vector<string> vec_str = m_uufile.ExtractStringList(strXmlFile, "nodeinfo");
	vector<string>::iterator itr = vec_str.begin();
	for (; itr != vec_str.end(); itr++)
	{
		T_PPEERCONF pPeerConf = new T_PEERCONF;
		
		pPeerConf->tPeerAddr.uiIP = inet_addr(m_uufile.GetItemData((*itr), "serverip").c_str());
		pPeerConf->tPeerAddr.uiPort = atoi((m_uufile.GetItemData((*itr), "serverport")).c_str());
		pPeerConf->tPeerAddrOut.uiIP = inet_addr(m_uufile.GetItemData((*itr), "outserverip").c_str());
		pPeerConf->tPeerAddrOut.uiPort = atoi((m_uufile.GetItemData((*itr), "outserverport")).c_str());
		pPeerConf->uiPeerState = atoi(m_uufile.GetItemData((*itr), "nodestate").c_str());
		strncpy(pPeerConf->strName, m_uufile.GetItemData((*itr), "nodename").c_str(), MAX_NODE_NAME_LEN);

		g_confFile.vecPeerConf.push_back(pPeerConf);
	}

	return true;
}
char* CCommonStruct::Time2String(time_t time1)
{
	static char szTime[1024]="";
	memset(szTime, 0, 1024);
	struct tm tm1;
#ifdef WIN32
	localtime_s(&tm1, &time1);
#else
	localtime_r(&time1, &tm1 );
#endif
	sprintf( szTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
		tm1.tm_year+1900, tm1.tm_mon+1, tm1.tm_mday,
		tm1.tm_hour, tm1.tm_min,tm1.tm_sec);
	return szTime;
}

string CCommonStruct::HashToString(const T_SHA256& arGuid)
{
	char buf[100];
     sprintf( buf, 
		 //"{%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X}",
	 "{%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x}",
		 arGuid.pID[0],  arGuid.pID[1],  arGuid.pID[2],  arGuid.pID[3], 
			arGuid.pID[4],  arGuid.pID[5],  arGuid.pID[6],  arGuid.pID[7], 
			arGuid.pID[8],  arGuid.pID[9],  arGuid.pID[10], arGuid.pID[11], 
			arGuid.pID[12], arGuid.pID[13], arGuid.pID[14], arGuid.pID[15],
			arGuid.pID[16], arGuid.pID[17], arGuid.pID[18], arGuid.pID[19], 
			arGuid.pID[20], arGuid.pID[21], arGuid.pID[22], arGuid.pID[23], 
			arGuid.pID[24], arGuid.pID[25], arGuid.pID[26], arGuid.pID[27], 
			arGuid.pID[28], arGuid.pID[29], arGuid.pID[30], arGuid.pID[31]);
     return string(buf);
}

T_SHA256 CCommonStruct::StringToHash(const char *str)
{
//	int t1,t2,t3,t4;
	T_SHA256 guid;
	sscanf(str,
	     // "{%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X}",
		 "{%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x}",
			 &guid.pID[0],  &guid.pID[1],  &guid.pID[2],  &guid.pID[3], 
	         &guid.pID[4],  &guid.pID[5],  &guid.pID[6],  &guid.pID[7], 
	         &guid.pID[8],  &guid.pID[9],  &guid.pID[10],  &guid.pID[11], 
	         &guid.pID[12],  &guid.pID[13],  &guid.pID[14],  &guid.pID[15],  
			 &guid.pID[16],  &guid.pID[17],  &guid.pID[18],  &guid.pID[19], 
			 &guid.pID[20],  &guid.pID[21],  &guid.pID[22],  &guid.pID[23], 
			 &guid.pID[24],  &guid.pID[25],  &guid.pID[26],  &guid.pID[27], 
			 &guid.pID[28],  &guid.pID[29],  &guid.pID[30],  &guid.pID[31]);

	return guid;
}

