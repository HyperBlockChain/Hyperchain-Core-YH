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

#include "attestation_reg.h"
#include "ui_attestation_reg.h"
#include "util/md5.h"
#include "util/commonutil.h"
#include "mainwindow.h"
#include "HChainP2PManager/interface/QtInterface.h"

#include <QMessageBox>
#include <QTextCodec>

#include <QDir>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QCryptographicHash>
#include <QFileInfo>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>

#include <QtGlobal>
#include <QFileDialog>

extern MainWindow* g_mainWindow();

attestation_reg::attestation_reg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::attestation_reg)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    ui->setupUi(this);
	hideDropDrag();
    setConnection();

}

attestation_reg::~attestation_reg()
{
    delete ui;
}

void attestation_reg::dragEnterEvent(QDragEnterEvent *event){

	event->acceptProposedAction();
}

void attestation_reg::dropEvent(QDropEvent *event){
	QRect area = ui->frameFileArea->geometry();

	QPoint areagp = QPoint(area.x(), area.y());
	areagp = ui->frame_2->mapToGlobal(areagp);
	QRect areag = QRect(areagp.x(), areagp.y(), area.width(), area.height());

	QPoint point = event->pos();
	QPoint gpoint = this->mapToGlobal(point);

	if (areag.contains(gpoint)){
        ui->digitalFingerprintLabel->setText(tr("DigitalFingerprint"));

		ui->labelFileName->setText(QStringLiteral(""));
		ui->labelFileName->hide();

		const QMimeData*qm = event->mimeData();
		QString path = qm->urls()[0].toLocalFile();

        QFile f(path);

        if(f.open(QIODevice::ReadOnly)){

            fpath_ = path;

            QCryptographicHash h(QCryptographicHash::Sha512);
            h.addData(&f);

            sha512_ = h.result().toHex();
            QString s1 = sha512_.mid(0, 32);
            QString s2 = sha512_.mid(32, 32);
            QString s3 = sha512_.mid(64, 32);
            QString s4 = sha512_.mid(96, 32);

            QString str512 = QString("%1 %2 %3 %4").arg(s1).arg(s2).arg(s3).arg(s4);
            ui->digitalFingerprintLabel->setText(str512);

            QFileInfo finfo(path);

			ui->labelFileName->setText(finfo.fileName());
			ui->labelFileName->show();

			f.close();

		}
    }
}

void attestation_reg::onHistory()
{
    g_mainWindow()->onAttestationHistory();
}

void attestation_reg::onReset()
{
    sha512_.clear();
    fpath_.clear();

	ui->labelFileName->clear();
	ui->labelFileName->hide();

	ui->editRightOwner->clear();

}

void attestation_reg::onPublicAdd()
{
    QString key = ui->editPublicKey->text();
    QString val = ui->editPublicVal->text();

    if(key.isEmpty() || val.isEmpty()){
        return;
    }

    int index = getAdded(key);
    if(index >= 0){
        publicInfo_[index].v = val;
    }else{
        PublicInfo info;
        info.index = publicInfo_.size() + 1;
        info.k = key;
        info.v = val;
        publicInfo_.push_back(info);
    }

    QString txt = formatPublicInfo();
    ui->textEdit->setText(txt);

    ui->editPublicKey->clear();
    ui->editPublicVal->clear();
}

void attestation_reg::onPublicClear()
{

	if (publicInfo_.size() > 0)
	{
		publicInfo_.pop_back();
	}

	if (publicInfo_.size() > 0)
	{
		QString txt = formatPublicInfo();
		ui->textEdit->setText(txt);
	}
	else
	{
		ui->textEdit->clear();
	}

}

QString attestation_reg::formatPublicInfo()
{
    int size = publicInfo_.size();

    QString ret;

    for(int i = 0; i < size; i++){
        if(ret.length()){
            ret += QString("\n");
        }

        QString tmp = QString("%1 : %2").arg(publicInfo_.at(i).k).arg(publicInfo_.at(i).v);

        ret += tmp;
    }

    return ret;
}

int attestation_reg::getAdded(QString key)
{
    int size = publicInfo_.size();

    for(int i = 0; i < size; i++){
        if(publicInfo_.at(i).k == key){
            return i;
        }
    }

    return -1;
}

QString attestation_reg::convertPublicInfo2Json()
{
    QJsonArray arr;

    int size = publicInfo_.size();
    for(int i = 0; i < size; i++){
        QJsonObject t;

        const PublicInfo & item = publicInfo_.at(i);
        t.insert(QString("index"), item.index);
        t.insert(QString("key"), item.k);
        t.insert(QString("value"), item.v);

        arr.append(t);
    }

	QString json= nullptr;
	if (size > 0)
	{
		QJsonDocument doc(arr);
		QByteArray ba = doc.toJson(QJsonDocument::Compact);
		QString json(ba);
		return json;
	}

    return json;
}

void attestation_reg::setConnection()
{
	QString qssbuttonblue = "QPushButton {\
				   color: white;\
				   }\
				   QPushButton:enabled {\
				   background: rgb(0, 165, 235);\
				   color: white;\
				   }\
				   QPushButton:!enabled {\
				   background: gray;\color: rgb(200, 200, 200);\
				   }\
				   QPushButton:enabled:hover {\
				   background: rgb(0, 180, 255);\
				   }\
				   QPushButton:enabled:pressed {\
				   background: rgb(0, 140, 215);\
				   }";
	ui->btnHistory->setStyleSheet(qssbuttonblue);
	ui->btnCommit->setStyleSheet(qssbuttonblue);
	ui->btnReset->setStyleSheet(qssbuttonblue);
	ui->btnAddPublic->setStyleSheet(qssbuttonblue);
	ui->btnClear->setStyleSheet(qssbuttonblue);
	ui->btnOpenFile->setStyleSheet(qssbuttonblue);

    connect(ui->btnHistory, &QPushButton::clicked, this, &attestation_reg::onHistory);
    connect(ui->btnCommit, &QPushButton::clicked, this, &attestation_reg::onCommit);
	connect(ui->btnReset, &QPushButton::clicked, this, &attestation_reg::onReset);
    connect(ui->btnAddPublic, &QPushButton::clicked, this, &attestation_reg::onPublicAdd);
    connect(ui->btnClear, &QPushButton::clicked, this, &attestation_reg::onPublicClear);

	connect(ui->btnOpenFile, &QPushButton::clicked, this, &attestation_reg::onOpenFile);
}

void attestation_reg::onCommit()
{
    if(sha512_.isEmpty()){
       return;
    }

    QSharedPointer<TEVIDENCEINFO> evi = QSharedPointer<TEVIDENCEINFO>(new TEVIDENCEINFO);

	evi->cFileName = ui->labelFileName->text().toStdString();
    evi->cCustomInfo = convertPublicInfo2Json().toStdString();
	evi->cRightOwner = ui->editRightOwner->toPlainText().toStdString();
    evi->cFileHash = sha512_.toStdString();
    evi->iFileState = CONFIRMING;

	evi->tRegisTime = QDateTime::currentDateTime().toTime_t();

    QFileInfo finfo(fpath_);
    evi->iFileSize = finfo.size();

	SetFilePoeRecord(evi.data());

    ui->digitalFingerprintLabel->setText(tr("DigitalFingerprint"));

	ui->labelFileName->clear();
	ui->labelFileName->hide();
    ui->textEdit->clear();

    sha512_.clear();
    fpath_.clear();
    publicInfo_.clear();

    g_mainWindow()->addEvidence(evi);
    g_mainWindow()->onAttestationRecord(evi);
}

void attestation_reg::onOpenFile()
{
	QFile f;
	QString path = QFileDialog::getOpenFileName(this, QString("Select File"), QString("/"), QString("(*.*)"));
	f.setFileName(path);
	if (f.open(QIODevice::ReadOnly ))
	{
		fpath_ = path;

		QCryptographicHash h(QCryptographicHash::Sha512);
		h.addData(&f);

		sha512_ = h.result().toHex();
		QString s1 = sha512_.mid(0, 32);
		QString s2 = sha512_.mid(32, 32);
		QString s3 = sha512_.mid(64, 32);
		QString s4 = sha512_.mid(96, 32);

		QString str512 = QString("%1 %2 %3 %4").arg(s1).arg(s2).arg(s3).arg(s4);
		ui->digitalFingerprintLabel->setText(str512);

		QFileInfo finfo(path);

		ui->labelFileName->setText(finfo.fileName());
		ui->labelFileName->show();
		f.close();
	}
}

void attestation_reg::hideDropDrag()
{
	ui->frameFileArea->hide();
}