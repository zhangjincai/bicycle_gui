#include "qrcode.h"
#include "ui_qrcode.h"


#include <QPushButton>
#include <QVBoxLayout>

QrCode::QrCode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QrCode)
{
    setWindowFlags(Qt::CustomizeWindowHint);

    ui->setupUi(this);

    myQrcode = new MyQrenCode("43693",this);

    //myQrcode->setGeometry(50, 310, 140, 140);
}

QrCode::~QrCode()
{
    delete myQrcode;
    if(myQrcode!=NULL)
    {
        myQrcode->qr_flag = true;
        qDebug()<<"myQrcode->qr_flag = true";
    }
}

void QrCode::paintEvent(QPaintEvent *)
{
#if 0
    myQrcode->resize(140, 140);
    qDebug()<<this->width()<<myQrcode->width();

#endif
    myQrcode->move(50, 310);
    myQrcode->show();
    qDebug()<<this->width()<<myQrcode->width();
}


