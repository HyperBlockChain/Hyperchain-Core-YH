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

#include "singleapp.h"
#include <QtNetwork/QLocalSocket>
#include <QFileInfo>

#define TIME_OUT                (500)   


SingleApplication::SingleApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , w(NULL)
    , _isRunning(false)
    , _localServer(NULL)
{
    _serverName = QString("com-hypercore-qt");

    _initLocalConnection();
}

bool SingleApplication::isRunning() {
    return _isRunning;
}

void SingleApplication::_newLocalConnection() {
    QLocalSocket *socket = _localServer->nextPendingConnection();

    if(socket) {
        socket->waitForReadyRead(2*TIME_OUT);
        delete socket;

        _activateWindow();
    }
}

void SingleApplication::_initLocalConnection()
{
    _isRunning = false;    

    QLocalSocket socket;
    socket.connectToServer(_serverName);
    if(socket.waitForConnected(TIME_OUT))
    {
        _isRunning = true;
    }
    else
    {
        _newLocalServer();
    }
}

void SingleApplication::_newLocalServer() {
    _localServer = new QLocalServer(this);

    connect(_localServer, SIGNAL(newConnection()), this, SLOT(_newLocalConnection()));

    if(!_localServer->listen(_serverName)) {
        if(_localServer->serverError() == QAbstractSocket::AddressInUseError) {

            QLocalServer::removeServer(_serverName);

            _localServer->listen(_serverName);
        }
    }
}

void SingleApplication::_activateWindow() {
    if (w) {
        w->show();
        w->showNormal();
        w->raise();
        w->activateWindow();
    }
}

