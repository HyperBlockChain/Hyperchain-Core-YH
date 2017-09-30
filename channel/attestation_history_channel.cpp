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

#include "attestation_history_channel.h"
#include "p2p/interface/QtInterface.h"
#include "mainwindow.h"
#include "util/commonutil.h"
#include "db/dbmgr.h"
#include <QSharedPointer>
#include <QVariant>
#include <QVariantMap>

#include <QDebug>
#include <QMessageBox>

extern MainWindow* g_mainWindow();
static int BLOCK_NUM = 1;

attestation_history_channel::attestation_history_channel(QObject *parent) : QObject(parent)
{

}

void attestation_history_channel::onHtmlReady()
{
    getEvidence();
}

quint64 attestation_history_channel::getEvidenceCount()
{
  
    QList<QSharedPointer<TEVIDENCEINFO> > evis = g_mainWindow()->getEvidence();

    return evis.size();
}

void attestation_history_channel::checkEvidenceInfo(QVariantMap evidenceInfo)
{


    QSharedPointer<TEVIDENCEINFO> pEvidenceInfo = QSharedPointer<TEVIDENCEINFO>(new TEVIDENCEINFO);

    convertEvidenceVariantMap2Struct(pEvidenceInfo.data(), evidenceInfo);

    MainWindow *wnd = g_mainWindow();

    wnd->onAttestationRecord(pEvidenceInfo);
}

void attestation_history_channel::getEvidence(int page, int pageSize)
{
    QList<QVariant> li;


    QList<TEVIDENCEINFO> evis;
    DBmgr::instance()->getEvidences(evis, -1);

    for(auto item: evis){
        QVariantMap v;

        convertEvidenceStruct2VariantMap(v, &item);

        v["blockNum"] = BLOCK_NUM++;

        li<<v;
    }

    emit sigGetEvidence(QVariant(li));
}

void attestation_history_channel::addEvidence(QSharedPointer<TEVIDENCEINFO> evidence)
{
    QVariantMap v;

    convertEvidenceStruct2VariantMap(v, evidence.data());

    emit sigAddEvidence(v);
}

void attestation_history_channel::updateEvidence(std::string hash, TEVIDENCEINFO* evi, int type)
{
    if(1 == type){
        QVariantMap v;
        v["iFileState"] = CONFIRMED;
        emit sigUpdateEvidence(QString::fromUtf8(hash.c_str()), v, type);
    }
}

void attestation_history_channel::delMultiEvidence(QVariant blockNumArray)
{
    QMessageBox::about(nullptr, "delMultiEvidence", "delMultiEvidence");

    QList<QVariant> li = blockNumArray.toList();

    for(auto item : li){
        quint64 blockNum = item.toULongLong();
        qDebug()<<"attestation_history_channel::delMultiEvidence: "<< blockNum;
    }
}

void attestation_history_channel::exportAllEvidence()
{
    QMessageBox::about(nullptr, "exportAllEvidence", "exportAllEvidence");
}

void attestation_history_channel::exportEvidence(QVariant blockNumArray)
{
    QMessageBox::about(nullptr, "exportEvidence", "exportEvidence");

    QList<QVariant> li = blockNumArray.toList();

    for(auto item : li){
        quint64 blockNum = item.toULongLong();
        qDebug()<<"attestation_history_channel::exportEvidence: "<< blockNum;
    }
}

void attestation_history_channel::verifyEvidence(QVariantMap evidence)
{
    g_mainWindow()->showVerificationWnd(evidence);
}
