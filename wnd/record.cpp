/*Copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or?https://opensource.org/licenses/MIT.

Permission is hereby granted, free of charge, to any person obtaining a copy of this?
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,?
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "record.h"
#include "ui_record.h"
#include "mainwindow.h"
#include <QPushButton>
#include <QDateTime>

extern MainWindow* g_mainWindow();

record::record(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::record)
{
	ui->setupUi(this);
	connect(ui->pushButtonHistory, &QPushButton::clicked, this, &record::onHistory);
	connect(ui->pushButtonBack, &QPushButton::clicked, this, &record::onBack);


	QString qssbuttonblue = "QPushButton {\
				   color: white;\
				   }\
				   QPushButton:enabled {\
				   background: rgb(0, 165, 235);\
				   color: white;\
				   }\
				   QPushButton:!enabled {\
				   background: gray;\
				   color: rgb(200, 200, 200);\
				   }\
				   QPushButton:enabled:hover {\
				   background: rgb(0, 180, 255);\
				   }\
				   QPushButton:enabled:pressed {\
				   background: rgb(0, 140, 215);\
				   }";
	ui->pushButtonHistory->setStyleSheet(qssbuttonblue);
	ui->pushButtonBack->setStyleSheet(qssbuttonblue);
	
}

record::~record()
{
	delete ui;
}

void record::retranslateUi()
{
	ui->retranslateUi(this);
}

void record::setEvidence(QSharedPointer<TEVIDENCEINFO> evidence)
{
	QString filesize = getfilesize(evidence->iFileSize);
	QString qtime = QDateTime::fromTime_t(evidence->tRegisTime).toString("yyyy-MM-dd hh:mm:ss");


	ui->labelFileName->setText((QString::fromLocal8Bit(evidence->cFileName.c_str())));	
	ui->labelFileSize->setText(filesize);
	ui->textEditOwer->setText((QString::fromLocal8Bit(evidence->cRightOwner.c_str())));
	ui->labelTime->setText(qtime);
	ui->textEditHash->setText((QString::fromLocal8Bit(evidence->cFileHash.c_str())));
	ui->textEditInfo->setText((QString::fromLocal8Bit(evidence->cCustomInfo.c_str())));
	
}

void record::onHistory()
{
	g_mainWindow()->onAttestationHistory();
}


void record::onBack()
{
	g_mainWindow()->GoBack();
}



QString record::getfilesize(uint64 nsize)
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