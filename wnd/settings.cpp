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

#include "settings.h"
#include "ui_settings.h"
#include "HChainP2PManager/interface/QtInterface.h"



settings::settings(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::settings)
{
	ui->setupUi(this);
	InitTableView();
}

settings::~settings()
{
	delete ui;
}

void settings::retranslateUi()
{
	ui->retranslateUi(this);
}

void settings::Update_NodeInfo(string info, string ip, uint16 port)
{
	ip = ip + ":" + to_string(port);
	ui->labelname->setText(QString::fromLocal8Bit(ip.c_str()));
	ui->labelip->setText(QString::fromLocal8Bit(ip.c_str()));
}

void settings::Update_ServerInfo(VEC_T_PPEERCONF info)
{
	string name, port;
	char ip[24] = { 0 };
	struct in_addr addPeerIP;	
	
	for (int i = 0; i < info.size(); i++)
	{		
		memset(ip, 0, sizeof(ip));
		name = info[i]->strName;		
		port = to_string(info[i]->tPeerAddrOut.uiPort);
		
		addPeerIP.s_addr = info[i]->tPeerAddrOut.uiIP;
		strcpy(ip, inet_ntoa(addPeerIP));

		node_model->setItem(i, 0, new QStandardItem(QString::fromLocal8Bit(name.c_str())));
		node_model->setItem(i, 1, new QStandardItem(QString::fromLocal8Bit(ip)));
		node_model->setItem(i, 2, new QStandardItem(QString::fromLocal8Bit(port.c_str())));
	
	}

}
void settings::InitTableView()
{
	node_model = new QStandardItemModel();
	node_model->setColumnCount(3);
	node_model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Identity"));
	node_model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("IP"));
	node_model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Port"));
	ui->tableViewNode->setModel(node_model);

	ui->tableViewNode->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	ui->tableViewNode->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui->tableViewNode->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	ui->tableViewNode->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	ui->tableViewNode->setColumnWidth(0, 130);
	ui->tableViewNode->setColumnWidth(1, 130);
	ui->tableViewNode->setColumnWidth(2, 70);

	ui->tableViewNode->setEditTriggers(QAbstractItemView::NoEditTriggers);


}