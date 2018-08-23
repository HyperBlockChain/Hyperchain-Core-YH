/*Copyright 2016-2018 hyperchain.net (Hyperchain)

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

#include "reg_verification_frameless.h"
#include "customui/base_frameless_wnd.h"
#include "reg_verification.h"
#include "util/commonutil.h"
#include "mainwindow.h"


#include <QVariantMap>

extern MainWindow* g_mainWindow();

reg_verification_frameless::reg_verification_frameless(QObject *parent) : QObject(parent)
{
    init();
}

void reg_verification_frameless::onClose()
{
    g_mainWindow()->verificationWndShow(false);
}

void reg_verification_frameless::updateDetail(QVariantMap generalInfo)
{
    if(dlg_.data() == nullptr){
        return;
    }

    verification_wnd_->updateInfo(generalInfo);

    show(true);
}

void reg_verification_frameless::show(bool bShow)
{
    if(bShow){

        dlg_->show();
        g_mainWindow()->verificationWndShow(true);
    }else{
        dlg_->hide();
    }
}

void reg_verification_frameless::retranslateUI()
{
    if(verification_wnd_){
        verification_wnd_->retranslateUI();
    }
}

void reg_verification_frameless::init()
{
    dlg_ = QSharedPointer<base_frameless_wnd>(new base_frameless_wnd());
    connect(dlg_.data(), &base_frameless_wnd::sigClose, this, &reg_verification_frameless::onClose);

    dlg_->setAcceptDrops(true);
    dlg_->showTitleBar(true);
    dlg_->setScale(false);
    dlg_->setGeometry(QRect(0,0,410,365));

    dlg_->setLogoPic(logoPath(), 20, QPoint(6,3), 1);
    dlg_->setMinMaxCloseButton(QSize(25, 25), true, closeBtnStyle());

    dlg_->hide();

    verification_wnd_ = new reg_verification(dlg_.data());
    dlg_->addWidget(verification_wnd_);
}
