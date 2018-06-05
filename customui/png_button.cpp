#include "png_button.h"
#include <QDebug>
#include <QEvent>

PngButton::PngButton(QWidget *parent):QPushButton(parent)
{

}

PngButton::~PngButton()
{

}

void PngButton::setPic(QString normal, QString hover, QString press)
{
    QString style = QString("QPushButton{border-image: url(%1);}"
                            "QPushButton:hover{border-image: url(%2);}"
                            "QPushButton:pressed{border-image: url(%3);}").arg(normal).arg(hover).arg(press);

    setStyleSheet(style);
}
