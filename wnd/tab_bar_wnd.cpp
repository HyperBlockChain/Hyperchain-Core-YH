/*Copyright 2017 hyperchain.net  (Hyper Block Chain)
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

#include "tab_bar_wnd.h"
#include "ui_tab_bar_wnd.h"
#include "customui/base_frameless_wnd.h"

#include "wnd/mine_hyperchain.h"
#include "wnd/chain_browser.h"
#include "wnd/attestation_reg.h"
#include "wnd/attestation_record.h"
#include "wnd/attestation_history.h"
#include "wnd/node_set.h"
#include "wnd/dev_mode.h"

tab_bar_wnd::tab_bar_wnd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tab_bar_wnd)
{
    ui->setupUi(this);
}

tab_bar_wnd::~tab_bar_wnd()
{
    delete ui;
}

void tab_bar_wnd::setConnect()
{

}

void tab_bar_wnd::createChildWnd(base_frameless_wnd *flWnd)
{

}

void tab_bar_wnd::onHyperChainClick()
{
}

void tab_bar_wnd::onChainBrowerClick()
{
}

void tab_bar_wnd::onChainAttestationClick()
{
}

void tab_bar_wnd::onNodeSetClick()
{
}

void tab_bar_wnd::onDevModeClick()
{
}

void tab_bar_wnd::tabClick(E_TAB_TYPE type)
{

}

void tab_bar_wnd::showHyperChain()
{
}

void tab_bar_wnd::showChainBrower()
{
}

void tab_bar_wnd::showAttestation()
{
}

void tab_bar_wnd::showNodeSet()
{
}

void tab_bar_wnd::showDevMode()
{
}

void tab_bar_wnd::hideWnd()
{

}

void tab_bar_wnd::resizeChild()
{

}


