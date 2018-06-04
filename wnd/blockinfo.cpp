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
#include "blockinfo.h"
#include "ui_blockinfo.h"
#include "util/commonutil.h"
#include <QDateTime>
#include <QDebug>

#include "HChainP2PManager/interface/QtInterface.h"
#include <iostream>
#include <sstream>
#include <string>

blockinfo::blockinfo(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::blockinfo)
{
	ui->setupUi(this);
	init();
}

blockinfo::~blockinfo()
{
	delete ui;
}

void blockinfo::retranslateUi()
{
	ui->retranslateUi(this);
}

void blockinfo::show(bool bShow)
{
	setHidden(!bShow);
}

void blockinfo::refreshNodeInfo(QSharedPointer<THBLOCKDLGINFO> pNodeInfo)
{
	QString blocknum = QString::number(pNodeInfo->iBlockNo, 10);
	QString time = QDateTime::fromTime_t(pNodeInfo->iCreatTime).toString("yyyy-MM-dd hh:mm:ss");
	QString localblocknum = QString::number(pNodeInfo->iLocalBlockNum, 10);
	QString hhsh = QString::fromLocal8Bit(pNodeInfo->strHHash.c_str());

	ui->labelBlockNum->setText(blocknum);
	ui->labelRegistTime->setText(time);
	ui->labelLocalblocknum->setText(localblocknum);
	ui->textEditHash->setText(hhsh);

}

string blockinfo::w2s(const wstring& ws)
{
	string curLocale = setlocale(LC_ALL, NULL);

	setlocale(LC_ALL, "chs");

	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	string result = _Dest;
	delete[]_Dest;

	setlocale(LC_ALL, curLocale.c_str());

	return result;
}

void blockinfo::refreshNodeDlgInfo(uint64 nblocknum)
{
	wostringstream oss;
	wstring wsNum;
	uint64 num = 0;

	oss << nblocknum;
	wsNum = oss.str();

	json::value  root = QueryByWeb(nblocknum);
	num = root[wsNum].size();
	if (num > 0)
	{
		QString blocknum = QString::number(nblocknum, 10);
		QString time = QDateTime::fromTime_t(root[wsNum][0][L"ctime"].as_number().to_uint64()).toString("yyyy-MM-dd hh:mm:ss");
		QString localblocknum = QString::number((num - 1), 10);
		QString hhsh = QString::fromLocal8Bit(w2s(root[wsNum][0][L"hhash"].to_string()).c_str());

		ui->labelBlockNum->setText(blocknum);
		ui->labelRegistTime->setText(time);
		ui->labelLocalblocknum->setText(localblocknum);
		ui->textEditHash->setText(hhsh);
	}

}

void blockinfo::leaveEvent(QEvent *event)
{
	if (!isHidden())
	{
		hide();

	}
	return QWidget::leaveEvent(event);
}

void blockinfo::init()
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	setWindowModality(Qt::NonModal);
	setStyleSheet("background-color: rgb(105,105,105);");
	setWindowOpacity(0.7);
	setMinimumSize(224, 250);
	hide();

	connect(ui->pushButtonCheck, &QPushButton::clicked, this, &blockinfo::onCheck);
}

void blockinfo::onCheck()
{
	hide();
	string strnum = ui->labelBlockNum->text().toStdString();
	emit sigShowBrowserInfo(strnum);
}