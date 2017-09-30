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

#ifndef MINE_HYPERCHAIN_H
#define MINE_HYPERCHAIN_H

#include "p2p/headers/inter_public.h"
#include <QWidget>
#include <QSharedPointer>
#include <QTimer>

namespace Ui {
class mine_hyperchain;
}

class block_info_dlg;

class mine_hyperchain : public QWidget
{
    Q_OBJECT

public:
    explicit mine_hyperchain(QWidget *parent = 0);
    ~mine_hyperchain();

public:
    void retranslateUi();

public slots:
    void onSigShowNodeInfo(QPoint gpos, QSharedPointer<TBLOCKINFO> pNodeInfo);
    void onSigHideNodeInfo();
    void onHideNodeInfoDlgTimer();

    void onUpdateTimer();

private:
    void hideCheckBtn();

private:
    Ui::mine_hyperchain *ui;

    block_info_dlg* nid_ = nullptr;
    QTimer hideTimer_;

    bool isNidShow_ = false;
};

#endif // MINE_HYPERCHAIN_H
