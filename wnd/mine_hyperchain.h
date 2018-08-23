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

#ifndef MINE_HYPERCHAIN_H
#define MINE_HYPERCHAIN_H

#include "HChainP2PManager/headers/inter_public.h"
#include "HChainP2PManager/headers/commonstruct.h"
#include <QWidget>
#include <QSharedPointer>
#include <QTimer>
#include <QTime>

namespace Ui {
class mine_hyperchain;
}

class blockinfo;
class statusbar;

class mine_hyperchain : public QWidget
{
    Q_OBJECT

public:
    explicit mine_hyperchain(QWidget *parent = 0);
    ~mine_hyperchain();

	
public:
    void retranslateUi();
	void hideblockinfo();

private:
	void InitDate();	
	void InitBuddyTime();
	void StartBuddyTime();
	void StopBuddyTime();
	void ShowPng();
	void clearDate_Buddy();
	bool GetHBlockDlgInfo(uint64 blocknum, THBLOCKDLGINFO & hinfo);
	string w2s(const wstring& ws);

signals:
	void SIG_BrowserInfo(string blocknum);

public slots:
	void onSigShowBrowserInfo(string blocknum);
	void onSigShowNodeDlgInfo(QPoint gpos, uint64 blocknum);	
	void onSigShowStatusMes(QString msg);
	void updateTime();

	void Update_BuddyStartTime(time_t stime, uint64 blocknumber);
	void Update_LocalBuddyChainInfo(LIST_T_LOCALCONSENSUS chaininfo);
	void Update_HyperBlock(string hash, time_t time, uint64 blocknumber);
	void Update_NodeStatus(uint16 status);
	void Update_GlobleBuddyChainNum(uint16 number);
	void Update_ConnectNodeUpdate(uint32 betternum, uint32 normalnum, uint32 badnum, uint32 downnum);
	void Update_SendPoeNum(uint32 number);
	void Update_ReceivePoeNum(uint32 number);	
	void Update_BuddyStop();
	void Update_HyperBlockNumFromLocal(list<uint64> HyperBlockNum);
	void Update_StatusMes(string msg);
protected:
	void mousePressEvent(QMouseEvent *event);

private:
    Ui::mine_hyperchain *ui;

	statusbar*			_minehy_statusBar = nullptr;

	blockinfo *binfo = nullptr;
	QTimer hideTimer_;

	uint16	m_gbuddychainnum;
	uint32	m_betternum;
	uint32	m_normalnum;
	uint32	m_badnum;
	uint32	m_downnum;
	uint32	m_sendpoenum;
	uint32	m_recvepoenum;
	uint16	m_buddystatus;
	uint64	m_currentblocknumber;
	uint64	m_nextblocknumber;

	QTimer  *m_timer;      
	QTime	*m_timerecord;  
	bool	m_bstart;  

	QList<QSharedPointer<THBLOCKDLGINFO> > li;
	list<uint64> list_ChainData;
};

#endif // MINE_HYPERCHAIN_H
