/*Copyright 2017 hyperchain.net  (Hyperchain)
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
#ifndef TAB_BAR_WND_H
#define TAB_BAR_WND_H

#include <QWidget>

class base_frameless_wnd;

namespace Ui {
class tab_bar_wnd;
}

enum E_TAB_TYPE{
    MINE_HYPER_CHAIN = 1,
    CHAIN_BROWER,
    HYPER_CHAIN_ATTESTATION,
    NODE_SET,
    DEVELOPER_MOD,
	TOKEN,
	ABOUT,
    HYPER_CHAIN_ATTESTATION_HISTORY,
    HYPER_CHAIN_ATTESTATION_REG
};

class mine_hyperchain;
class chain_browser;
class attestation_reg;
class attestation_record;
class attestation_history;
class node_set;
class dev_mode;
class tab_bar_wnd;

class tab_bar_wnd : public QWidget
{
    Q_OBJECT

public:
    explicit tab_bar_wnd(QWidget *parent = 0);
    ~tab_bar_wnd();

public:
    E_TAB_TYPE _curTab;

	const QString		frame_backgroundcolor = "background-color: rgb(15,105,181)";
	const QString		label_backgroundcolor = "background-color: rgb(15,105,181)";
public:
    void createChildWnd(base_frameless_wnd* flWnd);

private:
    void setConnect();

private slots:
    void onHyperChainClick();
    void onChainBrowerClick();
    void onChainAttestationClick();
    void onNodeSetClick();
    void onDevModeClick();

private:
    void tabClick(E_TAB_TYPE type);
    void showHyperChain();
    void showChainBrower();
    void showAttestation();
    void showNodeSet();
    void showDevMode();

    void hideWnd();

private:
    void resizeChild();

public:
    Ui::tab_bar_wnd *ui;

private:

};

#endif
