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
#ifndef HISTORY_H
#define HISTORY_H

#include "HChainP2PManager/headers/inter_public.h"
#include "HChainP2PManager/interface/QtInterface.h"

#include <QVariantMap>
#include <QSharedPointer>
#include <QWidget>
#include <QSharedPointer>

namespace Ui {
	class history;
}

class history : public QWidget
{
	Q_OBJECT

public:
	explicit history(QWidget *parent);
	~history();

public:
	void onLoad();
	void retranslateUi();
	void addDatetoList(TEVIDENCEINFO evis);
	void insertDatetoList(TEVIDENCEINFO evis, int index = 0 );
	void updateList(QSharedPointer<TEVIDENCEINFO> evidence);
	void updateEvidence(std::string hash, QSharedPointer<TEVIDENCEINFO> evidence, int type);
	void onBack();
	void DeleteItem(TEVIDENCEINFO *evidence);

private:
	Ui::history *ui;
};

#endif
