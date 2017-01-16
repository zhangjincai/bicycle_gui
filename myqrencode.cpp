#include <QDebug>
#include "myqrencode.h"

MyQrenCode::MyQrenCode(QString str, QWidget *parent) :
    QWidget(parent)
{    
    setWindowFlags(Qt::SplashScreen);
    //setAutoFillBackground(true);

    qr_flag = false;
    qr = NULL;
    border = 5;    //二维码白色边框大小

    setString(str);

    //QPixmap pix(":/bicycle_qrcode.jpg");

    setWindowFlags(Qt::SubWindow); //窗口没有按钮,不然主界面响应不了按键


}

MyQrenCode::~MyQrenCode()
{
    if(qr != NULL)
    {
        qr_flag = false;
        QRcode_free(qr);
    }
}

void MyQrenCode::setBorder(int len)
{
    border = len;
}

int MyQrenCode::getBorder()
{
    return border;
}

void MyQrenCode::paintEvent(QPaintEvent *)
{
    if(qr_flag == false)
        return;
    QPainter painter(this);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);

    //this->setFixedSize(80,80); //设置窗体固定大小

    painter.drawRect(0, 0, width(), height());
    if(qr != NULL)
    {
        draw(painter, width(), height());
    }
}

void MyQrenCode::draw(QPainter &painter, int width, int height)
{
    if(qr_flag == false)
        return;
    QColor foreground(Qt::black);
    painter.setBrush(foreground);
    const int qr_width = qr->width > 0 ? qr->width : 1;
    double scale_x = (width-2*border) / qr_width;
    double scale_y = (height-2*border) / qr_width;
    for( int y = 0; y < qr_width; y ++)
    {
        for(int x = 0; x < qr_width; x++)
        {
            unsigned char b = qr->data[y * qr_width + x];
            if(b & 0x01)
            {
                QRectF r(x * scale_x + border, y * scale_y + border, scale_x, scale_y);
                painter.drawRects(&r, 1);
            }
        }
    }
#if 0
    label_img  = new QLabel(this);
    label_img->setGeometry(width/2-20,height/2-20,40,40);
    label_img->setPixmap(pix);

    label_img->show();
#endif
}

QSize MyQrenCode::sizeHint()  const
{
    QSize s;
    if(qr_flag == false)
        return s;
    if(qr != NULL)
    {
        int qr_width = qr->width > 0 ? qr->width : 1;
        s = QSize(qr_width*6 + 2*border, qr_width*6 + 2*border);
    }
    else
    {
        s = QSize(100, 100);
    }
    return s;
}

QSize MyQrenCode::minimumSizeHint()  const
{
    QSize s;
    if(qr_flag == false)
        return s;
    if(qr != NULL)
    {
        int qr_width = qr->width > 0 ? qr->width : 1;
        s = QSize(qr_width*6 + 2*border, qr_width*6 + 2*border);
    }
    else
    {
        s = QSize(100, 100);
    }
    return s;
}

int MyQrenCode::getQRWidth() const
{
    if(qr != NULL)
    {
        return qr->width;
    }
    else
    {
        return 0;
    }
}

void MyQrenCode::setString(QString str)
{
    string = str;
    if(qr != NULL)
    {
        QRcode_free(qr);
    }
    qr = QRcode_encodeString(string.toStdString().c_str(),
                             1,
                             QR_ECLEVEL_H,
                             QR_MODE_8,
                             1);

    qr_flag = true;
}

bool MyQrenCode::saveImage(QString fileName, int size)
{
    if(size != 0 && !fileName.isEmpty())
    {
        QImage image(size, size, QImage::Format_Mono);
        QPainter painter(&image);
        QColor background(Qt::white);
        painter.setBrush(background);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, size, size);
        if(qr != NULL)
        {
            draw(painter, size, size);
        }
        return image.save(fileName);
    }
    else
    {
        return false;
    }
}





