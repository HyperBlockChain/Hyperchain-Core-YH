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
#include "browser.h"
#include "ui_browser.h"
#include "HChainP2PManager/interface/QtInterface.h"
#include <QRegExp>
#include <QDateTime>
#include <iostream>
#include <sstream>
#include <string>
#include "mainwindow.h"
#include "wnd/statusbar.h"
#include "common.h"


#ifdef WIN32
#include <codecvt>
#endif


#include <string>
#include <locale>
using namespace std;

extern MainWindow* g_mainWindow();

browser::browser(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::browser)
{
	ui->setupUi(this);
	_statusBar = new statusbar(this);
	ui->verticalLayout->addWidget(_statusBar);
	

	QRegExp rx("[0-9]+$");
	QRegExpValidator *validator = new QRegExpValidator(rx, this);
	ui->lineEditBlockNumber->setValidator(validator);

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
	ui->pushButton_Search->setStyleSheet(qssbuttonblue);
	
	connect(ui->pushButton_Search, &QPushButton::clicked, this, &browser::search);
	InitTableView();
}

browser::~browser()
{
	delete ui;
}

void browser::retranslateUi()
{
	ui->retranslateUi(this);
}

void browser::InitHead()
{
	if (block_model)
	{
		block_model->setColumnCount(9);
		block_model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Id"));
		block_model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Hid"));
		block_model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Chain Num"));
		block_model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Time"));
		block_model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Hash"));
		block_model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("HHash"));
		block_model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("Hash Prev"));
		block_model->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit("Type"));
		block_model->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit("PayLoad"));

		ui->tableViewBlock->setModel(block_model);
	}
}


void browser::InitTableView()
{
	block_model = new QStandardItemModel();
	InitHead();
	ui->tableViewBlock->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	ui->tableViewBlock->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableViewBlock->setEditTriggers(QAbstractItemView::NoEditTriggers);

}

const string& browser::tstringToUtf8(const utility::string_t &str)
{
#ifdef _UTF16_STRINGS
	wstring_convert<codecvt_utf8<wchar_t> > strCnv;
	return strCnv.to_bytes(str);
#else
	return str;
#endif
}

void browser::search()
{
	stringstream ss;
	ostringstream_t oss;
	uint64 blockNum;
	string_t tsNum;
	string strnum = ui->lineEditBlockNumber->text().toStdString();
	int num = 0;

	ss << strnum;
	ss >> blockNum;

	oss << blockNum;
	tsNum = oss.str();

	json::value  root = QueryByWeb(blockNum);
	num = root[tsNum].size();
	block_model->clear();
	InitHead();

	
	if (num > 0)
	{				
		string hash = "";
		string id = "";
		string type = "";
		string hid = "";
		string hhash = "";
		string hash_prev = "";
		string payload = "";
		uint64 utime = 0;
		QString qtime = "";
		string chain_num = "";

		for (int i = 0; i < num; i++)
		{
			hash = t2s(root[tsNum][i][_XPLATSTR("hash")].serialize());
			id = t2s(root[tsNum][i][_XPLATSTR("id")].serialize());
			type = t2s(root[tsNum][i][_XPLATSTR("type")].serialize());
			hid = t2s(root[tsNum][i][_XPLATSTR("hid")].serialize());
			hhash = t2s(root[tsNum][i][_XPLATSTR("hhash")].serialize());
			hash_prev = t2s(root[tsNum][i][_XPLATSTR("hash_prev")].serialize());
			payload = t2s(root[tsNum][i][_XPLATSTR("payload")].serialize());
			utime = root[tsNum][i][_XPLATSTR("ctime")].as_number().to_uint64();// as_integer();
			qtime = QDateTime::fromTime_t(utime).toString("yyyy-MM-dd hh:mm:ss");
			chain_num = t2s(root[tsNum][i][_XPLATSTR("chain_num")].serialize());

			block_model->setItem(i, 0, new QStandardItem(QString::fromLocal8Bit(id.c_str())));
			block_model->setItem(i, 1, new QStandardItem(QString::fromLocal8Bit(hid.c_str())));
			block_model->setItem(i, 2, new QStandardItem(QString::fromLocal8Bit(chain_num.c_str())));
			block_model->setItem(i, 3, new QStandardItem(qtime));
			block_model->setItem(i, 4, new QStandardItem(QString::fromLocal8Bit(hash.c_str())));
			block_model->setItem(i, 5, new QStandardItem(QString::fromLocal8Bit(hhash.c_str())));
			block_model->setItem(i, 6, new QStandardItem(QString::fromLocal8Bit(hash_prev.c_str())));
			block_model->setItem(i, 7, new QStandardItem(QString::fromLocal8Bit(type.c_str())));
			block_model->setItem(i, 8, new QStandardItem(QString::fromLocal8Bit(payload.c_str())));
		}
		_statusBar->OutPutString(tr("Data returned successfully"));
	}
	else
	{		
		_statusBar->OutPutString(tr("Can not retrieve block information from local, try synchronizing with other nodes, please wait..."), "color: rgb(220,20,60)");
	}
}

void browser::showinfo(string blocknum)
{
	ui->lineEditBlockNumber->setText(QString::fromLocal8Bit(blocknum.c_str()));
	search();
}