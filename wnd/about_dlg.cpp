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

#include "about_dlg.h"

#include "util/commonutil.h"
#include "customui/base_frameless_wnd.h"
#include "mainwindow.h"

#include <QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QApplication>

extern MainWindow* g_mainWindow();

about_dlg::about_dlg(QObject *parent) : QObject(parent)
{
    init();
}

void about_dlg::show()
{
    dlg_->show();
    dlg_->activateWindow();
}

void about_dlg::init()
{
    dlg_ = QSharedPointer<base_frameless_wnd>(new base_frameless_wnd());
    dlg_->hide();

    dlg_->showTitleBar(true);
    dlg_->setGeometry(QRect(0,0,400,400));

    dlg_->setLogoPic(logoPath(), 20, QPoint(6,3), 1);
    dlg_->setMinMaxCloseButton(QSize(25, 25), true, closeBtnStyle());

    QWebEngineView *view = new QWebEngineView((QWidget*)dlg_.data()->content_);
    view->setAcceptDrops(false);
    dlg_->addWidget(view);


#ifdef QT_DEBUG
    #if defined (Q_OS_WIN)
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../../ui/view/about.html");
    #else
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../ui/view/about.html");
    #endif
#else
    QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("ui/view/about.html");
#endif

    view->page()->load(QUrl(str));
}
