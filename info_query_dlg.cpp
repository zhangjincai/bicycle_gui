#include <QtGui>
#include <QDebug>
#include "info_query_dlg.h"

#define BINFO_RETURN_PERIOD					(10)

InfoQueryDlg::InfoQueryDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	


	timerCount = BINFO_RETURN_PERIOD;
	slots_timerDone();
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);


	connect(this, SIGNAL(signals_infoQueryQuit()), this, SLOT(close()));  //�ر�info query dialog����
	connect(this, SIGNAL(signals_entryTicketInfoQryDlg()), this, SLOT(slots_entryTicketInfoQryDlg()));  //Ʊ����Ϣ��ѯ
	connect(this, SIGNAL(signals_entryMemberInfoQryDlg()), this, SLOT(slots_enrtyMemberInfoQry()));   //��Ա��Ϣ��ѯ
	connect(this, SIGNAL(signals_entryRentInfoQryDlg()), this, SLOT(slots_entryRentInfoQryDlg()));   //�⻹����¼��ѯ

	connect(this, SIGNAL(signals_entryNearbySiteInfoQryDlg()), this, SLOT(slots_entryNearbySiteInfoQryDlg()));   //����������Ϣ��ѯ
}

InfoQueryDlg::~InfoQueryDlg(void)
{
	
}

void InfoQueryDlg::slots_timerDone(void)
{
	QString string;

	string.sprintf("��ʾ��%02d����Զ�������һ������,��[�˳�]��ֱ�ӷ���", timerCount);
		
	if(timerCount == 0)
		emit signals_infoQueryQuit();

	label_infoQryShow->setText(string);
	timerCount--;
}

void InfoQueryDlg::slots_entryTicketInfoQryDlg(void)
{
	if(timer.isActive())
	{
		timer.stop();
	}
	
	m_pTicketInfoQryDlg = new TicketInfoQueryDlg;
	m_pTicketInfoQryDlg->exec();

	if(m_pTicketInfoQryDlg != NULL)
	{
		delete m_pTicketInfoQryDlg;
		m_pTicketInfoQryDlg = NULL;
	}

	timerCount = BINFO_RETURN_PERIOD;
	timer.start(1000);
}

void InfoQueryDlg::slots_enrtyMemberInfoQry(void)
{
	if(timer.isActive())
	{
		timer.stop();
	}

	m_pMemberInfoQryDlg = new MemberInfoQueryDlg;
	m_pMemberInfoQryDlg->exec();

	if(m_pMemberInfoQryDlg != NULL)
	{
		delete m_pMemberInfoQryDlg;
		m_pMemberInfoQryDlg = NULL;
	}
	
	timerCount = BINFO_RETURN_PERIOD;
	timer.start(1000);
}

void InfoQueryDlg::slots_entryRentInfoQryDlg(void)
{
	if(timer.isActive())
	{
		timer.stop();
	}

	m_pRentInfoQryDlg = new RentInfoQueryDlg;
	m_pRentInfoQryDlg->exec();

	if(m_pRentInfoQryDlg != NULL)
	{
		delete m_pRentInfoQryDlg;
		m_pRentInfoQryDlg = NULL;
	}
	
	timerCount = BINFO_RETURN_PERIOD;
	timer.start(1000);
}

#if 1
/* ����������Ϣ��ѯ add by zjt at 2016-11-04 */
void InfoQueryDlg::slots_entryNearbySiteInfoQryDlg(void)
{
	if(timer.isActive())
	{
		timer.stop();
	}

	m_pNearbySiteInfoQryDlg = new NearbySiteInfoQueryDlg;
	m_pNearbySiteInfoQryDlg->exec();

	if(m_pNearbySiteInfoQryDlg != NULL)
	{
		delete m_pNearbySiteInfoQryDlg;
		m_pNearbySiteInfoQryDlg = NULL;
	}
	
	timerCount = BINFO_RETURN_PERIOD;
	timer.start(1000);
}
#endif

void InfoQueryDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_infoQueryQuit();

	if(e->key() == Qt::Key_1)
		emit signals_entryTicketInfoQryDlg();

	if(e->key() == Qt::Key_2)
		emit signals_entryMemberInfoQryDlg();

	if(e->key() == Qt::Key_3)
		emit signals_entryRentInfoQryDlg();

	if(e->key() == Qt::Key_4)
		emit signals_entryNearbySiteInfoQryDlg();

	timerCount = BINFO_RETURN_PERIOD;
	
	return;
}










