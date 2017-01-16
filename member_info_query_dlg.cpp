#include <QtGui>
#include <QDebug>
#include "member_info_query_dlg.h"
#include "helper.h"
#include "lib_general.h"
#include "lib_lnt.h"

#include <time.h>


#define BINFO_REFRESH_PERIOD					(25)
#define BINFO_RETURN_PERIOD						(20)

/* Ʊ�ֱ�ǣ������жϷ���ͨ������Ч�� add by zjc at 2016-12-05 */
#define MEMBER_TYPE_NORMAL	0x00
#define MEMBER_TYPE_TIMES	0x01
#define MEMBER_TYPE_DAYS	0x02
#define MEMBER_TYPE_MONTHS	0x03
#define MEMBER_TYPE_YEARS	0x04
#define MEMBER_TYPE_ADMIN	0xFE


MemberInfoQueryDlg::MemberInfoQueryDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	


	m_ret = LIB_LNT_ERROR;
	m_stat = 0;
	timerFlag = 0;
	adminCardType = LNT_ADMIN_CARD_TYPE_NOT;
	

	__allWidgetClear();
	
	timerCount = BINFO_REFRESH_PERIOD;
	slots_timerDone();
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);

	connect(this, SIGNAL(signals_manageAccept()), this, SLOT(accept()));
	connect(this, SIGNAL(signals_manageReject()), this, SLOT(reject()));
}


MemberInfoQueryDlg::~MemberInfoQueryDlg(void)
{
	
}

void MemberInfoQueryDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_memberInfoQryShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_memberInfoQryShow->setText(tr("��ʾ:��Ա��Ϣ��ѯ�ɹ�"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_ERROR:
		{
			label_memberInfoQryShow->setText(tr("��ʾ:��Ա��Ϣ��ѯʧ��"));	
		}
		break;
	}	
}

void MemberInfoQueryDlg::slots_timerDone(void)
{
	static int ret = LNT_MOC_ERROR_CARD_OP_FAIL;
	QString memberShow;

	if(timerFlag == 0)
	{
		__allWidgetClear();
		
		ret = __getPageConfig();
		if(ret == LNT_MOC_ERROR_CARD_NOTAG)
		{
			memberShow.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ������", timerCount);
			label_memberInfoQryShow->setText(memberShow);	
		}
		
		if(timerCount == 0)
		{
			label_memberInfoQryShow->setText(tr("��ʾ:��Ա��Ϣ��ѯ��ʱ,�Զ�������һ������, ��[�˳�]��ֱ�ӷ���"));
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
		if(ret == LNT_MOC_ERROR_NORMAL)
			memberShow.sprintf("��ʾ:��Ա��Ϣ��ѯ�ɹ�, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		else
			memberShow.sprintf("��ʾ:��Ա��Ϣ��ѯʧ��, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);

		if(ret == LNT_MOC_ERROR_M1)
			memberShow.sprintf("��ʾ:��֧�ָ����Ϳ�����, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);

		if(ret == LNT_MOC_ERROR_CARD_OP_FAIL)
			memberShow.sprintf("��ʾ:��֧�ָ����Ϳ���ѯ, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
			
		if(ret == LNT_MOC_ERROR_LILIN_NOT_MEMBER_CARD)
			memberShow.sprintf("��ʾ:��֧�ַǻ�Ա����ѯ, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);

		if(ret == LNT_MOC_ERROR_CARD_NAMELIST)
			memberShow.sprintf("��ʾ:�ÿ�Ϊ��������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		
		label_memberInfoQryShow->setText(memberShow);

		if(timerCount == 0)
		{
			timerFlag = 2;
			emit signals_manageReject();
		}
	}

	timerCount--;
}

void MemberInfoQueryDlg::__rent_flag_to_bit(struct rent_info *rinfo, struct rent_flag *flag)
{
	if((rinfo == NULL) || (flag == NULL))
		return;

	flag->b0 = rinfo->rent_flag & 0x1;
	flag->b1 = (rinfo->rent_flag >> 1) & 0x1;
	flag->b2 = (rinfo->rent_flag >> 2) & 0x1;
	flag->b3 = (rinfo->rent_flag >> 3) & 0x1;
}

void MemberInfoQueryDlg::__allWidgetClear(void)
{
	label_memberInfoQryShow->clear();
	label_rentFlag->clear();
	label_ticketType->clear();
	label_startTime->clear();
	label_accum->clear();
	label_curDealDate->clear();
	label_freeTime->clear();
	label_points->clear();
}

int MemberInfoQueryDlg::__getPageConfig(void)
{
	int retval = LIB_LNT_ERROR;
	m_stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	QString string;
	lib_lnt_qry_ticket_info_ack_t lnt_qry_ticket;
	lib_lnt_rent_info_req_t lnt_rent_req;
	lib_lnt_rent_info_ack_t lnt_rent_ack;
	struct rent_flag rf;
	
	memset(&lnt_qry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));
	memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
	memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));
	memset(&rf, 0, sizeof(struct rent_flag));
	
	m_ret = lib_lnt_qry_ticket_info(&lnt_qry_ticket, &m_stat, 300);
	if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_CARD_NAMELIST))
	{
		return LNT_MOC_ERROR_CARD_NAMELIST;
	}
	else if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_NORMAL))
	{
		/* ���һ���Ƿ���� */
		 adminCardType = lib_lnt_is_admin_card();
		 if((adminCardType == LNT_ADMIN_CARD_TYPE_RD) || (adminCardType == LNT_ADMIN_CARD_TYPE_RW))
		 	goto TODO;

		if(lnt_qry_ticket.ticket.pursetype == 0x01)  //��֧��M1��
			return LNT_MOC_ERROR_CARD_OP_FAIL;  
			
		pmflag_t pf;
		memcpy(&pf, &(lnt_qry_ticket.ticket.pmflag), 1);
		if(((lnt_qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1)) \
			|| (lnt_qry_ticket.ticket.pmflag == 0xFF)) //��Ա�� /* �������� add by zjc at 2016-10-17 */
		{
			return LNT_MOC_ERROR_LILIN_NOT_MEMBER_CARD;  
		}

		//�޸��⻹����� at 2016-11-24
		/*
		��ͨ�⳵					0x80
		�⳵�߷����⳵				0xC0
		�⳵�ͷ����⳵				0xA0
		�����߷��ڻ���				0x40
		�����ͷ��ڻ���				0x20
		��ͨ����					0x00
		��׮����					0x10
		*/
TODO:		
		lnt_rent_req.id = 0x68;
		m_ret = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &m_stat, 300);
		if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_NORMAL))
		{
			//lib_printf_v2("RENT ACK:", (unsigned char *)&lnt_rent_ack, sizeof(lnt_rent_ack), 16);
			
			/* �⻹����־ */
			__rent_flag_to_bit(&(lnt_rent_ack.rent), &rf);

			if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 0) && (rf.b3 == 1))
			{
				label_rentFlag->setText(tr("��ͨ�⳵"));
			}
			else if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 1) && (rf.b3 == 1))
			{
				label_rentFlag->setText(tr("�⳵�߷����⳵"));
			}
			else if((rf.b0 == 0) && (rf.b1 == 1) && (rf.b2 == 0) && (rf.b3 == 1))
			{
				label_rentFlag->setText(tr("�����߷����⳵"));
			}
			else if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 1) && (rf.b3 == 0))
			{
				label_rentFlag->setText(tr("�����߷��ڻ���"));
			}
			else if((rf.b0 == 0) && (rf.b1 == 1) && (rf.b2 == 0) && (rf.b3 == 0))
			{
				label_rentFlag->setText(tr("�⳵�߷��ڻ���"));
			}
			else if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 0) && (rf.b3 == 0))
			{
				label_rentFlag->setText(tr("��ͨ����"));
			}
			else if(rf.b0 == 1)
			{
				label_rentFlag->setText(tr("�ӳٻ���"));
			}
			else
			{
				label_rentFlag->setText(tr("�⻹����־�쳣"));
			}
			
            //qDebug("\n=============member count:%d==============\n",LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));//����

		#if 1
			/* ����ͨ����Ч���ж� ����ʱ�����ڵ�ǰʱ�����Ϊ��Ч  add by zjc at 2016-12-06 */
			int valid_flag = 0; //��Ч�Ա�־ 0:��Ч 1:��Ч ������static!!!
			m_ret = lib_lnt_is_valid_card();
			if(m_ret == LNT_CARD_VALID)
			{
				//printf("----------This Card is Valid!\n");
				valid_flag = 1;
			}
			else if(m_ret == LNT_CARD_UNVALID)
			{
				//printf("----------This Card is Unvalid!\n");
				valid_flag = 0;  
			}
		#endif

			/* Ʊ�ֱ�־ */
			string.clear();
			switch(lnt_rent_ack.rent.ticket_flag)
			{
				case TICKET_FLAG_ORDINARG:
					label_ticketType->setText(tr("��ͨ��"));
                    string.sprintf("-");
					break;

				case TICKET_FLAG_TIME:
					label_ticketType->setText(tr("�ο�"));
					if(valid_flag == 1)
                    	string.sprintf("%d(��)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					else 
						string.sprintf("%d(��)-�ѹ���", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
                    break;

				case TICKET_FLAG_DAY:
					label_ticketType->setText(tr("�쿨"));
					if(valid_flag == 1)
                    	string.sprintf("%d(��)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					else 
						string.sprintf("%d(��)-�ѹ���", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;

				case TICKET_FLAG_MON:
					label_ticketType->setText(tr("�¿�"));
					if(valid_flag == 1)
                    	string.sprintf("%d(��)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					else 
						string.sprintf("%d(��)-�ѹ���", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;
					
				case TICKET_FLAG_YEAR:
					label_ticketType->setText(tr("�꿨"));
					if(valid_flag == 1)
                    	string.sprintf("%d(��)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					else 
						string.sprintf("%d(��)-�ѹ���", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;
					
				case TICKET_FLAG_SPECIAL:
					label_ticketType->setText(tr("ר����"));
                    string.sprintf("%d", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;		
					
				case TICKET_FLAG_OLD:
					label_ticketType->setText(tr("ԭ�ͻ���"));
                    string.sprintf("%d(��)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;	

				case TICKET_FLAG_ADMIN:
					label_ticketType->setText(tr("����"));
					if(valid_flag == 1)
                    	string.sprintf("%d", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					else 
						string.sprintf("%d(��)-�ѹ���", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;	
			}

			if(lnt_rent_ack.rent.ticket_flag == TICKET_FLAG_ADMIN)  //����ǹ���
			{
				switch(adminCardType)
				{
					case LNT_ADMIN_CARD_TYPE_RD:
					{
						label_ticketType->setText(tr("��ͨ����"));
					}
					break;

					case LNT_ADMIN_CARD_TYPE_RW:
					{
						label_ticketType->setText(tr("��������"));
					}
					break;
				}
				
				//label_accum->setText(tr("0"));
				//label_startTime->setText(tr("0000-00-00"));
				label_curDealDate->setText(tr("��"));
				label_freeTime->setText(tr("��"));
				label_points->setText(tr("��"));

				/* ���ù�����Ϣ */
				lnt_admin_card_info_ack_t acard_info;
				memset(&acard_info, 0, sizeof(lnt_admin_card_info_ack_t));
				memcpy(&acard_info, &lnt_rent_ack, sizeof(lnt_admin_card_info_ack_t));

				/* ����ʱ�� */
				unsigned int i_start_time = acard_info.admin_36bytes.A9;
				unsigned char s_start_time[3] = {0};
				memcpy(&s_start_time, &i_start_time, 3);
				if(i_start_time != 0)
				{
					string.sprintf("20%02d-%02d-%02d", lib_hex_to_bcd(s_start_time[0]),
					lib_hex_to_bcd(s_start_time[1]),  lib_hex_to_bcd(s_start_time[2]));
					label_startTime->setText(string);
				}
				else
				{
					label_startTime->setText(tr("0000-00-00"));
				}

				/* ���� */
				if(valid_flag == 1)
					string.sprintf("%d(��)", LNT_NTOHS(acard_info.admin_36bytes.A7));
				else 
				{
					label_accum->setStyleSheet("color:red;"); 
					string.sprintf("%d(��)-�ѹ���", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
				}
				label_accum->setText(string);

				return LNT_MOC_ERROR_NORMAL;  //����ʾ������
			}


			/* ���� ���� ���� ���� */
			//label_accum->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
			//printf("--------valid_flag:%d\n", valid_flag);
			if((valid_flag == 0) && (lnt_rent_ack.rent.ticket_flag != MEMBER_TYPE_NORMAL))
				label_accum->setStyleSheet("color:red;"); 
			label_accum->setText(string);
			
			/* ����ʱ�� */
			unsigned int start_time = 0;
			memcpy(&start_time, lnt_rent_ack.rent.start_time, 3);
			if(start_time != 0)
			{
				string.sprintf("20%02d-%02d-%02d", lib_hex_to_bcd(lnt_rent_ack.rent.start_time[0]),
				lib_hex_to_bcd(lnt_rent_ack.rent.start_time[1]),  lib_hex_to_bcd(lnt_rent_ack.rent.start_time[2]));
				label_startTime->setText(string);
			}
			else   
			{
				label_startTime->setText(tr("0000-00-00"));
			}

			/* ���ν�������ʱ�� */
			unsigned int cur_deal_date = 0;
			memcpy(&cur_deal_date, lnt_rent_ack.rent.cur_deal_date, 4);
			if(cur_deal_date != 0)
			{
				string.sprintf("20%02d-%02d-%02d %02d:%02d", lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[0]),
				lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[1]),  lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[2]),
				lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[3]), lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[4]));
				label_curDealDate->setText(string);
			}
			else
			{
				label_curDealDate->setText(tr("0000-00-00 00:00"));
			}   

			/* ���ʱ�� */
			int free_time = lnt_rent_ack.rent.free_time * 15;
			string.clear();
			if(valid_flag == 1)
				string.sprintf("%d����", free_time);
			else
				string.sprintf("-");
			label_freeTime->setText(string);

			/* ���û��� */
			string.clear();
			//printf("rent.points:%02X\n", lnt_rent_ack.rent.points);
			string.sprintf("%d", LNT_NTOHS(lnt_rent_ack.rent.points)); //2016-05-17   1����=1ë 
			label_points->setText(string);
				
			retval = LNT_MOC_ERROR_NORMAL;
		}
		else
			retval = m_stat;
	}
	else
		retval = m_stat;


	return retval;
}

void MemberInfoQueryDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_manageAccept();	
}




