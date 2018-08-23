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

#include "chain_data_status.h"

#include <QPainter>
#include <QtGlobal>
#include <QMouseEvent>
#include <QTime>
#include <QFontMetrics>
#include <QDebug>
#include "../wnd/blockinfo.h"


chain_data_status::chain_data_status(QWidget *parent) : QLabel(parent)
{
	initstring();
    setMouseTracking(true);	
}
void chain_data_status::initstring()
{
	genesisblock = tr("GENESIS BLOCK");
	recentblocl = tr("THE LATEST BLOCK");
}

void chain_data_status::mouseMoveEvent(QMouseEvent *event)
{	
    int x = event->pos().x();
    int y = event->pos().y();

    
	QMapIterator<int, uint64> item(map_BlockNum);



    while (item.hasNext()) {
        item.next();
#ifdef WIN32
        if(y > 0 && y < labelH_ && (x >= item.key() - 2 && x <= item.key() + 2)){
#else
        if(y > 0 && y < labelH_ && (x >= item.key() - 4 && x <= item.key() + 4)){
#endif

			int x = this->width() + 20;
			int y = 0;
			QPoint p(x, y);			
			QPoint gp = mapToGlobal(p);
			
			emit sigShowNodeDlgInfo(gp, item.value());

            event->accept();
            return;
        }
    }


}

void chain_data_status::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void chain_data_status::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void chain_data_status::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    static QFontMetrics fm = painter.fontMetrics();

	painter.drawRect(QRect(labelLR_, 0, width() - labelLR_ * 2 - ncutlen, labelH_));
	map_BlockNum.clear();


    int index = 0;
	for (auto item : items_BlockNum)
	{
		int pos = calVLinePos(item);
	
        painter.drawLine(QPoint(labelLR_ + pos, 1), QPoint(labelLR_ + pos, labelH_-1));

        if(index == 0){
			painter.setPen(QColor(0, 160, 230));
			painter.drawText(QRect(labelLR_ + 1, labelH_ + 5, 20, 15), QStringLiteral("0"));
			painter.drawText(QRect(labelLR_ + 1, labelH_ + 22, 80, 15), genesisblock);
        }
		 
		if (item == myFirstBlockNum_)		
		{
            QString numStr = QString("%1").arg(myFirstBlockNum_);
            int w = fm.width(numStr);

			painter.drawText(QRect((width() - w - labelLR_ - ncutlen), labelH_ + 5, w, 15), QString("%1").arg(myFirstBlockNum_));

			w = fm.width(recentblocl);

			painter.drawText(QRect((width() - w - labelLR_ - ncutlen), labelH_ + 22, w, 15), recentblocl);
        }

		map_BlockNum.insert(labelLR_ + pos, item);
        index++;
    }
}

void chain_data_status::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
}

int chain_data_status::calVLinePos(uint64 num)
{
	int w = this->width() - labelLR_ * 2 - ncutlen;
    int pos = (int)((num * 1.0) / lastBlockNum_ * w);
    return pos;
}

void chain_data_status::retranslateUi()
{
	initstring();
}