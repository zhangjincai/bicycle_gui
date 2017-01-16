#include <QtGui>
#include <QDebug>

#include "on_off_card_service_dlg.h"
#include "member_on_card_service_dlg.h"



extern int g_CardServiceFlag;  //开销卡服务标识
int memberTypeCheck;           //开卡会员类型标识

#define BINFO_RETURN_PERIOD					(10)


MemberOnCardServiceDlg::MemberOnCardServiceDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	


	timerCount = BINFO_RETURN_PERIOD;
	slots_timerDone();
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);
	
        connect(this, SIGNAL(signals_OnCardServiceQuit()), this, SLOT(accept()));
        connect(this, SIGNAL(signals_entryOnOffCardServiceDlg()), this, SLOT(slots_entryOnOffCardServiceDlg()));

}

MemberOnCardServiceDlg::~MemberOnCardServiceDlg(void)
{
	
}


void MemberOnCardServiceDlg::slots_timerDone(void)
{
	QString string;

        string.sprintf("提示：请选择要办理的会员类型, %02d秒后自动返回上一级界面,按[退出]键直接返回", timerCount);
		
	if(timerCount == 0)
		emit signals_OnCardServiceQuit();

        label_memberOnCardServiceShow->setText(string);
	timerCount--;
}


/*开卡会员类型选择*/
void MemberOnCardServiceDlg::slots_entryOnOffCardServiceDlg(void)
{
	if(timer.isActive())
	{
		timer.stop();
	}
		
        m_pOnOffCardServieDlg = new OnOffCardServiceDlg;
        m_pOnOffCardServieDlg->exec();

        if(m_pOnOffCardServieDlg != NULL)
	{
                delete m_pOnOffCardServieDlg;
                m_pOnOffCardServieDlg = NULL;
		g_CardServiceFlag = CARD_SERVICE_NOT;
	}		

	timerCount = BINFO_RETURN_PERIOD;
	timer.start(1000);
}




/*开卡会员类型的按键定义*/
void MemberOnCardServiceDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_OnCardServiceQuit();
		

        if(e->key() == Qt::Key_1)
	{
                memberTypeCheck = 0;   //对应票种标记

                g_CardServiceFlag = CARD_SERVICE_ON_NORMAL;
                emit signals_entryOnOffCardServiceDlg();

	}
        else if(e->key() == Qt::Key_2)
        {
                memberTypeCheck = 1;

                g_CardServiceFlag = CARD_SERVICE_ON_TIME;
                emit signals_entryOnOffCardServiceDlg();
        }
        else if(e->key() == Qt::Key_3)
        {
                memberTypeCheck = 2;

                g_CardServiceFlag = CARD_SERVICE_ON_DAY;
                emit signals_entryOnOffCardServiceDlg();
        }
        else if(e->key() == Qt::Key_4)
        {
                memberTypeCheck = 3;

                g_CardServiceFlag = CARD_SERVICE_ON_MONTH;
                emit signals_entryOnOffCardServiceDlg();
        }
        else if(e->key() == Qt::Key_5)
        {
                memberTypeCheck = 4;

                g_CardServiceFlag = CARD_SERVICE_ON_YEAR;
                emit signals_entryOnOffCardServiceDlg();
        }

	timerCount = BINFO_RETURN_PERIOD;

		
	return;
}

