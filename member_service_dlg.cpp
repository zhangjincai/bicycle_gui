#include <QtGui>
#include <QDebug>
#include "member_service_dlg.h"



int g_CardServiceFlag;  //开销卡服务标识


#define BINFO_RETURN_PERIOD					(10)


MemberServiceDlg::MemberServiceDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	


	timerCount = BINFO_RETURN_PERIOD;
	slots_timerDone();
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);
	
	connect(this, SIGNAL(signals_OnCardServiceQuit()), this, SLOT(accept()));

        connect(this, SIGNAL(signals_entryMemberOnCardServiceDlg()), this, SLOT(slots_entryMemberOnCardServiceDlg()));

	connect(this, SIGNAL(signals_entryOffCardServiceDlg()), this, SLOT(slots_entryOnOffCardServiceDlg()));  
	connect(this, SIGNAL(signals_delayReturnBicycleDlg()), this, SLOT(slots_entrydelayReturnBicyleDlg()));  
        connect(this, SIGNAL(signals_entryRechargeServiceDlg()), this, SLOT(slots_entryRechargeServiceDlg()));
}

MemberServiceDlg::~MemberServiceDlg(void)
{
	
}


void MemberServiceDlg::slots_timerDone(void)
{
	QString string;

        string.sprintf("提示：请选择要办理的服务类型, %02d秒后自动返回上一级界面,按[退出]键直接返回", timerCount);
		
	if(timerCount == 0)
		emit signals_OnCardServiceQuit();

	label_memberServiceShow->setText(string);
	timerCount--;
}

void MemberServiceDlg::slots_entryOnOffCardServiceDlg(void)
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

void MemberServiceDlg::slots_entryMemberOnCardServiceDlg() //进入开卡类型选择页面
{
        if(timer.isActive())
        {
                timer.stop();
        }

        m_pMemberOnCardServiceDlg = new MemberOnCardServiceDlg;
        m_pMemberOnCardServiceDlg->exec();

        if(m_pMemberOnCardServiceDlg != NULL)
        {
                delete m_pMemberOnCardServiceDlg;
                m_pMemberOnCardServiceDlg = NULL;
                g_CardServiceFlag = CARD_SERVICE_NOT;
        }

        timerCount = BINFO_RETURN_PERIOD;
        timer.start(1000);
}

void MemberServiceDlg::slots_entrydelayReturnBicyleDlg(void)
{
	if(timer.isActive())
	{
		timer.stop();
	}
		
	m_pDelayReturnBicyleDlg = new DelayReturnBicycleDlg;
	m_pDelayReturnBicyleDlg->exec();

	if(m_pDelayReturnBicyleDlg != NULL)
	{
		delete m_pDelayReturnBicyleDlg;
		m_pDelayReturnBicyleDlg = NULL;
	}		

	timerCount = BINFO_RETURN_PERIOD;
	timer.start(1000);
}

//2016-04-12 add by zjc
void MemberServiceDlg::slots_entryRechargeServiceDlg(void)
{
        if(timer.isActive())
        {
                timer.stop();
        }

        m_pRechargeServieDlg = new RechargeServiceDlg;
        m_pRechargeServieDlg->exec();

        if(m_pRechargeServieDlg != NULL)
        {
                delete m_pRechargeServieDlg;
                m_pRechargeServieDlg = NULL;
                g_CardServiceFlag = CARD_SERVICE_NOT;
        }

        timerCount = BINFO_RETURN_PERIOD;
        timer.start(1000);
}

void MemberServiceDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_OnCardServiceQuit();
		

        else if(e->key() == Qt::Key_1)   //开卡
	{
		g_CardServiceFlag = CARD_SERVICE_ON;
                emit signals_entryMemberOnCardServiceDlg();
	}

        else if(e->key() == Qt::Key_2)   //销卡
	{
		g_CardServiceFlag = CARD_SERVICE_OFF;
		emit signals_entryOffCardServiceDlg();
	}

	
        else if(e->key() == Qt::Key_3)   //延时还车
	{
		emit signals_delayReturnBicycleDlg();
	}

        else if(e->key() == Qt::Key_4)   //充值	//2016-04-12 by zjc
        {
                g_CardServiceFlag = CARD_SERVICE_RECHARGE;
                emit signals_entryRechargeServiceDlg();
        }

	timerCount = BINFO_RETURN_PERIOD;
		
	return;
}

