#include <QtGui>
#include <QDebug>
#include "member_service_dlg.h"



int g_CardServiceFlag;  //�����������ʶ


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

        string.sprintf("��ʾ����ѡ��Ҫ����ķ�������, %02d����Զ�������һ������,��[�˳�]��ֱ�ӷ���", timerCount);
		
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

void MemberServiceDlg::slots_entryMemberOnCardServiceDlg() //���뿪������ѡ��ҳ��
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
		

        else if(e->key() == Qt::Key_1)   //����
	{
		g_CardServiceFlag = CARD_SERVICE_ON;
                emit signals_entryMemberOnCardServiceDlg();
	}

        else if(e->key() == Qt::Key_2)   //����
	{
		g_CardServiceFlag = CARD_SERVICE_OFF;
		emit signals_entryOffCardServiceDlg();
	}

	
        else if(e->key() == Qt::Key_3)   //��ʱ����
	{
		emit signals_delayReturnBicycleDlg();
	}

        else if(e->key() == Qt::Key_4)   //��ֵ	//2016-04-12 by zjc
        {
                g_CardServiceFlag = CARD_SERVICE_RECHARGE;
                emit signals_entryRechargeServiceDlg();
        }

	timerCount = BINFO_RETURN_PERIOD;
		
	return;
}

