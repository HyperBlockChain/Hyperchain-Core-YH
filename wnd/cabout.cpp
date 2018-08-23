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
#include "cabout.h"
#include "ui_about.h"
#include "util/commonutil.h"
#include <QImage>
#include <QPixmap>


Cabout::Cabout(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::about)
{
	ui->setupUi(this);
	init();
}

Cabout::~Cabout()
{
	delete ui;
}

void Cabout::retranslateUi()
{	
	ui->retranslateUi(this);
	Hyperlink();
}

void Cabout::Hyperlink()
{
	ui->label_home->setOpenExternalLinks(true);
	ui->label_home->setText("<style> a {text-decoration: none} </style> <a style='color: rgb(6, 145, 205);' href=\"http://www.hyperchain.net/view/home_en.html\">http://www.hyperchain.net/view/home_en.html");
	ui->label_whitepaper->setOpenExternalLinks(true);
	ui->label_whitepaper->setText("<style> a {text-decoration: none} </style> <a style='color: rgb(6, 145, 205);' href=\"http://www.hyperchain.net/blog/archives/311\">http://www.hyperchain.net/blog/archives/311");
	ui->label_dev->setOpenExternalLinks(true);
	ui->label_dev->setText("<style> a {text-decoration: none} </style> <a style='color: rgb(6, 145, 205);' href=\"https://github.com/HyperBlockChain/HyperchainCore\">https://github.com/HyperBlockChain/HyperchainCore");
	ui->label_git->setOpenExternalLinks(true);
	ui->label_git->setText("<style> a {text-decoration: none} </style> <a style='color: rgb(6, 145, 205);' href=\"https://github.com/HyperBlockChain\">https://github.com/HyperBlockChain");
	ui->label_restapi->setOpenExternalLinks(true);
	ui->label_restapi->setText("<style> a {text-decoration: none} </style> <a style='color: rgb(6, 145, 205);' href=\"http://www.hyperchain.net\">restapi");
	ui->label_localrest->setOpenExternalLinks(true);

	QString HtmlPath = QCoreApplication::applicationDirPath();
	HtmlPath += "/index.html";
	HtmlPath = "file:///" + HtmlPath;

	QString strlocalrest = "<style> a {text-decoration: none} </style> <a style='color: rgb(6, 145, 205);' href=\"";
	strlocalrest += HtmlPath;
	strlocalrest += "\">http://127.0.0.1:8080/index.html";
	ui->label_localrest->setText(strlocalrest);
	
}

void Cabout::init()
{
	ShowPng();
	Hyperlink();
}

void Cabout::ShowPng()
{
	QPixmap pix;
	QString strAboutLogoPath = AboutLogoPath();
	pix.load(strAboutLogoPath);
	ui->label_logo->setPixmap(pix);
}
