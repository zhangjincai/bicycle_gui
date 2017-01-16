#include <QDebug>
#include "lib_general.h"
#include "lib_serialnumber.h"
#include "on_off_card_task.h"
#include "lib_records.h"
#include "lib_lnt.h"
#include "config.h"


#define CONFIRM_BUTTON	1	//������ʾ��ȷ�ϼ�

#define LNT_CPU_RECHARGE_LIMIT				(100000) //��λ:��


extern lib_records_file_t *g_rec_file;


struct task_err_code
{
	unsigned short task_step;  //������
	unsigned char task_stat;  //״̬��
	unsigned char task_info; //��Ϣ��
	unsigned char task_ret;  //������
};

OnOffCardTask::OnOffCardTask(struct TaskData *data)
{
	memcpy(&taskdata, data, sizeof(struct TaskData));

	taskStep = TASKSTEP_INIT;
	f_operate = 0;
	begin_times = 0;
	end_times = 0;
}

void OnOffCardTask::emit_TaskRunStep(enum TaskRunStep step)
{
	struct TaskResult *result = (struct TaskResult *)malloc(sizeof(struct TaskResult));
	if(result != NULL)
	{
		result->type = 0x88;
		result->serviceType = taskdata.serviceType;
		result->taskRunStep = step;
		result->result = TASK_RESULT_OP_PRINTF_FREE;
			
		emit signals_Result(result);
	}
}

void OnOffCardTask::run()
{
	struct TaskResult *result = (struct TaskResult *)malloc(sizeof(struct TaskResult));
	if(result == NULL)
	{
		fprintf(stderr, "result is NULL\n");
		return;
	}
	
	int ret = -1;
	int sockfd = -1;
	int apply_result = 0;
	int r_cpu_load = 0xff;
	unsigned char trade_record[90] = {0};
	struct task_err_code taskstep_err_code;
	unsigned char member_type = 0x00;	  
	unsigned int bal = 0;
	unsigned int ticket_blance = 0;
	unsigned int consume_time = 0;
	unsigned int consume_sn, register_sn;
	unsigned char stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	unsigned char SAVE_pid[8] = {0};  //�����������
	unsigned char macaddr[6] = {0};  //MAC��ַ 
	lnt_qry_ticket_info_ack_t qry_ticket;  
	lnt_rent_info_req_t rent_info_req;
	lnt_rent_info_ack_t rent_info_ack;
	enum AGENT_ERR agent_err = AGENT_ERR_UNKNOWN;
	unsigned int fee = 0;
	pmflag_t pf;
	lib_lnt_consume_req_t lnt_cse;
	agent_retransmission_data_t retrans;
	lib_lnt_trade_record_ack_t lnt_trade_rec;
	agent_extra_data_t extra;  //ֻ��memset��ʼ��һ��
	lnt_agent_on_card_stage1_CTS_t on_card_stage1;
	lnt_agent_on_card_stage2_STC_t on_card_stage2;
	lnt_member_process_stage2_STC_t member_stage2;
	lnt_agent_off_card_stage6_STC_t off_card_stage6_STC;
	lnt_packet_recharge_stage2_STC_t recharge_stage2;
	lnt_packet_recharge_stage4_STC_t recharge_stage4;
	lnt_packet_recharge_stage6_STC_t recharge_stage6;
	lnt_packet_recharge_stage8_STC_t recharge_stage8;
	lnt_packet_recharge_stage10_STC_t recharge_stage10;
	lnt_packet_recharge_stage12_STC_t recharge_stage12;
	lnt_agent_recharge_stage4_STC_t recharge_stage4_STC;

	lib_lnt_config_t config;
	memset(&config, 0, sizeof(config));

	memset(&taskstep_err_code, 0, sizeof(struct task_err_code));
	taskstep_err_code.task_ret = AGENT_ERR_OK;

	unsigned char extra_result;	//���濪�������
	
	
	/* ��ʼִ�в��� */
	if(taskStep == TASKSTEP_INIT)
	{
		LIB_LNT_log_open();
		
		time(&begin_times);

		emit_TaskRunStep(TaskRunStep_p1);  //�����Ա��Ƭ����[1]

		result->result = TASK_RESULT_OP_LOOP;
		result->step = TASKSTEP_INIT;
		strcpy(result->s_info, "���ڲ��鿨Ƭ����,���Ե�......");
		emit signals_Result(result);

		taskStep = TASKSTEP_START;
	}
	
	/* ��ѯƱ����Ϣ */
	if(taskStep == TASKSTEP_START)
	{
		memset(&extra, 0, sizeof(agent_extra_data_t)); //extra
		memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));

		stat = LNT_MOC_ERROR_CARD_OP_FAIL;
		ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 400);
		if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
		{
			fprintf(stderr,"\n[qry_ticket_info] Stat: %02X, ret: %d\n", stat, ret);
			
		#if 1//add by zjc
			//����ͨģ�鴮�ڳ�ʼ��
			memset(&config,0,sizeof(lib_lnt_config_t));
			strcpy((char *)config.tty,"/dev/ttyO1");
			config.baudrate = 115200;
		
			ret = lnt_init(&config);
			if(ret < 0){
				fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
			}
		#endif
		
			/* ����Ƿ�CPU��*/
			if(stat == LNT_MOC_ERROR_M1) 
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_QRY_TICKET;
				sprintf(result->s_info, "��֧�ָ����Ϳ�����,���˿�--������:%02X,״̬��:%02X", TASKSTEP_QRY_TICKET, stat);
				emit signals_Result(result);
				goto Done;
			}

			result->result = TASK_RESULT_OP_LOOP_FREE;
			result->step = TASKSTEP_QRY_TICKET;

			if(stat == LNT_MOC_ERROR_CARD_NOTAG)
				sprintf(result->s_info, "��ⲻ����,�밴ָʾ���²���--������:%02X,״̬��:%02X", TASKSTEP_QRY_TICKET, stat);
			else if(stat == LNT_MOC_ERROR_CARD_NAMELIST) //��������
				sprintf(result->s_info, "�ÿ�Ϊ��������,������ִ�п���������,���˿�--������:%02X,״̬��:%02X", TASKSTEP_QRY_TICKET, stat);
			else
				sprintf(result->s_info, "��ȡ��Ƭ��Ϣʧ��,���˿��Ժ�����--������:%02X,״̬��:%02X", TASKSTEP_QRY_TICKET, stat);
			
			emit signals_Result(result);
			goto Done;	
		}

	
		ticket_blance = qry_ticket.ticket.blance;   //����Ʊ�����

		fprintf(stderr,"------Before Operation, Blance: %d-----\n", ticket_blance);
		
		memcpy(SAVE_pid, qry_ticket.ticket.pid, 8); //���濨��������
		memcpy(&extra.pid, qry_ticket.ticket.pid, 8); 

		taskStep = TASKSTEP_QRY_TICKET;
	}

	//printf("taskdata.serviceType:%d\n",taskdata.serviceType);
	/**********************************************��������*******************************************/
	if(taskdata.serviceType == CARD_SERVICE_ON_NORMAL || taskdata.serviceType == CARD_SERVICE_ON_TIME	\
		|| taskdata.serviceType == CARD_SERVICE_ON_DAY || taskdata.serviceType == CARD_SERVICE_ON_MONTH || taskdata.serviceType == CARD_SERVICE_ON_YEAR)
	{
		
		if(taskStep == TASKSTEP_QRY_TICKET)
		{
			#if 0 //����֮ǰ��������д��ʧ����Ҫ��д������Ϣ
			/* ����Ƿ��ѿ��� */
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
			if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1))
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_QRY_TICKET;

				sprintf(result->s_info, "�ÿ��ѿ�ͨ��Ա,������ִ�п�������,���˿�--������:%02X", TASKSTEP_PMFLAG);
				emit signals_Result(result);
				goto Done;
			}
			#endif
			
			taskStep = TASKSTEP_PMFLAG;
		}

		/* ���ӵ��������� */
		if(taskStep == TASKSTEP_PMFLAG)
		{
			sockfd =  lib_lnt_on_card_connect_to_agent();
			if(sockfd <= 0)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_SOCKET;
				sprintf(result->s_info, "ϵͳ����ʧ��,���˿��Ժ�����--������:%02X", TASKSTEP_SOCKET);
				emit signals_Result(result);
				goto Done;
			}

			lib_setfd_noblock(sockfd); //����Ϊ������
			
			taskStep = TASKSTEP_ON_STAGE1TO2;
		}

		/* ִ�п���1-2�׶� */
		if(taskStep == TASKSTEP_ON_STAGE1TO2)
		{
			memset(&rent_info_req, 0, sizeof(lnt_rent_info_req_t));
			memset(&rent_info_ack, 0, sizeof(lnt_rent_info_ack_t));
			memset(&on_card_stage1, 0, sizeof(lnt_agent_on_card_stage1_CTS_t));
			memset(&on_card_stage2, 0, sizeof(lnt_agent_on_card_stage2_STC_t));
			
			//��ѯ�⻹����Ϣ
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			rent_info_req.id = 0x68;
			
			ret = lib_lnt_qry_rent_info(&rent_info_req, &rent_info_ack, &stat, 400);
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
			{
			#if 1//add by zjc
				//����ͨģ�鴮�ڳ�ʼ��
				memset(&config,0,sizeof(lib_lnt_config_t));
				strcpy((char *)config.tty,"/dev/ttyO1");
				config.baudrate = 115200;
			
				ret = lnt_init(&config);
				if(ret < 0){
					fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
				}
			#endif
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_QRY_RENT;

				sprintf(result->s_info, "��ȡ��Ƭ��Ϣʧ��,���˿��Ժ�����--������:%02X,״̬��:%02X", TASKSTEP_QRY_RENT, stat);
				emit signals_Result(result);
				goto Done;
			}
	
			memcpy(on_card_stage1.pid, SAVE_pid, 8); //pid
			lib_get_macaddr("eth1", (char *)&on_card_stage1.macaddr);  //MAC
			memcpy(macaddr, on_card_stage1.macaddr, 6);  //����MAC
			memset(on_card_stage1.pki, 0xff, 4);
			strcpy((char *)on_card_stage1.id_card, (char *)taskdata.id_card);
			memcpy(on_card_stage1.phone_number, taskdata.phone_number, 6); //�绰����
			on_card_stage1.member_type = taskdata.member_type; //��Ա����//
			on_card_stage1.member_count = LNT_HTONS(taskdata.member_typeCount);
			memcpy(&on_card_stage1.ticket_info, &qry_ticket.ticket, sizeof(struct ticket_info));
			memcpy(&on_card_stage1.rent_info, &rent_info_ack.rent, sizeof(struct rent_info));
			lib_lnt_utils_time_bcd_yymmddhhMMss(on_card_stage1.time);  //ʱ��

		#if 1
			printf("\n--------------Request Member_type:%d\n", on_card_stage1.member_type);
			printf("--------------Request Member_typeCount:%d\n\n", taskdata.member_typeCount);		
		#endif
			
			#if 1
			switch(taskdata.member_type)	//�����������������������������Ϸ���
			{	
				case 0x01:	//������������:1~100
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_COUNTS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "������Ŀ����������Ϸ�,����������Ϸ��Ĵ���[%d~%d]��ִ�п�ͨ����--������:%02X", MEMBER_LOWER_LIMIT, MEMBER_COUNTS_LIMIT, TASKSTEP_QRY_RENT);
						emit signals_Result(result);
						goto Done;
					}
				break;
					
				case 0x02:	//������������:1~30
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_DAYS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "������Ŀ����������Ϸ�,����������Ϸ�������[%d~%d]��ִ�п�ͨ����--������:%02X", MEMBER_LOWER_LIMIT, MEMBER_DAYS_LIMIT, TASKSTEP_QRY_RENT);
						emit signals_Result(result);
						goto Done;
					}
				break;
					
				case 0x03:	//������������:1~12
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_MONTHS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "������Ŀ����������Ϸ�,����������Ϸ�������[%d~%d]��ִ�п�ͨ����--������:%02X", MEMBER_LOWER_LIMIT, MEMBER_MONTHS_LIMIT, TASKSTEP_QRY_RENT);
						emit signals_Result(result);
						goto Done;
					}
				break;

				case 0x04:	//������������:1~5
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_YEARS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "������Ŀ����������Ϸ�,����������Ϸ�������[%d~%d]��ִ�п�ͨ����--������:%02X", MEMBER_LOWER_LIMIT, MEMBER_YEARS_LIMIT, TASKSTEP_QRY_RENT);
						emit signals_Result(result);
						goto Done;
					}
				break;
			}
		#endif		

			agent_err = lib_lnt_on_card_stage1TO2_process(sockfd, &on_card_stage1, &on_card_stage2);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE; 
				result->step = TASKSTEP_ON_STAGE1TO2;
				sprintf(result->s_info, "ϵͳ��æ,���˿��Ժ�����--������:%02X,������:%02X", TASKSTEP_ON_STAGE1TO2, agent_err);
				emit signals_Result(result);
				goto Done;
			}

			apply_result = on_card_stage2.apply_result; //������
		#if 1
			on_card_stage2.member_count = LNT_NTOHS(on_card_stage2.member_count);

			printf("\n--------------Apply Member_type:%d\n", on_card_stage2.member_type);
			printf("--------------Apply Member_typeCount:%d\n\n", on_card_stage2.member_count);
			
			if((taskdata.member_type != on_card_stage2.member_type) || \
				(taskdata.member_typeCount != on_card_stage2.member_count))
			{
				printf("--------------Member Apply Error!\n"); //���صĿ�������������Ĳ�һ��
			}
		#endif

			LIB_LNT_log_vsprintf("****************Begin On Card Process*****************\n");
			LIB_LNT_log_vsprintf("PKT SN:%d, PID: %02x %02x %02x %02x %02x %02x %02x %02x\n", lib_pkt_sn_RO_get(), \
				SAVE_pid[0], SAVE_pid[1], SAVE_pid[2], SAVE_pid[3], SAVE_pid[4], SAVE_pid[5], SAVE_pid[6], SAVE_pid[7]);
			LIB_LNT_log_vsprintf("PKT SN:%d, On Card Apply Result: 0x%02X\n", lib_pkt_sn_RO_get(), apply_result);
			printf("On Card Apply Result: 0x%02X\n",apply_result);

			switch(apply_result) 
			{
				case 0x01: //������
				{
					strcpy(result->s_info, "�ÿ�Ϊ�Ƿ���,����ִ�д���...");
					emit signals_Result(result);
					
					goto BlackListDeal;  //��������������
				}
				break;

				case 0x02: //���֤��ʽ��Ч
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "�����[���֤����]��ʽ��Ч,���˿�");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x03: //�ֻ������ʽ��Ч
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "�����[�ֻ�����]��ʽ��Ч,���˿�");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x10: //������
				{
					fee = LNT_NTOHL(on_card_stage2.fee); //�ֽ���ת��
					bal = qry_ticket.ticket.blance; //���

					if(fee > bal)  //�ж�����Ƿ��㹻
					{
						/* ���� */
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_ON_STAGE1TO2;
						sprintf(result->s_info, "�ÿ����Ϊ%02d.%02dԪ,����%d.%02dԪ,���ֵ������ִ�п�ͨ��Ա����", bal / 100, bal % 100, fee / 100, fee % 100);
						emit signals_Result(result);
						goto Done;
					}
					else
					{
						/* ����㹻 */
						result->result = TASK_RESULT_OP_LOOP;
						result->step = TASKSTEP_ON_STAGE1TO2;
						sprintf(result->s_info, "����ִ�п�ͨ��Ա����,����۳�Ѻ��%d.%02dԪ,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......", fee / 100, fee % 100);
						emit signals_Result(result);
							
						taskStep = TASKSTEP_ON_CONSUME;  //��һ��ִ�п۷�
					}
				}
				break;

				case 0x11: //�����ѣ�δ����
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_STAGE1TO2;
					sprintf(result->s_info, "����ִ�п�ͨ��Ա����,�ÿ��ѿ۷�,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......");
					emit signals_Result(result);
							
					taskStep = TASKSTEP_ON_STAGE5TO6;	 //Ѻ����
				}
				break;

				case 0x12: //���֤����������������
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "�����֤���뿪��������������,���˿�");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x13: //�ѿ���
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "�ÿ��ѿ�ͨ��Ա,������ִ�п�������,���˿�");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x14: //���˹�����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info,  "�����˹���ѯ����,���˿�");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x15: //����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "�ÿ�����,���ֵ������ִ�п�ͨ��Ա����");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x16: //ֻ������������
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_STAGE1TO2;
					sprintf(result->s_info, "����ִ�л�Ա��ͨ����,�ÿ��Ѹ�Ѻ��,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......");
					emit signals_Result(result);
							
					taskStep = TASKSTEP_ON_STAGE5TO6;	 //Ѻ����
				}
				break;

				case 0xFF: //��ͣ����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OUT_OF_SERVICE;
					strcpy(result->s_info, "�豸��ͣ�ù��ܷ���,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				default:  //Ĭ��
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "ִ�л�Ա��ͨ����ʧ��,��������쳣,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;
			}
		}

		/* ִ�п�����Ѻ�� */
		if(taskStep == TASKSTEP_ON_CONSUME)  //��Ѻ��׶�
		{
			emit_TaskRunStep(TaskRunStep_p2);  //�۳��û�Ѻ��[1]
			
			unsigned char f_type = 0;
			
			memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
			memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));

			LIB_LNT_log_vsprintf("PKT SN:%d, On Card Consume Fee: %2d.%02d(Y)\n", lib_pkt_sn_RO_get(), fee / 100, fee % 100);
			
			lnt_cse.amount = fee;
			lnt_cse.fee = fee;
			lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
			memcpy(lnt_cse.phyiscalid, SAVE_pid, 8);
			lnt_cse.aty = LNT_ATY_RETURN; //��Ѻ��  0x17

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			ret = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &stat, &f_type);	//���Ѻͻ�ȡ���׼�¼
			if(ret != LNT_EOK) 
			{
				if(f_type == LNT_GET_TRANS)  //��ȡ���׼�¼ʧ��
				{
					//add by zjc 2016-03-22
					LIB_LNT_log_vsprintf("PKT SN:%d, On Card Get Trade Record Failed, Status: %02X\n", lib_pkt_sn_RO_get(), stat);

					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					ret = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //��ȡ���Ѽ�¼ʧ�����ٶ�ȡһ��
					if((ret <=  0) || (stat != LNT_MOC_ERROR_NORMAL))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_ON_CONSUME;
						sprintf(result->s_info, "��Ա��ͨ����ʧ��,�������������˹���ѯ����--������:%02X,״̬��:%02X,������:%02X", TASKSTEP_ON_CONSUME, stat, f_type);
						emit signals_Result(result);
						goto Done;
					}

					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_CONSUME;
					emit signals_Result(result);
				}
				else	//����ʧ��
				{
					//add by zjc 2016-03-22
					LIB_LNT_log_vsprintf("PKT SN:%d, On Card Consume Failed, Status: %02X\n", lib_pkt_sn_RO_get(), stat);
					
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_CONSUME;
					sprintf(result->s_info, "��Ա��ͨ����ʧ��,�������������˹���ѯ����--������:%02X,״̬��:%02X,������:%02X", TASKSTEP_ON_CONSUME, stat, f_type);
					emit signals_Result(result);
					goto Done;
				}
			}
			else  //�ɹ�
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, On Card Consume Trade OK\n", lib_pkt_sn_RO_get());
					
				result->result = TASK_RESULT_OP_LOOP;
				result->step = TASKSTEP_ON_CONSUME;
				emit signals_Result(result);
			}

			memcpy(&trade_record, &(lnt_trade_rec.record), sizeof(trade_record)); //���׼�¼
			//�����׼�¼�������ݿ�
			lib_records_consume_insert_data(g_rec_file, (int)lib_pkt_sn_RO_get(), lnt_trade_rec.record.pid, lnt_trade_rec.record.lid, trade_record);  //�������Ѽ�¼
			
			taskStep = TASKSTEP_ON_STAGE3TO4;
		}

		/* ����3-4�׶�*/
		if(taskStep == TASKSTEP_ON_STAGE3TO4)  //���׼�¼�ϱ�
		{
			lnt_agent_on_card_stage3_CTS_t on_card_stage3;
			lnt_agent_on_card_stage4_STC_t on_card_stage4;

			memset(&retrans, 0, sizeof(agent_retransmission_data_t));
			memset(&on_card_stage3, 0, sizeof(lnt_agent_on_card_stage3_CTS_t));
			memset(&on_card_stage4, 0, sizeof(lnt_agent_on_card_stage4_STC_t));

			memcpy(&on_card_stage3.pid, &SAVE_pid, 8); //PID
			memcpy(&on_card_stage3.macaddr, macaddr, 6);
			memset(on_card_stage3.pki, 0xff, 4);
			
			lib_SN_consume_read32(&consume_sn);
			on_card_stage3.trade_sn = consume_sn;	//������ˮ��
			
			memcpy(&on_card_stage3.trecord, &lnt_trade_rec.record, sizeof(struct trade_record)); //���׼�¼
			lib_lnt_utils_time_bcd_yymmddhhMMss(on_card_stage3.time);

			agent_err = lib_lnt_on_card_stage3TO4_process(sockfd, &on_card_stage3, &on_card_stage4, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload Consume Record Failed\n", lib_pkt_sn_RO_get());

				/* ���׼�¼�ϴ�ʧ�ܣ��������ݿ�,�ش� [1]*/
				lib_records_reconsume_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);

				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE3TO4;
				sprintf(result->s_info, "��Ա��ͨ����ʧ��,�������������˹���ѯ����--������:%02X,������:%02X", TASKSTEP_ON_STAGE3TO4, agent_err);
				emit signals_Result(result);
				goto Done;
			}
			LIB_LNT_log_vsprintf("PKT SN:%d, Upload Consume Record OK\n", lib_pkt_sn_RO_get());	//���׼�¼�ϴ��ɹ�
		
			taskStep = TASKSTEP_ON_STAGE5TO6;
			
		}

		/* ����5-6�׶� */
		if(taskStep == TASKSTEP_ON_STAGE5TO6) //Ѻ���ʼ����DD73��DD74
		{
			emit_TaskRunStep(TaskRunStep_p3);  //ִ�л�Ա��ͨ

			/* ��Ϣ��ѯ */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);	
			if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
			{
				if(memcmp(SAVE_pid, qry_ticket.ticket.pid, 8) != 0) //���Ų�����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE5TO6;
					sprintf(result->s_info, "����ִ��ʧ��,���Ų�һ��,���˿�");
					emit signals_Result(result);
					goto Done;
				}

				//���Ѻ���־�Ƿ��Ѿ��޸ĳɹ� 2016-04-26
				memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
				if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1))	//���޸�Ѻ���־��֮ǰд��ʧ��
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_STAGE5TO6;
					sprintf(result->s_info, "���ڲ�д������Ϣ,�벻Ҫ�ƶ���Ƭ�����ĵȴ�......");
					emit signals_Result(result);
				
					taskStep = TASKSTEP_ON_SET_RENT;	
					goto SET_RENT_INFO;	//��д������Ϣ
				}
			}

			memset(&member_stage2, 0, sizeof(lnt_member_process_stage2_STC_t));
			memset(&extra, 0, sizeof(agent_extra_data_t));

			memcpy(&extra.pid, &SAVE_pid, 8);
			memcpy(&extra.macaddr, macaddr, 6);
			
			agent_err = lib_lnt_on_card_stage5TO6_process(sockfd, &extra, &member_stage2, &stat);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE5TO6;
				sprintf(result->s_info, "��Ա��ͨ����ʧ��,������ִ�л�Ա��ͨ����,�糢�Զ�κ���Ȼʧ��,�����˹�����");
				emit signals_Result(result);
				goto Done;
			}
			taskStep = TASKSTEP_DEPOSIT;
		}

		/* Ѻ���� */
		if(taskStep == TASKSTEP_DEPOSIT)
		{
			lnt_r_deposit_process_req_t dproc_req;
			lnt_r_deposit_process_ack_t dproc_ack;

			memset(&dproc_req, 0, sizeof(lnt_r_deposit_process_req_t));
			memset(&dproc_ack, 0, sizeof(lnt_r_deposit_process_ack_t));

			stat = LNT_MOC_ERROR_UNKNOW;
			memcpy(&dproc_req.data, &member_stage2.INFO, sizeof(lnt_r_deposit_process_req_t));

			ret = lib_lnt_r_deposit_process(&dproc_req, &dproc_ack, &stat, 600);
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
			{
				fprintf(stderr,"\nOn Card  Process: [deposit process] Stat: %02X\n",stat);

			#if 1//add by zjc
				//����ͨģ�鴮�ڳ�ʼ��
				memset(&config,0,sizeof(lib_lnt_config_t));
				strcpy((char *)config.tty,"/dev/ttyO1");
				config.baudrate = 115200;
			
				ret = lnt_init(&config);
				if(ret < 0){
					fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
				}
			#endif
			}

			taskstep_err_code.task_step = TASKSTEP_DEPOSIT;
			taskstep_err_code.task_stat = stat;
			taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;
			extra.result = 0x00; //����ϱ�
			extra_result = extra.result; //���濪�����

			/* Ѻ������ʧ��,������Ѻ���־û���޸ĳɹ�,����Ҫѯ���ж��Ƿ��Ѿ��޸�Ѻ���־�ɹ� */
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			ret = -1;
			
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 600);
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))
			{
				fprintf(stderr,"--------qry_ticket_info failed!--------\n");
				LIB_LNT_log_vsprintf("PKT SN:%d, ON Card Check Pmflag Failed, Stat: %02X, Ret: %d\n", lib_pkt_sn_RO_get(), stat, ret);
			}
			
			memset(&pf, 0, sizeof(pmflag_t));
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
			//(stat == LNT_MOC_ERROR_NORMAL) ����Ʊ����ѯ����ж� add by zjc at 2016-10-27
			if((stat == LNT_MOC_ERROR_NORMAL) && (qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1))  //�޸�Ѻ���־�ɹ�
			{		
				LIB_LNT_log_vsprintf("PKT SN:%d, On Card Deposit Process OK\n", lib_pkt_sn_RO_get());

				taskStep = TASKSTEP_ON_SET_RENT;
			}
			else	//�޸�Ѻ���־�޸�ʧ�ܻ��߼��Ѻ���־ʧ��
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, On Card Deposit Process Failed\n", lib_pkt_sn_RO_get());
				
				taskstep_err_code.task_step = TASKSTEP_DEPOSIT;
				taskstep_err_code.task_stat = stat;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;
				
				extra.result = 0xFF;  //�޸�Ѻ���־���߼��Ѻ���־ʧ��
				extra_result = extra.result;	//���濪�����
			
				taskStep = TASKSTEP_ON_STAGE7TO8; //������������ϱ�	
			}
		}

SET_RENT_INFO:
		/* �����⻹����Ϣ */
		if(taskStep == TASKSTEP_ON_SET_RENT)
		{
			result->result = TASK_RESULT_OP_LOOP;
			result->step = TASKSTEP_DEPOSIT;
			sprintf(result->s_info, "����ִ��д������,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......");
			emit signals_Result(result);
			
			/* ��ѯƱ����Ϣ */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);	
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))
			{
				fprintf(stderr,"\nOn Card  Process: [qry_ticket_info] Stat: %02X\n",stat);

			#if 1 //add by zjc
				//����ͨģ�鴮�ڳ�ʼ��
				memset(&config,0,sizeof(lib_lnt_config_t));
				strcpy((char *)config.tty,"/dev/ttyO1");
				config.baudrate = 115200;
			
				ret = lnt_init(&config);
				if(ret < 0){
					fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
				}
			#endif
			}

			/* �����⻹����Ϣ(д�뿪����Ϣ)*/
			lib_lnt_rent_info_t rent_info;
			memset(&rent_info, 0, sizeof(lib_lnt_rent_info_t));

			stat = LNT_MOC_ERROR_UNKNOW;
			rent_info.id = 0x68;
			rent_info.length = sizeof(lib_rent_info_t);

			if(on_card_stage2.member_type != 0)
			{
				rent_info.rent.free_time = 2;	//��ͨ�����ʱ��Ϊ0,����ͨ�����ʱ��Ϊ2(2��λ=30����)
			}

			memset(&rent_info.rent.provider, 0x01, 2); //������Ӫ�̱��� add by zjc at 2016-11-22
			rent_info.rent.ticket_flag = on_card_stage2.member_type; //��Ա����
			rent_info.rent.s_un.acc.accum = LNT_NTOHS(on_card_stage2.member_count); //�޶�
			lib_lnt_utils_time_bcd_yymmdd(rent_info.rent.start_time); //����ʱ��
			memcpy(&(rent_info.rent.site_no), &macaddr, 6); //վ����
			
			ret = lib_lnt_set_rent_info(&rent_info, &stat, 500);
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))
			{
				fprintf(stderr,"\nOn Card  Process: [set_rent_info] Stat: %02X\n",stat);

			#if 1 //add by zjc
				//����ͨģ�鴮�ڳ�ʼ��
				memset(&config,0,sizeof(lib_lnt_config_t));
				strcpy((char *)config.tty,"/dev/ttyO1");
				config.baudrate = 115200;
			
				ret = lnt_init(&config);
				if(ret < 0){
					fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
				}
			#endif

				extra.result = 0x02;  //д��ʧ�� add by zjc at 2016-10-31
			}
			
			/* ����д�� */
			memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
			memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
	
			lnt_cse.amount = 0;
			lnt_cse.fee = 0;
			lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
			memcpy(lnt_cse.phyiscalid, SAVE_pid, 8);
			lnt_cse.aty = LNT_ATY_RETURN; //��Ѻ��
			
			stat = LNT_MOC_ERROR_UNKNOW;
			unsigned char f_type = 0;

			ret = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &stat, &f_type);
			if(ret != LNT_EOK)
			{	
				//����д��ʧ�ܺ��ϱ��������	2016-04-26
				if(f_type == LNT_GET_TRANS)	//��ȡ���׼�¼ʧ��
				{
					stat = LNT_MOC_ERROR_UNKNOW;
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 500);  

					extra.result = 0x00; //����д���ɹ�
					extra_result = extra.result; //���濪�����
					taskStep = TASKSTEP_ON_STAGE7TO8;
				}
				else	//����ʧ��
				{
					//printf("-----------------------\n");
					extra.result = 0x02;  //����д��ʧ��
					extra_result = extra.result;	//���濪�����
					taskStep = TASKSTEP_ON_STAGE7TO8;
				}
			}
			
			taskStep = TASKSTEP_ON_STAGE7TO8;
		}

		/* ����7-8�׶� */
		if(taskStep == TASKSTEP_ON_STAGE7TO8)
		{
			lnt_agent_on_card_stage8_STC_t on_card_stage8;

			memset(&retrans, 0, sizeof(agent_retransmission_data_t));
			memset(&on_card_stage8, 0, sizeof(lnt_agent_on_card_stage8_STC_t));
			memset(&extra, 0, sizeof(agent_extra_data_t)); 
			extra.result = extra_result; 
			lib_SN_default_read32(&register_sn);
			extra.local_pkt_sn = register_sn;  //������ˮ��
			//memcpy(&extra.pid, &qry_ticket.ticket.pid, 8);
			memcpy(&extra.pid, &SAVE_pid, 8);	//�ñ����,��ֹ���һ�β�ѯƱ����Ϣʧ�ܵ���ȫΪ0
			memcpy(&extra.macaddr, macaddr, 6);
			
			agent_err = lib_lnt_on_card_stage7TO8_process(sockfd, &extra, &on_card_stage8, &retrans); 
			if(agent_err != AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload On Card Record Failed\n\n", lib_pkt_sn_RO_get());

				/* �ϴ�ʧ�ܣ��������ݿ�,�ش�[2] */
				lib_records_oncard_ack_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);
			}
			else
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload On Card Record OK\n\n", lib_pkt_sn_RO_get());
			}

			emit_TaskRunStep(TaskRunStep_p4);  //��Ա��ͨ�ɹ�[4]
						
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
			bal = qry_ticket.ticket.blance;

			time(&end_times);
			consume_time = end_times - begin_times; //��ʱ����

			/* ���Ӽ��Ѻ���־ʧ�ܻ���д��ʧ�ܵ���ʾ��Ϣ add by zjc at 2016-10-28 */
			if(extra.result == 0xFF) //�޸�Ѻ���־���߼��Ѻ���־ʧ��
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE7TO8;
				sprintf(result->s_info, "��Ա��ͨ�����쳣,���ѯƱ����Ϣ��ȷ�ϻ�Ա��ͨ�Ƿ�ɹ�,�������������˹���ѯ����--������:%02X, �����:%02X", agent_err, extra.result);
				emit signals_Result(result);
			}
			else if(extra.result == 0x02) //������Ϣд��ʧ��
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE7TO8;
				sprintf(result->s_info, "��Ա��ͨд��ʧ��,�밴\"ִ�п�ͨ����\"���в�д��,�������������˹���ѯ����--������:%02X, �����:%02X", agent_err, extra.result);
				emit signals_Result(result);
			}
			else //extra.result = 0x00
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE7TO8;
				sprintf(result->s_info, "��Ա��ͨ�ɹ�,����Ʊ�����Ϊ%d.%02dԪ,ȫ���̺�ʱ%d��,���˿�--������:%02X", bal / 100, bal % 100, consume_time, agent_err);
				emit signals_Result(result);
			}
		}
	}

	

/**********************************************��������*******************************************/
	if(taskdata.serviceType == CARD_SERVICE_OFF)
	{
		if(taskStep == TASKSTEP_QRY_TICKET)
		{
			#if 0	//����ǰ�����ж�Ѻ���ǣ���Ϊ�����ϴ�������������δ��ֵ�����
			/* ����Ƿ��ѿ��� */
			pmflag_t pf;
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
			if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1))
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_PMFLAG;

				sprintf(result->s_info, "�ÿ�δ��ͨ��Ա,����ִ��ע������--������:%02X", TASKSTEP_PMFLAG);
				emit signals_Result(result);
				goto Done;
			}
			#endif
			
			taskStep = TASKSTEP_PMFLAG;
		}
			
		/* ���ӵ��������� */
		if(taskStep == TASKSTEP_PMFLAG)
		{
			sockfd =  lib_lnt_on_card_connect_to_agent();
			if(sockfd <= 0)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_SOCKET;
				sprintf(result->s_info, "ϵͳ����ʧ��,���˿��Ժ�����--������:%02X", TASKSTEP_SOCKET);
				emit signals_Result(result);
				goto Done;
			}

			lib_setfd_noblock(sockfd); //����Ϊ������
			
			taskStep = TASKSTEP_OFF_STAGE1TO2;
		}

		/* ִ������1-2�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE1TO2)
		{
			lnt_agent_off_card_stage1_CTS_t off_card_stage1_CTS;
			lnt_agent_off_card_stage2_STC_t off_card_stage2_STC;

			memset(&rent_info_req, 0, sizeof(lnt_rent_info_req_t));
			memset(&rent_info_ack, 0, sizeof(lnt_rent_info_ack_t));
			memset(&off_card_stage1_CTS, 0, sizeof(lnt_agent_off_card_stage1_CTS_t));
			memset(&off_card_stage2_STC, 0, sizeof(lnt_agent_off_card_stage2_STC_t));

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			rent_info_req.id = 0x68;
			ret = lib_lnt_qry_rent_info(&rent_info_req, &rent_info_ack, &stat, 300);
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
			{
				printf("\n-----------qry_rent_info, stat:%02x, ret:%d-----------\n",stat,ret);
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_QRY_RENT;

				sprintf(result->s_info, "��ȡƱ����Ϣʧ��,���˿��Ժ�����--������:%02X,״̬��:%02X", TASKSTEP_QRY_RENT, stat);
				emit signals_Result(result);
				goto Done;
			}

			//printf("==========rent_flag:%02x\n", rent_info_ack.rent.rent_flag);
			
			if((rent_info_ack.rent.rent_flag & (0x08)) != 0) //���⳵
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_QRY_RENT;

				sprintf(result->s_info, "���Ȼ���,��ִ�л�Աע������--������:%02X,״̬��:%02X", TASKSTEP_QRY_RENT, stat);
				emit signals_Result(result);
				goto Done;
			}

		#if 1
			/* ����Ƿ�Ϊ����ͨ���Լ��Ƿ���δʹ����Ĵ��������������������� */ //2016-04-15 add by zjc  
			if((rent_info_ack.rent.ticket_flag != 0x00) && (LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum) > 0))
			{
				/* ����Ʊ����Ч���ж�  add by zjc at 2016-12-06 */
				ret = lib_lnt_is_valid_card();
				if(ret == LNT_CARD_VALID)
				{
					//printf("----------This Card is Valid!\n");
					
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_QRY_RENT;
					
					result->memberCount = LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum);	//������ʾ�û�����δʹ������Ż�
					//printf("------------memberCount1:%d\n", result->memberCount);
					
					switch(rent_info_ack.rent.ticket_flag)
					{
						case 0x01:
							sprintf(result->s_info, "�ÿ�Ϊ[�ο�],ʣ���Żݴ���[%d]��,ȷ��Ҫע����Ա�밴\"ִ��ע������\"! ��\"�˳�\"������һ������", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;

						case 0x02:
							sprintf(result->s_info, "�ÿ�Ϊ[�쿨],ʣ���Ż�����[%d]��,ȷ��Ҫע����Ա�밴\"ִ��ע������\"! ��\"�˳�\"������һ������", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;

						case 0x03:
							sprintf(result->s_info, "�ÿ�Ϊ[�¿�],ʣ���Ż�����[%d]��,ȷ��Ҫע����Ա�밴\"ִ��ע������\"! ��\"�˳�\"������һ������", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;
	         
						case 0x04:
							sprintf(result->s_info, "�ÿ�Ϊ[�꿨],ʣ���Ż�����[%d]��,ȷ��Ҫע����Ա�밴\"ִ��ע������\"! ��\"�˳�\"������һ������", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;
						
						default:
							sprintf(result->s_info, "�ÿ�����δ������Żݷ���,ȷ��Ҫע����Ա�밴\"ִ��ע������\"! ��\"�˳�\"������һ������");
						break;
					}
					
					#if CONFIRM_BUTTON
					printf("--------taskdata.confirmCnt:%d----------\n",taskdata.confimeCnt);
					if(taskdata.confimeCnt < 2)	//ͣס��ʾ�û�ȷ���Ƿ�����
					{
						result->result = TASK_RESULT_OP_LOOP_FREE; //TASK_RESULT_OP_LOOP;
						result->step = TASKSTEP_QRY_RENT;

						result->memberCount = 0xFFFF;	
						emit signals_Result(result);	
						
						goto Done;
					}
					#endif
				}
				else if(ret == LNT_CARD_UNVALID)
				{
					printf("----------This Card is Unvalid!\n");
				}
			}
		#endif
		
			emit_TaskRunStep(TaskRunStep_p2);  //�����ԱȨ��[2]
			
			memcpy(off_card_stage1_CTS.pid, SAVE_pid, 8); //pid
			lib_get_macaddr("eth1", (char *)&off_card_stage1_CTS.macaddr);  //MAC
			memcpy(macaddr, off_card_stage1_CTS.macaddr, 6);  //����MAC
			memset(off_card_stage1_CTS.pki, 0xff, 4);
			strcpy((char *)off_card_stage1_CTS.id_card, (char *)taskdata.id_card);	//���֤����
			memcpy(&off_card_stage1_CTS.ticket_info, &qry_ticket.ticket, sizeof(struct ticket_info));
			memcpy(&off_card_stage1_CTS.rent_info, &rent_info_ack.rent, sizeof(struct rent_info));
			lib_lnt_utils_time_bcd_yymmddhhMMss(off_card_stage1_CTS.time);  //ʱ��

			agent_err = lib_lnt_off_card_stage1TO2_process(sockfd, &off_card_stage1_CTS, &off_card_stage2_STC, &extra);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_OFF_STAGE1TO2;
				sprintf(result->s_info, "ϵͳ��æ,���˿��Ժ�����--������:%02X,������:%02X", TASKSTEP_OFF_STAGE1TO2, agent_err);
				emit signals_Result(result);
				goto Done;	
			}

			apply_result = off_card_stage2_STC.apply_result; //������

			LIB_LNT_log_vsprintf("****************Begin Off Card Process****************\n");
			LIB_LNT_log_vsprintf("PKT SN:%d, PID: %02x %02x %02x %02x %02x %02x %02x %02x\n", lib_pkt_sn_RO_get(), \
				SAVE_pid[0], SAVE_pid[1], SAVE_pid[2], SAVE_pid[3], SAVE_pid[4], SAVE_pid[5], SAVE_pid[6], SAVE_pid[7]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Apply Result: 0x%02X\n", lib_pkt_sn_RO_get(), apply_result);

			printf("Off Card Apply Result: 0x%02X\n",apply_result);
			
			switch(apply_result) 
			{
				case 0x01: //��������
				{
					#if 0
					//result->step = TASKSTEP_OFF_STAGE1TO2;
					//result->result = TASK_RESULT_OP_FAIL_FREE;
					strcpy(result->s_info, "�ÿ�Ϊ�Ƿ���,����ִ�д���...");
					emit signals_Result(result);  
					#endif

					goto BlackListDeal;  //��������������
				}
				break;
				
				case 0x02: //���֤��ʽ��Ч
				{
					result->step = TASKSTEP_OFF_STAGE1TO2;
					result->result = TASK_RESULT_OP_FAIL_FREE;
					strcpy(result->s_info, "�����[���֤����]��ʽ��Ч,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				case 0x20: //��������
				{
					/*
					 * ������ķ���0x20��������,���Ǹÿ�Ѻ���־��δ��������ֱ������Ѻ����ֱ�ӳ�ֵ
					 * ���������������һ��ԭ�����޸�Ѻ���־��û�а�Ѻ���־����ɹ��ϴ������ģ�����û�м�¼��һֱ��Ϊ�ÿ�û������
					 */
					#if 0 //by zjc at 2016-10-31
					/* ����Ƿ��ѿ��� */
					pmflag_t pf;
					memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
					if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1)) //δ����
					{
						f_operate = 0x20;  //ֱ�ӽ����ֵ�׶�
					}
					#endif
					
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "����ִ�л�Աע������,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......");
					emit signals_Result(result);  
					
					taskStep = TASKSTEP_OFF_STAGE7TO8; //Ѻ���� DD73 DD74
				}
				break;

				case 0x21: //δ����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "û�иÿ���ͨ��¼,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0x22: //���֤δ����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "û�и����֤����Ŀ�����¼,���˿�");
					emit signals_Result(result);  
					goto Done;    
				}
				break;

				case 0x23: //��������δ��ֵ
				{	
					/*
					 * �Ѿ�ȥ��Ѻ���־������û�г�ֵ�ɹ���״̬
					 */
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "�ÿ���ע��,����δ�����û�Ѻ��,���ڻ�ȡ������,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......");
					emit signals_Result(result); 

					taskStep = TASKSTEP_OFF_STAGE5TO6;
				}
				break;

				case 0x24: //ֻ������������ֵ
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "�ÿ�ִֻ�л�Աע������,�����û�Ѻ��������ڵ�����ƽ̨����,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......");
					emit signals_Result(result); 
					
					taskStep = TASKSTEP_OFF_STAGE7TO8;  //ֱ��Ѻ�����
					
				}
				break;

				case 0x14: //���˹�����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "�����˹���ѯ����,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0xFF: //��ͣ����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OUT_OF_SERVICE;
					strcpy(result->s_info, "�豸��ͣ�ù��ܷ���,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				default:  //Ĭ��
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "��Աע��ʧ��,��������쳣,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;
			}
		}

		/* ִ������7-8�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE7TO8)  //Ѻ�����һ�׶�  DD73 DD74
		{	
			emit_TaskRunStep(TaskRunStep_p3);   //ִ�л�Աע��[3]

			/* ����Ѻ���־�ļ�飬���֮ǰ����ֻ�Ǽ��Ѻ���־ʧ�ܵ���ʵ�����Ѿ������ɹ��Ļ��Ϳ�������Ѻ���� add by zjc at 2016-10-28 */
			#if 1
			pmflag_t pf;
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
			
			if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1)) //������
			{		
				LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Deposit Was Processed\n", lib_pkt_sn_RO_get());

				if(apply_result == 0x20) //��������
				{
					taskStep = TASKSTEP_OFF_STAGE5TO6;
					goto OFF_CARD_RECHARGE; //�Ѿ��޸�Ѻ���־��������ֵ����
				}
				else if(apply_result == 0x24) //ֻ��������ֵ
				{
					extra.result = 0x10; //�޸�Ѻ���־�ɹ�(������,�����ֵ)
					taskStep = TASKSTEP_OFF_STAGE9TO10;
					goto UPLOAD_OFF_CARD_RESULT; //�Ѿ��޸�Ѻ���־��������������ϱ�
				}
			}
			#endif
		
			memset(&member_stage2, 0, sizeof(lnt_member_process_stage2_STC_t));
			agent_err = lib_lnt_off_card_stage7TO8_process(sockfd, &extra, &member_stage2); //Ѻ���ʼ�� DD73 DD74
			if(agent_err != AGENT_ERR_OK)
			{
				#if 0
				taskstep_err_code.task_step = TASKSTEP_OFF_STAGE7TO8;
				taskstep_err_code.task_ret = agent_err;
				
				extra.result = 0xFF;  //�޸�Ѻ���־ʧ��
				taskStep = TASKSTEP_OFF_STAGE9TO10;  //������¼�ϴ�(������һ�׶�ʧ�ܹر����׽��֣������޷��ϴ�,�ϴ�ʧ�ܵ�Ĭ��Ϊ����ʧ��)
				#else //by zjc at 2016-10-31
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE7TO8;
				sprintf(result->s_info, "��Աע��ʧ��,���˿�--������:%02X,������:%02X", TASKSTEP_OFF_STAGE7TO8, agent_err);
				emit signals_Result(result);
				goto Done;	
				#endif
			}
			else	
			{
				extra.result = 0x00; 
				taskStep = TASKSTEP_DEPOSIT;
			}
		}

		/* ִ��Ѻ���� */
		if(taskStep == TASKSTEP_DEPOSIT)
		{
			lnt_r_deposit_process_req_t dproc_req;
			lnt_r_deposit_process_ack_t dproc_ack;
			memset(&dproc_req, 0, sizeof(lnt_r_deposit_process_req_t));
			memset(&dproc_ack, 0, sizeof(lnt_r_deposit_process_ack_t));

			stat = LNT_MOC_ERROR_UNKNOW;
			memcpy(&dproc_req.data, &member_stage2.INFO, sizeof(lnt_r_deposit_process_req_t));
			ret = lib_lnt_r_deposit_process(&dproc_req, &dproc_ack, &stat, 600);
			if(ret < 0 || stat != LNT_MOC_ERROR_NORMAL) 
			{
				fprintf(stderr,"\nOff Card  Process: [Deposit Process] Stat: %02X\n",stat);

				#if 1 //add by zjc
				//����ͨģ�鴮�ڳ�ʼ��
				memset(&config,0,sizeof(lib_lnt_config_t));
				strcpy((char *)config.tty,"/dev/ttyO1");
				config.baudrate = 115200;
			
				ret = lnt_init(&config);
				if(ret < 0){
					fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
				}
				#endif

				taskstep_err_code.task_step = TASKSTEP_DEPOSIT;
				taskstep_err_code.task_stat = stat;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;

				extra.result = 0xFF; //����ϱ�  
			}
			
			/* Ѻ������״̬������Ϊ�ж�Ѻ���־�Ƿ�ɹ��ı�׼ */
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			ret = -1;
			
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 500); //300->500 2016-10-27
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))  //���Ӳ�ѯʧ�ܵĴ��� add by zjc at 2016-10-27 
			{
				fprintf(stderr,"\n[qry_ticket_info] Stat: %02X, ret: %d\n", stat, ret);
				
				LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Check Pmflag Failed, Stat: %02X, Ret: %d\n", lib_pkt_sn_RO_get(), stat, ret);
			}
			
			memset(&pf, 0, sizeof(pmflag_t)); 
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);

			//���Ѻ���־ʧ�ܻ���Ѻ����ʧ��
			if((stat != LNT_MOC_ERROR_NORMAL) || ((stat == LNT_MOC_ERROR_NORMAL) && \
				(qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1)))  //Ѻ���־��û���޸ĳɹ�
			{
				taskstep_err_code.task_step = TASKSTEP_DEPOSIT;
				taskstep_err_code.task_stat = stat;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;
				
				extra.result = 0xFF;  //�޸�Ѻ���־ʧ��
			}
			//��Э�����������ϱ������Ƿ���Ҫ��ֵ 2016-03-04
			else if(apply_result == 0x20){ //��������
				extra.result = 0x00; //�޸�Ѻ���־�ɹ�(������,�����ֵ)
			}
			else if(apply_result == 0x24){ //0x24:ֻ������������ֵ
				extra.result = 0x10; //�޸�Ѻ���־�ɹ�(������,�����ֵ)
			}

			if(extra.result != 0xFF){ //�޸�Ѻ���־�ɹ�
				LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Deposit Process OK\n", lib_pkt_sn_RO_get());
			}
			
			taskStep = TASKSTEP_OFF_STAGE9TO10;
		}

UPLOAD_OFF_CARD_RESULT:
		/* 
		 * ִ������9-10�׶� : ��һ�׶���Ѻ���־�޸ı����ϱ��׶�
		 */
		if(taskStep == TASKSTEP_OFF_STAGE9TO10) //������¼�ϴ�
		{
			lnt_agent_off_card_stage10_STC_t off_card_stage10_STC;  
			memset(&off_card_stage10_STC, 0, sizeof(lnt_agent_off_card_stage10_STC_t));
			memset(&retrans, 0, sizeof(agent_retransmission_data_t));

			agent_err = lib_lnt_off_card_stage9TO10_process(sockfd, &extra, &off_card_stage10_STC, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload Off Card Deposit Record Failed\n", lib_pkt_sn_RO_get());

				/* �ϴ�ʧ�ܣ��������ݿ�,�ش� [3]*/
				lib_records_offcard_ack_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);

				#if 1 
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE9TO10;
				sprintf(result->s_info, "��Աע��ʧ��,���˿�--������:%02X,������:%02X", TASKSTEP_OFF_STAGE9TO10, agent_err);
				emit signals_Result(result);
				goto Done;	
				#endif
			}
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Upload Off Card Deposit Record OK\n", lib_pkt_sn_RO_get());

			if(taskstep_err_code.task_ret != AGENT_ERR_OK) //ǰ������޸�Ѻ���־ʧ�����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE9TO10;
				sprintf(result->s_info, "��Աע������ʧ��,�糢�Զ�κ�,��Ȼʧ��,�����˹���ѯ����--������:%02X, ������:%02X, ״̬��:%02X", taskstep_err_code.task_step, agent_err, taskstep_err_code.task_stat);
				emit signals_Result(result);
				goto Done;
			}
			
			if(apply_result == 0x24) //ֻ������������ֵ
			{
				time(&end_times);
				consume_time = end_times - begin_times;
			
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE9TO10;
				sprintf(result->s_info, "��Աע���ɹ�,���̺�ʱΪ%d��,���˿�,������:%02X", consume_time, agent_err);
				emit signals_Result(result);
				goto Done;
			}
			else if((apply_result == 0x20) || (apply_result == 0x23)) //������������������δ��ֵ
			{
				taskStep = TASKSTEP_OFF_STAGE5TO6;	//��ȡ������
			}
		}

OFF_CARD_RECHARGE: //�����ĳ�ֵ����
		/* ִ������5-6�׶� */ 
		if(taskStep == TASKSTEP_OFF_STAGE5TO6) //��ȡ��ֵ������
		{
			memset(&off_card_stage6_STC, 0, sizeof(lnt_agent_off_card_stage6_STC_t));
			agent_err = lib_lnt_off_card_stage5TO6_process(sockfd, &extra, &off_card_stage6_STC);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE5TO6;
				sprintf(result->s_info, "��ȡ��ֵ������ʧ��,�糢�Զ�κ�,��Ȼʧ��,�����˹���ѯ����--������:%02X,������:%02X", TASKSTEP_OFF_STAGE5TO6, agent_err);
				emit signals_Result(result);
				goto Done;	
			}
			//lib_printf_v2("--------recharge order---------",(unsigned char *)&off_card_stage6_STC.order,sizeof(off_card_stage6_STC.order),16);
			//lib_printf_v2("--------recharge amount---------",(unsigned char *)&off_card_stage6_STC.recharge,sizeof(off_card_stage6_STC.recharge),16);

			#if 1
			/* ����ֵ���� */
			unsigned int amount = LNT_NTOHL(off_card_stage6_STC.recharge); //Ҫ��ֵ���
			if((ticket_blance + amount) > LNT_CPU_RECHARGE_LIMIT)
			{
				result->step = TASKSTEP_OFF_STAGE5TO6;
				result->result = TASK_RESULT_OP_FAIL_FREE;
				sprintf(result->s_info, "�ÿ���ֵ%02d.%02dԪ��ᳬ��%d.%02dԪ�������,���������û�Ѻ�����,�����˹���ѯ����,���˿�", amount / 100, amount % 100, 
						LNT_CPU_RECHARGE_LIMIT / 100, LNT_CPU_RECHARGE_LIMIT % 100);
				emit signals_Result(result);  
				goto Done;
			}
			#endif		
				
			taskStep = TASKSTEP_OFF_STAGE11TO12; //��ȡ�����ɹ���ִ�г�ֵ

			#if 0 //by zjc at 2016-10-31
			if(f_operate == 0x20) //��������δ��ֵ--�޸�Ѻ���־�ɹ�������ֵʧ�ܵ������2��
				taskStep = TASKSTEP_OFF_STAGE11TO12; //ִ�г�ֵ
			#endif
		}

		/* ִ������11-12�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE11TO12) //DD11 ��ֵ��һ�׶�
		{
			emit_TaskRunStep(TaskRunStep_p4);   //�����û�Ѻ��[4]
			/* ��Ϣ��ѯ */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);	
			if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
			{
				if(memcmp(SAVE_pid, qry_ticket.ticket.pid, 8) != 0) //���Ų�����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE11TO12;
					sprintf(result->s_info, "��Աע��ʧ��,���ֿ��Ų�һ��,���˿�");
					emit signals_Result(result);
					goto Done;
				}
			}

			bal = LNT_NTOHL(off_card_stage6_STC.recharge);
				
			result->result = TASK_RESULT_OP_LOOP;
			result->step = TASKSTEP_OFF_STAGE11TO12;
			sprintf(result->s_info, "����ִ�з����û�Ѻ�����,����Ѻ����Ϊ%d.%02dԪ,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......", bal / 100, bal % 100);
			emit signals_Result(result);

			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Recharge Order: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", lib_pkt_sn_RO_get(), off_card_stage6_STC.order[0],off_card_stage6_STC.order[1],\
				off_card_stage6_STC.order[2],off_card_stage6_STC.order[3],off_card_stage6_STC.order[4],off_card_stage6_STC.order[5],off_card_stage6_STC.order[6],\
				off_card_stage6_STC.order[7],off_card_stage6_STC.order[8],off_card_stage6_STC.order[9]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Recharge Fee %d.%02d(Y)\n", lib_pkt_sn_RO_get(),  bal / 100, bal % 100);
	
			recharge_info_t recharge;
			memset(&recharge_stage2, 0, sizeof(lnt_packet_recharge_stage2_STC_t));
			recharge.apply_type = 0x00; //��Э��ֻ��0x00  2016-03-04

			memcpy(&recharge.recharge_amount, &off_card_stage6_STC.recharge, 4); //��ֵ��� [��λ��ǰ]
			memcpy(&recharge.order_num, &off_card_stage6_STC.order, 10); //������ 

			agent_err = lib_lnt_off_card_stage11TO12_process(sockfd, &extra, &recharge,  &recharge_stage2);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE11TO12;
				sprintf(result->s_info, "��Աע��ʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,������:%02X", TASKSTEP_OFF_STAGE11TO12, agent_err);
				emit signals_Result(result);
				goto Done;	
			}     

			if(recharge_stage2.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE13TO14;
				
				if(recharge_stage2.retcode == 0xDD)
				{
					sprintf(result->s_info, "ϵͳ��æ���»�Աע��ʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage2.retcode);
				}else{
					sprintf(result->s_info, "��Աע��ʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage2.retcode);
				}
				
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_OFF_STAGE13TO14;
		}
	
		/* ִ������13-14�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE13TO14)
		{
			memset(&recharge_stage4, 0, sizeof(lnt_packet_recharge_stage4_STC_t));
			
			agent_err = lib_lnt_off_card_stage13TO14_process(sockfd, &extra, &recharge_stage2, &recharge_stage4);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE13TO14;
				sprintf(result->s_info, "��Աע��ʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,������:%02X,״̬��:%02X", TASKSTEP_OFF_STAGE13TO14, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage4.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE13TO14;
				sprintf(result->s_info, "��Աע��ʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage4.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_OFF_STAGE15TO16;
		}
		
		/* ִ������15-16�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE15TO16)
		{
			memset(&recharge_stage6, 0, sizeof(lnt_packet_recharge_stage6_STC_t));
			
			agent_err = lib_lnt_off_card_stage15TO16_process(sockfd, &extra, &recharge_stage4, &recharge_stage6);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE15TO16;
				sprintf(result->s_info, "��Աע��ʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,������:%02X,״̬��:%02X", TASKSTEP_OFF_STAGE15TO16, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage6.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE15TO16;
				sprintf(result->s_info, "��Աע��ʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_OFF_STAGE15TO16, recharge_stage6.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_OFF_STAGE17TO18;
		}
		
		/* 
		 * ִ������17-18�׶� ����ͨ��̨����DD18���ĺ�,����Ϊ��ֵ�Ѿ��ɹ�
		 */
		if(taskStep == TASKSTEP_OFF_STAGE17TO18)
		{
			memset(&recharge_stage8, 0, sizeof(lnt_packet_recharge_stage8_STC_t));
			
			agent_err = lib_lnt_off_card_stage17TO18_process(sockfd, &extra, &recharge_stage6, &recharge_stage8);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE17TO18;
				sprintf(result->s_info, "�����û�Ѻ��ʧ��,���鿨�����,�������������˹���ѯ����,���˿�--������:%02X,������:%02X,״̬��:%02X", TASKSTEP_OFF_STAGE17TO18, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage8.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE17TO18;
				sprintf(result->s_info, "�����û�Ѻ��ʧ��,���鿨�����,�������������˹���ѯ����,���˿�--������:%02X,��Ϣ��:%02X", TASKSTEP_OFF_STAGE17TO18, recharge_stage8.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Off Card Process: [DD18] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			taskStep = TASKSTEP_OFF_STAGE19TO20;
		}

		/* ִ������19-20�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE19TO20)
		{
			extra.result = 0x00; 
			
			memset(&taskstep_err_code, 0, sizeof(struct task_err_code));
			memset(&recharge_stage10, 0, sizeof(lnt_packet_recharge_stage10_STC_t));

			/* ����R_CPU_LOAD*/
			agent_err = lib_lnt_off_card_stage19TO20_process(sockfd, &extra, &recharge_stage8, &recharge_stage10, &r_cpu_load); //CPUȦ��
			if(r_cpu_load != 0x00)  //˵����ֵд��ʧ��
			{
				taskstep_err_code.task_stat = extra.r_stat;
				extra.result = 0xFF;
				taskstep_err_code.task_step = TASKSTEP_OFF_STAGE19TO20;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;

				taskStep = TASKSTEP_OFF_STAGE23TO24;
				goto DONE_TASKSTEP_OFF_STAGE23TO24;
			}
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Process: [R_CPU_LOAD] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD�Ѿ��ɹ���˵����ֵ�Ѿ��ɹ�
			{
				extra.result = 0x00;
				taskstep_err_code.task_ret = AGENT_ERR_OK;

				taskStep = TASKSTEP_OFF_STAGE23TO24;
				goto DONE_TASKSTEP_OFF_STAGE23TO24;
			}

			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Off Card Process: [DD20] Ok\n", lib_pkt_sn_RO_get());//add by zjc
			
			taskStep = TASKSTEP_OFF_STAGE21TO22;
		}

		/* ִ������21-22�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE21TO22)
		{	
			memset(&recharge_stage12, 0, sizeof(lnt_packet_recharge_stage12_STC_t));
			
			agent_err = lib_lnt_off_card_stage21TO22_process(sockfd, &extra, &recharge_stage10, &recharge_stage12);
			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD�Ѿ��ɹ���˵����ֵ�Ѿ��ɹ�
			{
				extra.result = 0x00;
				taskstep_err_code.task_ret = AGENT_ERR_OK;

				taskStep = TASKSTEP_OFF_STAGE23TO24;
				goto DONE_TASKSTEP_OFF_STAGE23TO24;
			}
			
			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Off Card Process: [DD22] Ok\n", lib_pkt_sn_RO_get());//add by zjc
			
			taskStep = TASKSTEP_OFF_STAGE23TO24;
			
		}

DONE_TASKSTEP_OFF_STAGE23TO24:
		/* ִ������23-24�׶� */
		if(taskStep == TASKSTEP_OFF_STAGE23TO24)
		{
			/*�ϱ���ֵ�����źͳ�ֵ��� zjc,2016-03-24*/
			memcpy(&extra.order, &off_card_stage6_STC.order, 10); //������ 
			memcpy(&extra.recharge, &off_card_stage6_STC.recharge, 4); //��ֵ��� [��λ��ǰ]

			lnt_agent_off_card_stage24_STC_t off_card_stage24_STC;
			
			memset(&off_card_stage24_STC, 0, sizeof(lnt_agent_off_card_stage24_STC_t));
			memset(&retrans, 0, sizeof(agent_retransmission_data_t));

			agent_err = lib_lnt_off_card_stage23TO24_process(sockfd, &extra, &off_card_stage24_STC, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				/* ��ֵ����ϴ�ʧ�ܣ��������ݿ�,�ش� [4]*/
				lib_records_offcard_ack_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);
			}

			if(agent_err == AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload Off Card Recharge Record OK\n\n", lib_pkt_sn_RO_get());
			}

			if(taskstep_err_code.task_ret != AGENT_ERR_OK) 
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE23TO24;
				sprintf(result->s_info, "�����û�Ѻ��ʧ��,���鿨�����,�������������˹���ѯ����,���˿�--������:%02X, ״̬��:%02X", taskstep_err_code.task_step, taskstep_err_code.task_stat);
				emit signals_Result(result);
				goto Done;
			}

			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_qry_ticket_info(&qry_ticket, &stat, 400);
			bal = qry_ticket.ticket.blance;

			emit_TaskRunStep(TaskRunStep_p5);   //ע��ע���ɹ�[5]
			
			time(&end_times);
			consume_time = end_times - begin_times;

			/* ���Ӽ��Ѻ���־ʧ�ܡ�д��ʧ�ܻ��߳�ֵʧ�ܵ���ʾ��Ϣ add by zjc at 2016-10-28 */
			if(extra.result == 0xFF) //�޸�Ѻ���־���߼��Ѻ���־ʧ��
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_OFF_STAGE23TO24;
				sprintf(result->s_info, "��Աע�������쳣,���ѯƱ����Ϣ��ȷ�ϻ�Աע���Ƿ�ɹ�,�������������˹���ѯ����--������:%02X, �����:%02X", agent_err, extra.result);
				emit signals_Result(result);
			}
			else 
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_OFF_STAGE23TO24;
				sprintf(result->s_info, "��Աע���ɹ�,����Ʊ�����Ϊ%d.%02dԪ,ȫ���̺�ʱΪ%d��,���˿�,������:%02X", bal / 100, bal % 100, consume_time, agent_err);
				emit signals_Result(result);
			}
		}
	}


	/**********************************************��ֵ����*******************************************/
	if(taskdata.serviceType == CARD_SERVICE_RECHARGE) //��ֵ
	{
		/* ���ӵ��������� */
		if(taskStep == TASKSTEP_QRY_TICKET)
		{
			sockfd =  lib_lnt_on_card_connect_to_agent();
			if(sockfd <= 0)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_SOCKET;
				sprintf(result->s_info,"ϵͳ����ʧ��,���˿��Ժ�����--������:%02X", TASKSTEP_SOCKET);
				emit signals_Result(result);
				goto Done;
			}

			lib_setfd_noblock(sockfd); //����Ϊ������
			
			taskStep = TASKSTEP_RECHARGE_STAGE1TO2;
		}
		
		//emit_TaskRunStep(TaskRunStep_p1);  //�����Ա��Ƭ����[1]
		
		/* ִ�г�ֵ1-2�׶� */
		if(taskStep == TASKSTEP_RECHARGE_STAGE1TO2)
		{
			lnt_agent_recharge_stage1_CTS_t recharge_stage1_CTS;
			lnt_agent_recharge_stage2_STC_t recharge_stage2_STC;
		
			memset(&recharge_stage1_CTS, 0, sizeof(lnt_agent_recharge_stage1_CTS_t));
			memset(&recharge_stage2_STC, 0, sizeof(lnt_agent_recharge_stage2_STC_t));
		
			memcpy(recharge_stage1_CTS.pid, SAVE_pid, 8); //pid
			lib_get_macaddr("eth1", (char *)&recharge_stage1_CTS.macaddr);	//MAC
			memcpy(macaddr, recharge_stage1_CTS.macaddr, 6);  //����MAC
			memset(recharge_stage1_CTS.pki, 0xff, 4);
			memcpy(&recharge_stage1_CTS.ticket_info, &qry_ticket.ticket, sizeof(struct ticket_info));
			memcpy(&recharge_stage1_CTS.order, &taskdata.order_number, 10);	//��ֵ������,�û�����
			lib_lnt_utils_time_bcd_yymmddhhMMss(recharge_stage1_CTS.time);	//ʱ��
		
			agent_err = lib_lnt_recharge_stage1TO2_process(sockfd, &recharge_stage1_CTS, &recharge_stage2_STC, &extra);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE1TO2;
				sprintf(result->s_info,"ϵͳ��æ,���˿��Ժ�����--������:%02X,������:%02X", TASKSTEP_RECHARGE_STAGE1TO2, agent_err);
				emit signals_Result(result);
				goto Done;	
			}

			apply_result = recharge_stage2_STC.apply_result; //������
			printf("Recharge Apply Result: 0x%02X\n\n",apply_result);
			
			LIB_LNT_log_vsprintf("****************Begin Recharge Process****************\n");
			LIB_LNT_log_vsprintf("PKT SN:%d, PID: %02x %02x %02x %02x %02x %02x %02x %02x\n", lib_pkt_sn_RO_get(), \
				SAVE_pid[0], SAVE_pid[1], SAVE_pid[2], SAVE_pid[3], SAVE_pid[4], SAVE_pid[5], SAVE_pid[6], SAVE_pid[7]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Apply Result: 0x%02X\n", lib_pkt_sn_RO_get(), apply_result);

			lib_printf("Input OrderNum",recharge_stage1_CTS.order,10);

			switch(apply_result)     
			{				      
				case 0x10: //�����ֵ
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "���ڻ�ȡ��ֵ������,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......");
					emit signals_Result(result);  
					
					taskStep = TASKSTEP_RECHARGE_STAGE3TO4;
				}
				break;

				case 0x11: //��Ӧ�����Ų�����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "������ĳ�ֵ�����Ų�����,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				case 0x12: //��Ӧ��������ʧЧ(�Ѿ���ֵ��)
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "������ĳ�ֵ��������ʧЧ,��������Ч�ĳ�ֵ��������ִ�г�ֵ����");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				case 0x13: //��ֵ��Ȳ���
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "��ֵ��Ȳ���,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0x14: //���˹�����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "�����˹���ѯ����,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0xFF: //��ͣ����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OUT_OF_SERVICE;
					strcpy(result->s_info, "�豸��ͣ�ù��ܷ���,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				default:  //Ĭ��
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "��Ա��ֵʧ��,��������쳣,���˿�");
					emit signals_Result(result);  
					goto Done;
				}
				break;
			}
		}

		emit_TaskRunStep(TaskRunStep_p2);  //�����ԱȨ��[2]
		
		/* ִ�г�ֵ3-4�׶� */ //��ȡ��ֵ������
		if(taskStep == TASKSTEP_RECHARGE_STAGE3TO4)
		{
			memset(&recharge_stage4, 0, sizeof(lnt_agent_recharge_stage4_STC_t));
			
			agent_err = lib_lnt_recharge_stage3TO4_process(sockfd, &extra, &recharge_stage4_STC);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE3TO4;
				sprintf(result->s_info,"��ȡ��ֵ������ʧ��,�糢�Զ�κ�,��Ȼʧ��,�����˹���ѯ����--������:%02X,������:%02X", TASKSTEP_RECHARGE_STAGE3TO4, agent_err);
				emit signals_Result(result);
				goto Done;	
			}
			
			//lib_printf_v2("--------recharge order---------",(unsigned char *)&recharge_stage4_STC.order,sizeof(recharge_stage4_STC.order),16);
			//lib_printf_v2("--------recharge amount---------",(unsigned char *)&recharge_stage4_STC.recharge,sizeof(recharge_stage4_STC.recharge),16);

			#if 1
			/* ����ֵ���� */
			unsigned int amount = LNT_NTOHL(recharge_stage4_STC.recharge); //Ҫ��ֵ���
			if((ticket_blance + amount) > LNT_CPU_RECHARGE_LIMIT)
			{
				result->step = TASKSTEP_RECHARGE_STAGE3TO4;
				result->result = TASK_RESULT_OP_FAIL_FREE;
				sprintf(result->s_info,"�ÿ���ֵ%02d.%02dԪ��ᳬ��%d.%02dԪ�������,�������Ա��ֵ����,��������,�����˹���ѯ����,���˿�", amount / 100, amount % 100, 
								LNT_CPU_RECHARGE_LIMIT / 100, LNT_CPU_RECHARGE_LIMIT % 100);
				emit signals_Result(result);
				goto Done;
			}
			#endif		
			#if 1
			sprintf(result->s_info,"�ó�ֵ�����Ŷ�Ӧ�ĳ�ֵ���Ϊ%02d.%02dԪ,����ִ�г�ֵ����,�����ĵȴ���ֵ���......", amount / 100, amount % 100);
			emit signals_Result(result);
			#endif
			taskStep = TASKSTEP_RECHARGE_STAGE5TO6; //��ȡ�����ɹ���ִ�г�ֵ
		}

		/* ִ�г�ֵ5-6�׶� */
		if(taskStep == TASKSTEP_RECHARGE_STAGE5TO6) //DD11 ��ֵ��һ�׶�
		{
			emit_TaskRunStep(TaskRunStep_p3);   //��ʼ��Ա��ֵ[3]
			
			/* ��Ϣ��ѯ */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 500);	
			if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
			{
				if(memcmp(SAVE_pid, qry_ticket.ticket.pid, 8) != 0) //���Ų�����
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE5TO6;
					sprintf(result->s_info,"��Ա��ֵʧ��,���ֿ��Ų�һ��,���˿�");
					emit signals_Result(result);
					goto Done;
				}
			}

			bal = LNT_NTOHL(recharge_stage4_STC.recharge);
				
			result->result = TASK_RESULT_OP_LOOP;
			result->step = TASKSTEP_RECHARGE_STAGE5TO6;
			sprintf(result->s_info,"����ִ�л�Ա��ֵ����,��ֵ���Ϊ%d.%02dԪ,�벻Ҫ�ƶ���Ƭ,�����ĵȴ�......", bal / 100, bal % 100);
			emit signals_Result(result);
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Order: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", lib_pkt_sn_RO_get(), recharge_stage4_STC.order[0],recharge_stage4_STC.order[1],\
				recharge_stage4_STC.order[2],recharge_stage4_STC.order[3],recharge_stage4_STC.order[4],recharge_stage4_STC.order[5],recharge_stage4_STC.order[6],\
				recharge_stage4_STC.order[7],recharge_stage4_STC.order[8],recharge_stage4_STC.order[9]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Fee %d.%02d(Y)\n", lib_pkt_sn_RO_get(),  bal / 100, bal % 100);

			recharge_info_t recharge;
			memset(&recharge_stage2, 0, sizeof(lnt_packet_recharge_stage2_STC_t));
			recharge.apply_type = 0x00; //��Э��ֻ��0x00  2016-03-04

			memcpy(&recharge.recharge_amount, &recharge_stage4_STC.recharge, 4); //��ֵ��� [��λ��ǰ]
			memcpy(&recharge.order_num, &recharge_stage4_STC.order, 10); //������ 

			agent_err = lib_lnt_recharge_stage5TO6_process(sockfd, &extra, &recharge, &recharge_stage2);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE5TO6;
				sprintf(result->s_info,"��Ա��ֵʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,������:%02X", TASKSTEP_RECHARGE_STAGE5TO6, agent_err);
				emit signals_Result(result);
				goto Done;	
			}     

			if(recharge_stage2.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE5TO6;

				if(recharge_stage2.retcode == 0xDD)
				{
					sprintf(result->s_info, "ϵͳ��æ���»�Ա��ֵʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage2.retcode);
				}else{
					sprintf(result->s_info,"��Ա��ֵʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_RECHARGE_STAGE5TO6, recharge_stage2.retcode);
				}
				
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_RECHARGE_STAGE7TO8;
		}
		
		/* ִ�г�ֵ7-8�׶� */
		if(taskStep == TASKSTEP_RECHARGE_STAGE7TO8)
		{
			memset(&recharge_stage4, 0, sizeof(lnt_packet_recharge_stage4_STC_t));
			
			agent_err = lib_lnt_recharge_stage7TO8_process(sockfd, &extra, &recharge_stage2, &recharge_stage4);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE7TO8;
				sprintf(result->s_info,"��Ա��ֵʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,������:%02X,״̬��:%02X", TASKSTEP_RECHARGE_STAGE7TO8, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage4.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE7TO8;
				sprintf(result->s_info,"��Ա��ֵʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_RECHARGE_STAGE7TO8, recharge_stage4.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_RECHARGE_STAGE9TO10;
		}
			
		/* ִ�г�ֵ9-10�׶� */
		if(taskStep == TASKSTEP_RECHARGE_STAGE9TO10)
		{
			memset(&recharge_stage6, 0, sizeof(lnt_packet_recharge_stage6_STC_t));
			
			agent_err = lib_lnt_recharge_stage9TO10_process(sockfd, &extra, &recharge_stage4, &recharge_stage6);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE9TO10;
				sprintf(result->s_info,"��Ա��ֵʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,������:%02X,״̬��:%02X", TASKSTEP_RECHARGE_STAGE9TO10, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage6.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE9TO10;
				sprintf(result->s_info,"��Ա��ֵʧ��,�糢�Զ�κ���ʧ��,���ҿͷ�����--������:%02X,��Ϣ��:%02X", TASKSTEP_RECHARGE_STAGE9TO10, recharge_stage6.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_RECHARGE_STAGE11TO12;
		}
		
		/* 
		 * ִ�г�ֵ11-12�׶� ����ͨ��̨����DD18���ĺ�,����Ϊ��ֵ�Ѿ��ɹ�
		 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE11TO12)
		{
			memset(&recharge_stage8, 0, sizeof(lnt_packet_recharge_stage8_STC_t));
			
			agent_err = lib_lnt_recharge_stage11TO12_process(sockfd, &extra, &recharge_stage6, &recharge_stage8);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE11TO12;
				sprintf(result->s_info,"��Ա��ֵʧ��,���鿨�����,�������������˹���ѯ����,���˿�--������:%02X,������:%02X,״̬��:%02X", TASKSTEP_RECHARGE_STAGE11TO12, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage8.retcode != 0x00) //0x00����
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE11TO12;
				sprintf(result->s_info,"��Ա��ֵʧ��,���鿨�����,�������������˹���ѯ����,���˿�--������:%02X,��Ϣ��:%02X", TASKSTEP_RECHARGE_STAGE11TO12, recharge_stage8.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Process: [DD18] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			taskStep = TASKSTEP_RECHARGE_STAGE13TO14;
		}

		/* ִ�г�ֵ13-14�׶� */
		if(taskStep == TASKSTEP_RECHARGE_STAGE13TO14)
		{
			extra.result = 0x00; 
			
			memset(&taskstep_err_code, 0, sizeof(struct task_err_code));
			memset(&recharge_stage10, 0, sizeof(lnt_packet_recharge_stage10_STC_t));

			/* ����R_CPU_LOAD*/
			agent_err = lib_lnt_recharge_stage13TO14_process(sockfd, &extra, &recharge_stage8, &recharge_stage10, &r_cpu_load); //CPUȦ��
			if(r_cpu_load != 0x00)  //˵����ֵд��ʧ��
			{
				taskstep_err_code.task_stat = extra.r_stat;
				extra.result = 0xff;
				taskstep_err_code.task_step = TASKSTEP_RECHARGE_STAGE13TO14;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;

				taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
				goto DONE_TASKSTEP_RECHARGE_STAGE17TO18;	//�ϱ���ֵ���
			}
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Process: [R_CPU_LOAD] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD�Ѿ��ɹ���˵����ֵ�Ѿ��ɹ�
			{
				extra.result = 0x00;
				taskstep_err_code.task_ret = AGENT_ERR_OK;

				taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
				goto DONE_TASKSTEP_RECHARGE_STAGE17TO18;	//�ϱ���ֵ���
			}

			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Recharge Process: [DD20] Ok\n", lib_pkt_sn_RO_get());//add by zjc
						
			taskStep = TASKSTEP_RECHARGE_STAGE15TO16;
		}

		/* ִ�г�ֵ15-16�׶� */
		if(taskStep == TASKSTEP_RECHARGE_STAGE15TO16)
		{	
			memset(&recharge_stage12, 0, sizeof(lnt_packet_recharge_stage12_STC_t));
			
			agent_err = lib_lnt_recharge_stage15TO16_process(sockfd, &extra, &recharge_stage10, &recharge_stage12);
			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD�Ѿ��ɹ���˵����ֵ�Ѿ��ɹ�
			{
				extra.result = 0x00;
				taskstep_err_code.task_ret = AGENT_ERR_OK;

				taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
				goto DONE_TASKSTEP_RECHARGE_STAGE17TO18;	//�ϱ���ֵ���
			}
			
			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Recharge Process: [DD22] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			
			taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
		}

	DONE_TASKSTEP_RECHARGE_STAGE17TO18:
		/* ִ�г�ֵ17-18�׶� */
		if(taskStep == TASKSTEP_RECHARGE_STAGE17TO18)
		{
			/*�ϱ���ֵ�����źͳ�ֵ��� zjc,2016-03-24*/
			memcpy(&extra.order, &recharge_stage4_STC.order, 10); //������ 
			memcpy(&extra.recharge, &recharge_stage4_STC.recharge, 4); //��ֵ��� [��λ��ǰ]

			lnt_agent_off_card_stage24_STC_t off_card_stage24_STC;
			memset(&off_card_stage24_STC, 0, sizeof(lnt_agent_off_card_stage24_STC_t));
			memset(&retrans, 0, sizeof(agent_retransmission_data_t));

			agent_err = lib_lnt_recharge_stage17TO18_process(sockfd, &extra, &off_card_stage24_STC, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				/* ��ֵ����ϴ�ʧ�ܣ��������ݿ�,�ش� [4]*/
				lib_records_recharge_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);
			}

			if(agent_err == AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload Recharge Record OK\n\n", lib_pkt_sn_RO_get());
			}

			if(taskstep_err_code.task_ret != AGENT_ERR_OK) 
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE17TO18;
				sprintf(result->s_info,"��Ա��ֵʧ��,���鿨�����,�������������˹���ѯ����,���˿�--������:%02X, ״̬��:%02X", taskstep_err_code.task_step, taskstep_err_code.task_stat);
				emit signals_Result(result);
				goto Done;
			}

			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
			bal = qry_ticket.ticket.blance;


			emit_TaskRunStep(TaskRunStep_p4);	//ע���ֵ�ɹ�[4]

			time(&end_times);
			consume_time = end_times - begin_times;
			
			result->result = TASK_RESULT_OP_LOOP_FREE;
			result->step = TASKSTEP_RECHARGE_STAGE17TO18;
			;(result->s_info,"��Ա��ֵ�ɹ�,����Ʊ�����Ϊ%d.%02dԪ,ȫ���̺�ʱΪ%d��,���˿�,������:%02X", bal / 100, bal % 100, consume_time, agent_err);
			emit signals_Result(result);
		}
	}


Done:
	LIB_LNT_log_close();
	if(sockfd > 0)
	{
		lib_lnt_on_card_close_to_agent(sockfd);  //�ر��׽���
	}
	
	return;  //�������������,ִ����Ϻ�ֱ�ӷ���
		
BlackListDeal:  //����������
		memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
		stat = LNT_MOC_ERROR_CARD_OP_FAIL;
		
		ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
		if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
		{
			lib_lnt_blacklist_record_ack_t blacklist_rec;
			memset(&blacklist_rec, 0, sizeof(lib_lnt_blacklist_record_ack_t));
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;

			memcpy(&(blacklist_rec.blacklist.lid), &(qry_ticket.ticket.lid), 8);  //�߼�����
			memcpy(&(blacklist_rec.blacklist.pid), &(qry_ticket.ticket.pid), 8);  //������
			lib_lnt_set_blacklist_flag(&blacklist_rec, &stat, 500);	//�úڲ���
		}
		
		memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
		stat = LNT_MOC_ERROR_CARD_OP_FAIL;
		
		ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
		if((ret > 0) && (stat == LNT_MOC_ERROR_CARD_NAMELIST))    //��������
		{
			if(taskdata.serviceType == CARD_SERVICE_ON)
			{
				memcpy(&(extra.pid), &(qry_ticket.ticket.pid), 8);	//������
				memcpy(&(extra.lid), &(qry_ticket.ticket.lid), 8);  //�߼�����
				
				lib_lnt_on_card_blacklist_process(sockfd, &extra);	//�ϱ�������������
			}
			else if(taskdata.serviceType == CARD_SERVICE_OFF)
			{
				memcpy(&(extra.pid), &(qry_ticket.ticket.pid), 8);	//������
				memcpy(&(extra.lid), &(qry_ticket.ticket.lid), 8);  //�߼�����
				
				lib_lnt_off_card_blacklist_process(sockfd, &extra);	//�ϱ�������������
			}
		}
			
		time(&end_times);
		consume_time = end_times - begin_times;
	#if 0
		result->result = TASK_RESULT_OP_LOOP_FREE;
		sprintf(result->s_info, "������������ɹ�,ȫ���̺�ʱΪ%d��,���˿�", consume_time);
		emit signals_Result(result);
	#endif
		LIB_LNT_log_close();
		if(sockfd > 0)
		{
			lib_lnt_on_card_close_to_agent(sockfd);  //�ر��׽���
		}
}


