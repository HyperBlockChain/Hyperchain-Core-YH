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
#ifndef PARTENER_CHAIN_H
#define PARTENER_CHAIN_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QString>
#include <QList>

#include "HChainP2PManager/headers/inter_public.h"
#include "hc_label.h"

struct PartnerNode{
    PartnerNode() = default;
    ~PartnerNode() = default;

public:
    quint64 index = 0;
	_ePoeReqState state = RECV;
    QString info;
};

class partener_chain : public QFrame
{
public:
    partener_chain(QWidget* parent);
	QList<hc_label*> m_hcLabel_;

public:

    void setNodeInfo(int chainNum);
    void setNodeInfo(VEC_T_NODEINFO& vec);

    void showNodes();
	void clearNodes();

private:
    void initNodes(int chainNum);
	void clear();

private:
    QList<PartnerNode> parentNodes_;

};

#endif