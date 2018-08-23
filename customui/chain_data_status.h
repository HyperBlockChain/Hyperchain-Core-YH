/*copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or https://opensource.org/licenses/MIT.

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef CHAIN_DATA_STATUS_H
#define CHAIN_DATA_STATUS_H

#include "HChainP2PManager/headers/inter_public.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QVariantMap>

class blockinfo;

class chain_data_status : public QLabel
{
    Q_OBJECT
public:
    explicit chain_data_status(QWidget *parent = 0);


public:
    
    void setBlockNum(uint64 myFirstBlockNum, uint64 lastBlockNum){
        myFirstBlockNum_ = myFirstBlockNum;
        lastBlockNum_    = lastBlockNum;
    }

	void setBlocks(QList<QSharedPointer<THBLOCKDLGINFO> >& items){
		items_ = items;
	}

	void setBlocksNum(list<uint64>& list_ChainData)
	{
		items_BlockNum = list_ChainData;
	}
	


	void setblockinfo(blockinfo *pinfo){ mbinfo = pinfo; }
	void retranslateUi();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);



signals:
	void sigShowNodeInfo(QPoint gPoint, QSharedPointer<THBLOCKDLGINFO> pNodeInfo);
	void sigShowNodeDlgInfo(QPoint gPoint, uint64 blocknum);
    void sigHideNodeInfo();

public slots:

private:
    int calVLinePos(uint64 num);
	void initstring();

private:
	int labelLR_ = 2;
	int labelH_ = 50;
	int ncutlen = 20;
	blockinfo *mbinfo = nullptr;   
    uint64 myFirstBlockNum_ = 0;
    uint64 lastBlockNum_ = 0;

	QList<QSharedPointer<THBLOCKDLGINFO> > items_;

	QMap<int, QSharedPointer<THBLOCKDLGINFO> > mapPix_;


	list<uint64> items_BlockNum;
	QMap<int, uint64 > map_BlockNum;

	QString genesisblock;
	QString recentblocl;

};

#endif 
