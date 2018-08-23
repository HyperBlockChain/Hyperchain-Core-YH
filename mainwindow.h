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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>
#include <QSystemTrayIcon>
#include <QTranslator>
#include <QTimer>
#include <QVariantMap>
#include <QList>
#include <QSharedPointer>

#include "wnd/tab_bar_wnd.h"
#include "customui/base_frameless_wnd.h"
#include "HChainP2PManager/headers/inter_public.h"

class mine_hyperchain;
class chain_browser;
class attestation_reg;
class attestation_record;
class attestation_history;
class dev_mode;
class QMenu;
class reg_verification_frameless;
class about_dlg;
class QtNotify;
class settings;
class token;
class browser;
class history;
class record;
class Cabout;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:

    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent * event);

	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;

public:
    void showAttestationReg(bool show);
    void showVerificationWnd(QVariantMap info);
    void updateFinish();
    void verificationWndShow(bool show){
        _verificationWndShow = show;
    }

	void GoBack();
	void BuddyRetry(QSharedPointer<TEVIDENCEINFO> evi);

    int getLanguage(){
        return langIndex_;
    }
    
    void addLog(QString log);
	QtNotify* GetQtNotify();

public:
    void addEvidence(QSharedPointer<TEVIDENCEINFO> evi, int index = 0);
    void updateEvidence();


	void updateEvidenceByHash(string hash, time_t time, uint64 blocknumber);

    QList<QSharedPointer<TEVIDENCEINFO> >& getEvidence();

public slots:
    void onHyperChainClick();
    void onChainBrowerClick();
    void onChainAttestationClick();
    void onNodeSetClick();
    void onDevModeClick();
	void onTokenClick();
	void onAboutClick();


    void onAttestationHistory();
    void onAttestationRecord(QSharedPointer<TEVIDENCEINFO> evidence);
	void onDeleteHistoryItem(TEVIDENCEINFO * evi);

    void trayIconClicked(QSystemTrayIcon::ActivationReason reason);
	void SIG_BrowserInfo(string blocknum);
	void Update_BuddyFailed(string hash, time_t time);

signals:

private slots:
    void hcQuit();
    void changeLanguage();
    void about();

private:
    void tabClick(E_TAB_TYPE type);
    void showHyperChain();
    void showChainBrower();
    void showAttestation();
    void showAttestationHistory();
    void showAttestationRecord();
    void showNodeSet();
    void showDevMode();
	void showToken();
	void showAbout();

    void hideContentWnd();
    void resetTabTextColor();
    void resizeChild();
	void initNoConfiringList();

private:
    void connectCtrlSignal();

private:
    void createActions();
    void createTrayIcon();

private:
    void retranslateUi();

private:
    bool runningUpdate_ = false;

    E_TAB_TYPE _curTab;
	E_TAB_TYPE _oldTab;
    mine_hyperchain*     _hyperChainWnd = nullptr;    
    attestation_reg*     _attestationRegWnd = nullptr;
	settings*			_settingsWnd = nullptr;
	browser*			_browserWnd = nullptr;
	token*				_tokenWnd = nullptr;
	history*			_historyWnd = nullptr;
	record*				_recordWnd = nullptr;
	Cabout*				_AboutWnd = nullptr;

    dev_mode*            _devModeWnd = nullptr;
    reg_verification_frameless* _verificationWnd = nullptr;

    tab_bar_wnd*         _tabBar;	
	QtNotify*			_notify = nullptr;

private:
    QAction *quitAction;
    QAction *aboutAction;
    QAction *showWndAction;
    QAction *changeLang;
    QMenu *trayIconMenu;
    QSystemTrayIcon *trayIcon;

    int langIndex_ = 0;
    QTranslator appTranslatorZh_;
    QTranslator appTranslatorEn_;

private:
    QList<QSharedPointer<TEVIDENCEINFO> > listEvi_;

    bool _verificationWndShow = false;
};

#endif // MAINWINDOW_H
