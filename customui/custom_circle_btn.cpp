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

#include "custom_circle_btn.h"

#include <QPainter>

custom_circle_btn::custom_circle_btn(QWidget *parent) : QWidget(parent)
{

}

void custom_circle_btn::mouseMoveEvent(QMouseEvent *event)
{
Q_UNUSED(event);
}

void custom_circle_btn::mousePressEvent(QMouseEvent *event)
{
Q_UNUSED(event);
}

void custom_circle_btn::mouseReleaseEvent(QMouseEvent *event)
{
Q_UNUSED(event);
}

void custom_circle_btn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QRegion r1(QRect(100, 100, 200, 80),   
                     QRegion::Ellipse);

    QRegion r2(QRect(100, 120, 90, 30));   
    QRegion r3 = r1.intersected(r2);       

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QColor(255, 0, 0));
    painter.setClipRegion(r3);

    painter.drawEllipse(QPoint(0,0), 5,5);
}
