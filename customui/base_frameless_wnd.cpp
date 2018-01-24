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

#include "base_frameless_wnd.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

#include <QPainter>
#include <QBitmap>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QFrame>
#include <QPoint>
#include <QLabel>
#include <QPushButton>
#include <QTextCodec>

const static unsigned char m_hexData[588] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
0x00, 0x00, 0x00, 0x1D, 0x00, 0x00, 0x00, 0x1D,
0x08, 0x06, 0x00, 0x00, 0x00, 0x56, 0x93, 0x67,
0x0F, 0x00, 0x00, 0x00, 0x01, 0x73, 0x52, 0x47,
0x42, 0x00, 0xAE, 0xCE, 0x1C, 0xE9, 0x00, 0x00,
0x00, 0x06, 0x62, 0x4B, 0x47, 0x44, 0x00, 0xFF,
0x00, 0xFF, 0x00, 0xFF, 0xA0, 0xBD, 0xA7, 0x93,
0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73,
0x00, 0x00, 0x0D, 0xD7, 0x00, 0x00, 0x0D, 0xD7,
0x01, 0x42, 0x28, 0x9B, 0x78, 0x00, 0x00, 0x00,
0x07, 0x74, 0x49, 0x4D, 0x45, 0x07, 0xDA, 0x03,
0x0A, 0x01, 0x3B, 0x39, 0x35, 0x19, 0x02, 0x5A,
0x00, 0x00, 0x00, 0x22, 0x74, 0x45, 0x58, 0x74,
0x43, 0x6F, 0x6D, 0x6D, 0x65, 0x6E, 0x74, 0x00,
0x43, 0x72, 0x65, 0x61, 0x74, 0x65, 0x64, 0x20,
0x77, 0x69, 0x74, 0x68, 0x20, 0x47, 0x49, 0x4D,
0x50, 0x20, 0x6F, 0x6E, 0x20, 0x61, 0x20, 0x4D,
0x61, 0x63, 0x87, 0xA8, 0x77, 0x43, 0x00, 0x00,
0x01, 0x9E, 0x49, 0x44, 0x41, 0x54, 0x48, 0xC7,
0xED, 0x97, 0xDD, 0x4E, 0xC2, 0x30, 0x18, 0x86,
0x9F, 0xD6, 0x2D, 0x28, 0x18, 0x34, 0x72, 0xE2,
0x9D, 0x79, 0x8D, 0xDC, 0x9C, 0x3F, 0x84, 0x09,
0x64, 0x6C, 0xF5, 0xE4, 0x1D, 0xF9, 0xA8, 0x1D,
0x76, 0x80, 0xEE, 0xC4, 0x25, 0x4D, 0xB7, 0xA6,
0xED, 0xD3, 0xF7, 0xEB, 0xCF, 0xDB, 0x39, 0xFA,
0x1F, 0xC7, 0x75, 0x9E, 0x90, 0xD3, 0xB1, 0x8B,
0xDE, 0xDD, 0x05, 0xB0, 0x90, 0x82, 0x17, 0x09,
0x98, 0x03, 0xBC, 0x01, 0xFA, 0x33, 0xA1, 0xAD,
0x01, 0xB7, 0x16, 0x5E, 0x24, 0x54, 0xDE, 0x28,
0x15, 0xE6, 0xDD, 0x9F, 0x01, 0x6C, 0x94, 0xF6,
0xE6, 0x3D, 0x58, 0xA5, 0xCE, 0x00, 0x4B, 0x60,
0x02, 0xDC, 0x2A, 0x2F, 0x55, 0x9E, 0x13, 0xEA,
0x4E, 0x59, 0x03, 0xD4, 0xC0, 0x0E, 0xD8, 0x2A,
0x47, 0xE5, 0x47, 0xE1, 0xF5, 0xEA, 0x7C, 0x02,
0xDC, 0x03, 0x73, 0xE5, 0x77, 0x02, 0xFB, 0x4C,
0x68, 0x2B, 0xE0, 0x06, 0x58, 0x03, 0xAB, 0x28,
0xDC, 0x4D, 0x11, 0xCD, 0x65, 0x21, 0x85, 0x0F,
0xC0, 0x02, 0x78, 0x12, 0x7C, 0x92, 0xA1, 0xD6,
0xAA, 0xDC, 0x09, 0xF6, 0xAA, 0xFA, 0x7B, 0x0D,
0xA4, 0xE9, 0x0B, 0xEF, 0x04, 0x98, 0x09, 0xF8,
0x0C, 0x3C, 0x02, 0x53, 0xD5, 0xCD, 0x81, 0xEE,
0x81, 0x4F, 0xB5, 0x41, 0xE1, 0x5D, 0xAB, 0xCC,
0x01, 0xAE, 0x48, 0x2C, 0xA2, 0x52, 0x21, 0x9D,
0x0B, 0xB8, 0x50, 0x98, 0xCB, 0x4C, 0x68, 0x2D,
0x08, 0x02, 0xBD, 0x45, 0xEB, 0xE2, 0x9B, 0x52,
0x1F, 0x2D, 0xA6, 0xA9, 0x54, 0xCF, 0x32, 0xE6,
0xD5, 0xCE, 0x67, 0xD0, 0x9C, 0xDA, 0x85, 0xE8,
0x53, 0x4A, 0xFB, 0xB6, 0x4D, 0xA9, 0xF4, 0x92,
0xB9, 0x5D, 0x96, 0xAA, 0x6F, 0xB7, 0xDC, 0xD1,
0x40, 0xFD, 0x89, 0x13, 0xC9, 0xAA, 0x1F, 0xB2,
0x4F, 0x7D, 0x74, 0xB8, 0x90, 0x03, 0x8D, 0x8F,
0xC0, 0xA1, 0x47, 0xE1, 0x8F, 0xED, 0x3C, 0x23,
0x3C, 0xFF, 0xD0, 0xD1, 0xA0, 0x21, 0x4A, 0x43,
0x7D, 0xB4, 0xB7, 0x9D, 0x3F, 0xE1, 0xF4, 0xD6,
0x0B, 0xDB, 0x81, 0xB6, 0xD6, 0x46, 0xD0, 0x23,
0x78, 0xD1, 0x03, 0xB5, 0x5E, 0x58, 0x2B, 0x2D,
0x07, 0x9C, 0x48, 0x75, 0xCA, 0x47, 0x63, 0xA8,
0x55, 0x65, 0xBD, 0xF0, 0x13, 0xA8, 0x04, 0x1A,
0x72, 0xF6, 0x56, 0x6A, 0xBB, 0x33, 0xEE, 0x72,
0x50, 0x5F, 0x24, 0x14, 0x76, 0x5E, 0xB8, 0x32,
0x4E, 0xB1, 0x39, 0xC3, 0x65, 0xDE, 0xD5, 0xC7,
0xC6, 0x80, 0x43, 0x4A, 0x69, 0xE7, 0x85, 0x95,
0xBC, 0xB0, 0x73, 0x8A, 0x4B, 0xFC, 0xB4, 0xD2,
0x77, 0x63, 0x95, 0x06, 0x75, 0xD4, 0x8D, 0x72,
0x0B, 0x7C, 0xE8, 0x7B, 0x6B, 0xAC, 0xE9, 0x92,
0x9B, 0xC3, 0x56, 0x7D, 0x87, 0x78, 0x21, 0xB5,
0x66, 0x94, 0xA8, 0x52, 0x75, 0xC5, 0x3B, 0x52,
0x37, 0xAF, 0xC9, 0x3B, 0xEE, 0x6F, 0xDF, 0x06,
0x83, 0xFB, 0xC3, 0x7B, 0xEF, 0x61, 0xCF, 0x8E,
0x72, 0xC3, 0x77, 0x63, 0xFC, 0xCB, 0x7C, 0x01,
0x3F, 0xD0, 0xC3, 0x34, 0xFA, 0xD4, 0x35, 0x7A,
0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44,
0xAE, 0x42, 0x60, 0x82 };

base_frameless_wnd::base_frameless_wnd(QWidget* parent, QString logoPath, bool showTitleBar) : QWidget(parent)
  ,logoPath_(logoPath)
  ,showTitleBar_(showTitleBar)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    QFont font(QString::fromLocal8Bit("微软雅黑"));
    font.setPointSize(9);
    setFont(font);

    init();
}

void base_frameless_wnd::init()
{
  
    this->setWindowFlags(Qt::FramelessWindowHint);

#ifdef WIN32
    m_pixShadow.loadFromData(m_hexData, 588);

    this->setAttribute(Qt::WA_TranslucentBackground, true);


    HWND hwnd = (HWND)this->winId();
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
#endif

    title_ = new QFrame(this);
    title_->setObjectName(QStringLiteral("title_"));
    title_->setAttribute(Qt::WA_TranslucentBackground, false);
    QSizePolicy titlePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titlePolicy.setHorizontalStretch(0);
    titlePolicy.setVerticalStretch(0);
    title_->setSizePolicy(titlePolicy);
    title_->setMinimumSize(QSize(0, 40));
    title_->setMaximumSize(QSize(16777215, 40));

    content_ = new QFrame(this);
    content_->setObjectName(QStringLiteral("content_"));
    QSizePolicy contentPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    contentPolicy.setHorizontalStretch(0);
    contentPolicy.setVerticalStretch(0);
    content_->setSizePolicy(contentPolicy);
    content_->setMinimumSize(QSize(0, 0));
    content_->setMaximumSize(QSize(16777215, 16777215));



    title_->setStyleSheet(QString("background-color: rgb(255, 255, 255);"));
    content_->setStyleSheet(QString("background-color: rgb(255, 255, 255);"));

    widgetLayout_ = new QVBoxLayout(this);
    widgetLayout_->setObjectName(QStringLiteral("verticalLayout"));
    widgetLayout_->setSpacing(0);
#ifdef WIN32
    widgetLayout_->setMargin(3);
#else
    widgetLayout_->setMargin(0);
#endif

    contentLayout_ = new QVBoxLayout(content_);
    contentLayout_->setObjectName(QStringLiteral("cverticalLayout"));
    contentLayout_->setSpacing(0);
    contentLayout_->setMargin(0);

    widgetLayout_->addWidget(title_);
    widgetLayout_->addWidget(content_);
}

void base_frameless_wnd::addWidget(QWidget *widget)
{
    contentLayout_->addWidget(widget);
}

void base_frameless_wnd::showTitleBar(bool show)
{
    showTitleBar_ = show;
    if(showTitleBar_){
        title_->show();
    }else{
        title_->hide();
    }
}

void base_frameless_wnd::setLogoPic(QString picPath, int size, QPoint topLeft, int scaleType)
{
    logoPath_ = picPath;
    logoTopLeft_ = topLeft;
    logoSize_ = size;
    logoScalType_ = scaleType;

    QPixmap pixmap;
    bool bl = pixmap.load(logoPath_);
    Q_ASSERT(bl);

    if(bl){
        if(1 == scaleType){
            logoPixmap_ = pixmap.scaledToWidth(size);
        }else{
            logoPixmap_ = pixmap.scaledToHeight(size);
        }

        if(logoLabel_){
            delete logoLabel_;
        }

        logoLabel_ = new QLabel(title_);
        logoLabel_->setGeometry(QRect(logoTopLeft_, logoPixmap_.size()));
        logoLabel_->setPixmap(logoPixmap_);
    }
}

void base_frameless_wnd::setMinMaxCloseButton(QSize btnSize,
                                              bool showClose, QString closeBtnStyle,
                                              bool showMin, QString minBtnStyle,
                                              bool showMax, QString maxBtnStyle)
{
    btnSize_ = btnSize;

    QRect tr = title_->geometry();

    if(showClose){
        if(!closeBtn_){
            closeBtn_ = new QPushButton(title_);
        }

        closeBtn_->setStyleSheet(closeBtnStyle);

        connect(closeBtn_, &QPushButton::clicked, this, &base_frameless_wnd::onClose);
    }

    if(showMin){
        if(!minBtn_){
            minBtn_ = new QPushButton(title_);
        }

        minBtn_->setStyleSheet(minBtnStyle);
    }

    if(showMax){
        if(!maxBtn_){
            maxBtn_ = new QPushButton(title_);
        }

        maxBtn_->setStyleSheet(maxBtnStyle);
    }
}

void base_frameless_wnd::setScale(bool scale)
{
    ifScale_ = scale;
}

void base_frameless_wnd::onClose()
{
    hide();

    emit sigClose();
}

void base_frameless_wnd::resizeEvent(QResizeEvent *event)
{
    resetBtnPos();

    QWidget::resizeEvent(event);
}

void base_frameless_wnd::focusOutEvent(QFocusEvent *event)
{
    emit sigFocusOut(event);
    QWidget::focusOutEvent(event);
}

void base_frameless_wnd::enterEvent(QEvent *event)
{
    emit sigEnter(event);
    QWidget::enterEvent(event);
}

void base_frameless_wnd::leaveEvent(QEvent *event)
{
    emit sigLeave(event);
    QWidget::leaveEvent(event);
}

#ifdef Q_OS_LINUX
    void base_frameless_wnd::mousePressEvent(QMouseEvent *event){
        if (event->button() == Qt::LeftButton) {
            QPoint Wndpoint = mapFromGlobal(event->globalPos());
            int xPos = Wndpoint.x();
            int yPos = Wndpoint.y();

            QRect title = title_->geometry();
            if(title.contains(xPos, yPos)){
                _drag = true;
                _dragPosition = event->globalPos() - this->pos();
                event->accept();
                return;
            }
        }

        QWidget::mousePressEvent(event);
    }

    void base_frameless_wnd::mouseMoveEvent(QMouseEvent *event){
        if (_drag && (event->buttons() && Qt::LeftButton)) {
                move(event->globalPos() - _dragPosition);
                event->accept();
                return;
        }

        QWidget::mouseMoveEvent(event);
    }

    void base_frameless_wnd::mouseReleaseEvent(QMouseEvent *event){
        _drag = false;
    }

#endif

void base_frameless_wnd::resetBtnPos()
{
    const int BTN_MARGIN = 2;
    const int BTN_TOP = 2;
    QRect tr = title_->geometry();

    if(closeBtn_){
        closeBtn_->setGeometry(QRect(tr.width() - (btnSize_.width() + 2), BTN_TOP, btnSize_.width(), btnSize_.height()));
    }

    if(maxBtn_){
       maxBtn_->setGeometry(QRect(tr.width() - (btnSize_.width()*2 + BTN_MARGIN), BTN_TOP, btnSize_.width(), btnSize_.height()));
    }

    if(minBtn_){
        if(maxBtn_){
            minBtn_->setGeometry(QRect(tr.width() - (btnSize_.width()*3 + BTN_MARGIN), BTN_TOP, btnSize_.width(), btnSize_.height()));
        }else{
            minBtn_->setGeometry(QRect(tr.width() - (btnSize_.width()*2 + BTN_MARGIN), BTN_TOP, btnSize_.width(), btnSize_.height()));
        }
    }
}


bool base_frameless_wnd::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef WIN32
    return winEvent(eventType, message, result);
#else
    return QWidget::nativeEvent(eventType, message, result);
#endif
}

#ifdef WIN32
bool base_frameless_wnd::winEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    MSG* msg = reinterpret_cast<MSG*>(message);

    switch (msg->message)
    {
    case WM_NCHITTEST:{
        QPoint Wndpoint = mapFromGlobal(QCursor::pos());
        int xPos = Wndpoint.x();
        int yPos = Wndpoint.y();

        if(closeBtn_){
            QRect rect = closeBtn_->geometry();
            if(rect.contains(xPos-6, yPos)){
                return false;
            }
        }

        if(minBtn_){
            QRect rect = minBtn_->geometry();
            if(rect.contains(xPos-6, yPos)){
                return false;
            }
        }

        if(maxBtn_){
            QRect rect = maxBtn_->geometry();
            if(rect.contains(xPos-6, yPos-10)){
                return false;
            }
        }

        QRect title = title_->geometry();
        if(title.contains(xPos, yPos)){
            *result = HTCAPTION;
            return true;
        }

        QRect content = content_->geometry();
        if(content.contains(xPos, yPos)){
            return false;
        }

        if (ifScale_)
        {
            if (xPos > WndstretchMin && xPos < WndstretchMax)
                *result = HTLEFT;
            if (xPos >(this->width() - WndMargins) && xPos < (this->width() - WndstretchMin))
                *result = HTRIGHT;
            if (yPos > WndstretchMin && yPos < WndstretchMax)
                *result = HTTOP;
            if (yPos >(this->height() - WndstretchMax) && yPos < (this->height() - WndstretchMin))
                *result = HTBOTTOM;
            if (xPos > WndstretchMin && xPos < WndstretchMax && yPos > WndstretchMin && yPos < WndstretchMax)
                *result = HTTOPLEFT;
            if (xPos >(this->width() - WndstretchMax) && xPos < (this->width() - WndstretchMin) && yPos > WndstretchMin && yPos < WndstretchMax)
                *result = HTTOPRIGHT;
            if (xPos > WndstretchMin && xPos < WndstretchMax && yPos >(this->height() - WndstretchMax) && yPos < (this->height() - WndstretchMin))
                *result = HTBOTTOMLEFT;
            if (xPos >(this->width() - WndstretchMax) && xPos < (this->width() - WndstretchMin) && yPos >(this->height() - WndstretchMax) && yPos < (this->height() - WndstretchMin))
                *result = HTBOTTOMRIGHT;

            return true;
        }
    }
    case WM_NCCALCSIZE:
            return true;

    case WM_GETMINMAXINFO:
    {
        if (::IsZoomed(msg->hwnd)) {
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            frame.left = abs(frame.left);
            frame.top = abs(frame.bottom);
            this->setContentsMargins(frame.left, frame.top, frame.right, frame.bottom);
        }
        else {
            this->setContentsMargins(2, 2, 2, 2);
        }

        *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);

        return true;
    }
    break;

    default:
        break;
    }

    return QWidget::nativeEvent(eventType, message, result);;
}
#endif

void base_frameless_wnd::paintEvent(QPaintEvent *event)
{
#ifdef WIN32
    QPainter painter(this);

    static QColor colItemBg(255, 255, 255); 

    QSize szThis = this->rect().size();
    QPixmap pixShadowBg = drawNinePatch(szThis, m_pixShadow);
    painter.drawPixmap(this->rect(), pixShadowBg);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.fillRect(pixShadowBg.rect(), QColor(0, 0, 0, 150));

    painter.setPen(colItemBg);   
    painter.setBrush(colItemBg);
    painter.drawRect(WndMargins, WndMargins, szThis.width() - WndMargins * 2, szThis.height() - WndMargins * 2);
    this->setMask(pixShadowBg.mask()); 
#endif
    QWidget::paintEvent(event);
}

#ifdef WIN32
const QPixmap base_frameless_wnd::drawNinePatch(QSize szDst, const QPixmap &srcPix)
{

    QPixmap dstPix(szDst);
    dstPix.fill(QColor(255, 255, 255, 0));
    QPainter painter;
    painter.begin(&dstPix);

    int nW = szDst.width();
    int nH = szDst.height();

    int nWBg = srcPix.width();
    int nHBg = srcPix.height();      
	QPoint m_ptBgLT(WndMargins, WndMargins);
    QPoint m_ptBgRB(nWBg - WndMargins, nHBg - WndMargins);

    QPoint ptDstLT(m_ptBgLT.x(), m_ptBgLT.y());
    QPoint ptDstRB(nW - (nWBg - m_ptBgRB.x()), nH - (nHBg - m_ptBgRB.y()));

    painter.drawPixmap(QRect(0, 0, ptDstLT.x(), ptDstLT.y()), srcPix, QRect(0, 0, m_ptBgLT.x(), m_ptBgLT.y()));
    painter.drawPixmap(QRect(ptDstLT.x(), 0, ptDstRB.x() - ptDstLT.x(), ptDstLT.y()), srcPix, QRect(m_ptBgLT.x(), 0, m_ptBgRB.x() - m_ptBgLT.x(), m_ptBgLT.y()));
    painter.drawPixmap(QRect(ptDstRB.x(), 0, nW - ptDstRB.x(), ptDstLT.y()), srcPix, QRect(m_ptBgRB.x(), 0, nWBg - m_ptBgRB.x(), m_ptBgLT.y()));
    painter.drawPixmap(QRect(0, ptDstLT.y(), ptDstLT.x(), ptDstRB.y() - ptDstLT.y()), srcPix, QRect(0, m_ptBgLT.y(), m_ptBgLT.x(), m_ptBgRB.y() - m_ptBgLT.y()));
    painter.drawPixmap(QRect(ptDstLT.x(), ptDstLT.y(), ptDstRB.x() - ptDstLT.x(), ptDstRB.y() - ptDstLT.y()), srcPix, QRect(m_ptBgLT.x(), m_ptBgLT.y(), m_ptBgRB.x() - m_ptBgLT.x(), m_ptBgRB.y() - m_ptBgLT.y()));
    painter.drawPixmap(QRect(ptDstRB.x(), ptDstLT.y(), nW - ptDstRB.x(), ptDstRB.y() - ptDstLT.y()), srcPix, QRect(m_ptBgRB.x(), m_ptBgLT.y(), nWBg - m_ptBgRB.x(), m_ptBgRB.y() - m_ptBgLT.y()));
    painter.drawPixmap(QRect(0, ptDstRB.y(), ptDstLT.x(), nH - ptDstRB.y()), srcPix, QRect(0, m_ptBgRB.y(), m_ptBgLT.x(), nHBg - m_ptBgRB.y()));
    painter.drawPixmap(QRect(ptDstLT.x(), ptDstRB.y(), ptDstRB.x() - ptDstLT.x(), nH - ptDstRB.y()), srcPix, QRect(m_ptBgLT.x(), m_ptBgRB.y()-1, m_ptBgRB.x() - m_ptBgLT.x(), nHBg - m_ptBgRB.y()));
    painter.drawPixmap(QRect(ptDstRB.x(), ptDstRB.y(), nW - ptDstRB.x(), nH - ptDstRB.y()), srcPix, QRect(m_ptBgRB.x(), m_ptBgRB.y(), nWBg - m_ptBgRB.x(), nHBg - m_ptBgRB.y()));

    painter.end();
    return dstPix;
}
#endif
