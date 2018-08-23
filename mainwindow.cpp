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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui_tab_bar_wnd.h"
#include "ui_attestation_reg.h"

#include "wnd/mine_hyperchain.h"
#include "wnd/attestation_reg.h"
#include "wnd/attestation_record.h"
#include "wnd/attestation_history.h"
#include "wnd/record.h"


#include "wnd/browser.h"
#include "wnd/settings.h"
#include "wnd/token.h"
#include "wnd/history.h"
#include "wnd/cabout.h"

#include "wnd/dev_mode.h"
#include "wnd/reg_verification_frameless.h"
#include "wnd/about_dlg.h"

#include "util/commonutil.h"
#include "db/dbmgr.h"
#include "db/RestApi.h"
#include "HChainP2PManager/interface/QtNotify.h"

#include <QResizeEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QMenu>
#include <QSystemTrayIcon>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QMetaType>

static MainWindow* g_w = nullptr;
MainWindow* g_mainWindow(){
    return g_w;
}

static std::string make_db_path(){
    QString doc = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    doc += QString("/hyperchain/hp");

    QDir d;
    d.mkpath(doc);

    return doc.toStdString();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    std::string dbpath = make_db_path();
    dbpath += "/hyperchain.db";
    DBmgr::instance()->open(dbpath.c_str());

    g_w = this;

    createActions();
    createTrayIcon();

    _tabBar = new tab_bar_wnd(this);
    ui->verticalLayout->addWidget(_tabBar);

    _hyperChainWnd			= new mine_hyperchain(this);
    _attestationRegWnd      = new attestation_reg(this);
	_recordWnd				 = new record(this);
	_historyWnd				= new history(this);
	_devModeWnd				= new dev_mode(this);

	_settingsWnd			= new settings(this);
	_browserWnd			    = new browser(this);
	_tokenWnd				= new token(this);    
	_notify					= new QtNotify();
	_AboutWnd				= new Cabout(this);


    ui->verticalLayout->addWidget(_hyperChainWnd);
	ui->verticalLayout->addWidget(_browserWnd);
    ui->verticalLayout->addWidget(_attestationRegWnd);
	ui->verticalLayout->addWidget(_recordWnd);
	ui->verticalLayout->addWidget(_historyWnd);
	ui->verticalLayout->addWidget(_settingsWnd);
    ui->verticalLayout->addWidget(_devModeWnd);
	ui->verticalLayout->addWidget(_tokenWnd);
	ui->verticalLayout->addWidget(_AboutWnd);

		
    connectCtrlSignal();

	qRegisterMetaType<uint16>("uint16");
	qRegisterMetaType<uint32>("uint32");
	qRegisterMetaType<uint64>("uint64");
	qRegisterMetaType<time_t>("time_t");
	qRegisterMetaType<string>("string");
	qRegisterMetaType<LIST_T_LOCALCONSENSUS>("LIST_T_LOCALCONSENSUS");
	qRegisterMetaType<T_PEERCONF>("T_PEERCONF");
	qRegisterMetaType<TEVIDENCEINFO>("TEVIDENCEINFO");
	qRegisterMetaType<list<uint64>>("list<uint64>");
	
	

	connect(_notify, &QtNotify::SIG_BuddyStartTime,	_hyperChainWnd, &mine_hyperchain::Update_BuddyStartTime);
	connect(_notify, &QtNotify::SIG_LocalBuddyChainInfo,_hyperChainWnd, &mine_hyperchain::Update_LocalBuddyChainInfo);
	connect(_notify, &QtNotify::SIG_HyperBlock, _hyperChainWnd, &mine_hyperchain::Update_HyperBlock);
	connect(_notify, &QtNotify::SIG_NodeStatus, _hyperChainWnd, &mine_hyperchain::Update_NodeStatus);
	connect(_notify, &QtNotify::SIG_GlobleBuddyChainNum, _hyperChainWnd, &mine_hyperchain::Update_GlobleBuddyChainNum);
	connect(_notify, &QtNotify::SIG_ConnectNodeUpdate, _hyperChainWnd, &mine_hyperchain::Update_ConnectNodeUpdate);
	connect(_notify, &QtNotify::SIG_SendPoeNum, _hyperChainWnd, &mine_hyperchain::Update_SendPoeNum);
	connect(_notify, &QtNotify::SIG_ReceivePoeNum, _hyperChainWnd, &mine_hyperchain::Update_ReceivePoeNum);
	connect(_notify, &QtNotify::SIG_BuddyStop, _hyperChainWnd, &mine_hyperchain::Update_BuddyStop);
	connect(_notify, &QtNotify::SIG_HyperBlockNumFromLocal, _hyperChainWnd, &mine_hyperchain::Update_HyperBlockNumFromLocal);
	connect(_notify, &QtNotify::SIG_StatusMes, _hyperChainWnd, &mine_hyperchain::Update_StatusMes);

	connect(_notify, &QtNotify::SIG_NodeInfo, _settingsWnd, &settings::Update_NodeInfo);
	connect(_notify, &QtNotify::SIG_ServerInfo, _settingsWnd, &settings::Update_ServerInfo);
	connect(_hyperChainWnd, &mine_hyperchain::SIG_BrowserInfo, this, &MainWindow::SIG_BrowserInfo);
	connect(_notify, &QtNotify::SIG_BuddyFailed, this, &MainWindow::Update_BuddyFailed);

    tabClick(MINE_HYPER_CHAIN);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    resizeChild();
}

void MainWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    resizeChild();

    if(_verificationWnd){
        if(_verificationWndShow){
            _verificationWnd->show(true);
            _verificationWnd->dlg_->activateWindow();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(_verificationWnd){
        if(_verificationWndShow){
            _verificationWnd->show(false);
        }
    }

    hide();

    event->ignore();
}

void MainWindow::resizeChild(){

}

void MainWindow::tabClick(E_TAB_TYPE type)
{
	_oldTab = _curTab;
    _curTab = type;

    if(MINE_HYPER_CHAIN == _curTab){
        showHyperChain();
    }
    else if(CHAIN_BROWER == _curTab){
        showChainBrower();
    }else if(HYPER_CHAIN_ATTESTATION == _curTab){
        showAttestation();
    }else if(NODE_SET == _curTab){
        showNodeSet();
    }else if(DEVELOPER_MOD == _curTab){
        showDevMode();
	}
	else if (TOKEN == _curTab){
		showToken();
	}
	else if (ABOUT == _curTab)
	{
		showAbout();
	}
	else if (HYPER_CHAIN_ATTESTATION_HISTORY == _curTab)
	{
		showAttestationHistory();
	}
	else if (HYPER_CHAIN_ATTESTATION_REG == _curTab)
	{
		showAttestationRecord();
	}

    resizeChild();
}

void MainWindow::GoBack()
{
	tabClick(_oldTab);
}

void MainWindow::onHyperChainClick()
{
    tabClick(MINE_HYPER_CHAIN);
}

void MainWindow::onChainBrowerClick()
{
    tabClick(CHAIN_BROWER);
}

void MainWindow::onChainAttestationClick()
{
    tabClick(HYPER_CHAIN_ATTESTATION);
}

void MainWindow::onNodeSetClick()
{
    tabClick(NODE_SET);
}

void MainWindow::onDevModeClick()
{
    tabClick(DEVELOPER_MOD);
}

void MainWindow::onTokenClick()
{
	tabClick(TOKEN);
}

void MainWindow::onAboutClick()
{
	tabClick(ABOUT);
}
void MainWindow::onAttestationHistory()
{
	tabClick(HYPER_CHAIN_ATTESTATION_HISTORY);
}

void MainWindow::onAttestationRecord(QSharedPointer<TEVIDENCEINFO> evidence)
{
	_recordWnd->setEvidence(evidence);
	tabClick(HYPER_CHAIN_ATTESTATION_REG);
    
}

void MainWindow::trayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger){
        if(isHidden()){
            show();
            return;
        }
    }else if(reason == QSystemTrayIcon::DoubleClick){
        showNormal();
        return;
    }
}

void MainWindow::hcQuit()
{	
	TEVIDENCEINFO evi;
	DBmgr::instance()->updateEvidence(evi, 3); 

	if (DBmgr::instance()->isOpen())
	{
		DBmgr::instance()->close();
	}

	RestApi::stopRest();
    trayIcon->hide();
    hide();
    qApp->quit();
}

void MainWindow::changeLanguage()
{
    if(langIndex_ == 0 )
	{
        langIndex_ = 1;
        appTranslatorZh_.load(Trans_Zh_Path());
        qApp->installTranslator(&appTranslatorZh_);
    }
	else
	{
        langIndex_ = 0;
		appTranslatorEn_.load(Trans_En_Path());
        qApp->installTranslator(&appTranslatorEn_);
    }

    _tabBar->ui->retranslateUi(_tabBar);
    _attestationRegWnd->ui->retranslateUi(_attestationRegWnd);
    _hyperChainWnd->retranslateUi();


	_browserWnd->retranslateUi();
	_recordWnd->retranslateUi();
	_historyWnd->retranslateUi();
	_settingsWnd->retranslateUi();
    _devModeWnd->retranslateUi();
	_tokenWnd->retranslateUi();
	_AboutWnd->retranslateUi();

    if(_verificationWnd){
        _verificationWnd->retranslateUI();
    }

    retranslateUi();
}

void MainWindow::about()
{
}

void MainWindow::showHyperChain()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelMyHyperChain->resetFont(tab_label::SELECTED_COLOR);
    _hyperChainWnd->show();

}

void MainWindow::showChainBrower()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelChainBrowser->resetFont(tab_label::SELECTED_COLOR);
	_browserWnd->show();
}

void MainWindow::showAttestation()
{
	hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelChainAttestation->resetFont(tab_label::SELECTED_COLOR);
    _attestationRegWnd->show();
}

void MainWindow::showAttestationHistory()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelChainAttestation->resetFont(tab_label::SELECTED_COLOR);
	_historyWnd->show();
}

void MainWindow::showAttestationRecord()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelChainAttestation->resetFont(tab_label::SELECTED_COLOR);
	_recordWnd->show();

}

void MainWindow::showNodeSet()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelNodeSet->resetFont(tab_label::SELECTED_COLOR);
	_settingsWnd->show();
}

void MainWindow::showDevMode()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelDevMode->resetFont(tab_label::SELECTED_COLOR);
    _devModeWnd->show();
}

void MainWindow::showToken()
{
	hideContentWnd();	
	resetTabTextColor();

	_tabBar->ui->labelToken->resetFont(tab_label::SELECTED_COLOR);
	_tokenWnd->show();
}

void MainWindow::showAbout()
{
	hideContentWnd();
	resetTabTextColor();

	_tabBar->ui->labelAbout->resetFont(tab_label::SELECTED_COLOR);
	_AboutWnd->show();
}

void MainWindow::connectCtrlSignal()
{
    connect(_tabBar->ui->labelMyHyperChain, &tab_label::clicked, this, &MainWindow::onHyperChainClick);
    connect(_tabBar->ui->labelChainAttestation, &tab_label::clicked, this, &MainWindow::onChainAttestationClick);
    connect(_tabBar->ui->labelChainBrowser, &tab_label::clicked, this, &MainWindow::onChainBrowerClick);
    connect(_tabBar->ui->labelNodeSet, &tab_label::clicked, this, &MainWindow::onNodeSetClick);
    connect(_tabBar->ui->labelDevMode, &tab_label::clicked, this, &MainWindow::onDevModeClick);
	connect(_tabBar->ui->labelToken, &tab_label::clicked, this, &MainWindow::onTokenClick);
	connect(_tabBar->ui->labelAbout, &tab_label::clicked, this, &MainWindow::onAboutClick);

}

void MainWindow::createActions()
{
    quitAction = new QAction(tr("quit"), this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::hcQuit);
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("HyperChain"));

	trayIcon->setIcon(QIcon(logoPath()));
	setWindowIcon(QIcon(logoPath()));

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconClicked);

    trayIcon->show();
}

void MainWindow::retranslateUi()
{
   ui->retranslateUi(this);

   quitAction->setText(QApplication::translate("MainWindow", ("quit"), Q_NULLPTR));
}


void MainWindow::hideContentWnd(){
    _hyperChainWnd->hide();
	_hyperChainWnd->hideblockinfo();
	_browserWnd->hide();
    _attestationRegWnd->hide();
	_recordWnd->hide();
	_historyWnd->hide();
	_settingsWnd->hide();
    _devModeWnd->hide();
	_tokenWnd->hide();
	_AboutWnd->hide();
}

void MainWindow::resetTabTextColor()
{
    _tabBar->ui->labelMyHyperChain->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelChainBrowser->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelChainAttestation->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelNodeSet->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelDevMode->resetFont(tab_label::NORMAL_COLOR);
	_tabBar->ui->labelToken->resetFont(tab_label::NORMAL_COLOR);
	_tabBar->ui->labelAbout->resetFont(tab_label::NORMAL_COLOR);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    Q_UNUSED(event);
}

void MainWindow::dropEvent(QDropEvent *event){
    Q_UNUSED(event);
}

void MainWindow::showAttestationReg(bool show)
{
    if(show){
        _tabBar->ui->labelChainAttestation->show();
    }else{
        _tabBar->ui->labelChainAttestation->hide();
    }
}

void MainWindow::showVerificationWnd(QVariantMap info)
{
    if(_verificationWnd == nullptr){
        _verificationWnd = new reg_verification_frameless(this);
    }

	QRect rect = _recordWnd->geometry();
	QPoint gp = _recordWnd->mapToGlobal(QPoint(rect.x(), rect.y()));
    QRect vRect = _verificationWnd->dlg_->geometry();
    int vw = vRect.width();
    int vh = vRect.height();

    _verificationWnd->dlg_->move(gp.x() + (rect.width() - vw) / 2, gp.y() + 50);

    _verificationWnd->updateDetail(info);
    _verificationWnd->dlg_->activateWindow();
}

void MainWindow::updateFinish()
{
	runningUpdate_ = false;
}

void MainWindow::addEvidence(QSharedPointer<TEVIDENCEINFO> evi, int index)
{
    listEvi_.append(evi);
    DBmgr::instance()->insertEvidence(*evi.data());
	
	_historyWnd->insertDatetoList(*evi, index);
}

void MainWindow::updateEvidence()
{
    for(auto evi: listEvi_){
        if(evi->iFileState == CONFIRMING){
            evi->iFileState = CONFIRMED;
			_historyWnd->updateEvidence(evi->cFileHash, evi, 1);

            DBmgr::instance()->updateEvidence(*evi, 1);

            break;
        }
    }
}

void MainWindow::updateEvidenceByHash(string hash, time_t time, uint64 blocknumber)
{
	for (auto evi : listEvi_)
	{
		if ( (hash.compare(evi->cFileHash) == 0) && (time == evi->tRegisTime))
		{
			evi->iBlocknum = blocknumber;
			evi->iFileState = CONFIRMED;
			_historyWnd->updateEvidence(evi->cFileHash, evi, 1);
			DBmgr::instance()->updateEvidence(*evi, 4); //hash and time
			break;
		}
	}
}


QList<QSharedPointer<TEVIDENCEINFO> > &MainWindow::getEvidence()
{
    return listEvi_;
}

void MainWindow::addLog(QString log){
    _devModeWnd->addLog(log);
}

QtNotify* MainWindow::GetQtNotify()
{
	if (_notify)
		return _notify;
	else
		return nullptr;
}

void MainWindow::SIG_BrowserInfo(string blocknum)
{
	_browserWnd->showinfo(blocknum);
	onChainBrowerClick();
}

void MainWindow::Update_BuddyFailed(string hash, time_t time)
{
	for (auto evi : listEvi_)
	{
		if ((hash.compare(evi->cFileHash) == 0) && (time == evi->tRegisTime))
		{
			evi->iFileState = REJECTED;
			_historyWnd->updateEvidence(evi->cFileHash, evi, 1);
			DBmgr::instance()->updateEvidence(*evi, 2);
			break;
		}
	}
}

void MainWindow::onDeleteHistoryItem(TEVIDENCEINFO * evi)
{	
	for (int i = 0; i < listEvi_.size(); i++)
	{
		if ((evi->cFileHash.compare(listEvi_.at(i)->cFileHash) == 0) && (evi->tRegisTime == listEvi_.at(i)->tRegisTime))
		{
			listEvi_.removeAt(i);
			break;
		}
	}
	DBmgr::instance()->delEvidence(*evi);
	_historyWnd->DeleteItem(evi);
	
}

void MainWindow::BuddyRetry(QSharedPointer<TEVIDENCEINFO> evi)
{
	SetFilePoeRecord(evi.data());
	bool bfind = false;

	for (auto levi : listEvi_)
	{
		if ((levi->cFileHash.compare(evi->cFileHash) == 0) && (levi->tRegisTime == evi->tRegisTime))
		{
			levi->iFileState = evi->iFileState;		
			bfind = true;
			break;
		}
	}

	if (!bfind)
	{
		listEvi_.append(evi);
	}
	DBmgr::instance()->updateEvidence(*evi, 2);

}



void MainWindow::initNoConfiringList()
{
	QList<TEVIDENCEINFO> evis;
	DBmgr::instance()->getNoConfiringList(evis);

	for (auto e : evis)
	{
		QSharedPointer<TEVIDENCEINFO> qevi = QSharedPointer<TEVIDENCEINFO>(new TEVIDENCEINFO);
		qevi->cCustomInfo = e.cCustomInfo;
		qevi->cFileHash = e.cFileHash;
		qevi->cFileName = e.cFileName;
		qevi->cRightOwner = e.cRightOwner;
		qevi->iBlocknum = e.iBlocknum;
		qevi->iFileSize = e.iFileSize;
		qevi->iFileState = e.iFileState;
		qevi->tRegisTime = e.tRegisTime;
		listEvi_.append(qevi);
	}
}



