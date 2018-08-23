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

#include "history.h"
#include "ui_history.h"

#include "HChainP2PManager/interface/QtInterface.h"
#include "mainwindow.h"
#include "util/commonutil.h"
#include "customui/history_item.h"
#include "db/dbmgr.h"

#include <QVariant>
#include <QDebug>
#include <QMessageBox>
#include<QListWidget>
#include <QListWidgetItem>

extern MainWindow* g_mainWindow();
static int BLOCK_NUM = 1;


history::history(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::history)
{
	ui->setupUi(this);
	
	connect(ui->pushButton_back, &QPushButton::clicked, this, &history::onBack);

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
	ui->pushButton_back->setStyleSheet(qssbuttonblue);
	onLoad();
}

history::~history()
{
	delete ui;
}

void history::retranslateUi()
{
	ui->retranslateUi(this);
}

void history::onLoad()
{
	QList<TEVIDENCEINFO> evis;
	DBmgr::instance()->getEvidences(evis, -1);
	for (auto item : evis)
	{
		addDatetoList(item);
	}
}

void history::updateEvidence(string hash, QSharedPointer<TEVIDENCEINFO> evidence, int type)
{	
	if (1 == type)
	{
		updateList(evidence);
	}
}

void history::addDatetoList(TEVIDENCEINFO evis)
{
	history_item *custom = new history_item(this, evis);
	QListWidgetItem *listItem = new QListWidgetItem();

	listItem->setSizeHint(QSize(custom->m_width, 70));
	ui->listWidget->addItem(listItem);
	ui->listWidget->setItemWidget(listItem, custom);
}

void history::insertDatetoList(TEVIDENCEINFO evis, int index)
{
	history_item *custom = new history_item(this, evis, true);
	QListWidgetItem *listItem = new QListWidgetItem();

	listItem->setSizeHint(QSize(custom->m_width, 70));
	ui->listWidget->insertItem(index, listItem);
	ui->listWidget->setItemWidget(listItem, custom);
}


void history::updateList(QSharedPointer<TEVIDENCEINFO> evidence)
{
	QListWidgetItem *listItem = nullptr;
	history_item *custom = nullptr;
	
	for (int i = 0; i < ui->listWidget->count(); i++)
	{
		listItem = ui->listWidget->item(i);
		custom = (history_item *)(ui->listWidget->itemWidget(listItem));
		if ((custom->m_evis.cFileHash.compare(evidence->cFileHash) == 0) && (custom->m_evis.tRegisTime == evidence->tRegisTime))
		{			
			custom->update(evidence);
			break;
		}

	}

	ui->listWidget->update();
}

void history::onBack()
{
	g_mainWindow()->GoBack();
}

void history::DeleteItem(TEVIDENCEINFO *evidence)
{
	QListWidgetItem *listItem = nullptr;
	history_item *custom = nullptr;

	for (int i = 0; i < ui->listWidget->count(); i++)
	{
		listItem = ui->listWidget->item(i);
		custom = (history_item *)(ui->listWidget->itemWidget(listItem));
		if ((custom->m_evis.cFileHash.compare(evidence->cFileHash) == 0) && (custom->m_evis.tRegisTime == evidence->tRegisTime))
		{
			ui->listWidget->removeItemWidget(listItem);
			delete listItem;
			break;
		}
	}

	ui->listWidget->update();
}
