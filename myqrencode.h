#ifndef MYQRENCODE_H
#define MYQRENCODE_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QLabel>
#include "qrencode.h"

class MyQrenCode : public QWidget
{
    Q_OBJECT

public:
    MyQrenCode(QString str, QWidget *parent = 0);
    ~MyQrenCode();

    void setString(QString str);
    int getQRWidth() const;
    bool saveImage(QString name, int size);

    void setBorder(int len);
    int getBorder();

    QRcode *qr;
    bool qr_flag;


protected:
    void paintEvent(QPaintEvent *);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

private:
    QString string;
    int border;
    QLabel *label_img;
    QPixmap pix;

    void draw(QPainter &painter, int width, int height);
};

#endif // MYQRENCODE_H
