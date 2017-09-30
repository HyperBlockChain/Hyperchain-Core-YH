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

#ifndef CHAIN_DATA_STATUS_H
#define CHAIN_DATA_STATUS_H

#include "p2p/headers/inter_public.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QVariantMap>

class chain_data_status : public QLabel
{
    Q_OBJECT
public:
    explicit chain_data_status(QWidget *parent = 0);


public:
    void setBlockNum(uint64 myFirstBlockNum, uint64 lastBlockNum){
        myFirstBlockNum_ = myFirstBlockNum;
        lastBlockNum_    = lastBlockNum;
    }

    void setBlocks(QList<QSharedPointer<TBLOCKINFO> >& items){
        items_ = items;
    }

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:
    void sigShowNodeInfo(QPoint gPoint, QSharedPointer<TBLOCKINFO> pNodeInfo);
    void sigHideNodeInfo();

public slots:

private:
    int calVLinePos(uint64 num);

private:
    int labelLR_ = 20;
    int labelH_ = 50;

    QList<QSharedPointer<TBLOCKINFO> > items_;
    uint64 myFirstBlockNum_ = 0;
    uint64 lastBlockNum_ = 0;

    QMap<int, QSharedPointer<TBLOCKINFO> > mapPix_;

};

#endif // CHAIN_DATA_STATUS_H
