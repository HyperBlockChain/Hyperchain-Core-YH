/*copyright 2016-2018 hyperchain.net (Hyperchain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT。
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



#ifndef BASE_FRAMELESS_WND_H
#define BASE_FRAMELESS_WND_H

#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QSize>

class QVBoxLayout;
class QLabel;
class QFrame;
class QPushButton;

enum WndShowMode
{
    e_WndShow_Unknow = 0,
    e_WndShow_Show,
    e_WndShow_Hide

};

#define WndMargins 6
#define WndstretchMin 0
#define WndstretchMax 8

class base_frameless_wnd : public QWidget
{
    Q_OBJECT

public:
    base_frameless_wnd(QWidget* parent = nullptr, QString logoPath = QString(""), bool showTitleBar = true);

public:
    void addWidget(QWidget* widget);
    void showTitleBar(bool show);

    void setLogoPic(QString picPath, int size = 30, QPoint topLeft = QPoint(0,0), int scaleType = 1);

    void setMinMaxCloseButton(QSize btnSize=QSize(30,30),
                              bool showClose = true, QString closeBtnStyle = QString(""),
                              bool showMin = false, QString minBtnStyle=QString(""),
                              bool showMax = false, QString maxBtnStyle=QString(""));

    void setScale(bool scale);

private slots:
    void onClose();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);

    void focusOutEvent(QFocusEvent *event);
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent *event);

#ifdef Q_OS_LINUX
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
#endif

signals:
    void sigEnter(QEvent * event);
    void sigLeave(QEvent *event);
    void sigFocusOut(QFocusEvent *event);
    void sigClose();

private:
    void init();
    void resetBtnPos();

#ifdef WIN32
private:
    bool winEvent(const QByteArray &eventType, void *message, long *result);
    const QPixmap drawNinePatch(QSize szDst, const QPixmap &srcPix);
#endif

public:
    QFrame *title_;
    QFrame *content_;
#ifdef Q_OS_LINUX
    bool _drag = false;
    QPoint _dragPosition;
#endif

private:
    QPixmap m_pixShadow;

    bool ifScale_ = true;
    bool showTitleBar_ = true;

    QString logoPath_ = QString("");
    QPixmap logoPixmap_;
    QLabel  *logoLabel_ = nullptr;
    QPoint  logoTopLeft_ = QPoint(0,0);
    int     logoScalType_ = 1;
    int     logoSize_ = 0;

    QPushButton *minBtn_ = nullptr;
    QPushButton *maxBtn_ = nullptr;
    QPushButton *closeBtn_ = nullptr;
    QSize btnSize_ = QSize(0,0);

    QVBoxLayout *widgetLayout_ = nullptr;
    QVBoxLayout *contentLayout_ = nullptr;
};

#endif
