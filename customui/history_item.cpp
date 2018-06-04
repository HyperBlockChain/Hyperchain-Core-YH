/*copyright 2016-2018 hyperchain.net (Hyperchain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT。
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

#include <QDateTime>
#include <QTextCodec>
#include "history_item.h"
#include "util/commonutil.h"
#include "mainwindow.h"
extern MainWindow* g_mainWindow();

history_item::history_item(QWidget *parent, TEVIDENCEINFO evis,  bool insert, bool update) : QWidget(parent)
{
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
	m_evis = evis;
	InitTitle(insert, update);

}

void history_item::retranslateUi()
{
	InitTitle(false, true);
}

void history_item::InitTitle(bool insert, bool update)
{
	q_time = tr("Submit At:");
	q_filename = tr("File Name:");
	q_filesize = tr("Size:");
	q_ower = tr("File Owner:");
	q_status = tr("Chain Status:");
	q_blocknum = tr("Blockchain Index:");
	q_verification = tr("Verify");
	q_details = tr("Details");
	q_delete = tr("Delete");
	q_retry = tr("Retry");

	int ntile = 4;
	int nwspace = 30;
	QVariantMap v;
	int ntop = 6;
	int nleft = 0;
	convertEvidenceStruct2VariantMap(v, &m_evis);

	if (!update)
	{
		t_time = new QLabel(this);
	}
	t_time->move(nleft, ntop);
	t_time->setText(q_time);
	t_time->setStyleSheet("background-color: transparent;font-size:15px;color:#0f69b5");
	t_time->adjustSize();

	nleft += t_time->width() + ntile;
	QString qz = QDateTime::fromTime_t(v["tRegisTime"].toULongLong()).toString("yyyy-MM-dd hh:mm:ss");
	if (!update)
	{
		m_time = new QLabel(this);
	}
	m_time->move(nleft, ntop);
	m_time->setText(qz);
	m_time->setStyleSheet("background-color: transparent;font-size:14px;color:#656565");
	m_time->adjustSize();

	nleft = 0;
	ntop += 20;
	if (!update)
	{
		t_filename = new QLabel(this);
	}
	t_filename->move(nleft, ntop);
	t_filename->setText(q_filename);
	t_filename->setStyleSheet("background-color: transparent;font-size:15px;color:#0f69b5");
	t_filename->adjustSize();

	nleft += t_filename->width() + ntile;
	if (!update)
	{
		m_filename = new QLabel(this);
	}
	m_filename->move(nleft, ntop);
	m_filename->setText(v["cFileName"].toString());
	m_filename->setStyleSheet("background-color: transparent;font-size:14px;color:#656565");
	m_filename->adjustSize();
	if (insert)
	{
		m_filename->resize(m_filename->width() + 20, m_filename->height());
	}

	nleft += m_filename->width() + nwspace;
	if (!update)
	{
		t_filesize = new QLabel(this);
	}
	t_filesize->move(nleft, ntop);
	t_filesize->setText(q_filesize);
	t_filesize->setStyleSheet("background-color: transparent;font-size:15px;color:#0f69b5");
	t_filesize->adjustSize();

	if (insert)
	{
		t_filesize->resize(t_filesize->width() + 26, t_filesize->height());
	}

	nleft += t_filesize->width() + ntile;
	QString qfilesize = getfilesize(v["iFileSize"].toULongLong());
	if (!update)
	{
		m_filesize = new QLabel(this);
	}
	m_filesize->move(nleft, ntop);
	m_filesize->setText(qfilesize);
	m_filesize->setStyleSheet("background-color: transparent;font-size:14px;color:#656565");
	m_filesize->adjustSize();

	nleft += m_filesize->width() + nwspace;
	if (!update)
	{
		t_blocknumber = new QLabel(this);
	}
	t_blocknumber->move(nleft, ntop);
	t_blocknumber->setText(q_blocknum);
	t_blocknumber->setStyleSheet("background-color: transparent;font-size:15px;color:#0f69b5");
	t_blocknumber->adjustSize();
	if (insert)
	{
		t_blocknumber->resize(t_blocknumber->width() + 20, t_blocknumber->height());
	}

	nleft += t_blocknumber->width() + ntile;
	if (!update)
	{
		m_blocknumber = new QLabel(this);
	}

	uint64 ublocknum = v["iBlocknum"].toULongLong();
	if (ublocknum == 0)
	{
		m_blocknumber->setText("");
	}
	else
	{
		QString blocknum = QString::number(ublocknum, 10);
		m_blocknumber->setText(blocknum);
	}

	m_blocknumber->move(nleft, ntop);
	m_blocknumber->setStyleSheet("background-color: transparent;font-size:14px;color:#656565");
	m_blocknumber->adjustSize();

	m_width = nleft + m_blocknumber->width();
	if (!update)
	{
		m_width += 120;
	}

	nleft =0;
	ntop += 20;
	if (!update)
	{
		t_ower = new QLabel(this);
	}
	t_ower->move(nleft, ntop);
	t_ower->setStyleSheet("background-color: transparent;font-size:15px;color:#0f69b5");
	t_ower->setText(q_ower);
	t_ower->adjustSize();

	nleft += t_ower->width() + ntile;
	if (!update)
	{
		m_ower = new QLabel(this);
	}
	m_ower->move(nleft, ntop);
	m_ower->setStyleSheet("background-color: transparent;font-size:14px;color:#656565");
	m_ower->setText(v["cRightOwner"].toString());
	m_ower->adjustSize();

	nleft += m_ower->width() + nwspace;
	if (!update)
	{
		t_status = new QLabel(this);
	}
	t_status->move(nleft, ntop);
	t_status->setStyleSheet("background-color: transparent;font-size:15px;color:#0f69b5");
	t_status->setText(q_status);
	t_status->adjustSize();
	if (insert)
	{
		t_status->resize(t_status->width() + 20, t_status->height());
	}

	nleft += t_status->width() + ntile;
	int istatus = v["iFileState"].toInt();
	QString qs = getstatus(istatus);
	if (!update)
	{
		m_status = new QLabel(this);
	}
	m_status->move(nleft, ntop);
	m_status->setStyleSheet("background-color: transparent;font-size:14px;color:#656565");
	m_status->setText(qs);
	m_status->adjustSize();

	nleft += m_status->width() + nwspace;
	if (!update)
	{
		m_verification = new QPushButton(this);
	}
	m_verification->move(nleft, ntop);
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
	m_verification->setStyleSheet(qssbuttonblue);
	m_verification->setText(q_verification);

	m_verification->adjustSize();
	if (istatus == CONFIRMED)
	{
		m_verification->setEnabled(true);
	}
	else
	{
		m_verification->setEnabled(false);
	}

	if (!update)
	{
		m_delete = new QPushButton(this);
	}
	m_delete->move(nleft, 0);
	QString qssbuttonPink = "QPushButton {\
				   color: white;\
				   }\
				   QPushButton:enabled {\
				   background: rgb(255,182,193);\
				   color: white;\
				   }\
				   QPushButton:!enabled {\
				   background: gray;\color: rgb(200, 200, 200);\
				   }\
				   QPushButton:enabled:hover {\
				   background: rgb(	255,192,203);\
				   }\
				   QPushButton:enabled:pressed {\
				   background: rgb(	219,112,147);\
				   }";
	m_delete->setStyleSheet(qssbuttonPink);
	m_delete->setText(q_delete);
	m_delete->adjustSize();

	nleft += m_verification->width() + nwspace;
	if (!update)
	{
		m_details = new QPushButton(this);
	}
	m_details->move(nleft, ntop);
	QString qssbuttongreen = "QPushButton {\
				   color: white;\
				   }\
				   QPushButton:enabled {\
				   background: rgb(50,205,50);\
				   color: white;\
				   }\
				   QPushButton:!enabled {\
				   background: gray;\color: rgb(200, 200, 200);\
				   }\
				   QPushButton:enabled:hover {\
				   background: rgb(	0,255,0);\
				   }\
				   QPushButton:enabled:pressed {\
				   background: rgb(	60,179,113);\
				   }";
	m_details->setStyleSheet(qssbuttongreen);
	m_details->setText(q_details);
	m_details->adjustSize();

	if (!update)
	{
		m_retry = new QPushButton(this);
	}
	m_retry->move(nleft, 0);
	m_retry->setStyleSheet(qssbuttonPink);
	m_retry->setText(q_retry);
	m_retry->adjustSize();

	if (istatus == REJECTED)
	{
		m_delete->setEnabled(true);
		m_retry->setEnabled(true);
	}
	else
	{
		m_delete->setEnabled(false);
		m_retry->setEnabled(false);
	}

	connect(m_verification, &QPushButton::clicked, this, &history_item::onVerification);
	connect(m_details, &QPushButton::clicked, this, &history_item::onDetails);
	connect(m_delete, &QPushButton::clicked, this, &history_item::onDelete);
	connect(m_retry, &QPushButton::clicked, this, &history_item::onRetry);

	if (nleft + m_details->width() > m_width)
	{
		m_width = nleft + m_details->width();
	}

}

QString history_item::getstatus(int status)
{
	QString qs = "";

	switch (status)
	{
	case RECV:
		qs = tr("RECV");
		break;

	case SEND:
		qs = tr("SEND");
		break;

	case STOP:
		qs = tr("STOP");
		break;

	case CONFIRMING:
		qs = tr("CONFIRMING");
		break;
	case CONFIRMED:
		qs = tr("CONFIRMED");
		break;

	case REJECTED:
		qs = tr("REJECTED");
		break;

	default:
		qs = tr("DEFAULT_STATE");
		break;
	}
	return qs;

}

QString history_item::getfilesize(uint64 nsize)
{
	double dsize = nsize;
	QString unit = " B";
	if (dsize > 1024)
	{
		dsize /= 1024;
		unit = " KB";
		if (dsize > 1024)
		{
			dsize /= 1024;
			unit = " MB";
			if (dsize > 1024)
			{
				dsize /= 1024;
				unit = " GB";
			}
		}
	}
	QString textSize = QString::number(dsize, 'f', 1);
	textSize.append(unit);
	return textSize;
}

void history_item::onVerification()
{
	QVariantMap v;
	int ntop = 0;
	int nleft = 0;
	convertEvidenceStruct2VariantMap(v, &m_evis);
	g_mainWindow()->showVerificationWnd(v);

}

void history_item::onDetails()
{
	QSharedPointer<TEVIDENCEINFO> evi = QSharedPointer<TEVIDENCEINFO>(new TEVIDENCEINFO);
	evi->cCustomInfo = m_evis.cCustomInfo;
	evi->cFileHash = m_evis.cFileHash;
	evi->cFileName = m_evis.cFileName;
	evi->cRightOwner = m_evis.cRightOwner;
	evi->iFileSize = m_evis.iFileSize;
	evi->iFileState = m_evis.iFileState;
	evi->tRegisTime = m_evis.tRegisTime;
	g_mainWindow()->onAttestationRecord(evi);
}

void history_item::onDelete()
{
	g_mainWindow()->onDeleteHistoryItem(&m_evis);
}

void history_item::onRetry()
{
	m_evis.iFileState = CONFIRMING;
	InitTitle(false, true);
	QSharedPointer<TEVIDENCEINFO> evi = QSharedPointer<TEVIDENCEINFO>(new TEVIDENCEINFO);
	evi->cCustomInfo = m_evis.cCustomInfo;
	evi->cFileHash = m_evis.cFileHash;
	evi->cFileName = m_evis.cFileName;
	evi->cRightOwner = m_evis.cRightOwner;
	evi->iFileSize = m_evis.iFileSize;
	evi->iFileState = m_evis.iFileState;
	evi->tRegisTime = m_evis.tRegisTime;
	g_mainWindow()->BuddyRetry(evi);
}

void history_item::update(QSharedPointer<TEVIDENCEINFO> evi)
{
	m_evis = *evi;
	InitTitle(false, true);

}