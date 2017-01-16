#include <QDebug>
#include "lib_general.h"
#include "lib_lnt.h"
#include "lib_gui.h"
#include "lib_serialnumber.h"
#include "thread_task_handle.h"

extern lib_gui_t *g_gui;

#define TTH_DBG								1

#define CHECK_EXCEPTION_PROCESS_RESAULT 	1 //����쳣����д���Ƿ�ɹ�

ThreadTaskHandle::ThreadTaskHandle(struct ThreadTaskData *data)
{
	memcpy(&threadTaskData, data, sizeof(struct ThreadTaskData));

	threadTaskStep = EX_TASK_STEP_LNT_DEVICE;
	begin_times = 0;
	end_times = 0;
	dealCnt = 0;
	doneFree = 1;
}

void ThreadTaskHandle::ThreadSleep(unsigned long secs)
{
	lib_sleep(secs);
}

void ThreadTaskHandle::mThreadSleep(unsigned long msecs)
{
	lib_msleep(msecs);
}

void ThreadTaskHandle::run()
{
	struct ThreadTaskResult *TaskResult = (struct ThreadTaskResult *)malloc(sizeof(struct ThreadTaskResult));
	if(TaskResult == NULL)
	{
		qDebug("Task Result is NULL\n");
		return;
	}

	switch(threadTaskData.taskType)
	{
		case TASK_TYPE_EXCEPTION:
		{
			ExceptionHandle(TaskResult);  //�쳣����
		}
		break;
	}
}

/*
 * �쳣��������
 */
int ThreadTaskHandle::ExceptionHandle(struct ThreadTaskResult *TaskResult)
{
	int retval = LIB_LNT_ERROR;
	int timerCount = threadTaskData.refresh_period;
	unsigned char stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	lib_lnt_qry_ticket_info_ack_t lnt_qry_ticket;
	lib_lnt_rent_info_req_t lnt_rent_req;
	lib_lnt_rent_info_ack_t lnt_rent_ack;
	gui_except_handle_req_t e_hndl_req;
	gui_except_handle_ack_t e_hndl_ack;
	unsigned char f_type;
	unsigned char rstat;
	unsigned int f_fee;
	unsigned char ex_pid[8] = {0};
	gui_exception_handle_record_t ex_record;
		
	threadTaskStep = EX_TASK_STEP_LNT_DEVICE;
	
	while(1)
	{
		if(threadTaskStep == EX_TASK_STEP_LNT_DEVICE)  //��������ѯ
		{
#ifdef TTH_DBG
			qDebug("----------------EX_TASK_STEP_LNT_DEVICE---%d-------------\n", EX_TASK_STEP_LNT_DEVICE);
#endif		
			timerCount = 0;
			retval = LIB_LNT_ERROR;
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_getversion_ack_t ver;
			retval = lib_lnt_get_version(&ver, &stat, 300);
			if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL))  //û�ж�����
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
				TaskResult->step = EX_TASK_STEP_LNT_DEVICE;
				sprintf(TaskResult->s_info, "��ʾ:�������쳣,����ִ���쳣�������");
				
				emit signals_ExceptionResult(TaskResult);
				return -1;
			}
			
			threadTaskStep = EX_TASK_STEP_REGISTER;
		}

		if(threadTaskStep == EX_TASK_STEP_REGISTER) //����ע��
		{
#ifdef TTH_DBG
			qDebug("----------------EX_TASK_STEP_REGISTER---%d-------------\n", EX_TASK_STEP_REGISTER);
#endif
			timerCount = 0;
			gui_access_state_t state;
			memset(&state, 0, sizeof(gui_access_state_t));

			/* ����������� */
			struct gui_ndev_access_pattern_config access_pattern_config;
			memset(&access_pattern_config, 0, sizeof(struct gui_ndev_access_pattern_config));
			retval = lib_gui_get_ndev_access_pattern_config(g_gui, &access_pattern_config, 500);
			if(retval == LIB_GUI_EOK)
			{
				if(access_pattern_config.using_wireless == 1) //����
				{
					long long rx_bytes;
					long long tx_bytes;
					retval = lib_get_network_flow("ppp0", &rx_bytes,&tx_bytes);
					if(retval != LIB_GE_EOK)
					{
						TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
						TaskResult->step = EX_TASK_STEP_REGISTER;
						sprintf(TaskResult->s_info,  "��ʾ:�����쳣,����ִ���쳣�������");
						
						emit signals_ExceptionResult(TaskResult);
						return -1;
					}
				}
			}

			if(lib_gui_access_state(g_gui, &state, 500) == LIB_GUI_EOK)  //����Ƿ��������ĳɹ�
			{
				if(state.connect_stat != 3)  //NDEV_NETSTAT_SESSION
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_REGISTER;
					sprintf(TaskResult->s_info,  "��ʾ:���������쳣,����ִ���쳣�������");
					
					emit signals_ExceptionResult(TaskResult);
					return -1;
				}
			}

			timerCount = threadTaskData.refresh_period - 1 - (8);
			threadTaskStep = EX_TASK_STEP_TICKET_QRY;
		}

		if(threadTaskStep == EX_TASK_STEP_TICKET_QRY) //ѯ��
		{
#ifdef TTH_DBG
			if(timerCount ==  threadTaskData.refresh_period - 1 - (8))
				qDebug("----------------EX_TASK_STEP_TICKET_QRY---%d-------------\n", EX_TASK_STEP_TICKET_QRY);
#endif
			memset(&lnt_qry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));
			retval = lib_lnt_qry_ticket_info(&lnt_qry_ticket, &stat, 300);
			if((retval > 0) && (stat == LNT_MOC_ERROR_CARD_NOTAG))  //û�п�
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_SHOW_LOOP;
				TaskResult->step = EX_TASK_STEP_TICKET_QRY;
				sprintf(TaskResult->s_info, "��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������", timerCount--);
				emit signals_ExceptionResult(TaskResult);

				mThreadSleep(800);

				if(timerCount == 0)
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_TICKET_QRY;
					sprintf(TaskResult->s_info, "��ʾ:��ⲻ����Ƭ");
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				
				continue;
			}
			else  if((retval > 0) && (stat == LNT_MOC_ERROR_M1))
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
				TaskResult->step = EX_TASK_STEP_TICKET_QRY;
				sprintf(TaskResult->s_info,"��ʾ:��֧�ָ����Ϳ�����");
				emit signals_ExceptionResult(TaskResult);
				return -1;
			}
			else if((retval > 0) && (stat == LNT_MOC_ERROR_NORMAL))
			{
				memcpy(ex_pid, lnt_qry_ticket.ticket.pid, 8);
				timerCount = 0;
				pmflag_t pf;
				memcpy(&pf, &(lnt_qry_ticket.ticket.pmflag), 1);  //Ѻ���־
                                if(lnt_qry_ticket.ticket.pursetype != 0x02)  //��CPU��
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_TICKET_QRY;
					sprintf(TaskResult->s_info,"��ʾ:��֧�ָ����Ϳ�����");
					emit signals_ExceptionResult(TaskResult);
					return -1;
				}
				
				if((lnt_qry_ticket.ticket.pmflag == 0xff) || (pf.pflag != 1))  
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_TICKET_QRY;
					sprintf(TaskResult->s_info,"��ʾ:��֧�ַǻ�Ա������");
					emit signals_ExceptionResult(TaskResult);
					return -1;
				}

				threadTaskStep = EX_TASK_STEP_RENT_INFO_QRY;
			}
			else if(stat == LNT_MOC_ERROR_CARD_NAMELIST)  //��������
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
				TaskResult->step = EX_TASK_STEP_LNT_DEVICE;
				sprintf(TaskResult->s_info, "��ʾ:�ÿ��Ǻ�������,�������쳣����");
				emit signals_ExceptionResult(TaskResult);
				return -1;
			}
			else
			{
				if(dealCnt < 3)
				{
					mThreadSleep(500);
					dealCnt++;
					continue;
				}

				dealCnt = 0;
			}
		}
		
		if(threadTaskStep == EX_TASK_STEP_RENT_INFO_QRY)
		{
#ifdef TTH_DBG
			qDebug("----------------EX_TASK_STEP_RENT_INFO_QRY---%d-------------\n", EX_TASK_STEP_RENT_INFO_QRY);
#endif
			memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
			memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lnt_rent_req.id = 0x68;
			retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
			if((retval > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
			{
				if(lnt_rent_ack.rent.rent_flag == 0x00) //�ѻ���
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_RENT_INFO_QRY;
					sprintf(TaskResult->s_info,"��ʾ:δ��⵽���쳣"); 
					emit signals_ExceptionResult(TaskResult);
					return 0;
				}
			}
				
			threadTaskStep = EX_TASK_STEP_EHANDLE_REQ;
		}
	
		if(threadTaskStep == EX_TASK_STEP_EHANDLE_REQ)
		{
#ifdef TTH_DBG
			qDebug("----------------EX_TASK_STEP_EHANDLE_REQ---%d-------------\n", EX_TASK_STEP_EHANDLE_REQ);
#endif
			memset(&e_hndl_req, 0, sizeof(gui_except_handle_req_t));
			lib_SN_except_read32(&m_serialNumber);  //��ˮ��
			e_hndl_req.sn = LNT_HTONL(m_serialNumber);
			memcpy(&e_hndl_req.pid, &lnt_qry_ticket.ticket.pid, 8); //������
			memcpy(&e_hndl_req.ticket, &lnt_qry_ticket.ticket, 42);  //����Ʊ����Ϣ
			memcpy(&e_hndl_req.rentinfo, &(lnt_rent_ack.rent), 36);  //�⻹����Ϣ

			//qDebug("m_serialNumber=%d\n", m_serialNumber);

            retval = lib_gui_except_handle_req(g_gui, &e_hndl_req, 300); //�����Ļ�ȡ�쳣��Ϣ
			if(retval == LIB_GUI_EOK)
			{
				timerCount = threadTaskData.refresh_period;
				threadTaskStep = EX_TASK_STEP_EHANDLE_ACK;
			}
		}

		if(threadTaskStep == EX_TASK_STEP_EHANDLE_ACK)
		{
#ifdef TTH_DBG
			if(timerCount ==  threadTaskData.refresh_period - 1)
				qDebug("----------------EX_TASK_STEP_EHANDLE_ACK---%d-------------\n", EX_TASK_STEP_EHANDLE_ACK);
#endif
			memset(&e_hndl_ack, 0, sizeof(gui_except_handle_ack_t));
			retval = lib_gui_except_handle_ack(g_gui, &e_hndl_ack, 800);
			if(retval == LIB_GUI_EEMPTY)
			{		
				TaskResult->result = THREAD_TASK_RESULT_OP_SHOW_LOOP;
				TaskResult->step = EX_TASK_STEP_TICKET_QRY;
				sprintf(TaskResult->s_info, "��ʾ:�쳣���������,�����ƶ���Ƭ�����ĵȴ� %02d��,����ϸ������Ļ�ϵ���ʾ", timerCount--);
				emit signals_ExceptionResult(TaskResult);

				mThreadSleep(800);
				
				if(timerCount == 0)
				{
					doneFree = 2; //��ʱ
					goto Done;
				}

				continue;
			}

			if(retval == LIB_GUI_EFULL)
			{
				//lib_printf_v2("e_hndl_ack:", (unsigned char *)&e_hndl_ack, sizeof(e_hndl_ack), 16);
				threadTaskStep = EX_TASK_STEP_EHANDLE_START;
			}
		}

		if(threadTaskStep == EX_TASK_STEP_EHANDLE_START)  //��ʼ���쳣����
		{
#ifdef TTH_DBG
			qDebug("----------------EX_TASK_STEP_EHANDLE_START---%d-------------\n", EX_TASK_STEP_EHANDLE_START);
#endif

			f_fee =  LNT_NTOHL(e_hndl_ack.fee);

			qDebug("------------e_hndl_ack.status:0x%02x---------------\n", e_hndl_ack.status);
			qDebug("------------e_hndl_ack.fee:%d---------------\n", f_fee);

			lib_printf("FEE:", (unsigned char *)&e_hndl_ack.fee, 4);
			
		#if 0
			//----------------for test------------------
			unsigned char ack_info[256] = {0};
			memcpy(ack_info, e_hndl_ack.info, 48);
			printf("strlen:%d\n", strlen((const char *)e_hndl_ack.info));
			printf("\n-------------ack_info:%s\n", ack_info);
			lib_printf("nack_info",e_hndl_ack.info,strlen((const char *)e_hndl_ack.info));
			e_hndl_ack.info[strlen((const char *)e_hndl_ack.info) - 1] = '\0';
			printf("\nack_info:%s\n", e_hndl_ack.info);

			TaskResult->result = THREAD_TASK_RESULT_OP_SHOW_LOOP;
			TaskResult->step = EX_TASK_STEP_EHANDLE_START;
			sprintf(TaskResult->s_info, "�쳣����״̬:"); //for test
			strcat(TaskResult->s_info, (const char*)e_hndl_ack.info); //for test
			emit signals_ExceptionResult(TaskResult);

			lib_sleep(1);
			//----------------for test------------------
		#endif

			switch(e_hndl_ack.status)
			{
				case GUI_EX_HDL_NORMAL: //����
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					sprintf(TaskResult->s_info,"��ʾ:�ÿ����쳣���쳣��¼δ�ϴ�,���Ժ����Ի������˹�����\n");
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_NOT_CARD:  //����δˢ��
				{
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* ��ѯ�⻹����Ϣ */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"��ʾ:����δˢ���쳣��������в�ѯƱ����Ϣʧ��,������");						doneFree = 1;
						goto Done;
					}
					
					/* �����⻹����Ϣ */
					lib_lnt_rent_info_t rent_info;
					memcpy(&rent_info.rent, &(lnt_rent_ack.rent), sizeof(struct rent_info));

					rent_info.id = 0x68;
					rent_info.length = sizeof(lib_rent_info_t );
					rent_info.rent.rent_flag = 0;  //�޸�Ϊ�ѻ���
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					retval = lib_lnt_set_rent_info(&rent_info, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"��ʾ:����δˢ���쳣���������д��ʧ��,������");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}
					
					/* ����д�� */
					lib_lnt_consume_req_t lnt_cse;
					lib_lnt_trade_record_ack_t lnt_trade_rec;
					
					memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lnt_cse.amount = f_fee;
					lnt_cse.fee = f_fee;
					lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
					memcpy(lnt_cse.phyiscalid, lnt_qry_ticket.ticket.pid, 8);
					lnt_cse.aty = LNT_ATY_RETURN; //��Ѻ��
					rstat = LNT_MOC_ERROR_CARD_OP_FAIL;
					
					retval = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &rstat, &f_type);
					if(retval != LNT_EOK)
					{
						if(f_type == LNT_GET_TRANS)  
						{
							stat = LNT_MOC_ERROR_CARD_OP_FAIL;
							memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
							retval = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //��ֹ��ȡ���Ѽ�¼ʧ��
							if((retval <  0) || (stat != LNT_MOC_ERROR_NORMAL))
							{
								doneFree = 1;
								goto Done;
							}
						}
						else	//����ʧ�� 2016-11-01
						{
                            sprintf(TaskResult->s_info,"��ʾ:����δˢ���쳣����ʧ��,������");
							goto Done;
						}
					}
										
					if(f_fee > 0)	
					{
						//printf("--------------------upload exception record----------------\n");
						memset(&ex_record, 0, sizeof(gui_exception_handle_record_t));
						ex_record.sn = LNT_HTONL(m_serialNumber);  
						memcpy(&(ex_record.pid), &(lnt_qry_ticket.ticket.pid), sizeof(ex_record.pid));
						memcpy(&(ex_record.record), &(lnt_trade_rec.record), sizeof(ex_record.record));

						//lib_printf_v2("ex_record:", (unsigned char *)&ex_record, sizeof(ex_record), 16);
						
						lib_gui_set_exception_handle_record(g_gui, &ex_record, 500);  //�ϴ����Ѽ�¼
					
						//sprintf(TaskResult->s_info,"��ʾ:����δˢ���쳣����ɹ�,�۳�%02d.%02dԪ", f_fee / 100, f_fee % 100);
					}

					
				#if CHECK_EXCEPTION_PROCESS_RESAULT
					/* ����д���Ƿ�ɹ����ж� add by zjc at 2016-08-12 */
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* ��ѯ�⻹����Ϣ */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if(lnt_rent_ack.rent.rent_flag != 0) 
					{
						sprintf(TaskResult->s_info,"��ʾ:����δˢ���쳣����ʱд��ʧ��,�����Ի����˹�����");
						goto Done;
					}
				#endif
				
					sprintf(TaskResult->s_info,"��ʾ:����δˢ���쳣����ɹ�,�۳�%02d.%02dԪ", f_fee / 100, f_fee % 100);

					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_NOT_COST:  //Ƿ��
				{
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* ��ѯ�⻹����Ϣ */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"��ʾ:Ƿ���쳣��������в�ѯƱ����Ϣʧ��,������");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}
					
					/* �����⻹����Ϣ */
					lib_lnt_rent_info_t rent_info;
					memcpy(&rent_info.rent, &(lnt_rent_ack.rent), sizeof(struct rent_info));

					rent_info.id = 0x68;
					rent_info.length = sizeof(lib_rent_info_t );
					rent_info.rent.rent_flag = 0;  //�޸�Ϊ�ѻ���
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					retval = lib_lnt_set_rent_info(&rent_info, &stat, 200);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"��ʾ:Ƿ���쳣���������д��ʧ��,������");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}

					/* ����д�� */
					lib_lnt_consume_req_t lnt_cse;
					lib_lnt_trade_record_ack_t lnt_trade_rec;
					
					memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lnt_cse.amount = f_fee;
					lnt_cse.fee = f_fee; //�۷�
					lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
					memcpy(lnt_cse.phyiscalid, lnt_qry_ticket.ticket.pid, 8);
					lnt_cse.aty = LNT_ATY_RETURN; //��Ѻ��
					rstat = LNT_MOC_ERROR_CARD_OP_FAIL;
					
					retval = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &rstat, &f_type);
					if(retval != LNT_EOK)
					{
						if(f_type == LNT_GET_TRANS)  
						{
							stat = LNT_MOC_ERROR_CARD_OP_FAIL;
							memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
							retval = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //��ֹ��ȡ���Ѽ�¼ʧ��
							if((retval <  0) || (stat != LNT_MOC_ERROR_NORMAL))
							{
								doneFree = 1;
								goto Done;
							}
						}
						else	//����ʧ�� 2016-05-17
						{
                            sprintf(TaskResult->s_info,"��ʾ:Ƿ���쳣����ʧ��,������");
							goto Done;
						}
					}

					if(f_fee > 0) //���ӷ����ж� add by zjc at 2016-11-01
					{
						//printf("--------------------upload exception record----------------\n");

						memset(&ex_record, 0, sizeof(gui_exception_handle_record_t));

						ex_record.sn = LNT_HTONL(m_serialNumber);  //���к�
						memcpy(&(ex_record.pid), &(lnt_qry_ticket.ticket.pid), sizeof(ex_record.pid));
						memcpy(&(ex_record.record), &(lnt_trade_rec.record), sizeof(ex_record.record));
						lib_gui_set_exception_handle_record(g_gui, &ex_record, 500);  //�ϴ����Ѽ�¼
					}
					
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
                    sprintf(TaskResult->s_info,"��ʾ:Ƿ���쳣����ɹ�,�۳�%02d.%02dԪ", f_fee / 100, f_fee % 100);
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_NOT_DEPOSIT: //δ��Ѻ��
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					sprintf(TaskResult->s_info,"��ʾ:�쳣����ʧ��,�ǻ�Ա��");
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;
			
				case GUI_EX_HDL_NOT_BIKE: //ˢ��δȡ��
				{
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* ��ѯ�⻹����Ϣ */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"��ʾ:ˢ��δȡ���쳣��������в�ѯƱ����Ϣʧ��,������");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}
					
					/* �����⻹����Ϣ */
					lib_lnt_rent_info_t rent_info;
					memcpy(&rent_info.rent, &(lnt_rent_ack.rent), sizeof(struct rent_info));

					rent_info.id = 0x68;
					rent_info.length = sizeof(lib_rent_info_t );
					rent_info.rent.rent_flag = 0;  //�޸�Ϊ�ѻ���
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					retval = lib_lnt_set_rent_info(&rent_info, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"��ʾ:ˢ��δȡ���쳣���������д��ʧ��,������");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}

					/* ����д�� */
					lib_lnt_consume_req_t lnt_cse;
					lib_lnt_trade_record_ack_t lnt_trade_rec;
					
					memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lnt_cse.amount = f_fee;
					lnt_cse.fee = f_fee;
					lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
					memcpy(lnt_cse.phyiscalid, lnt_qry_ticket.ticket.pid, 8);
					lnt_cse.aty = LNT_ATY_RETURN; //��Ѻ��
					rstat = LNT_MOC_ERROR_CARD_OP_FAIL;
					
					retval = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &rstat, &f_type);
					if(retval != LNT_EOK)
					{
						if(f_type == LNT_GET_TRANS)  
						{
							stat = LNT_MOC_ERROR_CARD_OP_FAIL;
							memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
							retval = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //��ֹ��ȡ���Ѽ�¼ʧ��
							if((retval <  0) || (stat != LNT_MOC_ERROR_NORMAL))
							{
								doneFree = 1;
								goto Done;
							}
						}
						else	//����ʧ�� 2016-05-17
						{
                            sprintf(TaskResult->s_info,"��ʾ:ˢ��δȡ���쳣����ʧ��,������");
							goto Done;
						}
					}

					if(f_fee > 0)
					{
						memset(&ex_record, 0, sizeof(gui_exception_handle_record_t));
						ex_record.sn = LNT_HTONL(m_serialNumber);  //���к�
						memcpy(&(ex_record.pid), &(lnt_qry_ticket.ticket.pid), sizeof(ex_record.pid));
						memcpy(&(ex_record.record), &(lnt_trade_rec.record), sizeof(ex_record.record));

						//printf("--------------------upload exception record----------------\n");
						lib_gui_set_exception_handle_record(g_gui, &ex_record, 500);  //�ϴ����Ѽ�¼
					
						//sprintf(TaskResult->s_info,"��ʾ:ˢ��δȡ���쳣����ɹ�,�۳�%02d.%02dԪ", f_fee / 100, f_fee % 100);
					}

				#if CHECK_EXCEPTION_PROCESS_RESAULT
					/* ����д���Ƿ�ɹ����ж� add by zjc at 2016-08-19 */
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* ��ѯ�⻹����Ϣ */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if(lnt_rent_ack.rent.rent_flag != 0) 
					{
						sprintf(TaskResult->s_info,"��ʾ:ˢ��δȡ���쳣����ʱд��ʧ��,�����Ի����˹�����");
						goto Done;
					}
				#endif
				
					sprintf(TaskResult->s_info,"��ʾ:ˢ��δȡ���쳣����ɹ�,�۳�%02d.%02dԪ", f_fee / 100, f_fee % 100);
					
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_MANUAL: //��̨�쳣
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					sprintf(TaskResult->s_info,"��ʾ:�����쳣,�����˹�����");
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;
				
				default:
				{
					doneFree = 1;
				}
			}
		}

		qDebug("---------threadTaskStep=%d, doneFree=%d\n", threadTaskStep, doneFree);
		
Done:		
		if(doneFree == 0)
			TaskResult->result = THREAD_TASK_RESULT_OP_TIMER_LOOP;
		else if(doneFree == 1)
			TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
		else if(doneFree == 2)
			TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
		else if(doneFree == 3)
			TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
		
		TaskResult->step = EX_TASK_STEP_FINISH;
		if(doneFree == 2)
			sprintf(TaskResult->s_info,"��ʾ:�쳣����ʱ");
		else
			sprintf(TaskResult->s_info,"��ʾ:�쳣����ʧ��");
		
		emit signals_ExceptionResult(TaskResult);
	
		return 0;
	}
	
	return 0;
}


