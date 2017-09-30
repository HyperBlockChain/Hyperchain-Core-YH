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

#include "attestation_history.h"
#include "ui_attestation_history.h"
#include "channel/attestation_history_channel.h"
#include "channel/node_set_channel.h"
#include "mainwindow.h"

#include <QtWidgets>
#include <QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QWebChannel>
#include <QTextCodec>

#include <QMessageBox>

extern MainWindow* g_mainWindow();

attestation_history::attestation_history(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::attestation_history)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    ui->setupUi(this);

    setAcceptDrops(false);

    view_ = new QWebEngineView(this);
    view_->setAcceptDrops(false);

    QWebChannel* channel = new QWebChannel(this);

    reg_ = new attestation_history_channel(this);
    channel->registerObject(QString("qHistory"), reg_);

    view_->page()->setWebChannel(channel);

#ifdef QT_DEBUG
    #if defined(Q_OS_WIN)
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../../ui/view/certificateHistory.html");
    #else
        QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("../ui/view/certificateHistory.html");
    #endif
#else
    QString str = QString("file:///%1/%2").arg(QApplication::applicationDirPath()).arg("ui/view/certificateHistory.html");
#endif

    view_->page()->load(QUrl(str));
    ui->verticalLayout->addWidget(view_);
}

attestation_history::~attestation_history()
{
    delete ui;
}

void attestation_history::updateEvidence()
{
    reg_->getEvidence();
}

void attestation_history::retranslateUi()
{
    emit reg_->sigChangeLang(g_mainWindow()->getLanguage());
}

void attestation_history::addEvidence(QSharedPointer<TEVIDENCEINFO> evidence)
{
    reg_->addEvidence(evidence);
}

void attestation_history::updateEvidence(string hash, QSharedPointer<TEVIDENCEINFO> evidence, int type)
{
    reg_->updateEvidence(hash, evidence.data(), type);
}
