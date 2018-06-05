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
#include "dev_mode.h"
#include "ui_dev_mode.h"
#include "channel/dev_mode_channel.h"

#include "customui/custom_circle_btn.h"
#include "customui/hc_label.h"
#include "customui/chain_data_status.h"
#include "mainwindow.h"

#include <QtWidgets>
#include <QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QWebChannel>
#include <QTextCodec>

extern MainWindow* g_mainWindow();

dev_mode::dev_mode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::dev_mode)
{
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

	ui->setupUi(this);

    view_ = new QWebEngineView(this);
    QWebChannel *channel = new QWebChannel(this);

    reg_ = new dev_mode_channel(this);
    channel->registerObject(QString("qDevMode"), reg_);

    view_->page()->setWebChannel(channel);

#ifdef QT_DEBUG
    #if defined(Q_OS_WIN)
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../../ui/view/dev-nodeinfo.html");
    #else
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../ui/view/dev-nodeinfo.html");
    #endif
#else
    QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("ui/view/dev-nodeinfo.html");
#endif

    view_->page()->load(QUrl(str));
    ui->verticalLayout->addWidget(view_);
}

dev_mode::~dev_mode()
{
    delete ui;
}

void dev_mode::retranslateUi()
{
    emit reg_->sigChangeLang(g_mainWindow()->getLanguage());
}

void dev_mode::addLog(QString log){
    emit reg_->sigAddLog(log);
}
