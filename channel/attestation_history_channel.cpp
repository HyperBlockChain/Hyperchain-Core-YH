/*copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or https://opensource.org/licenses/MIT.

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

#include "attestation_history_channel.h"
#include "HChainP2PManager/interface/QtInterface.h"
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
    //--sjc--fix 临时注释
    //return GetAllHistoryFileBolockNum();
    QList<QSharedPointer<TEVIDENCEINFO> > evis = g_mainWindow()->getEvidence();

    return evis.size();
}

void attestation_history_channel::checkEvidenceInfo(QVariantMap evidenceInfo)
{
//    _tEvidenceInfo
//    {
//        "cFileName":"文件名",
//        "cCustomInfo":"自定义信息",
//        "cRightOwner":"文件所有者",
//        "cFileHash":"文件hash",
//        "iFileState":1,
//        "tRegisTime":111111111,
//        "iFileSize":222222222
//    }

    QSharedPointer<TEVIDENCEINFO> pEvidenceInfo = QSharedPointer<TEVIDENCEINFO>(new TEVIDENCEINFO);

    convertEvidenceVariantMap2Struct(pEvidenceInfo.data(), evidenceInfo);

    MainWindow *wnd = g_mainWindow();

    wnd->onAttestationRecord(pEvidenceInfo);
}

void attestation_history_channel::getEvidence(int page, int pageSize)
{
    QList<QVariant> li;

//    VEC_T_BLOCKINFO vec = GetHistoryFileInfo(page*pageSize, pageSize);

//    if(vec.empty()){
//        QVariantMap v1;

//        v1["blockNum"] = 1;
//        v1["cFileName"] = QString("file1.txt");
//        v1["cCustomInfo"] = QString("cCustomInfo");
//        v1["cRightOwner"] = QString("cRightOwner");
//        v1["cFileHash"] = QString("cFileHash");
//        v1["iFileState"] = CONFIRMED;
//        v1["tRegisTime"] = (quint64)111111111;
//        v1["iFileSize"] = (quint64)1024;

//        li<<v1;

//        QVariantMap v2;
//        v2["blockNum"] = 2;
//        v2["cFileName"] = QString("file12.txt");
//        v2["cCustomInfo"] = QString("cCustomInfo2");
//        v2["cRightOwner"] = QString("cRightOwner2");
//        v2["cFileHash"] = QString("cFileHash2");
//        v2["iFileState"] = CONFIRMING;
//        v2["tRegisTime"] = (quint64)2222222;
//        v2["iFileSize"] = (quint64)2222222222;

//        li<<v2;
//    }else{
//        for(auto item : vec){
//            QVariantMap v;

//            v["blockNum"] = item->iBlockNum;
//            v["cFileName"] = QString::fromStdString(item->tFileInfo.cFileName);
//            v["cCustomInfo"] = QString::fromStdString(item->tFileInfo.cCustomInfo);
//            v["cRightOwner"] = QString::fromStdString(item->tFileInfo.cRightOwner);
//            v["cFileHash"] = QString::fromStdString(item->tFileInfo.cFileHash);
//            v["iFileState"] = item->tFileInfo.iFileState;
//            v["tRegisTime"] = item->tFileInfo.tRegisTime;
//            v["iFileSize"] = item->tFileInfo.iFileSize;

//            li<<v;
//        }
//    }


    //QList<QSharedPointer<TEVIDENCEINFO> > evis = g_mainWindow()->getEvidence();
    QList<TEVIDENCEINFO> evis;
    DBmgr::instance()->getEvidences(evis, -1);

    for(auto item: evis){
        QVariantMap v;

        //convertEvidenceStruct2VariantMap(v, item.data());
        convertEvidenceStruct2VariantMap(v, &item);

        //--sjc--fixto 内存中没有blockNum的值，怎么处理？
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
    //目前只更新状态
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
    //QMessageBox::about(nullptr, "verifyEvidence", "verifyEvidence");
    g_mainWindow()->showVerificationWnd(evidence);
}
