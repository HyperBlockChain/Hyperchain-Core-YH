/*Copyright 2017 hyperchain.net  (Hyper Block Chain)
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_tab_bar_wnd.h"
#include "ui_attestation_reg.h"

#include "wnd/mine_hyperchain.h"
#include "wnd/chain_browser.h"
#include "wnd/attestation_reg.h"
#include "wnd/attestation_record.h"
#include "wnd/attestation_history.h"
#include "wnd/node_set.h"
#include "wnd/dev_mode.h"
#include "wnd/reg_verification_frameless.h"
#include "wnd/about_dlg.h"

#include "util/commonutil.h"
#include "db/dbmgr.h"

#include <QResizeEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QMenu>
#include <QSystemTrayIcon>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

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
    _chainBrowserWnd        = new chain_browser(this);
    _attestationRegWnd      = new attestation_reg(this);
    _attestationRecordWnd   = new attestation_record(this);
    _attestationHistoryWnd  = new attestation_history(this);
    _nodeSetWnd             = new node_set(this);
    _devModeWnd             = new dev_mode(this);

    ui->verticalLayout->addWidget(_hyperChainWnd);
    ui->verticalLayout->addWidget(_chainBrowserWnd);
    ui->verticalLayout->addWidget(_attestationRegWnd);
    ui->verticalLayout->addWidget(_attestationRecordWnd);
    ui->verticalLayout->addWidget(_attestationHistoryWnd);
    ui->verticalLayout->addWidget(_nodeSetWnd);
    ui->verticalLayout->addWidget(_devModeWnd);

    connectCtrlSignal();

    setUpdateTimerConnect();

#ifdef QT_DEBUG
    updateTimer_.start(1000 * 30);
#else
    updateTimer_.start(1000 * 60 * 3);
#endif

    tabClick(MINE_HYPER_CHAIN);

    changeLanguage();
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

    resizeChild();
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

void MainWindow::onAttestationHistory()
{
    showAttestationHistory();
}

void MainWindow::onAttestationRecord(QSharedPointer<TEVIDENCEINFO> evidence)
{
    _attestationRecordWnd->setEvidence(evidence);
    showAttestationRecord();
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
    trayIcon->hide();
    hide();
    qApp->quit();
}

void MainWindow::changeLanguage()
{
    if(langIndex_ == 0 ){
        langIndex_ = 1;
        appTranslatorZh_.load(QString(":/translations/app_zh.qm"));
        qApp->installTranslator(&appTranslatorZh_);
    }else{
        langIndex_ = 0;
        appTranslatorEn_.load(QString(":/translations/app_en.qm"));
        qApp->installTranslator(&appTranslatorEn_);
    }

    _tabBar->ui->retranslateUi(_tabBar);
    _attestationRegWnd->ui->retranslateUi(_attestationRegWnd);
    _hyperChainWnd->retranslateUi();

    _chainBrowserWnd->retranslateUi();
    _attestationRecordWnd->retranslateUi();
    _attestationHistoryWnd->retranslateUi();
    _nodeSetWnd->retranslateUi();
    _devModeWnd->retranslateUi();

    if(_verificationWnd){
        _verificationWnd->retranslateUI();
    }

    retranslateUi();
}

void MainWindow::about()
{
    if(nullptr == _aboutWnd){
        _aboutWnd = new about_dlg(this);
    }

    QRect rect = geometry();
    QPoint gp = QPoint(rect.x(), rect.y());
    QRect vRect = _aboutWnd->dlg_->geometry();
    int vw = vRect.width();
    int vh = vRect.height();

    _aboutWnd->dlg_->move(gp.x() + (rect.width() - vw) / 2, gp.y() + (vh / 2));

    _aboutWnd->show();
}

void MainWindow::showHyperChain()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelMyHyperChain->resetFont(tab_label::SELECTED_COLOR);
    _hyperChainWnd->show();

    onUpdateTimer();

#ifdef QT_DEBUG
    updateTimer_.start(1000 * 30);
#else
    updateTimer_.start(1000 * 60 * 3);
#endif
}

void MainWindow::showChainBrower()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelChainBrowser->resetFont(tab_label::SELECTED_COLOR);
    _chainBrowserWnd->show();
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
    _attestationHistoryWnd->show();
}

void MainWindow::showAttestationRecord()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelChainAttestation->resetFont(tab_label::SELECTED_COLOR);
    _attestationRecordWnd->show();
}

void MainWindow::showNodeSet()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelNodeSet->resetFont(tab_label::SELECTED_COLOR);
    _nodeSetWnd->show();
}

void MainWindow::showDevMode()
{
    hideContentWnd();
    resetTabTextColor();

    _tabBar->ui->labelDevMode->resetFont(tab_label::SELECTED_COLOR);
    _devModeWnd->show();
}

void MainWindow::connectCtrlSignal()
{
    connect(_tabBar->ui->labelMyHyperChain, &tab_label::clicked, this, &MainWindow::onHyperChainClick);
    connect(_tabBar->ui->labelChainAttestation, &tab_label::clicked, this, &MainWindow::onChainAttestationClick);
    connect(_tabBar->ui->labelChainBrowser, &tab_label::clicked, this, &MainWindow::onChainBrowerClick);
    connect(_tabBar->ui->labelNodeSet, &tab_label::clicked, this, &MainWindow::onNodeSetClick);
    connect(_tabBar->ui->labelDevMode, &tab_label::clicked, this, &MainWindow::onDevModeClick);
}

void MainWindow::createActions()
{
    changeLang = new QAction(tr("Switch lang"), this);
    connect(changeLang, &QAction::triggered, this, &MainWindow::changeLanguage);

    showWndAction = new QAction(tr("show"), this);
    connect(showWndAction, &QAction::triggered, this, &MainWindow::show);

    aboutAction = new QAction(tr("about"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    quitAction = new QAction(tr("quit"), this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::hcQuit);
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(changeLang);
    trayIconMenu->addAction(showWndAction);
    trayIconMenu->addAction(aboutAction);
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

   changeLang->setText(QApplication::translate("MainWindow", ("Switch lang"), Q_NULLPTR));
   showWndAction->setText(QApplication::translate("MainWindow", ("show"), Q_NULLPTR));
   aboutAction->setText(QApplication::translate("MainWindow", ("about"), Q_NULLPTR));
   quitAction->setText(QApplication::translate("MainWindow", ("quit"), Q_NULLPTR));
}

void MainWindow::onUpdateTimer()
{
    if(runningUpdate_){
        return;
    }

    runningUpdate_ = true;

    emit sigUpdate();
}

void MainWindow::setUpdateTimerConnect()
{
    connect(&updateTimer_, &QTimer::timeout, this, &MainWindow::onUpdateTimer);

    //notify child update data
    connect(this, &MainWindow::sigUpdate, _hyperChainWnd, &mine_hyperchain::onUpdateTimer);
    connect(this, &MainWindow::sigUpdate, _chainBrowserWnd, &chain_browser::onUpdateTimer);
}

void MainWindow::hideContentWnd(){
    _hyperChainWnd->hide();
    _chainBrowserWnd->hide();
    _attestationRegWnd->hide();
    _attestationRecordWnd->hide();
    _attestationHistoryWnd->hide();
    _nodeSetWnd->hide();
    _devModeWnd->hide();
}

void MainWindow::resetTabTextColor()
{
    _tabBar->ui->labelMyHyperChain->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelChainBrowser->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelChainAttestation->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelNodeSet->resetFont(tab_label::NORMAL_COLOR);
    _tabBar->ui->labelDevMode->resetFont(tab_label::NORMAL_COLOR);
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

    QRect rect = _attestationRecordWnd->geometry();
    QPoint gp = _attestationRecordWnd->mapToGlobal(QPoint(rect.x(), rect.y()));
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

void MainWindow::addEvidence(QSharedPointer<TEVIDENCEINFO> evi)
{
    listEvi_.append(evi);

    DBmgr::instance()->insertEvidence(*evi.data());


    _attestationHistoryWnd->addEvidence(evi);
}

void MainWindow::updateEvidence()
{
    for(auto evi: listEvi_){
        if(evi->iFileState == CONFIRMING){
            evi->iFileState = CONFIRMED;
            _attestationHistoryWnd->updateEvidence(evi->cFileHash, evi, 1);

            DBmgr::instance()->updateEvidence(*evi, 1);

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
