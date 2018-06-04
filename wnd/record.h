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
#ifndef RECORD_H
#define RECORD_H

#include <QWidget>
#include <QSharedPointer>
#include "HChainP2PManager/headers/inter_public.h"
#include "HChainP2PManager/interface/QtInterface.h"

namespace Ui {
	class record;
}

class record : public QWidget
{
	Q_OBJECT

public:
	explicit record(QWidget *parent);
	~record();

public:
	void retranslateUi();
	void setEvidence(QSharedPointer<TEVIDENCEINFO> evidence);
	void onHistory();
	void onBack();
private:
	QString getfilesize(uint64 nsize);

private:
	Ui::record *ui;

};

#endif
