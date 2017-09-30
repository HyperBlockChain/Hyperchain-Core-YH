﻿/*Copyright 2017 hyperchain.net  (Hyper Block Chain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT.
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

#include "mine_hyperchain.h"
#include "ui_mine_hyperchain.h"

#include "block_info_dlg.h"
#include "p2p/interface/QtInterface.h"
#include "util/commonutil.h"
#include "mainwindow.h"

#include <QDebug>

extern MainWindow* g_mainWindow();

mine_hyperchain::mine_hyperchain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mine_hyperchain)
{
    ui->setupUi(this);

    hideCheckBtn();
    ui->frame_13->hide();

    nid_ = new block_info_dlg(this);

    connect(ui->labelChainDataStatus, &chain_data_status::sigShowNodeInfo, this, &mine_hyperchain::onSigShowNodeInfo);
    connect(ui->labelChainDataStatus, &chain_data_status::sigHideNodeInfo, this, &mine_hyperchain::onSigHideNodeInfo);
    connect(&hideTimer_, &QTimer::timeout, this, &mine_hyperchain::onHideNodeInfoDlgTimer);

    onUpdateTimer();
}

mine_hyperchain::~mine_hyperchain()
{
    delete ui;
}

void mine_hyperchain::retranslateUi()
{
    ui->retranslateUi(this);

    if(nid_){
        nid_->setLanguage(g_mainWindow()->getLanguage());
    }
}

void mine_hyperchain::onSigShowNodeInfo(QPoint gpos, QSharedPointer<TBLOCKINFO> pNodeInfo)
{
    static QSharedPointer<TBLOCKINFO> sP;

    bool bChanged = false;
	if (sP.data() == nullptr || sP->iBlockNo != pNodeInfo->iBlockNo){
        bChanged = true;
        sP = pNodeInfo;
    }

    if(bChanged){
        nid_->refreshNodeInfo(pNodeInfo);
    }else{
        ;
    }

    if(!isNidShow_){
        isNidShow_ = true;

        nid_->setGeometry(QRect(gpos.x(),gpos.y(), 200, 300));
        nid_->show(true);

        nid_->setFocus();
    }
}

void mine_hyperchain::onSigHideNodeInfo()
{
    if(isNidShow_){
        hideTimer_.start(100);
    }
}

void mine_hyperchain::onHideNodeInfoDlgTimer()
{
    if(!nid_->hasFcous()){
        isNidShow_ = false;
        nid_->show(false);

        hideTimer_.stop();
    }
}

void mine_hyperchain::onUpdateTimer()
{
	uint64 curFirstBlockNum = GetLatestHyperBlockNo();
	uint64 lastBlockNum = GetLatestHyperBlockNo();

    ui->labelChainDataStatus->setBlockNum(curFirstBlockNum, lastBlockNum);

	VEC_T_BLOCKINFO vec = GetBlockInfoByIndex(0, lastBlockNum);
    QList<QSharedPointer<TBLOCKINFO> > li;

    if(vec.empty()){
        ui->labelChainDataStatus->setBlockNum(2, 3);

        QSharedPointer<TBLOCKINFO> info = QSharedPointer<TBLOCKINFO>(new TBLOCKINFO);
		info->iBlockNo = 0;
		info->tPoeRecordInfo.tRegisTime = 1497694933456;
        li.push_back(info);

        QSharedPointer<TBLOCKINFO> info1 = QSharedPointer<TBLOCKINFO>(new TBLOCKINFO);
		info1->iBlockNo = 1;
		info1->tPoeRecordInfo.tRegisTime = 1497694943456;
        li.push_back(info1);

        QSharedPointer<TBLOCKINFO> info2 = QSharedPointer<TBLOCKINFO>(new TBLOCKINFO);
		info2->iBlockNo = 2;

		info2->tPoeRecordInfo.cFileName = QStringLiteral("沈加成.txt").toStdString();
		info2->tPoeRecordInfo.cCustomInfo = "asdfasd";
		info2->tPoeRecordInfo.cRightOwner = "sjc";
		info2->tPoeRecordInfo.cFileHash = "12345678123456781234567812345678012345678123456781234567812345678";
		info2->tPoeRecordInfo.iFileState = 4;
		info2->tPoeRecordInfo.tRegisTime = 1497694953456;
		info2->tPoeRecordInfo.iFileSize = 1048579;

        li.push_back(info2);

        QSharedPointer<TBLOCKINFO> info3 = QSharedPointer<TBLOCKINFO>(new TBLOCKINFO);
		info3->iBlockNo = 3;
        li.push_back(info3);

    }else{
        for(auto item : vec){
            QSharedPointer<TBLOCKINFO> info = QSharedPointer<TBLOCKINFO>(new TBLOCKINFO);
			info->iBlockNo = item->iBlockNo;
			info->tPoeRecordInfo = item->tPoeRecordInfo;

            delete item;

            li.push_back(info);
        }
    }

    ui->labelChainDataStatus->setBlocks(li);
    ui->labelChainDataStatus->update();

    
	int64 baseBlockNum = GetLatestHyperBlockNo();
    ui->labelBaseNodeNum->setText(QString("%1").arg(baseBlockNum));

    ui->labelAlternativeBlockNum->setText(tr("Alternative BN %1").arg(baseBlockNum + 1));

   
	uint16 allChainNum = GetElaspedTimeOfCurrentConsensus();
    ui->labelPartnerCount->setText(tr("All partner chain %1 rds").arg(allChainNum));

   
	uint16 confirmed = GetConfirmedChainNum();
    ui->labelConfirmed->setText(tr("Confirmed %1").arg(confirmed));

   
	int64 consensusTime = GetElaspedTimeOfCurrentConsensus();
    ui->labelConsensusTime->setText(tr("ConsensusTime %1").arg(secsToHourMinSecs(consensusTime)));

  
	VEC_T_NODEINFO vec1 = GetMyLocalChain();
    if(vec1.empty()){
        ui->myPartenerFrame->setNodeInfo(1);
    }else{
        ui->myPartenerFrame->setNodeInfo(vec1);
    }
    ui->myPartenerFrame->showNodes();
    
	VEC_T_NODEINFO vec2 = GetOtherLocalChain(2);
    if(vec2.empty()){
        ui->otherPartnerFrame0->setNodeInfo(2);
    }else{
        ui->otherPartnerFrame0->setNodeInfo(vec2);
    }
    ui->otherPartnerFrame0->showNodes();
   
	VEC_T_NODEINFO v = GetMyLocalChain();
    ui->labelMyPartnerFlag->setText(QString("%1").arg(1));
    ui->labelMyPartnerCount->setText(QString("%1").arg(v.size()));

	v = GetOtherLocalChain(2);
    ui->labelOtherPartnerFlag1->setText(QString("%1").arg(2));
    ui->labelOtherPartnerCount1->setText(QString("%1").arg(v.size()));

	
  
	uint32 responseWell = GetStrongNodeNum();
    ui->chainConectStatusWell->setText(QString("%1").arg(responseWell));

	uint32 normalNode = GetAverageNodeNum();
    ui->chainConectStatusOccasionallyNoResponse->setText(QString("%1").arg(normalNode));

	uint32 badNodeNum = GetWeakNodeNum();
    ui->chainConectStatusOccasionallyResponse->setText(QString("%1").arg(badNodeNum));

	uint32 downNode = GetOfflineNodeNum();
    ui->chainConectStatusNoResponse->setText(QString("%1").arg(downNode));

  
    uint32 reqSend = GetSendRegisReqNum(SEND);                  //已发送
    uint32 reqConfirming = GetSendRegisReqNum(CONFIRMING);      //待确认
    uint32 reqConfirmed =  GetSendRegisReqNum(CONFIRMED);       //已确认
    ui->labelMyRegSended->setText(QString("%1").arg(reqSend));
    ui->labelMyRegWaitforConfirm->setText(QString("%1").arg(reqConfirming));
    ui->labelMyRegConfirmed->setText(QString("%1").arg(reqConfirmed));


    if(0 == reqConfirming){
        g_mainWindow()->updateEvidence();
    }

    uint32 repRecved = GetRecvRegisRegNum(RECV);                //已收到
    uint32 repConfirming = GetRecvRegisRegNum(CONFIRMING);      //待确认
    uint32 repConfirmed = GetRecvRegisRegNum(CONFIRMED);        //已确认
    ui->labelRecvRegSended->setText(QString("%1").arg(repRecved));
    ui->labelRecvRegWaitforConfirm->setText(QString("%1").arg(repConfirming));
    ui->labelRecvRegConfirmed->setText(QString("%1").arg(repConfirmed));

    g_mainWindow()->updateFinish();
}

void mine_hyperchain::hideCheckBtn()
{
    ui->btnViewMyRegSend->hide();
    ui->btnViewMyRegConfirmed->hide();
    ui->btnViewMyRegWaitConfirm->hide();
    ui->btnViewMyRegRefused->hide();

    ui->btnViewRecvRegSend->hide();
    ui->btnViewRecvRegConfirmed->hide();
    ui->btnViewRecvRegWaitforConfirm->hide();
    ui->btnViewRecvRegRefused->hide();
}
