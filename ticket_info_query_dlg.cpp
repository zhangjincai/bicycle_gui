#include <QtGui>
#include <QDebug>
#include "ticket_info_query_dlg.h"
#include "helper.h"
#include "lib_general.h"

#define BINFO_REFRESH_PERIOD					(25)
#define BINFO_RETURN_PERIOD						(20)

TicketInfoQueryDlg::TicketInfoQueryDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	


	m_ret = LIB_LNT_ERROR;
	m_stat = 0;
	timerFlag = 0;
	memset(&lnt_gry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));

	__allWidgetClear();
	
	timerCount = BINFO_REFRESH_PERIOD;
	slots_timerDone();
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);

	connect(this, SIGNAL(signals_manageAccept()), this, SLOT(accept()));
	connect(this, SIGNAL(signals_manageReject()), this, SLOT(reject()));
}


TicketInfoQueryDlg::~TicketInfoQueryDlg(void)
{
	
}

void TicketInfoQueryDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_ticketInfoQryShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_ticketInfoQryShow->setText(tr("��ʾ:Ʊ����Ϣ��ѯ�ɹ�"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_ERROR:
		{
			label_ticketInfoQryShow->setText(tr("��ʾ:Ʊ����Ϣ��ѯʧ��"));	
		}
		break;
	}	
}

void TicketInfoQueryDlg::slots_timerDone(void)
{
	static int ret = LNT_MOC_ERROR_CARD_OP_FAIL;
	QString ticketShow;

	if(timerFlag == 0)
	{
		ret = __getPageConfig();
		 if(ret == LNT_MOC_ERROR_CARD_NOTAG)
		{
			ticketShow.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ������", timerCount);
			label_ticketInfoQryShow->setText(ticketShow);	
		}

		if(timerCount == 0)
		{
			label_ticketInfoQryShow->setText(tr("��ʾ:Ʊ����Ϣ��ѯ��ʱ,�Զ�������һ������, ��[�˳�]��ֱ�ӷ���"));
			Helper::__Sleep(1);
			timerFlag = 1;
			emit signals_manageReject();
		}

		if(ret != LNT_MOC_ERROR_CARD_NOTAG)
		{
			timerCount = BINFO_RETURN_PERIOD;
			timerFlag = 1;
		}
	}

	if(timerFlag == 1)
	{
		if(ret == LNT_MOC_ERROR_CARD_NAMELIST)
		{
			ticketShow.sprintf("��ʾ:�ÿ�Ϊ��������,��֧��Ʊ����Ϣ��ѯ����,���˿�, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		}
		else if(ret == LNT_MOC_ERROR_NORMAL)
			ticketShow.sprintf("��ʾ:Ʊ����Ϣ��ѯ�ɹ�, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		else if(ret == LNT_MOC_ERROR_M1)
			ticketShow.sprintf("��ʾ:�ÿ�Ϊ��CPU��,��֧�ָ����Ϳ�����, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		else
			ticketShow.sprintf("��ʾ:Ʊ����Ϣ��ѯʧ��, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);

		label_ticketInfoQryShow->setText(ticketShow);

		if(timerCount == 0)
		{
			timerFlag = 2;
			emit signals_manageReject();
		}
	}

	timerCount--;
}

void TicketInfoQueryDlg::__allWidgetClear(void)
{
	label_ticketInfoQryShow->clear();
	label_blance->clear();
	label_lid->clear();
	label_ctim->clear();
//	label_pid->clear();
	label_deposit->clear();
}

int TicketInfoQueryDlg::__getPageConfig(void)
{
	int retval = LIB_LNT_ERROR;
	QString string;
	
	m_stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	memset(&lnt_gry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));
	
	m_ret = lib_lnt_qry_ticket_info(&lnt_gry_ticket, &m_stat, 300);
	if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_NORMAL))
	{
		unsigned int yuan, fen;

		/* ��� */
		yuan = lnt_gry_ticket.ticket.blance /100;
		fen = lnt_gry_ticket.ticket.blance % 100;
		
		string.sprintf("%02d.%02d(Ԫ)", yuan, fen);
		label_blance->setText(string);

		/* �߼����� */
		string.sprintf("%02d%02d%02d%02d%02d", 
			lib_hex_to_bcd(lnt_gry_ticket.ticket.lid[3]), lib_hex_to_bcd(lnt_gry_ticket.ticket.lid[4]),
			lib_hex_to_bcd(lnt_gry_ticket.ticket.lid[5]), lib_hex_to_bcd(lnt_gry_ticket.ticket.lid[6]),
			lib_hex_to_bcd(lnt_gry_ticket.ticket.lid[7]));

		label_lid->setText(string);

#if 0
		/* ������ */
		string.sprintf("%02X%02X%02X%02X%02X%02X%02X%02X", lnt_gry_ticket.ticket.pid[0],
		lnt_gry_ticket.ticket.pid[1],lnt_gry_ticket.ticket.pid[2],lnt_gry_ticket.ticket.pid[3],
		lnt_gry_ticket.ticket.pid[4],lnt_gry_ticket.ticket.pid[5],lnt_gry_ticket.ticket.pid[6],lnt_gry_ticket.ticket.pid[7]);

		label_pid->setText(string);
#endif

		/* ��ֵʱ�� */
		string.sprintf("%02d%02d-%02d-%02d", lib_hex_to_bcd(lnt_gry_ticket.ticket.ctim[0]),
			 lib_hex_to_bcd(lnt_gry_ticket.ticket.ctim[1]),  lib_hex_to_bcd(lnt_gry_ticket.ticket.ctim[2]),
			 lib_hex_to_bcd(lnt_gry_ticket.ticket.ctim[3]));
		
		label_ctim->setText(string);


		/* ��ϵͳѺ���־λ */
		if(lnt_gry_ticket.ticket.pmflag == 0x80)
			label_deposit->setText(tr("����Ѻ��"));	
		else
			label_deposit->setText(tr("δ��Ѻ��"));	

		
		retval = LNT_MOC_ERROR_NORMAL;
	}
	else
		retval = m_stat;

#if 0
	if(retval == LNT_MOC_ERROR_NORMAL)
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);
	else
		__showConfigResult(DEV_R_CONFIG_RESULT_ERROR);
#endif

	return retval;
}


void TicketInfoQueryDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_manageAccept();	
}

