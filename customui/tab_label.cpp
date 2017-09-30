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

#include "tab_label.h"
#include <QTextCodec>
#include <QFont>

QColor tab_label::NORMAL_COLOR = QColor(136, 136, 136);
QColor tab_label::SELECTED_COLOR = QColor(106, 176, 239);

tab_label::tab_label(QWidget *parent) : QLabel(parent)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    resetFont(NORMAL_COLOR);

    adjustSize();
}

void tab_label::resetFont(QColor color)
{
    QPalette pe;
    pe.setColor(QPalette::WindowText, color);
    setPalette(pe);

    QFont font(QString::fromLocal8Bit("微软雅黑"));
    font.setPointSize(12);
    setFont(font);
}

void tab_label::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void tab_label::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    _clicked = true;
}

void tab_label::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if(_clicked){
        _clicked = false;
        emit clicked();
    }
}
