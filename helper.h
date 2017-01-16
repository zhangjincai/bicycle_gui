#ifndef __HELPER_H__
#define __HELPER_H__

#include <QObject>
#include <QListWidget>
#include <QSettings>
#include <QWidget>
#include <QDesktopWidget>
#include <QFile>
#include <QFileIconProvider>
#include <QTextCodec>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QTime>
#include <QDateTime>
#include <QDate>
#include <QCoreApplication>
#include <QFileDialog>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QCursor>
#include <QTimer>
#include <QApplication>
#include <QStyleFactory>
#include <QTextStream>
#include <QDebug>



/* ��־���� */
#define UI_LOG_RUN
#define TERMINAL_NO_PATH				"/opt/config/terminal_no_file.txt"

#define LNT_FIRMWARE_PATH				"/opt/config/lnt_firmware_version_file.txt"


#ifdef UI_LOG_RUN
#include <syslog.h>
#define SYS_LOG_EMERG(fmt, args...) 		syslog(LOG_EMERG, fmt, ##args)
#define SYS_LOG_ALERT(fmt, args...) 			syslog(LOG_ALERT, fmt, ##args)
#define SYS_LOG_CRIT(fmt, args...) 			syslog(LOG_CRIT, fmt, ##args)
#define SYS_LOG_ERR(fmt, args...) 			syslog(LOG_ERR, fmt, ##args)
#define SYS_LOG_WARNING(fmt, args...) 		syslog(LOG_WARNING, fmt, ##args) 
#define SYS_LOG_NOTICE(fmt, args...)  		syslog(LOG_NOTICE, fmt, ##args)
#define SYS_LOG_INFO(fmt, args...) 			syslog(LOG_INFO, fmt, ##args)
#define SYS_LOG_DEBUG(fmt, args...) 		syslog(LOG_DEBUG, fmt, ##args)
#else
#define SYS_LOG_EMERG(fmt, args...) 		fprintf(stderr, fmt, ##args)
#define SYS_LOG_ALERT(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_CRIT(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_ERR(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_WARNING(fmt, args...) 		fprintf(stderr, fmt, ##args)
#define SYS_LOG_NOTICE(fmt, args...) 		fprintf(stderr, fmt, ##args)
#define SYS_LOG_INFO(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_DEBUG(fmt, args...) 		fprintf(stderr, fmt, ##args)
#endif





enum  DEV_CONFIG_RESULT
{
	 DEV_R_CONFIG_RESULT_OK = 0,	//����ȷ
	 DEV_R_CONFIG_RESULT_ERROR,    //������
	 DEV_R_CONFIG_RESULT_TIMEOUT,  //����ʱ
	DEV_R_SAE_STATUS_ERROR, //�Ա�

	 



	 DEV_W_CONFIG_RESULT_OK,    //д��ȷ
	 DEV_W_CONFIG_RESULT_ERROR,     //д����
	 DEV_W_CONFIG_RESULT_TIMEOUT,  //д��ʱ
	 DEV_W_CONFIG_RESULT_NO_CHANGE,   //д����û�иı�
	 DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS,  //�Ƿ�IP
	 DEV_W_CONFIG_RESULT_REFRESH,  //ˢ��
	 
};

/*
* λ����
*/
#define helper_array_idx(idx, bit)		((idx) / (bit))
#define helper_bit_idx(idx, bit)			((idx) % (bit))

#define helper_clr_bit_num(addr, idx, bit, type)  \
		*(addr + helper_array_idx(idx, bit)) &= (~(((type)0x01) << helper_bit_idx(idx, bit))) \

#define helper_set_bit_num(addr, idx, bit, type)  \
		*(addr + helper_array_idx(idx, bit)) |= (((type)0x01) << helper_bit_idx(idx, bit)) \

#define helper_chk_bit_num(addr, idx, bit, type)  \
		(*(addr + helper_array_idx(idx, bit)) >> helper_bit_idx(idx, bit)) & ((type)0x01) \

#define helper_clr_bit_byte(addr, idx) 		(helper_clr_bit_num((addr), (idx), 8, unsigned char))
#define helper_set_bit_bype(addr, idx) 		(helper_set_bit_num((addr), (idx), 8, unsigned char))
#define helper_chk_bit_bype(addr, idx) 		(helper_chk_bit_num((addr), (idx), 8, unsigned char))

#define helper_clr_bit(addr, idx)				helper_clr_bit_byte((addr), (idx)) 
#define helper_set_bit(addr, idx)				helper_set_bit_bype((addr), (idx)) 
#define helper_chk_bit(addr, idx)				helper_chk_bit_bype((addr), (idx)) 

#define helper_bit_to_byte(bit)				(((bit) + 7) / 8) 


class Helper : public QObject
{
	
public:

    	/* ���ñ���ΪGB2312 */
    	static void __SetGB2312Code(void)
    	{
        	QTextCodec *codec=QTextCodec::codecForName("GB2312");
        	QTextCodec::setCodecForLocale(codec);
        	QTextCodec::setCodecForCStrings(codec);
        	QTextCodec::setCodecForTr(codec);
    	}

    	/* ���ñ���ΪUTF8 */
    	static void __SetUTF8Code(void)
    	{
        	QTextCodec *codec=QTextCodec::codecForName("UTF-8");
        	QTextCodec::setCodecForLocale(codec);
        	QTextCodec::setCodecForCStrings(codec);
        	QTextCodec::setCodecForTr(codec);
    	}

	  /* ���ñ���ΪUTF8 */
    	static void __SetGBKCode(void)
    	{
        	QTextCodec *codec=QTextCodec::codecForName("GBK");
        	QTextCodec::setCodecForLocale(codec);
        	QTextCodec::setCodecForCStrings(codec);
        	QTextCodec::setCodecForTr(codec);
    	}


	/* utf8תGB2312 */
	static QString __Utf8ToGB2312(QString s_utf8)
	{
		return QString::fromUtf8(s_utf8.toLocal8Bit().data());
	}
		

    	/* �ж��Ƿ���IP��ַ */
    	static bool __IsIpAddress(QString IP)
    	{
        	QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
        	return RegExp.exactMatch(IP);
    	}
		
    	/* ��ʱ */
    	static void __Sleep(const unsigned int sec)
    	{
        	QTime dieTime = QTime::currentTime().addMSecs(sec);
        	while ( QTime::currentTime() < dieTime ) 
		{
            		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        	}
    	}

	/* char ת QString */
	static void __CharToQString(char *str, QString &string)
	{
		string = QString(QLatin1String(str));
	}

	/* QString ת char */
	static char * __QStringToChar(QString &string)
	{
		return string.toLatin1().data();
	}
	

	/* int ת QString */
	static void __IntToQString(int n, QString &string)
	{
		string.setNum(n);
	}
	
	/* unsigned int ת QString */
	static void __UintToQString(unsigned int n, QString &string)
	{
		string.setNum(n);
	}

	/* short ת QString */
	static void __ShortToQString(short n, QString &string)
	{
		string.setNum(n);
	}	

	/* unsigned short ת QString */
	static void __UshortToQString(unsigned short n, QString &string)
	{
		string.setNum(n);
	}	

	/* QString ת unsigned short */
	static unsigned short __QStringToUshort(QString &string)
	{
		return string.toUShort();
	}

	/* double ת QString */
	static void __DoubleToQString(double n, QString &string)
	{
                string.setNum(n);
	}

        /* double ת QString */
        static void __DoubleToQStringV2(double n, QString &string)
        {
                string.setNum(n,10,5); //
        }

        /* gps��γ�ȸ�ʽת�� */
        static int leftShiftNum(int n)
        {
            switch(n)
            {
                case 0: return 1;
                case 1: return 10;
                case 2: return 100;
                case 3: return 1000;
                case 4: return 10000;
                case 5: return 100000;
                case 6: return 1000000;
                default: return -1;
            }

            return -1;
        }

        static int gpsTransform(double db, int *degree, int *minute, float *second, QString &printGps)
        {
            if(db<=0)
                return -1;

            printGps.clear();

            QString str;
            str.setNum(db,'g',10);

            QStringList strList= str.split(".",QString::SkipEmptyParts);

            int mySplit = 100;
            if(mySplit<0)
                return -1;

            int leftShift = leftShiftNum(strList.at(1).length());
            if(leftShift<0)
                return -1;

            *degree = QString(strList.at(0)).toInt(0,10)/mySplit;
            *minute = QString(strList.at(0)).toInt(0,10)%mySplit;
            *second = QString(strList.at(1)).toFloat(0)*60/leftShift;

            printGps.sprintf("%d��%d\'%.4f\"", *degree,*minute,*second);

            return 0;
        }
};

class FormHelper : public QObject
{
	
public:
	
	 /* ���������ʾ */
	static void __FormInCenter(QWidget *frm)
	{
		int frmX = frm->width();
		int frmY = frm->height();
		
		QDesktopWidget w;
		 int deskWidth = w.width();
		 int deskHeight = w.height();
		 QPoint movePoint(deskWidth / 2 - frmX / 2, deskHeight / 2 - frmY / 2);
		 frm->move(movePoint);
	}

	/* ���ر����� */
	static void __FormHideTitleBar(QWidget *frm)
	{
		frm->setWindowFlags(Qt::FramelessWindowHint);	
	}

	/* ����û����󻯰�ť */
    	static void __FormNoMaxButton(QWidget *frm)
    	{
        	frm->setWindowFlags(Qt::WindowMinimizeButtonHint);
    	}


    	/* ����û����󻯺���С����ť */
    	static void __FormOnlyCloseButton(QWidget *frm)
    	{
        	frm->setWindowFlags(Qt::WindowMinMaxButtonsHint);
        	frm->setWindowFlags(Qt::WindowCloseButtonHint);
    	}

    	/* ���ô���͸���� */
    	static void __FormOpacity(QWidget *frm,double value)
    	{
        	frm->setWindowOpacity(value);
    	}

  	 /* ���岻�ܸı��С */
    	static void __FormNotResize(QWidget *frm)
    	{
        	frm->setFixedSize(frm->width(),frm->height());
    	}

   	/* ����ǰ����ʾ */
    	static void __FormOnTop(QWidget *frm)
    	{
        	frm->setWindowFlags(Qt::WindowStaysOnTopHint);
    	}

  	 /* ��������ʾ */
    	static void __FormOnBottom(QWidget *frm)
    	{
        	frm->setWindowFlags(Qt::WindowStaysOnBottomHint);
   	}

   	/* ��ȡ�����С */
    	static QSize __FormGetDesktopSize()
    	{
        	QDesktopWidget desktop;
        	return QSize(desktop.availableGeometry().width(),desktop.availableGeometry().height());
    	}

	/* ���ÿؼ���Ȫ���� */
	static void __FormSetFont_wqy( int pointSize,  int weight, QWidget *frm)
	{
		/*
		font.setFamily("wqy-zenhei");
		font.setPointSize(pointSize);
		font.setWeight(weight);
		*/

		QFont font("wqy-zenhei", pointSize, weight);
		frm->setFont(font);
	}




	
};


#endif

