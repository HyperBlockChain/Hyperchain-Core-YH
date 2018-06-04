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
#include "token.h"
#include "ui_token.h"
#include "util/commonutil.h"
#include <QImage>
#include <QPixmap>
#include "wnd/statusbar.h"

token::token(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::token)
{
	ui->setupUi(this);
	_statusBar = new statusbar(this);
	ui->verticalLayout->addWidget(_statusBar);
	_statusBar->OutPutString(tr("This feature will be available soon!"), "color: rgb(220,20,60)");
	ShowPng();
}

token::~token()
{
	delete ui;
}

void token::retranslateUi()
{
	ui->retranslateUi(this);
}

void token::ShowPng()
{
	QPixmap pix;

	QString strTokenPath = TokenPath();
	QString strPassPath = PassPath();
	QString strScriptPath =ScriptPath();
	QString strContractPath =ContractPath();

	pix.load(strTokenPath);
	ui->label_token->setPixmap(pix);

	pix.load(strPassPath);
	ui->label_pass->setPixmap(pix);

	pix.load(strScriptPath);
	ui->label_script->setPixmap(pix);

	pix.load(strContractPath);
	ui->label_contract->setPixmap(pix);

}