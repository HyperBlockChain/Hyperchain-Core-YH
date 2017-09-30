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

#include "chain_data_status.h"
#include "wnd/block_info_dlg.h"


#include <QPainter>
#include <QtGlobal>
#include <QMouseEvent>
#include <QTime>
#include <QFontMetrics>
#include <QDebug>

chain_data_status::chain_data_status(QWidget *parent) : QLabel(parent)
{
    //setFrameShape(QFrame::Shape::Box);
    setMouseTracking(true);
}

void chain_data_status::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();

    //qDebug()<<"y = "<<y;

    if(y > labelH_ || y <= 0){
        emit sigHideNodeInfo();

        event->accept();
        //QWidget::mouseMoveEvent(event);

        return;
    }

    static bool s_showWnd = false;

    QMapIterator<int, QSharedPointer<TBLOCKINFO> > item(mapPix_);

    while (item.hasNext()) {
        item.next();
#ifdef WIN32
        if(y > 0 && y < labelH_ && (x >= item.key() - 2 && x <= item.key() + 2)){
#else
        if(y > 0 && y < labelH_ && (x >= item.key() - 4 && x <= item.key() + 4)){
#endif
            QSharedPointer<TBLOCKINFO> nodeInfo = item.value();
			qDebug() << "nodeInfo , blockNum: " << nodeInfo->iBlockNo;

            s_showWnd = true;

            emit sigShowNodeInfo(event->globalPos(), nodeInfo);

            event->accept();

            return;
        }
    }

    if(s_showWnd){
        s_showWnd = false;

        event->accept();
        emit sigHideNodeInfo();
    }
}

void chain_data_status::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void chain_data_status::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void chain_data_status::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    QPainter painter(this);

    static QFontMetrics fm = painter.fontMetrics();

    painter.drawRect(QRect(labelLR_, 0, width() - (labelLR_ * 2), labelH_));

    mapPix_.clear();


    int index = 0;
    for(auto item : items_){
		int pos = calVLinePos(item->iBlockNo);

        painter.drawLine(QPoint(labelLR_ + pos, 1), QPoint(labelLR_ + pos, labelH_-1));

        if(index == 0){
            //painter.fillRect(QRect(labelLR_+1, 1, pos - labelLR_ - 1, labelH_ - 1), Qt::green);
            painter.fillRect(QRect(labelLR_+1, 1, 10, labelH_ - 1), Qt::green);

            painter.drawText(QRect(18,labelH_ + 5, 20, 15), QStringLiteral("0"));
            painter.drawText(QRect(8,labelH_ + 22, 80, 15), tr("FIRST BLOCK"));
        }

		if (item->iBlockNo == myFirstBlockNum_){
            QString numStr = QString("%1").arg(myFirstBlockNum_);
            int w = fm.width(numStr);
            painter.drawText(QRect((labelLR_+pos) - (w/2) , labelH_ + 5, w, 15), QString("%1").arg(myFirstBlockNum_));

            w = fm.width(tr("MY FIRST BLOCK"));
            painter.drawText(QRect((labelLR_+pos) - (w/2), labelH_ + 22, w , 15), tr("MY FIRST BLOCK"));
        }

        mapPix_.insert(labelLR_ + pos, item);

        index++;
    }
}

void chain_data_status::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
}

int chain_data_status::calVLinePos(uint64 num)
{
    int w = this->width() - labelLR_*2;

    int pos = (int)((num * 1.0) / lastBlockNum_ * w);

    return pos;
}

