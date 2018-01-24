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

#ifndef BLOCK_DETAIL_DLG_H
#define BLOCK_DETAIL_DLG_H

#include <QObject>
#include <QSharedPointer>
#include "HChainP2PManager/headers/inter_public.h"

class QWebEngineView;
class base_frameless_wnd;
class block_detail_channel;

class block_detail_dlg : public QObject
{
    Q_OBJECT
public:
    explicit block_detail_dlg(QObject *parent = 0);

public:
    void updateDetail(QVariantMap generalInfo);
    void show(bool bShow);
    void setLanguage(int lang);
signals:

public slots:

private:
    void init();

private:
    QSharedPointer<base_frameless_wnd> dlg_;
    QWebEngineView *view_;
    block_detail_channel* reg_;
};

#endif // BLOCK_DETAIL_DLG_H