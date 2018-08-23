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

#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include "HChainP2PManager/headers/commonstruct.h"
#include <qstandarditemmodel.h>
#include <QTableView>
#include <QStatusBar>
#include <cpprest/details/basic_types.h>

class statusbar;

class browser;
namespace Ui {
	class browser;
}
class browser : public QWidget
{
	Q_OBJECT

public:
	explicit browser(QWidget *parent);
	~browser();

public:
	void retranslateUi();
	void search();	
	void showinfo(string blocknum);
	

private:
	void InitHead();
	void InitTableView();
	const string& tstringToUtf8(const utility::string_t &str);

private:
	Ui::browser *ui;
	QStandardItemModel  *block_model =nullptr;
	statusbar*			_statusBar = nullptr;
};

#endif // BROWSER_H
