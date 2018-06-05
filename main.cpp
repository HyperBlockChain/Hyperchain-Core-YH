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

#include "util/commonutil.h"

#include "customui/base_frameless_wnd.h"

#include "wnd/attestation_reg.h"
#include "wnd/dev_mode.h"
#include "singleapp.h"
#include "wnd/tab_bar_wnd.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QTranslator>
#include <thread>

#include "mainwindow.h"
#include "HChainP2PManager/interface/QtInterface.h"

#include "HChainP2PManager/headers/includeComm.h"
#include "db/RestApi.h"
#include "util/cppsqlite3.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    SingleApplication a(argc, argv);
    if(a.isRunning()){
        return 0;
    }

    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9005");

	bool bShow = true;

	switch (argc)
	{
	case 1:
		break;
	case 2:
	{
		if ( 0 == strcmp(argv[1], "-hide"))
		{
			bShow = false;
		}
		break;
	}
	default:
		break;
	}

	if (bShow)
	{
		MainWindow mainWnd;

		a.w = &mainWnd;

		mainWnd.show();

		int w = QApplication::desktop()->width();

		mainWnd.activateWindow();

		setNotify(mainWnd.GetQtNotify());

		getHyperBlockFromLocal();

		GetHyperBlockNumInfoFromLocal();
		runP2P(argc, argv);
		RestApi::startRest();

		return a.exec();
	}
	else
	{
		runP2P(argc, argv);
		while (1)
		{
			SLEEP(5*1000);
		}

		return 0;
	}

}
