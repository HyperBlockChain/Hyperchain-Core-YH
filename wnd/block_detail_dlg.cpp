/*Copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or https://opensource.org/licenses/MIT

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

#include "block_detail_dlg.h"
#include "channel/block_detail_channel.h"

#include "util/commonutil.h"
#include "customui/base_frameless_wnd.h"
#include "mainwindow.h"

#include <QTextCodec>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebChannel/QWebChannel>

#include <QApplication>
#include <QVariantMap>

extern MainWindow* g_mainWindow();

block_detail_dlg::block_detail_dlg(QObject *parent) : QObject(parent)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    init();
}

void block_detail_dlg::updateDetail(QVariantMap generalInfo)
{
    emit reg_->sigNewBlockDetail(generalInfo);
}

void block_detail_dlg::show(bool bShow)
{
    if(bShow){
        dlg_->show();
    }else{
        dlg_->hide();
    }
}

void block_detail_dlg::setLanguage(int lang)
{
    emit reg_->sigChangeLang(lang);
}

void block_detail_dlg::init()
{
    dlg_ = QSharedPointer<base_frameless_wnd>(new base_frameless_wnd());

    dlg_->showTitleBar(true);
    dlg_->setScale(false);
    dlg_->setGeometry(QRect(0, 0, 200, 300));

    dlg_->setLogoPic(logoPath(), 20, QPoint(6,3), 1);
    dlg_->setMinMaxCloseButton(QSize(25, 25), true, closeBtnStyle());

    dlg_->hide();

    Qt::WindowFlags flags = dlg_->windowFlags();
    flags |= Qt::WindowStaysOnTopHint ;
    dlg_->setWindowFlags(flags);

    view_ = new QWebEngineView((QWidget*)dlg_.data()->content_);
    view_->setAcceptDrops(false);
    dlg_->addWidget(view_);

    QWebChannel *channel = new QWebChannel(this);

    reg_ = new block_detail_channel(this);
    channel->registerObject(QString("qBlockDetail"), reg_);

    view_->page()->setWebChannel(channel);

#ifdef QT_DEBUG
    #if defined (Q_OS_WIN)
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../../ui/view/blockdetail.html");
    #else
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../ui/view/blockdetail.html");
    #endif
#else
    QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("ui/view/blockdetail.html");
#endif

    view_->page()->load(QUrl(str));
}
