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

#include "chain_browser.h"
#include "ui_chain_browser.h"
#include "channel/chain_browser_channel.h"
#include "mainwindow.h"

#include <QtWidgets>
#include <QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QWebChannel>
#include <QTextCodec>

#include <QMessageBox>

extern MainWindow* g_mainWindow();

chain_browser::chain_browser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::chain_browser)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    ui->setupUi(this);

    _view = new QWebEngineView(this);
    QWebChannel *channel = new QWebChannel(this);

    reg_ = new chain_browser_channel(this);
    channel->registerObject(QString("qBrowser"), reg_);

    _view->page()->setWebChannel(channel);

#ifdef QT_DEBUG
    #if defined (Q_OS_WIN)
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../../ui/view/hpcBrowser.html");
    #else
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../ui/view/hpcBrowser.html");
    #endif
#else
    QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("ui/view/hpcBrowser.html");
#endif

    _view->page()->load(QUrl(str));
    ui->verticalLayout->addWidget(_view);
}

chain_browser::~chain_browser()
{
    delete ui;
}

void chain_browser::retranslateUi()
{
    reg_->setLanguage(g_mainWindow()->getLanguage());
}

void chain_browser::onUpdateTimer()
{
    reg_->onHtmlReady();
}
