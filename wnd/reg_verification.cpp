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

#include "reg_verification.h"
#include "ui_reg_verification.h"

#include "util/commonutil.h"
#include "p2p/interface/QtInterface.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QCryptographicHash>

reg_verification::reg_verification(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::reg_verification)
{
    ui->setupUi(this);
}

reg_verification::~reg_verification()
{
    delete ui;
}

void reg_verification::updateInfo(QVariantMap info)
{
    evidenceInfo_ = info;

    QString fname = info["cFileName"].toString();
    ui->labelFileName->setText(fname);

    QString fsize = QString::number(info["iFileSize"].toLongLong());
    ui->labelFileSzie->setText(fsize);

    sha512_ = info["cFileHash"].toString();
    QString s1 = sha512_.mid(0, 32);
    QString s2 = sha512_.mid(32, 32);
    QString s3 = sha512_.mid(64, 32);
    QString s4 = sha512_.mid(96, 32);

    QString str512 = QString("%1 %2 %3 %4").arg(s1).arg(s2).arg(s3).arg(s4);
    ui->labelCheckedHash->setText(str512);
    ui->labelCheckedHash_2->setText(QString(""));

    ui->labelIfMatch->setText(tr("file \n unmatch"));
}

void reg_verification::retranslateUI()
{
    ui->retranslateUi(this);
}

void reg_verification::dragEnterEvent(QDragEnterEvent *event){
    //http://blog.csdn.net/a3631568/article/details/53819972
    event->acceptProposedAction();
}

void reg_verification::dropEvent(QDropEvent *event){
    QRect area = ui->frame->geometry();

    QPoint areagp = QPoint(area.x(), area.y());
    areagp = mapToGlobal(areagp);
    QRect areag = QRect(areagp.x(), areagp.y(), area.width(), area.height());

    QPoint point = event->pos();
    QPoint gpoint = this->mapToGlobal(point);

    if (areag.contains(gpoint)){
        const QMimeData*qm = event->mimeData();
        QString path = qm->urls()[0].toLocalFile();

        QFile f(path);

        if(f.open(QIODevice::ReadOnly)){
            //save file path
            QString fpath_ = path;

            QCryptographicHash h(QCryptographicHash::Sha512);
            h.addData(&f);

            QString sha = h.result().toHex();
            QString s1 = sha.mid(0, 32);
            QString s2 = sha.mid(32, 32);
            QString s3 = sha.mid(64, 32);
            QString s4 = sha.mid(96, 32);

            QString str512 = QString("%1 %2 %3 %4").arg(s1).arg(s2).arg(s3).arg(s4);
            ui->labelCheckedHash_2->setText(str512);


            TEVIDENCEINFO evi;
            convertEvidenceVariantMap2Struct(&evi, evidenceInfo_);

            std::string shautf8 = sha.toStdString();
			if (VerifyPoeRecord(shautf8, &evi)){
                ui->labelIfMatch->setText(tr("file \n match"));
            }else{
                ui->labelIfMatch->setText(tr("file \n unmatch"));
            }
//            if(sha == sha512_){
//                ui->labelIfMatch->setText(tr("file \n match"));
//            }else{
//                ui->labelIfMatch->setText(tr("file \n unmatch"));
//            }
        }
    }
}