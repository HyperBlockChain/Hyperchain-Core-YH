#ifndef HANDBUTTON_H
#define HANDBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QString>

class PngButton : public QPushButton
{
    Q_OBJECT
public:
    PngButton(QWidget *parent=NULL);
    ~PngButton();

public:
    void setPic(QString normal, QString hover, QString press);

protected:


private:


};

#endif // HANDBUTTON_H
