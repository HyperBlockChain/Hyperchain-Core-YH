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

#ifndef __COMMON_STRUCT_H__
#define __COMMON_STRUCT_H__


#include "includeComm.h"
#include "gen_int.h"
#include "inter_public.h"
#include "shastruct.h"

using namespace std;

#define DEF_ONE_DAY     (60 * 60 * 24)
#define MAX_IP_LEN		(32)
#define MAX_RECV_BUF_LEN (1024)
#define MAX_SEND_BUF_LEN (1024)
#define MAX_FILE_NAME_LEN (1024)
#define MAX_NODE_NAME_LEN (64)
#define MAX_CUSTOM_INFO_LEN (512)
#define MAX_SCRIPT_LEN (1024*2)
#define MAX_AUTH_LEN (64)
#define MAX_QUEED_LEN (32)
#define LISTEN_PORT (8115)
#define MAX_VER_LEN		(32)

#pragma pack(1)

enum _ep2pprotocoltypestate
{
	DEFAULT_STATE = 0,
	SEND_ON_CHAIN_RSP,
	RECV_ON_CHAIN_RSP,
	SEND_CONFIRM,
	RECV_CONFIRM,
	SEND_CONFIRM_RSP,
	RECV_CONFIRM_RSP
};

enum _erecvpagestate
{
	DEFAULT_RECV_STATE = 0,
	RECV_RSP,
	RECV_REQ
};
enum _eerrorno
{
	DEFAULT_ERROR_NO = 0,
	ERROR_NOT_NEWEST,
	ERROR_EXIST
};

typedef struct _tpeeraddress
{
	uint32	uiIP;
	uint32	uiPort;

	_tpeeraddress()
	{
	}
	_tpeeraddress(uint32 ip, uint32 port);

	_tpeeraddress& operator = (const _tpeeraddress& arRes);
	bool operator == (const _tpeeraddress& arRes) const;


	void SetPeerAddress(uint32 IP, uint32 Port);
	void SetIP(uint32 IP);
	void SetPort(uint32	Port);
	uint32 GetIP()const;
	uint32 GetPort()const;

	string GetIPString()const;
	
}T_PEERADDRESS, *T_PPEERADDRESS;


typedef struct _tblockbaseinfo
{
	uint64 uiID;				
	T_SHA256 tPreHash;			
	uint64 uiTime;				
	int8  strScript[MAX_SCRIPT_LEN];
	int8  strAuth[MAX_AUTH_LEN];
	T_SHA256 tHashSelf;

	_tblockbaseinfo()
	{
		uiID = 0;
		uiTime = 0;
		memset(strScript, 0, MAX_SCRIPT_LEN);
		memset(strAuth, 0, MAX_AUTH_LEN);
		memset(tPreHash.pID, 0, DEF_SHA256_LEN);
		memset(tHashSelf.pID, 0, DEF_SHA256_LEN);
	}

	_tblockbaseinfo(uint64 ID, uint64 Time, int8 *Auth, int8 *Script, T_SHA256 HashSelf, T_SHA256 PreHash);
	_tblockbaseinfo(uint64 ID, uint64 Time, int8 *Auth, int8 *Script, T_SHA256 HashSelf);
	_tblockbaseinfo(T_SHA256 PreHash, uint64 ID, uint64 Time, int8 *Auth, int8 *Script);

	_tblockbaseinfo& operator = (const _tblockbaseinfo& arRes);
	void SetBlockBaseInfo(uint64 ID, uint64 Time, int8 *Auth, int8 *Script, T_SHA256 HashSelf, T_SHA256 PreHash);
	void SetBlockBaseInfo(uint64 ID, uint64 Time, int8 *Auth, int8 *Script, T_SHA256 HashSelf);
	void SetBlockBaseInfo(uint64 ID, uint64 Time, int8 *Auth, int8 *Script);

	void SetID(uint64 ID);
	void SetTime(uint64 Time);
	void SetAuth(const char *Auth);
	void SetScript(const char *Script);
	void SetPreHash(T_SHA256 PreHash);
	void SetHashSelf(T_SHA256 HashSelf);

	uint64 GetID()const;
	uint64 GetTime()const;
	int8 * GetAuth();
	int8 * GetScript();
	T_SHA256 GetPreHash()const;
	T_SHA256& GetPreHash();
	T_SHA256 GetHashSelf()const;
	T_SHA256& GetHashSelf();


}T_BLOCKBASEINFO, *T_PBLOCKBASEINFO;

typedef struct _tblockbaseinfonew
{
	uint64 uiID;
	T_SHA256 tPreHash;
	uint64 uiTime;
	int8 *strScript;
	uint64 scriptLength;
	int8  strAuth[MAX_AUTH_LEN];
	T_SHA256 tHashSelf;

	_tblockbaseinfonew()
	{
		uiID = 0;
		uiTime = 0;
		strScript = NULL;
		scriptLength = 0;
		memset(strAuth, 0, MAX_AUTH_LEN);
		memset(tPreHash.pID, 0, DEF_SHA256_LEN);
		memset(tHashSelf.pID, 0, DEF_SHA256_LEN);
	}

	_tblockbaseinfonew(uint64 ID,T_SHA256 preHash,uint64 t,int8* script,
		uint64 scLen,int8 auth[],T_SHA256 hashSelf);

	~_tblockbaseinfonew();

	_tblockbaseinfonew& operator = (const _tblockbaseinfonew& arRes);

	void Set(uint64 ID, T_SHA256 preHash, uint64 t, int8* script,
		uint64 scLen, int8 auth[], T_SHA256 hashSelf);

	void SetScript(int8*script, uint64 scLen);

	void SetAuth(int8 auth[]);

	uint64 GetScriptLength()const;

	T_SHA256 GetPreviousHash()const;

	T_SHA256 GetSelfHash()const;

	uint64 GetID()const;


}T_BLOCKBASEINFONEW, *T_PBLOCKBASEINFONEW;

typedef struct _fileinfo
{
	uint64 uiFileSize;
	char fileName[MAX_FILE_NAME_LEN];
	char customInfo[MAX_CUSTOM_INFO_LEN];
	char tFileOwner[MAX_CUSTOM_INFO_LEN];
	T_SHA512 tFileHash;
	uint64 uiFileCreateTime;

	_fileinfo()
	{
		uiFileSize = 0;
		uiFileCreateTime = 0;
		memset(fileName, 0, MAX_FILE_NAME_LEN);
		memset(customInfo, 0, MAX_CUSTOM_INFO_LEN);
		memset(tFileHash.pID, 0, DEF_SHA512_LEN);
		memset(tFileOwner, 0, MAX_CUSTOM_INFO_LEN);
	}

	_fileinfo& operator = (const _fileinfo& arRes);
	void SetFileInfo(uint64 FileSize, const char *chFileName, const char *chCustomInfo, const char *chFileOwner, T_SHA512 FileHash, uint64 FileCreateTime);
	void SetFileInfo(uint64 FileSize, const char *chFileName, const char *chFileOwner, uint64 FileCreateTime);
	void SetFileInfo(uint64 FileSize, const char *chFileName, const char *chCustomInfo, const char *chFileOwner);
	void SetFileSize(uint64 FileSize);
	void SetFileName(const char *chFileName);
	void SetCustomInfo(const char *chCustomInfo);
	void SetFileOwner(const char *chFileOwner);
	void SetFileHash(T_SHA512 FileHash);
	void SetFileCreateTime(uint64 FileCreateTime);

	uint64	GetFileSize()const;
	char * GetFileName();
	char * GetCustomInfo();
	char * GetFileOwner();
	T_SHA512 GetFileHash()const;
	T_SHA512& GetFileHash();
	uint64 GetFileCreateTime()const;

}T_FILEINFO, *T_PFILEINFO;

typedef struct _tprivateblock
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHHash;

	T_FILEINFO tPayLoad;

	_tprivateblock()
	{
		memset(tHHash.pID, 0, DEF_SHA256_LEN);
	}
	_tprivateblock(_tblockbaseinfo tBBI, T_SHA256 tHH, T_FILEINFO tPL);
	_tprivateblock& operator = (const _tprivateblock& arRes);
	void SetPrivateBlock(_tblockbaseinfo tBBI, T_SHA256 tHH, T_FILEINFO tPL);
	void SetBlockBaseInfo(_tblockbaseinfo tBBI);
	void SetHHash(T_SHA256 tHH);
	void SetPayLoad(T_FILEINFO tPL);

	_tblockbaseinfo GetBlockBaseInfo()const;
	_tblockbaseinfo& GetBlockBaseInfo();
	T_SHA256 GetHHash()const;
	T_FILEINFO& GetPayLoad();


}T_PRIVATEBLOCK, *T_PPRIVATEBLOCK;

typedef struct _tlocalblock
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHHash;
	uint64  uiAtChainNum;

	T_PRIVATEBLOCK tPayLoad;

	_tlocalblock()
	{
		memset(tHHash.pID, 0, DEF_SHA256_LEN);
		uiAtChainNum = 1;
	}

	_tlocalblock(_tblockbaseinfo tBBI, T_SHA256 tHH, T_PRIVATEBLOCK tPL);
	_tlocalblock(_tblockbaseinfo tBBI, T_SHA256 tHH, uint64 AtChainNu, T_PRIVATEBLOCK tPL);
	_tlocalblock(_tblockbaseinfo tBBI, T_SHA256 tHH, uint64 AtChainNu);
	_tlocalblock& operator = (const _tlocalblock& arRes);
	void SetLocalBlock(_tblockbaseinfo tBBI, T_SHA256 tHH, uint64 AtChainNu, T_PRIVATEBLOCK tPL);
	void SetBlockBaseInfo(_tblockbaseinfo tBBI);
	void SetHHash(T_SHA256 tHH);
	void SetAtChainNum(uint64 Num);
	void SetPayLoad(T_PRIVATEBLOCK tPL);
	
	_tblockbaseinfo GetBlockBaseInfo()const;
	_tblockbaseinfo& GetBlockBaseInfo();
	T_SHA256 GetHHash()const;
	uint64 GetAtChainNum()const;
	T_PRIVATEBLOCK GetPayLoad()const;
	T_PRIVATEBLOCK& GetPayLoad();


}T_LOCALBLOCK, *T_PLOCALBLOCK;

typedef struct _tlocalblocknew
{
	_tblockbaseinfonew   tBlockBaseInfo;
	T_SHA256 tHHash;
	uint64  uiAtChainNum;
	int8 *userDefine;
	uint64 userDefineLength;

	_tlocalblocknew()
	{
		userDefine = NULL;
		userDefineLength = 0;
		memset(tHHash.pID, 0, DEF_SHA256_LEN);
		uiAtChainNum = 1;

		SetUserDefineDefault();
	}

	_tlocalblocknew(const _tblockbaseinfonew&bbi, T_SHA256 HHash, uint64 atChainNum, int8*ud, uint64 udLen);

	~_tlocalblocknew();

	void Set(const _tblockbaseinfonew&bbi, T_SHA256 HHash, uint64 atChainNum, int8*ud, uint64 udLen);

	void SetUserDefineDefault();

	_tlocalblocknew& operator = (const _tlocalblocknew& arRes);

	T_SHA256 GetParentHash()const;

	uint64 WhichChainAt()const;

	void SetUserDefine(int8*ud,uint64 udLen);

	const _tblockbaseinfonew& GetBaseInfo()const;


}T_LOCALBLOCKNEW, *T_PLOCALBLOCKNEW;

typedef list<T_LOCALBLOCK> LIST_T_LOCALBLOCK;
typedef LIST_T_LOCALBLOCK::iterator ITR_LIST_T_LOCALBLOCK;
typedef list<T_LOCALBLOCKNEW> LIST_T_LOCALBLOCKNEW;
typedef LIST_T_LOCALBLOCKNEW::iterator ITR_LIST_T_LOCALBLOCKNEW;

typedef struct _thyperblock
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHashAll;
	list<LIST_T_LOCALBLOCK> listPayLoad;
	int8 strVersion[MAX_VER_LEN];

	_thyperblock()
	{   
		memset(tHashAll.pID, 0, DEF_SHA256_LEN);
		strcpy(strVersion, "Ver0.7.1");
	}

	_thyperblock& operator = (const _thyperblock& arRes);
	void SetHyperBlock(_tblockbaseinfo tBBI, T_SHA256 tHA, list<LIST_T_LOCALBLOCK> LPayLoad);
	void SetBlockBaseInfo(_tblockbaseinfo tBBI);
	void SetHashAll(T_SHA256 tHA);
	void SetlistPayLoad(list<LIST_T_LOCALBLOCK> LPayLoad);

	_tblockbaseinfo GetBlockBaseInfo()const;
	_tblockbaseinfo& GetBlockBaseInfo();

	T_SHA256 GetHashAll();
	list<LIST_T_LOCALBLOCK> &GetlistPayLoad();
	void PushBack(LIST_T_LOCALBLOCK lb);

	int8* GetVersionString();
	
}T_HYPERBLOCK, *T_PHYPERBLOCK;



typedef struct _thyperblocknew
{
	_tblockbaseinfonew   tBlockBaseInfo;
	T_SHA256 tHashAll;
	list<LIST_T_LOCALBLOCKNEW> listPayLoad;
	T_SHA256 merkleRoot;
	std::vector<T_SHA256> leaves;

	_thyperblocknew()
	{
		memset(tHashAll.pID, 0, DEF_SHA256_LEN);
	}

	_thyperblocknew(const _thyperblock&block);

	_thyperblocknew& operator = (const _thyperblocknew& arRes);

	std::vector<T_SHA256> GetLeavesHash()const;	

	T_SHA256 GetMerkleRoot()const;

	T_SHA256 GetHashAll()const;

	const _tblockbaseinfonew& GetBaseInfo()const;

	void Clear();

private:

	void GetRoot();

}T_HYPERBLOCKNEW, *T_PHYPERBLOCKNEW;

typedef struct _thyperblocksend
{
	_tblockbaseinfo   tBlockBaseInfo;
	T_SHA256 tHashAll;

	_thyperblocksend()
	{
		memset(tHashAll.pID, 0, DEF_SHA256_LEN);
	}

	_thyperblocksend& operator = (const _thyperblocksend& arRes);
	void SetHyperBlockSend(_tblockbaseinfo tBBI, T_SHA256 tHA);
	void SetBlockBaseInfo(_tblockbaseinfo tBBI);
	void SetHashAll(T_SHA256 tHA);


	_tblockbaseinfo GetBlockBaseInfo()const;
	T_SHA256 GetHashAll()const;

}T_HYPERBLOCKSEND, *T_PHYPERBLOCKSEND;
typedef struct _tchainStateinfo 
{
	uint64 uiBlockNum;	

	_tchainStateinfo& operator = (const _tchainStateinfo& arRes);
	void SetBlockNum(uint64 BlockNum);
	uint64 GetBlockNum()const;
	
}T_CHAINSTATEINFO, *T_PCHAINSTATEINFO;


typedef struct _tpeerinfo
{
	T_PEERADDRESS tPeerInfoByMyself;
	T_PEERADDRESS tPeerInfoByOther;
	uint16 uiState;
	uint16 uiNatTraversalState;
	uint64 uiTime;
	int8 strName[MAX_NODE_NAME_LEN];
	uint16 uiNodeState;

	_tpeerinfo()
	{
		tPeerInfoByMyself.uiIP = 0;
		tPeerInfoByMyself.uiPort = LISTEN_PORT;
		tPeerInfoByOther.uiIP = 0;
		tPeerInfoByOther.uiPort = LISTEN_PORT;
		uiState = 0;
		uiNatTraversalState = 0;
		uiTime = 0;
		uiNodeState = DEFAULT_REGISREQ_STATE;
		memset(strName, 0, MAX_NODE_NAME_LEN);
	}
	
	_tpeerinfo& operator = (const _tpeerinfo& arRes);
	void SetPeerinfo(T_PEERADDRESS PeerInfoByMyself, T_PEERADDRESS PeerInfoByOther, uint16 State, uint16 NatTraversalState, uint64 Time, uint16 NodeState, int8 *Name);
	void SetPeerInfoByMyself(T_PEERADDRESS PeerInfoByMyself);
	void SetPeerInfoByOther(T_PEERADDRESS PeerInfoByOther);
	void SetState(uint16 State);
	void SetNatTraversalState(uint16 NatTraversalState);
	void SetTime(uint64 Time);
	void SetNodeState(uint16 NodeState);
	void SetName(int8 *Name);
	
	T_PEERADDRESS GetPeerInfoByMyself()const;
	T_PEERADDRESS GetPeerInfoByOther()const;
	uint16 GetState()const;
	uint16 GetNatTraversalState()const;
	uint64 GetTime()const;
	uint16 GetNodeState()const;
	int8* GetName();

}T_PEERINFO, *T_PPEERINFO;




typedef struct _tblockstateaddr
{
	T_PEERADDRESS tPeerAddr;
	T_PEERADDRESS tPeerAddrOut;

	_tblockstateaddr(){};
	_tblockstateaddr(T_PEERADDRESS PeerAddr, T_PEERADDRESS PeerAddrOut);
	_tblockstateaddr& operator = (const _tblockstateaddr& arRes);
	void SetBlockStateAddr(T_PEERADDRESS PeerAddr, T_PEERADDRESS PeerAddrOut);
	void SetPeerAddr(T_PEERADDRESS PeerAddr);
	void SetPeerAddrOut(T_PEERADDRESS PeerAddrOut);

	T_PEERADDRESS GetPeerAddr()const;
	T_PEERADDRESS GetPeerAddrOut()const;
	
}T_BLOCKSTATEADDR, *T_PBLOCKSTATEADDR;

typedef struct _tlocalconsensus
{
	T_BLOCKSTATEADDR tPeer;
	T_LOCALBLOCK  tLocalBlock;
	char strID[MAX_QUEED_LEN];
	uint64 uiRetryTime;
	char strFileHash[DEF_SHA512_LEN+1];

	_tlocalconsensus()
	{
		memset(strID, 0, MAX_QUEED_LEN);
		memset(strFileHash, 0, DEF_SHA512_LEN + 1);
		uiRetryTime = 0;
	}


	_tlocalconsensus(T_BLOCKSTATEADDR Peer, T_LOCALBLOCK  LocalBlock, const char * ID, uint64 RetryTime, const char *FileHash);
	_tlocalconsensus(T_BLOCKSTATEADDR Peer, T_LOCALBLOCK  LocalBlock, const char * ID, uint64 RetryTime);
	_tlocalconsensus& operator = (const _tlocalconsensus& arRes);
	void SetLoaclConsensus(T_BLOCKSTATEADDR Peer, T_LOCALBLOCK  LocalBlock, const char * ID, uint64 RetryTime, const char *FileHash);
	void SetLoaclConsensus(T_BLOCKSTATEADDR Peer, T_LOCALBLOCK  LocalBlock, const char * ID, uint64 RetryTime);
	void SetLoaclConsensus(T_BLOCKSTATEADDR Peer, T_LOCALBLOCK  LocalBlock);
	void SetPeer(T_BLOCKSTATEADDR  Peer);
	void SetLocalBlock(T_LOCALBLOCK  LocalBlock);
	void SetID(char * ID);
	void SetRetryTime(uint64 RetryTime);
	void SetFileHash(char *FileHash);

	T_BLOCKSTATEADDR GetPeer()const;
	T_LOCALBLOCK GetLocalBlock()const;
	T_LOCALBLOCK& GetLocalBlock();
	char * GetID();
	uint64 GetRetryTime()const;
	char * GetFileHash();


}T_LOCALCONSENSUS, *T_PLOCALCONSENSUS;

typedef struct _tglobalconsenus
{
	T_BLOCKSTATEADDR tPeer;
	T_LOCALBLOCK  tLocalBlock;
	uint64 uiAtChainNum;

	T_BLOCKSTATEADDR GetPeer()const;
	uint64 GetChainNo()const;

	T_LOCALBLOCK GetLocalBlock()const;

	void SetGlobalconsenus(T_BLOCKSTATEADDR Peer, T_LOCALBLOCK LocalBlock, uint64 AtChainNum);

	void SetPeer(const T_BLOCKSTATEADDR&addr);
	void SetLocalBlock(const T_LOCALBLOCK&block);
	void SetChainNo(uint64 no);

}T_GLOBALCONSENSUS, *T_PGLOBALCONSENSUS;

typedef struct _tbuddyinfo
{
	uint8 tType;
	
	uint32 bufLen;
	char *recvBuf;
	T_PEERADDRESS tPeerAddrOut;

	uint8 GetType()const;
	uint8 GetBufferLength()const;
	const char* GetBuffer()const;
	T_PEERADDRESS GetRequestAddress()const;

	void Set(uint8 t, uint32 bufferLen, char*receiveBuf, T_PEERADDRESS peerAddrOut);

}T_BUDDYINFO, *T_PBUDDYINFO;

typedef list<T_LOCALCONSENSUS> LIST_T_LOCALCONSENSUS;
typedef LIST_T_LOCALCONSENSUS::iterator ITR_LIST_T_LOCALCONSENSUS;

typedef list<T_PLOCALCONSENSUS> LIST_T_PLOCALCONSENSUS;
typedef LIST_T_PLOCALCONSENSUS::iterator ITR_LIST_T_PLOCALCONSENSUS;

typedef struct _tbuddyinfostate
{
	//T_SHA256 tBuddyHash;
	int8 strBuddyHash[DEF_STR_HASH256_LEN];
	uint8 uibuddyState;		//四次握手的状态
	T_PEERADDRESS tPeerAddrOut;
	LIST_T_LOCALCONSENSUS localList;
	_tbuddyinfostate()
	{
		memset(strBuddyHash, 0, DEF_STR_HASH256_LEN);
		uibuddyState = DEFAULT_STATE;
	}

	uint8 GetBuddyState()const;

	LIST_T_LOCALCONSENSUS GetList()const;

	T_PEERADDRESS GetPeerAddrOut()const;

	T_SHA256 GetBuddyHash()const;


	void Set(int8 buddyHash[],uint8 uibuddyState,T_PEERADDRESS addr);

	void LocalListPushBack(T_LOCALCONSENSUS  localBlockInfo);
	void LocalListClear();
	void LocalListSort();
	LIST_T_LOCALCONSENSUS& GetLocalConsensus();

	const int8 *GetBuddyHashEx()const;
	void SetPeerAddrOut(T_PEERADDRESS PeerAddrOut);
	void SetBuddyState(uint8 BuddyState);
	void SetBuddyHash(int8 * BuddyHash);
	void SetBuddyHashInit(int Num);

}T_BUDDYINFOSTATE, *T_PBUDDYINFOSTATE;

typedef struct _tsearchinfo
{    
	uint64 uiHyperID;
	uint64 uiTime;

	_tsearchinfo(){}
	_tsearchinfo(uint64 hyperID, uint64 t);
	void Set(uint64 hyperID, uint64 t);

	uint64 GetHyperID()const;

	uint64 GetCreateTime()const;

}T_SEARCHINFO, *T_PSEARCHINFO;

typedef list<LIST_T_LOCALCONSENSUS> LIST_LIST_GLOBALBUDDYINFO;
typedef LIST_LIST_GLOBALBUDDYINFO::iterator ITR_LIST_LIST_GLOBALBUDDYINFO;

typedef list<T_BUDDYINFO> LIST_T_BUDDYINFO;
typedef LIST_T_BUDDYINFO::iterator ITR_LIST_T_BUDDYINFO;

typedef list<T_PBUDDYINFOSTATE> LIST_T_PBUDDYINFOSTATE;
typedef LIST_T_PBUDDYINFOSTATE::iterator ITR_LIST_T_PBUDDYINFOSTATE;

typedef list<T_BUDDYINFOSTATE> LIST_T_BUDDYINFOSTATE;
typedef LIST_T_BUDDYINFOSTATE::iterator ITR_LIST_T_BUDDYINFOSTATE;

typedef map<string,T_SEARCHINFO> MAP_T_SEARCHONCHAIN;
typedef MAP_T_SEARCHONCHAIN::iterator ITR_MAP_T_SEARCHONCHAIN;

#pragma pack()

typedef list<T_PPEERINFO> LIST_T_PPEERINFO;
typedef LIST_T_PPEERINFO::iterator ITR_LIST_T_PPEERINFO;

typedef list<T_HYPERBLOCK> LIST_T_HYPERBLOCK;
typedef LIST_T_HYPERBLOCK::iterator ITR_LIST_T_HYPERBLOCK;
typedef list<T_HYPERBLOCKNEW> LIST_T_HYPERBLOCKNEW;
typedef LIST_T_HYPERBLOCKNEW::iterator ITR_LIST_T_HYPERBLOCKNEW;

typedef list<T_PBLOCKSTATEADDR> LIST_T_PBLOCKSTATEADDR;
typedef LIST_T_PBLOCKSTATEADDR::iterator ITR_LIST_T_PBLOCKSTATEADDR;

typedef map<uint64, LIST_T_PBLOCKSTATEADDR> MAP_BLOCK_STATE;
typedef MAP_BLOCK_STATE::iterator ITR_MAP_BLOCK_STATE;


typedef struct _tpeerconf
{
	T_PEERADDRESS tPeerAddr;
	T_PEERADDRESS tPeerAddrOut;
	uint16 uiPeerState;
	int8 strName[MAX_NODE_NAME_LEN];

	T_PEERADDRESS GetIntranetAddress()const;
	T_PEERADDRESS GetInternetAddress()const;

	uint16 GetPeerState()const;

	int8* GetNodeName()const;

}T_PEERCONF, *T_PPEERCONF;

typedef std::vector<T_PPEERCONF>	VEC_T_PPEERCONF;
typedef VEC_T_PPEERCONF::iterator   ITR_VEC_T_PPEERCONF;

typedef struct _tconffile
{
	uint16			uiSaveNodeNum;
	uint32			uiLocalIP;
	uint32			uiLocalPort;
	string          strLocalNodeName;
	string			strLogDir;
	VEC_T_PPEERCONF vecPeerConf;

	uint16 GetSaveNodeNum()const;

	uint32 GetLocalIP()const;

	uint32 GetLocalPort()const;

	string GetLocalNodeName()const;

	string GetLogDir()const;

	
}T_CONFFILE, *T_PCONFFILE;

class CCommonStruct
{
private:
	
	CCommonStruct();
	virtual ~CCommonStruct();

public:
	void static gettimeofday_update(struct timeval *ptr);
	static int CompareHash(const T_SHA256& arhashLocal, const T_SHA256& arhashGlobal);
	static void Hash256ToStr(char* getStr, T_PSHA256 phash);
	static void StrToHash256(unsigned char *des, char* getStr);

	static void Hash512ToStr(char* getStr, T_PSHA512 phash);
	static void hash512tostring(char* out, const char* szHash);
	static void StrToHash512(unsigned char *des, char* getStr);

	static T_SHA256 DistanceHash(const T_SHA256& arLeft, const T_SHA256& arRight);
	static string HashToString(const T_SHA256& arGuid);
	static T_SHA256 StringToHash(const char *str);
	static void ReplaceAll(string& str,const string& old_value,const string& new_value);
	static void ReparePath(string& astrPath);

	static bool ReadConfig();
	static string GetLocalIp();
	static char* Time2String(time_t time1);	

private:
#ifdef WIN32
	void static win_gettimeofday(struct timeval *tp);
#endif	
};

extern T_CONFFILE	g_confFile;

#endif 
