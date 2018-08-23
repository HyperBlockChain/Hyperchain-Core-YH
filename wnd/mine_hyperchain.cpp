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

#include "mine_hyperchain.h"
#include "ui_mine_hyperchain.h"

#include "blockinfo.h"
#include "HChainP2PManager/interface/QtInterface.h"
#include "util/commonutil.h"
#include "common.h"
#include "mainwindow.h"

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include "statusbar.h"


extern MainWindow* g_mainWindow();

mine_hyperchain::mine_hyperchain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mine_hyperchain)
{	
    ui->setupUi(this);

	_minehy_statusBar = new statusbar(this);
	ui->verticalLayout->addWidget(_minehy_statusBar);

	binfo = new blockinfo(this);
	ui->labelChainDataStatus->setblockinfo(binfo);

	ui->labelMyRegSended->SetMes("How many registration requests this node has sent");
	ui->labelRecvRegSended->SetMes("How many registration requests this node has received");

	ui->chainConectStatusWell->SetMes("How many Hyperchain nodes in strong connection state");
	ui->chainConectStatusOccasionallyNoResponse->SetMes("How many Hyperchain nodes in good connection state");
	ui->chainConectStatusOccasionallyResponse->SetMes("How many Hyperchain nodes in average connection state");
	ui->chainConectStatusNoResponse->SetMes("How many Hyperchain nodes in bad connection state");
	
	ui->labelBuddyChainCount->SetMes("How many Local Chains found in this round of consensus");
	ui->labelBuddyStatus->SetMes("Consensus Phase should be Local or Global");
	ui->labelCurrentBlockNumber->SetMes("The Hyperblock which now consensus is based");
	ui->labelNextBlockNumber->SetMes("The HyperBlock which now consensus will produce");


	ShowPng();
	li.clear();
	list_ChainData.clear();

	InitDate();
	InitBuddyTime();

	connect(ui->labelChainDataStatus, &chain_data_status::sigShowNodeDlgInfo, this, &mine_hyperchain::onSigShowNodeDlgInfo);
	connect(binfo, &blockinfo::sigShowBrowserInfo, this, &mine_hyperchain::onSigShowBrowserInfo);

	connect(ui->labelMyRegSended, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->labelRecvRegSended, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);

	connect(ui->chainConectStatusWell, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->chainConectStatusOccasionallyNoResponse, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->chainConectStatusOccasionallyResponse, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->chainConectStatusNoResponse, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->labelBuddyChainCount, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->labelBuddyStatus, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->labelCurrentBlockNumber, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);
	connect(ui->labelNextBlockNumber, &statulabel::sigSendStatusMes, this, &mine_hyperchain::onSigShowStatusMes);

}

mine_hyperchain::~mine_hyperchain()
{
    delete ui;
}


void mine_hyperchain::InitDate()
{
	m_gbuddychainnum = 0;
	m_betternum = 0;
	m_normalnum = 0;
	m_badnum = 0;
	m_downnum = 0;
	m_sendpoenum = 0;
	m_recvepoenum = 0;
	m_buddystatus = 0;
	m_currentblocknumber = 0;
	m_nextblocknumber = m_currentblocknumber + 1;

	QString qsloading = tr("Synchronize");
	ui->labelCurrentBlockNumber->setText(qsloading);
	ui->labelNextBlockNumber->setText(qsloading);
	ui->labelBuddyChainCount->setText(qsloading);
	ui->labelBuddyStatus->setText(qsloading);

}

void mine_hyperchain::retranslateUi()
{
    ui->retranslateUi(this);
}

void mine_hyperchain::onSigShowNodeDlgInfo(QPoint gpos, uint64 blocknum)
{
	static uint64 sblocknum;

	bool bChanged = false;
	if (sblocknum != blocknum)
	{
		bChanged = true;
		sblocknum = blocknum;
	}

	if (bChanged)
	{
	
		binfo->refreshNodeDlgInfo(blocknum);
	}

	if (binfo->isHidden())
	{
		binfo->setGeometry(QRect(gpos.x(), gpos.y(), 224, 250));
		binfo->show(true);
		binfo->setFocus();
	}
}

void mine_hyperchain::onSigShowBrowserInfo(string blocknum)
{
	emit SIG_BrowserInfo(blocknum);
}


void mine_hyperchain::clearDate_Buddy()
{
	StopBuddyTime();	
	Update_GlobleBuddyChainNum(0);
	Update_NodeStatus(IDLE);
	ui->myPartenerFrame->clearNodes();
}


void mine_hyperchain::Update_HyperBlock(string hash, time_t time, uint64 blocknumber)
{	
	if (!hash.empty() && time > 0)
	{
		g_mainWindow()->updateEvidenceByHash(hash, time, blocknumber);
	}	

	if (m_currentblocknumber != blocknumber)
	{
		m_currentblocknumber = blocknumber;
		m_nextblocknumber = m_currentblocknumber + 1;
		ui->labelCurrentBlockNumber->setText(QString("%1").arg(m_currentblocknumber));
		ui->labelNextBlockNumber->setText(QString("%1").arg(m_nextblocknumber));			
		ui->labelChainDataStatus->setBlockNum(m_currentblocknumber, m_currentblocknumber);
		
		list_ChainData.push_back(m_currentblocknumber);
		ui->labelChainDataStatus->setBlocksNum(list_ChainData);
		ui->labelChainDataStatus->update();
	}
}



bool mine_hyperchain::GetHBlockDlgInfo(uint64 blocknum, THBLOCKDLGINFO & hinfo)
{
	bool bret = false;
	ostringstream_t oss;
	string_t tsNum;
	uint64 num = 0;

	oss << blocknum;
	tsNum = oss.str();

	json::value  root = QueryByWeb(blocknum);
	num = root[tsNum].size();
	if (num > 0)
	{
		bret = true;

		hinfo.iBlockNo = blocknum;
		hinfo.iCreatTime = root[tsNum][0][_XPLATSTR("ctime")].as_number().to_uint64();
		hinfo.iLocalBlockNum = num - 1;
		hinfo.strHHash = t2s(root[tsNum][0][_XPLATSTR("hhash")].serialize());
	}

	return bret;
}

void mine_hyperchain::Update_NodeStatus(uint16 status)
{
	if (m_buddystatus != status)
	{
		m_buddystatus = status;

		switch (m_buddystatus)
		{
		case IDLE:
			ui->labelBuddyStatus->setText("IDLE");
			break;

		case LOCAL_BUDDY:
			ui->labelBuddyStatus->setText("LOCAL_BUDDY");
			break;

		case GLOBAL_BUDDY:
			ui->labelBuddyStatus->setText("GLOBAL_BUDDY");
			break;

		default:
			ui->labelBuddyStatus->setText("IDLE");
			break;
		}
		 
	}

}

void mine_hyperchain::Update_BuddyStartTime(time_t stime, uint64 blocknumber)
{		
	StartBuddyTime();
}

void mine_hyperchain::Update_BuddyStop()
{
	clearDate_Buddy();
}

void mine_hyperchain::Update_GlobleBuddyChainNum(uint16 number)
{
	if (m_gbuddychainnum != number)
	{
		m_gbuddychainnum = number;
		ui->labelBuddyChainCount->setText(tr("%1 rds").arg(m_gbuddychainnum));
	}

}

void mine_hyperchain::Update_LocalBuddyChainInfo(LIST_T_LOCALCONSENSUS chaininfo)
{	
	VEC_T_NODEINFO vecnode;
	for (auto item : chaininfo)
	{
		TNODEINFO info;
		info.strNodeIp = to_string(item.tPeer.tPeerAddrOut.uiIP) + ":" + to_string(item.tPeer.tPeerAddrOut.uiPort);
		info.uiNodeState = CONFIRMED;
		vecnode.push_back(info);

	}
	if (!vecnode.empty())
	{
		ui->myPartenerFrame->setNodeInfo(vecnode);		
		ui->myPartenerFrame->showNodes();		
	}
}

void mine_hyperchain::Update_ConnectNodeUpdate(uint32 betternum, uint32 normalnum, uint32 badnum, uint32 downnum)
{
	if (m_betternum != betternum)
	{
		m_betternum = betternum;
		ui->chainConectStatusWell->setText(QString("%1").arg(m_betternum));
	}

	if (m_normalnum != normalnum)
	{
		m_normalnum = normalnum;
		ui->chainConectStatusOccasionallyNoResponse->setText(QString("%1").arg(m_normalnum));
	}

	if (m_badnum != badnum)
	{
		m_badnum = badnum;
		ui->chainConectStatusOccasionallyResponse->setText(QString("%1").arg(m_badnum));
	}

	if (m_downnum != downnum)
	{
		m_downnum = downnum;
		ui->chainConectStatusNoResponse->setText(QString("%1").arg(m_downnum));
	}

}

void mine_hyperchain::Update_SendPoeNum(uint32 number)
{
	if (m_sendpoenum != number)
	{
		m_sendpoenum = number;
		ui->labelMyRegSended->setText(QString("%1").arg(m_sendpoenum));
	}

}

void mine_hyperchain::Update_ReceivePoeNum(uint32 number)
{
	if (m_recvepoenum != number)
	{
		m_recvepoenum = number;
		ui->labelRecvRegSended->setText(QString("%1").arg(m_recvepoenum));
	}
}

void mine_hyperchain::InitBuddyTime()
{
	m_timer = new QTimer;
	m_timerecord = new QTime(0, 0, 0);		

	ui->lcdBuddyTime->display(m_timerecord->toString("hh:mm:ss"));
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
}

void mine_hyperchain::StartBuddyTime()
{
	m_bstart = true;
	m_timer->start(1000);

}

void mine_hyperchain::StopBuddyTime()
{
	m_bstart = false;
	m_timer->stop();    
	m_timerecord->setHMS(0, 0, 0); 
	ui->lcdBuddyTime->display(m_timerecord->toString("hh:mm:ss")); 

}

void mine_hyperchain::updateTime()
{
	*m_timerecord = m_timerecord->addSecs(1);
	ui->lcdBuddyTime->display(m_timerecord->toString("hh:mm:ss"));
}

void mine_hyperchain::ShowPng()
{
	QPixmap pix;
	QString strChainStatusPath = ChainStatusPath();
	QString strChainRunStatusPath = ChainRunStatusPath();
	QString strNodeConnectStatusPath = NodeConnectStatusPath();
	QString strPoePath = PoePath();

	
	pix.load(strChainStatusPath);
	ui->png_ChainStatus->setPixmap(pix);

	pix.load(strChainRunStatusPath);
	ui->png_ChainRunStatus->setPixmap(pix);

	pix.load(strNodeConnectStatusPath);
	ui->png_Nodeconnectstatus->setPixmap(pix);

	pix.load(strPoePath);
	ui->png_Poe->setPixmap(pix);

}

void mine_hyperchain::hideblockinfo()
{
	if (binfo && !binfo->isHidden())
	{
		binfo->hide();
	}
}


void mine_hyperchain::Update_HyperBlockNumFromLocal(list<uint64> HyperBlockNum)
{
	for (list<uint64>::iterator it = HyperBlockNum.begin(); it != HyperBlockNum.end(); it++)
	{	
		list_ChainData.push_back(*it);
	}

	if (HyperBlockNum.size() > 0)
	{
		ui->labelChainDataStatus->setBlocksNum(list_ChainData);
		ui->labelChainDataStatus->update();
	}
}


void mine_hyperchain::onSigShowStatusMes(QString msg)
{
	_minehy_statusBar->OutPutString(msg);
}

void mine_hyperchain::mousePressEvent(QMouseEvent *event)
{
	if (!binfo->isHidden())
	{
		binfo->hide();
	}
}


void mine_hyperchain::Update_StatusMes(string msg)
{
	_minehy_statusBar->OutPutString(QString::fromLocal8Bit(msg.c_str()), "color: rgb(131, 186, 38)");
}