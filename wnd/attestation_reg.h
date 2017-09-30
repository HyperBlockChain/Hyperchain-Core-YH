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

#ifndef ATTESTATION_REG_H
#define ATTESTATION_REG_H

#include "common/hcdef.h"

#include <QWidget>
#include <QVector>
#include <QJsonObject>

namespace Ui {
class attestation_reg;
}

class attestation_reg : public QWidget
{
    Q_OBJECT

public:
    explicit attestation_reg(QWidget *parent = 0);
    ~attestation_reg();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:
    void setConnection();
    void onCommit();
    void onHistory();
    void onReset();
    void onPublicAdd();
    void onPublicClear();

private:
    QString formatPublicInfo();
    int getAdded(QString key);
    QString convertPublicInfo2Json();

public:
    Ui::attestation_reg *ui;

    QString fpath_;
    QString sha512_;

    QVector<PublicInfo> publicInfo_;
};

#endif // ATTESTATION_REG_H
