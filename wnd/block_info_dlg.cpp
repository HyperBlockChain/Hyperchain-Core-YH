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

#include "block_info_dlg.h"
#include "customui/base_frameless_wnd.h"

#include "channel/block_info_channel.h"


#include <QTextCodec>
#include <QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QWebChannel>

#include <QApplication>
#include <QVariantMap>
#include <QDebug>

block_info_dlg::block_info_dlg(QObject *parent) : QObject(parent)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    init();
}

void block_info_dlg::show(bool bShow)
{
    dlg_->setHidden(!bShow);
}

void block_info_dlg::setGeometry(QRect rect)
{
    dlg_->setGeometry(rect);
}

void block_info_dlg::refreshNodeInfo(QSharedPointer<TBLOCKINFO> pNodeInfo)
{
    static int64 MAX_TIME_S = 9999999999;
    QVariantMap m;

	m["blockNum"] = pNodeInfo->iBlockNo;
	m["fileName"] = QString::fromStdString(pNodeInfo->tPoeRecordInfo.cFileName);
	m["customInfo"] = QString::fromStdString(pNodeInfo->tPoeRecordInfo.cCustomInfo);
	m["rightOwner"] = QString::fromStdString(pNodeInfo->tPoeRecordInfo.cRightOwner);
	m["fileHash"] = QString::fromStdString(pNodeInfo->tPoeRecordInfo.cFileHash);
	m["regTime"] = pNodeInfo->tPoeRecordInfo.tRegisTime;
	m["fileSize"] = pNodeInfo->tPoeRecordInfo.iFileSize;
	m["fileState"] = pNodeInfo->tPoeRecordInfo.iFileState;

    emit reg_->sigNewBlockInfo(m);
}

bool block_info_dlg::hasFcous()
{
    return dlg_->hasFocus();
}

void block_info_dlg::setFocus()
{
    dlg_->setFocus();
}

void block_info_dlg::setLanguage(int lang)
{
    reg_->sigChangeLang(lang);
}

//bool block_info_dlg::eventFilter(QObject *watched, QEvent *event)
//{
//    if(watched == view_){
//        int t = event->type();
//        if(event->type() == QEvent::FocusOut){
//            dlg_->hide();
//            return true;
//        }else if(event->type() == QEvent::FocusIn){
//            qDebug()<<"QEvent::FocusIn";
//        }else if(event->type() == QEvent::MouseButtonPress){
//            qDebug()<<"QEvent::MouseButtonPress";
//        }
//    }

//    return QObject::eventFilter(watched, event);
//}

void block_info_dlg::onMouseEnter(QEvent *event)
{
    mouseEnter_ = true;
}

void block_info_dlg::onMouseLeave(QEvent *event)
{
    mouseEnter_ = false;

    dlg_->hide();
}

void block_info_dlg::init()
{
    dlg_ = QSharedPointer<base_frameless_wnd>(new base_frameless_wnd());

    dlg_->setScale(false);
    dlg_->showTitleBar(false);
    dlg_->setGeometry(QRect(0,0,200,300));
    dlg_->hide();

    connect(dlg_.data(), &base_frameless_wnd::sigEnter, this, &block_info_dlg::onMouseEnter);
    connect(dlg_.data(), &base_frameless_wnd::sigLeave, this, &block_info_dlg::onMouseLeave);

    Qt::WindowFlags flags = dlg_->windowFlags();
    flags |= Qt::ToolTip;
    dlg_->setWindowFlags(flags);

    view_ = new QWebEngineView((QWidget*)dlg_.data()->content_);
    //view_->installEventFilter(this);
    view_->setAcceptDrops(false);

    dlg_->addWidget(view_);

    QWebChannel *channel = new QWebChannel(this);

    reg_ = new block_info_channel(this);
    channel->registerObject(QString("qBlockInfo"), reg_);

    view_->page()->setWebChannel(channel);

#ifdef QT_DEBUG
    #if defined (Q_OS_WIN)
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../../ui/view/blockinfo.html");
    #else
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../ui/view/blockinfo.html");
    #endif
#else
    QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("ui/view/blockinfo.html");
#endif

    view_->page()->load(QUrl(str));
    //view_->page()->load(QUrl("www.baidu.com"));
}
