#include "png_button.h"
#include <QDebug>
#include <QEvent>




PngButton::PngButton(QWidget *parent):QPushButton(parent)
{
    //setPic(":/set_a.png", ":/set_hover.png", ":/set_press.png");
//    setPic(":/saas/saas/menu_a.pn",
//             ":/saas/saas/menu_hover.pn",
//             ":/saas/saas/menu_press.pn");
}

PngButton::~PngButton()
{

}

void PngButton::setPic(QString normal, QString hover, QString press)
{
    QString style = QString("QPushButton{border-image: url(%1);}"
                            "QPushButton:hover{border-image: url(%2);}"
                            "QPushButton:pressed{border-image: url(%3);}").arg(normal).arg(hover).arg(press);

//    QString style = QString("QPushButton{image: url(%1);border:0px;}"
//                            "QPushButton:hover{image: url(%2);border:0px;}"
//                            "QPushButton:pressed{image: url(%3);border:0px;}").arg(normal).arg(hover).arg(press);

//    QString style = QString("QPushButton{background-image: url(%1); border: 0px;}"
//                            "QPushButton:hover{background-image: url(%2); border: 0px;}"
//                            "QPushButton:pressed{background-image: url(%3); border: 0px;}").arg(normal).arg(hover).arg(press);

    setStyleSheet(style);
}
