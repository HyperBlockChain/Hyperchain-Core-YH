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
#ifndef HISTORY_ITEM_H
#define HISTORY_ITEM_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "HChainP2PManager/headers/inter_public.h"

class history_item : public QWidget
{
	Q_OBJECT

public:	
	history_item(QWidget *parent, TEVIDENCEINFO evis, bool insert = false, bool update = false);


public:	
	void update(QSharedPointer<TEVIDENCEINFO> evi);
	void retranslateUi();
	
public:
	TEVIDENCEINFO m_evis;
	int	m_width = 0;

private:
	void InitTitle(bool insert, bool update);
	QString getfilesize(uint64 nsize);
	QString getstatus(int status);
	void onDetails();
	void onVerification();

	void onDelete();
	void onRetry();

private:
	QLabel *m_time = nullptr;
	QLabel *m_filename = nullptr;
	QLabel *m_filesize = nullptr;
	QLabel *m_ower = nullptr;
	QLabel *m_status = nullptr;
	QLabel *m_blocknumber = nullptr;
	QPushButton *m_verification = nullptr;
	QPushButton *m_details = nullptr;
	QPushButton *m_delete = nullptr;
	QPushButton *m_retry = nullptr;

	QLabel *t_time = nullptr;
	QLabel *t_filename = nullptr;
	QLabel *t_filesize = nullptr;
	QLabel *t_ower = nullptr;
	QLabel *t_status = nullptr;
	QLabel *t_blocknumber = nullptr;
	

	QString q_time;
	QString q_filename;
	QString q_filesize;
	QString q_ower;
	QString q_status;
	QString q_blocknum;	
	QString	q_verification;
	QString q_details;	
	QString q_delete;
	QString q_retry;
};

#endif 
