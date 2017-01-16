#ifndef QRCODE_H
#define QRCODE_H

#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QDebug>
#include "myqrencode.h"

namespace Ui {
class QrCode;
}

class QrCode : public QWidget
{
    Q_OBJECT

public:
    explicit QrCode(QWidget *parent = 0);
    ~QrCode();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::QrCode *ui;

    QString string;
    MyQrenCode *myQrcode;
};

#endif // QRCODE_H
