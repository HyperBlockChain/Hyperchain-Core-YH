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

#include "node_set_channel.h"

#include "customui/base_frameless_wnd.h"
#include "mainwindow.h"
#include "HChainP2PManager/interface/QtInterface.h"

#include <QMessageBox>
#include <QVariantMap>

extern MainWindow* g_mainWindow();


node_set_channel::node_set_channel(QObject *parent) : QObject(parent)
{

}

void node_set_channel::callQTFunc(QString param)
{
    QMessageBox::about(nullptr, QString("I am from QT"), param);
}

void node_set_channel::showFramelessWnd()
{
    base_frameless_wnd * p = new base_frameless_wnd(nullptr);
    p->setGeometry(QRect(300,300, 300, 300));
    p->show();
    p->update();
}

void node_set_channel::myAppOpen(bool open)
{
    g_mainWindow()->showAttestationReg(open);
}

QVariantMap node_set_channel::getLocalInfo()
{
    std::string info;
    std::string ip;
    uint16 port;
	GetNodeInfo(info, ip, port);

    QVariantMap v;
    v["info"] = QString::fromUtf8(info.c_str());
    v["ip"] = QString::fromUtf8(ip.c_str());
    v["port"] = port;

    return v;
}
