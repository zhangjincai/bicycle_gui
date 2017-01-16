#include <QDebug>
#include "lib_general.h"
#include "lib_serialnumber.h"
#include "on_off_card_task.h"
#include "lib_records.h"
#include "lib_lnt.h"
#include "config.h"


#define CONFIRM_BUTTON	1	//销卡提示按确认键

#define LNT_CPU_RECHARGE_LIMIT				(100000) //单位:分


extern lib_records_file_t *g_rec_file;


struct task_err_code
{
	unsigned short task_step;  //错误码
	unsigned char task_stat;  //状态码
	unsigned char task_info; //信息码
	unsigned char task_ret;  //返回码
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
	unsigned char SAVE_pid[8] = {0};  //保存的物理卡号
	unsigned char macaddr[6] = {0};  //MAC地址 
	lnt_qry_ticket_info_ack_t qry_ticket;  
	lnt_rent_info_req_t rent_info_req;
	lnt_rent_info_ack_t rent_info_ack;
	enum AGENT_ERR agent_err = AGENT_ERR_UNKNOWN;
	unsigned int fee = 0;
	pmflag_t pf;
	lib_lnt_consume_req_t lnt_cse;
	agent_retransmission_data_t retrans;
	lib_lnt_trade_record_ack_t lnt_trade_rec;
	agent_extra_data_t extra;  //只能memset初始化一次
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

	unsigned char extra_result;	//保存开卡结果用
	
	
	/* 开始执行操作 */
	if(taskStep == TASKSTEP_INIT)
	{
		LIB_LNT_log_open();
		
		time(&begin_times);

		emit_TaskRunStep(TaskRunStep_p1);  //查验会员卡片资料[1]

		result->result = TASK_RESULT_OP_LOOP;
		result->step = TASKSTEP_INIT;
		strcpy(result->s_info, "正在查验卡片资料,请稍等......");
		emit signals_Result(result);

		taskStep = TASKSTEP_START;
	}
	
	/* 查询票卡信息 */
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
			//岭南通模块串口初始化
			memset(&config,0,sizeof(lib_lnt_config_t));
			strcpy((char *)config.tty,"/dev/ttyO1");
			config.baudrate = 115200;
		
			ret = lnt_init(&config);
			if(ret < 0){
				fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
			}
		#endif
		
			/* 检查是否CPU卡*/
			if(stat == LNT_MOC_ERROR_M1) 
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_QRY_TICKET;
				sprintf(result->s_info, "不支持该类型卡操作,请退卡--错误码:%02X,状态码:%02X", TASKSTEP_QRY_TICKET, stat);
				emit signals_Result(result);
				goto Done;
			}

			result->result = TASK_RESULT_OP_LOOP_FREE;
			result->step = TASKSTEP_QRY_TICKET;

			if(stat == LNT_MOC_ERROR_CARD_NOTAG)
				sprintf(result->s_info, "检测不到卡,请按指示重新操作--错误码:%02X,状态码:%02X", TASKSTEP_QRY_TICKET, stat);
			else if(stat == LNT_MOC_ERROR_CARD_NAMELIST) //黑名单卡
				sprintf(result->s_info, "该卡为黑名单卡,不允许执行开销卡操作,请退卡--错误码:%02X,状态码:%02X", TASKSTEP_QRY_TICKET, stat);
			else
				sprintf(result->s_info, "读取卡片信息失败,请退卡稍后再试--错误码:%02X,状态码:%02X", TASKSTEP_QRY_TICKET, stat);
			
			emit signals_Result(result);
			goto Done;	
		}

	
		ticket_blance = qry_ticket.ticket.blance;   //保存票卡余额

		fprintf(stderr,"------Before Operation, Blance: %d-----\n", ticket_blance);
		
		memcpy(SAVE_pid, qry_ticket.ticket.pid, 8); //保存卡的物理卡号
		memcpy(&extra.pid, qry_ticket.ticket.pid, 8); 

		taskStep = TASKSTEP_QRY_TICKET;
	}

	//printf("taskdata.serviceType:%d\n",taskdata.serviceType);
	/**********************************************开卡流程*******************************************/
	if(taskdata.serviceType == CARD_SERVICE_ON_NORMAL || taskdata.serviceType == CARD_SERVICE_ON_TIME	\
		|| taskdata.serviceType == CARD_SERVICE_ON_DAY || taskdata.serviceType == CARD_SERVICE_ON_MONTH || taskdata.serviceType == CARD_SERVICE_ON_YEAR)
	{
		
		if(taskStep == TASKSTEP_QRY_TICKET)
		{
			#if 0 //可能之前开卡出现写卡失败需要补写开卡信息
			/* 检查是否已开卡 */
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
			if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1))
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_QRY_TICKET;

				sprintf(result->s_info, "该卡已开通会员,无需再执行开卡操作,请退卡--错误码:%02X", TASKSTEP_PMFLAG);
				emit signals_Result(result);
				goto Done;
			}
			#endif
			
			taskStep = TASKSTEP_PMFLAG;
		}

		/* 连接到代理网关 */
		if(taskStep == TASKSTEP_PMFLAG)
		{
			sockfd =  lib_lnt_on_card_connect_to_agent();
			if(sockfd <= 0)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_SOCKET;
				sprintf(result->s_info, "系统连接失败,请退卡稍后再试--错误码:%02X", TASKSTEP_SOCKET);
				emit signals_Result(result);
				goto Done;
			}

			lib_setfd_noblock(sockfd); //设置为非阻塞
			
			taskStep = TASKSTEP_ON_STAGE1TO2;
		}

		/* 执行开卡1-2阶段 */
		if(taskStep == TASKSTEP_ON_STAGE1TO2)
		{
			memset(&rent_info_req, 0, sizeof(lnt_rent_info_req_t));
			memset(&rent_info_ack, 0, sizeof(lnt_rent_info_ack_t));
			memset(&on_card_stage1, 0, sizeof(lnt_agent_on_card_stage1_CTS_t));
			memset(&on_card_stage2, 0, sizeof(lnt_agent_on_card_stage2_STC_t));
			
			//查询租还车信息
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			rent_info_req.id = 0x68;
			
			ret = lib_lnt_qry_rent_info(&rent_info_req, &rent_info_ack, &stat, 400);
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
			{
			#if 1//add by zjc
				//岭南通模块串口初始化
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

				sprintf(result->s_info, "读取卡片信息失败,请退卡稍后再试--错误码:%02X,状态码:%02X", TASKSTEP_QRY_RENT, stat);
				emit signals_Result(result);
				goto Done;
			}
	
			memcpy(on_card_stage1.pid, SAVE_pid, 8); //pid
			lib_get_macaddr("eth1", (char *)&on_card_stage1.macaddr);  //MAC
			memcpy(macaddr, on_card_stage1.macaddr, 6);  //保存MAC
			memset(on_card_stage1.pki, 0xff, 4);
			strcpy((char *)on_card_stage1.id_card, (char *)taskdata.id_card);
			memcpy(on_card_stage1.phone_number, taskdata.phone_number, 6); //电话号码
			on_card_stage1.member_type = taskdata.member_type; //会员类型//
			on_card_stage1.member_count = LNT_HTONS(taskdata.member_typeCount);
			memcpy(&on_card_stage1.ticket_info, &qry_ticket.ticket, sizeof(struct ticket_info));
			memcpy(&on_card_stage1.rent_info, &rent_info_ack.rent, sizeof(struct rent_info));
			lib_lnt_utils_time_bcd_yymmddhhMMss(on_card_stage1.time);  //时间

		#if 1
			printf("\n--------------Request Member_type:%d\n", on_card_stage1.member_type);
			printf("--------------Request Member_typeCount:%d\n\n", taskdata.member_typeCount);		
		#endif
			
			#if 1
			switch(taskdata.member_type)	//开卡次数、天数、月数、年数检查合法性
			{	
				case 0x01:	//开卡次数限制:1~100
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_COUNTS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "您输入的开卡次数不合法,请重新输入合法的次数[%d~%d]再执行开通操作--错误码:%02X", MEMBER_LOWER_LIMIT, MEMBER_COUNTS_LIMIT, TASKSTEP_QRY_RENT);
						emit signals_Result(result);
						goto Done;
					}
				break;
					
				case 0x02:	//开卡天数限制:1~30
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_DAYS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "您输入的开卡天数不合法,请重新输入合法的天数[%d~%d]再执行开通操作--错误码:%02X", MEMBER_LOWER_LIMIT, MEMBER_DAYS_LIMIT, TASKSTEP_QRY_RENT);
						emit signals_Result(result);
						goto Done;
					}
				break;
					
				case 0x03:	//开卡月数限制:1~12
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_MONTHS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "您输入的开卡月数不合法,请重新输入合法的月数[%d~%d]再执行开通操作--错误码:%02X", MEMBER_LOWER_LIMIT, MEMBER_MONTHS_LIMIT, TASKSTEP_QRY_RENT);
						emit signals_Result(result);
						goto Done;
					}
				break;

				case 0x04:	//开卡年数限制:1~5
					if((taskdata.member_typeCount < MEMBER_LOWER_LIMIT) || (taskdata.member_typeCount > MEMBER_YEARS_LIMIT))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_QRY_RENT;

						sprintf(result->s_info, "您输入的开卡年数不合法,请重新输入合法的年数[%d~%d]再执行开通操作--错误码:%02X", MEMBER_LOWER_LIMIT, MEMBER_YEARS_LIMIT, TASKSTEP_QRY_RENT);
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
				sprintf(result->s_info, "系统繁忙,请退卡稍后再试--错误码:%02X,返回码:%02X", TASKSTEP_ON_STAGE1TO2, agent_err);
				emit signals_Result(result);
				goto Done;
			}

			apply_result = on_card_stage2.apply_result; //申请结果
		#if 1
			on_card_stage2.member_count = LNT_NTOHS(on_card_stage2.member_count);

			printf("\n--------------Apply Member_type:%d\n", on_card_stage2.member_type);
			printf("--------------Apply Member_typeCount:%d\n\n", on_card_stage2.member_count);
			
			if((taskdata.member_type != on_card_stage2.member_type) || \
				(taskdata.member_typeCount != on_card_stage2.member_count))
			{
				printf("--------------Member Apply Error!\n"); //返回的开卡参数和申请的不一致
			}
		#endif

			LIB_LNT_log_vsprintf("****************Begin On Card Process*****************\n");
			LIB_LNT_log_vsprintf("PKT SN:%d, PID: %02x %02x %02x %02x %02x %02x %02x %02x\n", lib_pkt_sn_RO_get(), \
				SAVE_pid[0], SAVE_pid[1], SAVE_pid[2], SAVE_pid[3], SAVE_pid[4], SAVE_pid[5], SAVE_pid[6], SAVE_pid[7]);
			LIB_LNT_log_vsprintf("PKT SN:%d, On Card Apply Result: 0x%02X\n", lib_pkt_sn_RO_get(), apply_result);
			printf("On Card Apply Result: 0x%02X\n",apply_result);

			switch(apply_result) 
			{
				case 0x01: //黑名单
				{
					strcpy(result->s_info, "该卡为非法卡,正在执行处理...");
					emit signals_Result(result);
					
					goto BlackListDeal;  //跳到黑名单处理
				}
				break;

				case 0x02: //身份证格式无效
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "输入的[身份证号码]格式无效,请退卡");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x03: //手机号码格式无效
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "输入的[手机号码]格式无效,请退卡");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x10: //允许开卡
				{
					fee = LNT_NTOHL(on_card_stage2.fee); //字节序转换
					bal = qry_ticket.ticket.blance; //余额

					if(fee > bal)  //判断余额是否足够
					{
						/* 余额不足 */
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_ON_STAGE1TO2;
						sprintf(result->s_info, "该卡余额为%02d.%02d元,不足%d.%02d元,请充值后重新执行开通会员操作", bal / 100, bal % 100, fee / 100, fee % 100);
						emit signals_Result(result);
						goto Done;
					}
					else
					{
						/* 余额足够 */
						result->result = TASK_RESULT_OP_LOOP;
						result->step = TASKSTEP_ON_STAGE1TO2;
						sprintf(result->s_info, "正在执行开通会员操作,将会扣除押金%d.%02d元,请不要移动卡片,并耐心等待......", fee / 100, fee % 100);
						emit signals_Result(result);
							
						taskStep = TASKSTEP_ON_CONSUME;  //下一步执行扣费
					}
				}
				break;

				case 0x11: //已消费，未开卡
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_STAGE1TO2;
					sprintf(result->s_info, "正在执行开通会员操作,该卡已扣费,请不要移动卡片,并耐心等待......");
					emit signals_Result(result);
							
					taskStep = TASKSTEP_ON_STAGE5TO6;	 //押金处理
				}
				break;

				case 0x12: //身份证开卡数量超过允许
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "该身份证号码开卡数量超过允许,请退卡");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x13: //已开卡
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "该卡已开通会员,无需再执行开卡操作,请退卡");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x14: //找人工处理
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info,  "请找人工咨询处理,请退卡");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x15: //余额不足
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "该卡余额不足,请充值后重新执行开通会员操作");
					emit signals_Result(result);
					goto Done;
				}
				break;

				case 0x16: //只开卡，不消费
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_STAGE1TO2;
					sprintf(result->s_info, "正在执行会员开通操作,该卡已付押金,请不要移动卡片,并耐心等待......");
					emit signals_Result(result);
							
					taskStep = TASKSTEP_ON_STAGE5TO6;	 //押金处理
				}
				break;

				case 0xFF: //暂停服务
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OUT_OF_SERVICE;
					strcpy(result->s_info, "设备暂停该功能服务,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				default:  //默认
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE1TO2;
					strcpy(result->s_info, "执行会员开通操作失败,服务出现异常,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;
			}
		}

		/* 执行开卡扣押金 */
		if(taskStep == TASKSTEP_ON_CONSUME)  //扣押金阶段
		{
			emit_TaskRunStep(TaskRunStep_p2);  //扣除用户押金[1]
			
			unsigned char f_type = 0;
			
			memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
			memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));

			LIB_LNT_log_vsprintf("PKT SN:%d, On Card Consume Fee: %2d.%02d(Y)\n", lib_pkt_sn_RO_get(), fee / 100, fee % 100);
			
			lnt_cse.amount = fee;
			lnt_cse.fee = fee;
			lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
			memcpy(lnt_cse.phyiscalid, SAVE_pid, 8);
			lnt_cse.aty = LNT_ATY_RETURN; //收押金  0x17

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			ret = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &stat, &f_type);	//消费和获取交易记录
			if(ret != LNT_EOK) 
			{
				if(f_type == LNT_GET_TRANS)  //获取交易记录失败
				{
					//add by zjc 2016-03-22
					LIB_LNT_log_vsprintf("PKT SN:%d, On Card Get Trade Record Failed, Status: %02X\n", lib_pkt_sn_RO_get(), stat);

					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					ret = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //读取消费记录失败则再读取一次
					if((ret <=  0) || (stat != LNT_MOC_ERROR_NORMAL))
					{
						result->result = TASK_RESULT_OP_FAIL_FREE;
						result->step = TASKSTEP_ON_CONSUME;
						sprintf(result->s_info, "会员开通操作失败,如有疑问请找人工咨询处理--错误码:%02X,状态码:%02X,类型码:%02X", TASKSTEP_ON_CONSUME, stat, f_type);
						emit signals_Result(result);
						goto Done;
					}

					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_CONSUME;
					emit signals_Result(result);
				}
				else	//消费失败
				{
					//add by zjc 2016-03-22
					LIB_LNT_log_vsprintf("PKT SN:%d, On Card Consume Failed, Status: %02X\n", lib_pkt_sn_RO_get(), stat);
					
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_CONSUME;
					sprintf(result->s_info, "会员开通操作失败,如有疑问请找人工咨询处理--错误码:%02X,状态码:%02X,类型码:%02X", TASKSTEP_ON_CONSUME, stat, f_type);
					emit signals_Result(result);
					goto Done;
				}
			}
			else  //成功
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, On Card Consume Trade OK\n", lib_pkt_sn_RO_get());
					
				result->result = TASK_RESULT_OP_LOOP;
				result->step = TASKSTEP_ON_CONSUME;
				emit signals_Result(result);
			}

			memcpy(&trade_record, &(lnt_trade_rec.record), sizeof(trade_record)); //交易记录
			//将交易记录插入数据库
			lib_records_consume_insert_data(g_rec_file, (int)lib_pkt_sn_RO_get(), lnt_trade_rec.record.pid, lnt_trade_rec.record.lid, trade_record);  //插入消费记录
			
			taskStep = TASKSTEP_ON_STAGE3TO4;
		}

		/* 开卡3-4阶段*/
		if(taskStep == TASKSTEP_ON_STAGE3TO4)  //交易记录上报
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
			on_card_stage3.trade_sn = consume_sn;	//交易流水号
			
			memcpy(&on_card_stage3.trecord, &lnt_trade_rec.record, sizeof(struct trade_record)); //交易记录
			lib_lnt_utils_time_bcd_yymmddhhMMss(on_card_stage3.time);

			agent_err = lib_lnt_on_card_stage3TO4_process(sockfd, &on_card_stage3, &on_card_stage4, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload Consume Record Failed\n", lib_pkt_sn_RO_get());

				/* 交易记录上传失败，插入数据库,重传 [1]*/
				lib_records_reconsume_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);

				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE3TO4;
				sprintf(result->s_info, "会员开通操作失败,如有疑问请找人工咨询处理--错误码:%02X,返回码:%02X", TASKSTEP_ON_STAGE3TO4, agent_err);
				emit signals_Result(result);
				goto Done;
			}
			LIB_LNT_log_vsprintf("PKT SN:%d, Upload Consume Record OK\n", lib_pkt_sn_RO_get());	//交易记录上传成功
		
			taskStep = TASKSTEP_ON_STAGE5TO6;
			
		}

		/* 开卡5-6阶段 */
		if(taskStep == TASKSTEP_ON_STAGE5TO6) //押金初始化、DD73、DD74
		{
			emit_TaskRunStep(TaskRunStep_p3);  //执行会员开通

			/* 信息查询 */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);	
			if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
			{
				if(memcmp(SAVE_pid, qry_ticket.ticket.pid, 8) != 0) //卡号不符合
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_ON_STAGE5TO6;
					sprintf(result->s_info, "开卡执行失败,卡号不一致,请退卡");
					emit signals_Result(result);
					goto Done;
				}

				//检查押金标志是否已经修改成功 2016-04-26
				memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
				if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1))	//已修改押金标志但之前写卡失败
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_ON_STAGE5TO6;
					sprintf(result->s_info, "正在补写开卡信息,请不要移动卡片并耐心等待......");
					emit signals_Result(result);
				
					taskStep = TASKSTEP_ON_SET_RENT;	
					goto SET_RENT_INFO;	//补写开卡信息
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
				sprintf(result->s_info, "会员开通操作失败,请重新执行会员开通操作,如尝试多次后仍然失败,请找人工处理");
				emit signals_Result(result);
				goto Done;
			}
			taskStep = TASKSTEP_DEPOSIT;
		}

		/* 押金处理 */
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
				//岭南通模块串口初始化
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
			extra.result = 0x00; //结果上报
			extra_result = extra.result; //保存开卡结果

			/* 押金处理返回失败,不代表押金标志没有修改成功,所以要询卡判断是否已经修改押金标志成功 */
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
			//(stat == LNT_MOC_ERROR_NORMAL) 增加票卡查询结果判断 add by zjc at 2016-10-27
			if((stat == LNT_MOC_ERROR_NORMAL) && (qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1))  //修改押金标志成功
			{		
				LIB_LNT_log_vsprintf("PKT SN:%d, On Card Deposit Process OK\n", lib_pkt_sn_RO_get());

				taskStep = TASKSTEP_ON_SET_RENT;
			}
			else	//修改押金标志修改失败或者检查押金标志失败
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, On Card Deposit Process Failed\n", lib_pkt_sn_RO_get());
				
				taskstep_err_code.task_step = TASKSTEP_DEPOSIT;
				taskstep_err_code.task_stat = stat;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;
				
				extra.result = 0xFF;  //修改押金标志或者检查押金标志失败
				extra_result = extra.result;	//保存开卡结果
			
				taskStep = TASKSTEP_ON_STAGE7TO8; //跳到开卡结果上报	
			}
		}

SET_RENT_INFO:
		/* 设置租还车信息 */
		if(taskStep == TASKSTEP_ON_SET_RENT)
		{
			result->result = TASK_RESULT_OP_LOOP;
			result->step = TASKSTEP_DEPOSIT;
			sprintf(result->s_info, "正在执行写卡操作,请不要移动卡片,并耐心等待......");
			emit signals_Result(result);
			
			/* 查询票卡信息 */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);	
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))
			{
				fprintf(stderr,"\nOn Card  Process: [qry_ticket_info] Stat: %02X\n",stat);

			#if 1 //add by zjc
				//岭南通模块串口初始化
				memset(&config,0,sizeof(lib_lnt_config_t));
				strcpy((char *)config.tty,"/dev/ttyO1");
				config.baudrate = 115200;
			
				ret = lnt_init(&config);
				if(ret < 0){
					fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
				}
			#endif
			}

			/* 设置租还车信息(写入开卡信息)*/
			lib_lnt_rent_info_t rent_info;
			memset(&rent_info, 0, sizeof(lib_lnt_rent_info_t));

			stat = LNT_MOC_ERROR_UNKNOW;
			rent_info.id = 0x68;
			rent_info.length = sizeof(lib_rent_info_t);

			if(on_card_stage2.member_type != 0)
			{
				rent_info.rent.free_time = 2;	//普通卡免费时长为0,非普通卡免费时长为2(2单位=30分钟)
			}

			memset(&rent_info.rent.provider, 0x01, 2); //服务运营商编码 add by zjc at 2016-11-22
			rent_info.rent.ticket_flag = on_card_stage2.member_type; //会员类型
			rent_info.rent.s_un.acc.accum = LNT_NTOHS(on_card_stage2.member_count); //限额
			lib_lnt_utils_time_bcd_yymmdd(rent_info.rent.start_time); //启用时间
			memcpy(&(rent_info.rent.site_no), &macaddr, 6); //站点编号
			
			ret = lib_lnt_set_rent_info(&rent_info, &stat, 500);
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))
			{
				fprintf(stderr,"\nOn Card  Process: [set_rent_info] Stat: %02X\n",stat);

			#if 1 //add by zjc
				//岭南通模块串口初始化
				memset(&config,0,sizeof(lib_lnt_config_t));
				strcpy((char *)config.tty,"/dev/ttyO1");
				config.baudrate = 115200;
			
				ret = lnt_init(&config);
				if(ret < 0){
					fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
				}
			#endif

				extra.result = 0x02;  //写卡失败 add by zjc at 2016-10-31
			}
			
			/* 消费写卡 */
			memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
			memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
	
			lnt_cse.amount = 0;
			lnt_cse.fee = 0;
			lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
			memcpy(lnt_cse.phyiscalid, SAVE_pid, 8);
			lnt_cse.aty = LNT_ATY_RETURN; //收押金
			
			stat = LNT_MOC_ERROR_UNKNOW;
			unsigned char f_type = 0;

			ret = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &stat, &f_type);
			if(ret != LNT_EOK)
			{	
				//消费写卡失败后上报开卡结果	2016-04-26
				if(f_type == LNT_GET_TRANS)	//获取交易记录失败
				{
					stat = LNT_MOC_ERROR_UNKNOW;
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 500);  

					extra.result = 0x00; //消费写卡成功
					extra_result = extra.result; //保存开卡结果
					taskStep = TASKSTEP_ON_STAGE7TO8;
				}
				else	//消费失败
				{
					//printf("-----------------------\n");
					extra.result = 0x02;  //消费写卡失败
					extra_result = extra.result;	//保存开卡结果
					taskStep = TASKSTEP_ON_STAGE7TO8;
				}
			}
			
			taskStep = TASKSTEP_ON_STAGE7TO8;
		}

		/* 开卡7-8阶段 */
		if(taskStep == TASKSTEP_ON_STAGE7TO8)
		{
			lnt_agent_on_card_stage8_STC_t on_card_stage8;

			memset(&retrans, 0, sizeof(agent_retransmission_data_t));
			memset(&on_card_stage8, 0, sizeof(lnt_agent_on_card_stage8_STC_t));
			memset(&extra, 0, sizeof(agent_extra_data_t)); 
			extra.result = extra_result; 
			lib_SN_default_read32(&register_sn);
			extra.local_pkt_sn = register_sn;  //报文流水号
			//memcpy(&extra.pid, &qry_ticket.ticket.pid, 8);
			memcpy(&extra.pid, &SAVE_pid, 8);	//用保存的,防止最近一次查询票卡信息失败导致全为0
			memcpy(&extra.macaddr, macaddr, 6);
			
			agent_err = lib_lnt_on_card_stage7TO8_process(sockfd, &extra, &on_card_stage8, &retrans); 
			if(agent_err != AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload On Card Record Failed\n\n", lib_pkt_sn_RO_get());

				/* 上传失败，插入数据库,重传[2] */
				lib_records_oncard_ack_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);
			}
			else
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload On Card Record OK\n\n", lib_pkt_sn_RO_get());
			}

			emit_TaskRunStep(TaskRunStep_p4);  //会员开通成功[4]
						
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
			bal = qry_ticket.ticket.blance;

			time(&end_times);
			consume_time = end_times - begin_times; //耗时计算

			/* 增加检查押金标志失败或者写卡失败的提示信息 add by zjc at 2016-10-28 */
			if(extra.result == 0xFF) //修改押金标志或者检查押金标志失败
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE7TO8;
				sprintf(result->s_info, "会员开通出现异常,请查询票卡信息以确认会员开通是否成功,如有疑问请找人工咨询处理--返回码:%02X, 结果码:%02X", agent_err, extra.result);
				emit signals_Result(result);
			}
			else if(extra.result == 0x02) //开卡信息写入失败
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE7TO8;
				sprintf(result->s_info, "会员开通写卡失败,请按\"执行开通操作\"进行补写卡,如有疑问请找人工咨询处理--返回码:%02X, 结果码:%02X", agent_err, extra.result);
				emit signals_Result(result);
			}
			else //extra.result = 0x00
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_ON_STAGE7TO8;
				sprintf(result->s_info, "会员开通成功,现在票卡余额为%d.%02d元,全过程耗时%d秒,请退卡--返回码:%02X", bal / 100, bal % 100, consume_time, agent_err);
				emit signals_Result(result);
			}
		}
	}

	

/**********************************************销卡流程*******************************************/
	if(taskdata.serviceType == CARD_SERVICE_OFF)
	{
		if(taskStep == TASKSTEP_QRY_TICKET)
		{
			#if 0	//销卡前无需判断押金标记，因为可能上次销卡是已销卡未充值的情况
			/* 检查是否已开卡 */
			pmflag_t pf;
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
			if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1))
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_PMFLAG;

				sprintf(result->s_info, "该卡未开通会员,不能执行注销操作--错误码:%02X", TASKSTEP_PMFLAG);
				emit signals_Result(result);
				goto Done;
			}
			#endif
			
			taskStep = TASKSTEP_PMFLAG;
		}
			
		/* 连接到代理网关 */
		if(taskStep == TASKSTEP_PMFLAG)
		{
			sockfd =  lib_lnt_on_card_connect_to_agent();
			if(sockfd <= 0)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_SOCKET;
				sprintf(result->s_info, "系统连接失败,请退卡稍后再试--错误码:%02X", TASKSTEP_SOCKET);
				emit signals_Result(result);
				goto Done;
			}

			lib_setfd_noblock(sockfd); //设置为非阻塞
			
			taskStep = TASKSTEP_OFF_STAGE1TO2;
		}

		/* 执行销卡1-2阶段 */
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

				sprintf(result->s_info, "读取票卡信息失败,请退卡稍后再试--错误码:%02X,状态码:%02X", TASKSTEP_QRY_RENT, stat);
				emit signals_Result(result);
				goto Done;
			}

			//printf("==========rent_flag:%02x\n", rent_info_ack.rent.rent_flag);
			
			if((rent_info_ack.rent.rent_flag & (0x08)) != 0) //已租车
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_QRY_RENT;

				sprintf(result->s_info, "请先还车,再执行会员注销操作--错误码:%02X,状态码:%02X", TASKSTEP_QRY_RENT, stat);
				emit signals_Result(result);
				goto Done;
			}

		#if 1
			/* 检查是否为非普通卡以及是否有未使用完的次数、天数、月数、年数 */ //2016-04-15 add by zjc  
			if((rent_info_ack.rent.ticket_flag != 0x00) && (LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum) > 0))
			{
				/* 增加票卡有效期判断  add by zjc at 2016-12-06 */
				ret = lib_lnt_is_valid_card();
				if(ret == LNT_CARD_VALID)
				{
					//printf("----------This Card is Valid!\n");
					
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_QRY_RENT;
					
					result->memberCount = LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum);	//用于提示用户还有未使用完的优惠
					//printf("------------memberCount1:%d\n", result->memberCount);
					
					switch(rent_info_ack.rent.ticket_flag)
					{
						case 0x01:
							sprintf(result->s_info, "该卡为[次卡],剩余优惠次数[%d]次,确认要注销会员请按\"执行注销操作\"! 按\"退出\"返回上一级界面", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;

						case 0x02:
							sprintf(result->s_info, "该卡为[天卡],剩余优惠天数[%d]天,确认要注销会员请按\"执行注销操作\"! 按\"退出\"返回上一级界面", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;

						case 0x03:
							sprintf(result->s_info, "该卡为[月卡],剩余优惠月数[%d]月,确认要注销会员请按\"执行注销操作\"! 按\"退出\"返回上一级界面", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;
	         
						case 0x04:
							sprintf(result->s_info, "该卡为[年卡],剩余优惠年数[%d]年,确认要注销会员请按\"执行注销操作\"! 按\"退出\"返回上一级界面", LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum));
						break;
						
						default:
							sprintf(result->s_info, "该卡还有未用完的优惠服务,确认要注销会员请按\"执行注销操作\"! 按\"退出\"返回上一级界面");
						break;
					}
					
					#if CONFIRM_BUTTON
					printf("--------taskdata.confirmCnt:%d----------\n",taskdata.confimeCnt);
					if(taskdata.confimeCnt < 2)	//停住提示用户确认是否销卡
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
		
			emit_TaskRunStep(TaskRunStep_p2);  //查验会员权限[2]
			
			memcpy(off_card_stage1_CTS.pid, SAVE_pid, 8); //pid
			lib_get_macaddr("eth1", (char *)&off_card_stage1_CTS.macaddr);  //MAC
			memcpy(macaddr, off_card_stage1_CTS.macaddr, 6);  //保存MAC
			memset(off_card_stage1_CTS.pki, 0xff, 4);
			strcpy((char *)off_card_stage1_CTS.id_card, (char *)taskdata.id_card);	//身份证号码
			memcpy(&off_card_stage1_CTS.ticket_info, &qry_ticket.ticket, sizeof(struct ticket_info));
			memcpy(&off_card_stage1_CTS.rent_info, &rent_info_ack.rent, sizeof(struct rent_info));
			lib_lnt_utils_time_bcd_yymmddhhMMss(off_card_stage1_CTS.time);  //时间

			agent_err = lib_lnt_off_card_stage1TO2_process(sockfd, &off_card_stage1_CTS, &off_card_stage2_STC, &extra);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_OFF_STAGE1TO2;
				sprintf(result->s_info, "系统繁忙,请退卡稍后再试--错误码:%02X,返回码:%02X", TASKSTEP_OFF_STAGE1TO2, agent_err);
				emit signals_Result(result);
				goto Done;	
			}

			apply_result = off_card_stage2_STC.apply_result; //申请结果

			LIB_LNT_log_vsprintf("****************Begin Off Card Process****************\n");
			LIB_LNT_log_vsprintf("PKT SN:%d, PID: %02x %02x %02x %02x %02x %02x %02x %02x\n", lib_pkt_sn_RO_get(), \
				SAVE_pid[0], SAVE_pid[1], SAVE_pid[2], SAVE_pid[3], SAVE_pid[4], SAVE_pid[5], SAVE_pid[6], SAVE_pid[7]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Apply Result: 0x%02X\n", lib_pkt_sn_RO_get(), apply_result);

			printf("Off Card Apply Result: 0x%02X\n",apply_result);
			
			switch(apply_result) 
			{
				case 0x01: //黑名单卡
				{
					#if 0
					//result->step = TASKSTEP_OFF_STAGE1TO2;
					//result->result = TASK_RESULT_OP_FAIL_FREE;
					strcpy(result->s_info, "该卡为非法卡,正在执行处理...");
					emit signals_Result(result);  
					#endif

					goto BlackListDeal;  //跳到黑名单处理
				}
				break;
				
				case 0x02: //身份证格式无效
				{
					result->step = TASKSTEP_OFF_STAGE1TO2;
					result->result = TASK_RESULT_OP_FAIL_FREE;
					strcpy(result->s_info, "输入的[身份证号码]格式无效,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				case 0x20: //允许销卡
				{
					/*
					 * 如果中心返回0x20允许销卡,但是该卡押金标志是未开卡，那直接跳过押金处理，直接充值
					 * 导致这种情况其中一个原因是修改押金标志后，没有把押金标志结果成功上传到中心，中心没有记录，一直认为该卡没有销卡
					 */
					#if 0 //by zjc at 2016-10-31
					/* 检查是否已开卡 */
					pmflag_t pf;
					memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
					if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1)) //未开卡
					{
						f_operate = 0x20;  //直接进入充值阶段
					}
					#endif
					
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "正在执行会员注销操作,请不要移动卡片,并耐心等待......");
					emit signals_Result(result);  
					
					taskStep = TASKSTEP_OFF_STAGE7TO8; //押金处理 DD73 DD74
				}
				break;

				case 0x21: //未开卡
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "没有该卡开通记录,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0x22: //身份证未开卡
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "没有该身份证号码的开卡记录,请退卡");
					emit signals_Result(result);  
					goto Done;    
				}
				break;

				case 0x23: //已销卡，未充值
				{	
					/*
					 * 已经去掉押金标志，但是没有充值成功的状态
					 */
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "该卡已注销,但还未返还用户押金,正在获取订单号,请不要移动卡片,并耐心等待......");
					emit signals_Result(result); 

					taskStep = TASKSTEP_OFF_STAGE5TO6;
				}
				break;

				case 0x24: //只销卡，不做充值
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "该卡只执行会员注销操作,返还用户押金操作将在第三方平台进行,请不要移动卡片,并耐心等待......");
					emit signals_Result(result); 
					
					taskStep = TASKSTEP_OFF_STAGE7TO8;  //直接押金操作
					
				}
				break;

				case 0x14: //找人工处理
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "请找人工咨询处理,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0xFF: //暂停服务
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OUT_OF_SERVICE;
					strcpy(result->s_info, "设备暂停该功能服务,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				default:  //默认
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE1TO2;
					strcpy(result->s_info, "会员注销失败,服务出现异常,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;
			}
		}

		/* 执行销卡7-8阶段 */
		if(taskStep == TASKSTEP_OFF_STAGE7TO8)  //押金处理第一阶段  DD73 DD74
		{	
			emit_TaskRunStep(TaskRunStep_p3);   //执行会员注销[3]

			/* 增加押金标志的检查，如果之前销卡只是检查押金标志失败但是实际上已经销卡成功的话就可以跳过押金处理 add by zjc at 2016-10-28 */
			#if 1
			pmflag_t pf;
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);
			
			if((qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1)) //已销卡
			{		
				LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Deposit Was Processed\n", lib_pkt_sn_RO_get());

				if(apply_result == 0x20) //允许销卡
				{
					taskStep = TASKSTEP_OFF_STAGE5TO6;
					goto OFF_CARD_RECHARGE; //已经修改押金标志的跳到充值流程
				}
				else if(apply_result == 0x24) //只销卡不充值
				{
					extra.result = 0x10; //修改押金标志成功(已销卡,不需充值)
					taskStep = TASKSTEP_OFF_STAGE9TO10;
					goto UPLOAD_OFF_CARD_RESULT; //已经修改押金标志的跳到销卡结果上报
				}
			}
			#endif
		
			memset(&member_stage2, 0, sizeof(lnt_member_process_stage2_STC_t));
			agent_err = lib_lnt_off_card_stage7TO8_process(sockfd, &extra, &member_stage2); //押金初始化 DD73 DD74
			if(agent_err != AGENT_ERR_OK)
			{
				#if 0
				taskstep_err_code.task_step = TASKSTEP_OFF_STAGE7TO8;
				taskstep_err_code.task_ret = agent_err;
				
				extra.result = 0xFF;  //修改押金标志失败
				taskStep = TASKSTEP_OFF_STAGE9TO10;  //销卡记录上传(由于这一阶段失败关闭了套接字，所以无法上传,上传失败的默认为销卡失败)
				#else //by zjc at 2016-10-31
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE7TO8;
				sprintf(result->s_info, "会员注销失败,请退卡--错误码:%02X,返回码:%02X", TASKSTEP_OFF_STAGE7TO8, agent_err);
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

		/* 执行押金处理 */
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
				//岭南通模块串口初始化
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

				extra.result = 0xFF; //结果上报  
			}
			
			/* 押金处理返回状态不能作为判断押金标志是否成功的标准 */
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			ret = -1;
			
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 500); //300->500 2016-10-27
			if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))  //增加查询失败的处理 add by zjc at 2016-10-27 
			{
				fprintf(stderr,"\n[qry_ticket_info] Stat: %02X, ret: %d\n", stat, ret);
				
				LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Check Pmflag Failed, Stat: %02X, Ret: %d\n", lib_pkt_sn_RO_get(), stat, ret);
			}
			
			memset(&pf, 0, sizeof(pmflag_t)); 
			memcpy(&pf, &(qry_ticket.ticket.pmflag), 1);

			//检查押金标志失败或者押金处理失败
			if((stat != LNT_MOC_ERROR_NORMAL) || ((stat == LNT_MOC_ERROR_NORMAL) && \
				(qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag == 1)))  //押金标志还没有修改成功
			{
				taskstep_err_code.task_step = TASKSTEP_DEPOSIT;
				taskstep_err_code.task_stat = stat;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;
				
				extra.result = 0xFF;  //修改押金标志失败
			}
			//新协议的销卡结果上报增加是否需要充值 2016-03-04
			else if(apply_result == 0x20){ //允许销卡
				extra.result = 0x00; //修改押金标志成功(已销卡,还需充值)
			}
			else if(apply_result == 0x24){ //0x24:只销卡，不做充值
				extra.result = 0x10; //修改押金标志成功(已销卡,不需充值)
			}

			if(extra.result != 0xFF){ //修改押金标志成功
				LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Deposit Process OK\n", lib_pkt_sn_RO_get());
			}
			
			taskStep = TASKSTEP_OFF_STAGE9TO10;
		}

UPLOAD_OFF_CARD_RESULT:
		/* 
		 * 执行销卡9-10阶段 : 这一阶段是押金标志修改报文上报阶段
		 */
		if(taskStep == TASKSTEP_OFF_STAGE9TO10) //销卡记录上传
		{
			lnt_agent_off_card_stage10_STC_t off_card_stage10_STC;  
			memset(&off_card_stage10_STC, 0, sizeof(lnt_agent_off_card_stage10_STC_t));
			memset(&retrans, 0, sizeof(agent_retransmission_data_t));

			agent_err = lib_lnt_off_card_stage9TO10_process(sockfd, &extra, &off_card_stage10_STC, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				LIB_LNT_log_vsprintf("PKT SN:%d, Upload Off Card Deposit Record Failed\n", lib_pkt_sn_RO_get());

				/* 上传失败，插入数据库,重传 [3]*/
				lib_records_offcard_ack_insert_data(g_rec_file, retrans.local_pkt_sn, retrans.pid, macaddr, retrans.pkt_len, retrans.pkt_data);

				#if 1 
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE9TO10;
				sprintf(result->s_info, "会员注销失败,请退卡--错误码:%02X,返回码:%02X", TASKSTEP_OFF_STAGE9TO10, agent_err);
				emit signals_Result(result);
				goto Done;	
				#endif
			}
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Upload Off Card Deposit Record OK\n", lib_pkt_sn_RO_get());

			if(taskstep_err_code.task_ret != AGENT_ERR_OK) //前面出现修改押金标志失败情况
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE9TO10;
				sprintf(result->s_info, "会员注销操作失败,如尝试多次后,仍然失败,请找人工咨询处理--错误码:%02X, 返回码:%02X, 状态码:%02X", taskstep_err_code.task_step, agent_err, taskstep_err_code.task_stat);
				emit signals_Result(result);
				goto Done;
			}
			
			if(apply_result == 0x24) //只销卡，不做充值
			{
				time(&end_times);
				consume_time = end_times - begin_times;
			
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE9TO10;
				sprintf(result->s_info, "会员注销成功,过程耗时为%d秒,请退卡,返回码:%02X", consume_time, agent_err);
				emit signals_Result(result);
				goto Done;
			}
			else if((apply_result == 0x20) || (apply_result == 0x23)) //允许销卡和已销卡但未充值
			{
				taskStep = TASKSTEP_OFF_STAGE5TO6;	//获取订单号
			}
		}

OFF_CARD_RECHARGE: //销卡的充值流程
		/* 执行销卡5-6阶段 */ 
		if(taskStep == TASKSTEP_OFF_STAGE5TO6) //获取充值订单号
		{
			memset(&off_card_stage6_STC, 0, sizeof(lnt_agent_off_card_stage6_STC_t));
			agent_err = lib_lnt_off_card_stage5TO6_process(sockfd, &extra, &off_card_stage6_STC);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE5TO6;
				sprintf(result->s_info, "获取充值订单号失败,如尝试多次后,仍然失败,请找人工咨询处理--错误码:%02X,返回码:%02X", TASKSTEP_OFF_STAGE5TO6, agent_err);
				emit signals_Result(result);
				goto Done;	
			}
			//lib_printf_v2("--------recharge order---------",(unsigned char *)&off_card_stage6_STC.order,sizeof(off_card_stage6_STC.order),16);
			//lib_printf_v2("--------recharge amount---------",(unsigned char *)&off_card_stage6_STC.recharge,sizeof(off_card_stage6_STC.recharge),16);

			#if 1
			/* 检查充值上限 */
			unsigned int amount = LNT_NTOHL(off_card_stage6_STC.recharge); //要充值金额
			if((ticket_blance + amount) > LNT_CPU_RECHARGE_LIMIT)
			{
				result->step = TASKSTEP_OFF_STAGE5TO6;
				result->result = TASK_RESULT_OP_FAIL_FREE;
				sprintf(result->s_info, "该卡充值%02d.%02d元后会超过%d.%02d元余额上限,不允许返还用户押金操作,请找人工咨询处理,请退卡", amount / 100, amount % 100, 
						LNT_CPU_RECHARGE_LIMIT / 100, LNT_CPU_RECHARGE_LIMIT % 100);
				emit signals_Result(result);  
				goto Done;
			}
			#endif		
				
			taskStep = TASKSTEP_OFF_STAGE11TO12; //获取订单成功后执行充值

			#if 0 //by zjc at 2016-10-31
			if(f_operate == 0x20) //已销卡，未充值--修改押金标志成功，但充值失败的情况【2】
				taskStep = TASKSTEP_OFF_STAGE11TO12; //执行充值
			#endif
		}

		/* 执行销卡11-12阶段 */
		if(taskStep == TASKSTEP_OFF_STAGE11TO12) //DD11 充值第一阶段
		{
			emit_TaskRunStep(TaskRunStep_p4);   //返回用户押金[4]
			/* 信息查询 */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);	
			if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
			{
				if(memcmp(SAVE_pid, qry_ticket.ticket.pid, 8) != 0) //卡号不符合
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OFF_STAGE11TO12;
					sprintf(result->s_info, "会员注销失败,出现卡号不一致,请退卡");
					emit signals_Result(result);
					goto Done;
				}
			}

			bal = LNT_NTOHL(off_card_stage6_STC.recharge);
				
			result->result = TASK_RESULT_OP_LOOP;
			result->step = TASKSTEP_OFF_STAGE11TO12;
			sprintf(result->s_info, "正在执行返还用户押金操作,返回押金金额为%d.%02d元,请不要移动卡片,并耐心等待......", bal / 100, bal % 100);
			emit signals_Result(result);

			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Recharge Order: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", lib_pkt_sn_RO_get(), off_card_stage6_STC.order[0],off_card_stage6_STC.order[1],\
				off_card_stage6_STC.order[2],off_card_stage6_STC.order[3],off_card_stage6_STC.order[4],off_card_stage6_STC.order[5],off_card_stage6_STC.order[6],\
				off_card_stage6_STC.order[7],off_card_stage6_STC.order[8],off_card_stage6_STC.order[9]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Recharge Fee %d.%02d(Y)\n", lib_pkt_sn_RO_get(),  bal / 100, bal % 100);
	
			recharge_info_t recharge;
			memset(&recharge_stage2, 0, sizeof(lnt_packet_recharge_stage2_STC_t));
			recharge.apply_type = 0x00; //新协议只有0x00  2016-03-04

			memcpy(&recharge.recharge_amount, &off_card_stage6_STC.recharge, 4); //充值金额 [高位在前]
			memcpy(&recharge.order_num, &off_card_stage6_STC.order, 10); //订单号 

			agent_err = lib_lnt_off_card_stage11TO12_process(sockfd, &extra, &recharge,  &recharge_stage2);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE11TO12;
				sprintf(result->s_info, "会员注销失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,返回码:%02X", TASKSTEP_OFF_STAGE11TO12, agent_err);
				emit signals_Result(result);
				goto Done;	
			}     

			if(recharge_stage2.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE13TO14;
				
				if(recharge_stage2.retcode == 0xDD)
				{
					sprintf(result->s_info, "系统繁忙导致会员注销失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage2.retcode);
				}else{
					sprintf(result->s_info, "会员注销失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage2.retcode);
				}
				
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_OFF_STAGE13TO14;
		}
	
		/* 执行销卡13-14阶段 */
		if(taskStep == TASKSTEP_OFF_STAGE13TO14)
		{
			memset(&recharge_stage4, 0, sizeof(lnt_packet_recharge_stage4_STC_t));
			
			agent_err = lib_lnt_off_card_stage13TO14_process(sockfd, &extra, &recharge_stage2, &recharge_stage4);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE13TO14;
				sprintf(result->s_info, "会员注销失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,返回码:%02X,状态码:%02X", TASKSTEP_OFF_STAGE13TO14, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage4.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE13TO14;
				sprintf(result->s_info, "会员注销失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage4.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_OFF_STAGE15TO16;
		}
		
		/* 执行销卡15-16阶段 */
		if(taskStep == TASKSTEP_OFF_STAGE15TO16)
		{
			memset(&recharge_stage6, 0, sizeof(lnt_packet_recharge_stage6_STC_t));
			
			agent_err = lib_lnt_off_card_stage15TO16_process(sockfd, &extra, &recharge_stage4, &recharge_stage6);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE15TO16;
				sprintf(result->s_info, "会员注销失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,返回码:%02X,状态码:%02X", TASKSTEP_OFF_STAGE15TO16, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage6.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE15TO16;
				sprintf(result->s_info, "会员注销失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_OFF_STAGE15TO16, recharge_stage6.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_OFF_STAGE17TO18;
		}
		
		/* 
		 * 执行销卡17-18阶段 岭南通后台返回DD18报文后,就认为充值已经成功
		 */
		if(taskStep == TASKSTEP_OFF_STAGE17TO18)
		{
			memset(&recharge_stage8, 0, sizeof(lnt_packet_recharge_stage8_STC_t));
			
			agent_err = lib_lnt_off_card_stage17TO18_process(sockfd, &extra, &recharge_stage6, &recharge_stage8);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE17TO18;
				sprintf(result->s_info, "返还用户押金失败,请检查卡内余额,如有疑问请找人工咨询处理,请退卡--错误码:%02X,返回码:%02X,状态码:%02X", TASKSTEP_OFF_STAGE17TO18, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage8.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_OFF_STAGE17TO18;
				sprintf(result->s_info, "返还用户押金失败,请检查卡内余额,如有疑问请找人工咨询处理,请退卡--错误码:%02X,信息码:%02X", TASKSTEP_OFF_STAGE17TO18, recharge_stage8.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Off Card Process: [DD18] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			taskStep = TASKSTEP_OFF_STAGE19TO20;
		}

		/* 执行销卡19-20阶段 */
		if(taskStep == TASKSTEP_OFF_STAGE19TO20)
		{
			extra.result = 0x00; 
			
			memset(&taskstep_err_code, 0, sizeof(struct task_err_code));
			memset(&recharge_stage10, 0, sizeof(lnt_packet_recharge_stage10_STC_t));

			/* 包含R_CPU_LOAD*/
			agent_err = lib_lnt_off_card_stage19TO20_process(sockfd, &extra, &recharge_stage8, &recharge_stage10, &r_cpu_load); //CPU圈存
			if(r_cpu_load != 0x00)  //说明充值写卡失败
			{
				taskstep_err_code.task_stat = extra.r_stat;
				extra.result = 0xFF;
				taskstep_err_code.task_step = TASKSTEP_OFF_STAGE19TO20;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;

				taskStep = TASKSTEP_OFF_STAGE23TO24;
				goto DONE_TASKSTEP_OFF_STAGE23TO24;
			}
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Off Card Process: [R_CPU_LOAD] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD已经成功，说明充值已经成功
			{
				extra.result = 0x00;
				taskstep_err_code.task_ret = AGENT_ERR_OK;

				taskStep = TASKSTEP_OFF_STAGE23TO24;
				goto DONE_TASKSTEP_OFF_STAGE23TO24;
			}

			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Off Card Process: [DD20] Ok\n", lib_pkt_sn_RO_get());//add by zjc
			
			taskStep = TASKSTEP_OFF_STAGE21TO22;
		}

		/* 执行销卡21-22阶段 */
		if(taskStep == TASKSTEP_OFF_STAGE21TO22)
		{	
			memset(&recharge_stage12, 0, sizeof(lnt_packet_recharge_stage12_STC_t));
			
			agent_err = lib_lnt_off_card_stage21TO22_process(sockfd, &extra, &recharge_stage10, &recharge_stage12);
			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD已经成功，说明充值已经成功
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
		/* 执行销卡23-24阶段 */
		if(taskStep == TASKSTEP_OFF_STAGE23TO24)
		{
			/*上报充值订单号和充值金额 zjc,2016-03-24*/
			memcpy(&extra.order, &off_card_stage6_STC.order, 10); //订单号 
			memcpy(&extra.recharge, &off_card_stage6_STC.recharge, 4); //充值金额 [高位在前]

			lnt_agent_off_card_stage24_STC_t off_card_stage24_STC;
			
			memset(&off_card_stage24_STC, 0, sizeof(lnt_agent_off_card_stage24_STC_t));
			memset(&retrans, 0, sizeof(agent_retransmission_data_t));

			agent_err = lib_lnt_off_card_stage23TO24_process(sockfd, &extra, &off_card_stage24_STC, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				/* 充值结果上传失败，插入数据库,重传 [4]*/
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
				sprintf(result->s_info, "返还用户押金失败,请检查卡内余额,如有疑问请找人工咨询处理,请退卡--错误码:%02X, 状态码:%02X", taskstep_err_code.task_step, taskstep_err_code.task_stat);
				emit signals_Result(result);
				goto Done;
			}

			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_qry_ticket_info(&qry_ticket, &stat, 400);
			bal = qry_ticket.ticket.blance;

			emit_TaskRunStep(TaskRunStep_p5);   //注意注销成功[5]
			
			time(&end_times);
			consume_time = end_times - begin_times;

			/* 增加检查押金标志失败、写卡失败或者充值失败的提示信息 add by zjc at 2016-10-28 */
			if(extra.result == 0xFF) //修改押金标志或者检查押金标志失败
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_OFF_STAGE23TO24;
				sprintf(result->s_info, "会员注销出现异常,请查询票卡信息以确认会员注销是否成功,如有疑问请找人工咨询处理--返回码:%02X, 结果码:%02X", agent_err, extra.result);
				emit signals_Result(result);
			}
			else 
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_OFF_STAGE23TO24;
				sprintf(result->s_info, "会员注销成功,现在票卡余额为%d.%02d元,全过程耗时为%d秒,请退卡,返回码:%02X", bal / 100, bal % 100, consume_time, agent_err);
				emit signals_Result(result);
			}
		}
	}


	/**********************************************充值流程*******************************************/
	if(taskdata.serviceType == CARD_SERVICE_RECHARGE) //充值
	{
		/* 连接到代理网关 */
		if(taskStep == TASKSTEP_QRY_TICKET)
		{
			sockfd =  lib_lnt_on_card_connect_to_agent();
			if(sockfd <= 0)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_SOCKET;
				sprintf(result->s_info,"系统连接失败,请退卡稍后再试--错误码:%02X", TASKSTEP_SOCKET);
				emit signals_Result(result);
				goto Done;
			}

			lib_setfd_noblock(sockfd); //设置为非阻塞
			
			taskStep = TASKSTEP_RECHARGE_STAGE1TO2;
		}
		
		//emit_TaskRunStep(TaskRunStep_p1);  //查验会员卡片资料[1]
		
		/* 执行充值1-2阶段 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE1TO2)
		{
			lnt_agent_recharge_stage1_CTS_t recharge_stage1_CTS;
			lnt_agent_recharge_stage2_STC_t recharge_stage2_STC;
		
			memset(&recharge_stage1_CTS, 0, sizeof(lnt_agent_recharge_stage1_CTS_t));
			memset(&recharge_stage2_STC, 0, sizeof(lnt_agent_recharge_stage2_STC_t));
		
			memcpy(recharge_stage1_CTS.pid, SAVE_pid, 8); //pid
			lib_get_macaddr("eth1", (char *)&recharge_stage1_CTS.macaddr);	//MAC
			memcpy(macaddr, recharge_stage1_CTS.macaddr, 6);  //保存MAC
			memset(recharge_stage1_CTS.pki, 0xff, 4);
			memcpy(&recharge_stage1_CTS.ticket_info, &qry_ticket.ticket, sizeof(struct ticket_info));
			memcpy(&recharge_stage1_CTS.order, &taskdata.order_number, 10);	//充值订单号,用户输入
			lib_lnt_utils_time_bcd_yymmddhhMMss(recharge_stage1_CTS.time);	//时间
		
			agent_err = lib_lnt_recharge_stage1TO2_process(sockfd, &recharge_stage1_CTS, &recharge_stage2_STC, &extra);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_LOOP_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE1TO2;
				sprintf(result->s_info,"系统繁忙,请退卡稍后再试--错误码:%02X,返回码:%02X", TASKSTEP_RECHARGE_STAGE1TO2, agent_err);
				emit signals_Result(result);
				goto Done;	
			}

			apply_result = recharge_stage2_STC.apply_result; //申请结果
			printf("Recharge Apply Result: 0x%02X\n\n",apply_result);
			
			LIB_LNT_log_vsprintf("****************Begin Recharge Process****************\n");
			LIB_LNT_log_vsprintf("PKT SN:%d, PID: %02x %02x %02x %02x %02x %02x %02x %02x\n", lib_pkt_sn_RO_get(), \
				SAVE_pid[0], SAVE_pid[1], SAVE_pid[2], SAVE_pid[3], SAVE_pid[4], SAVE_pid[5], SAVE_pid[6], SAVE_pid[7]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Apply Result: 0x%02X\n", lib_pkt_sn_RO_get(), apply_result);

			lib_printf("Input OrderNum",recharge_stage1_CTS.order,10);

			switch(apply_result)     
			{				      
				case 0x10: //允许充值
				{
					result->result = TASK_RESULT_OP_LOOP;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "正在获取充值订单号,请不要移动卡片,并耐心等待......");
					emit signals_Result(result);  
					
					taskStep = TASKSTEP_RECHARGE_STAGE3TO4;
				}
				break;

				case 0x11: //对应订单号不存在
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "您输入的充值订单号不存在,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				case 0x12: //对应订单号已失效(已经充值过)
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "您输入的充值订单号已失效,请输入有效的充值订单号再执行充值操作");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				case 0x13: //充值额度不符
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "充值额度不符,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0x14: //找人工处理
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "请找人工咨询处理,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;
				
				case 0xFF: //暂停服务
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_OUT_OF_SERVICE;
					strcpy(result->s_info, "设备暂停该功能服务,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;

				default:  //默认
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE1TO2;
					strcpy(result->s_info, "会员充值失败,服务出现异常,请退卡");
					emit signals_Result(result);  
					goto Done;
				}
				break;
			}
		}

		emit_TaskRunStep(TaskRunStep_p2);  //查验会员权限[2]
		
		/* 执行充值3-4阶段 */ //获取充值订单号
		if(taskStep == TASKSTEP_RECHARGE_STAGE3TO4)
		{
			memset(&recharge_stage4, 0, sizeof(lnt_agent_recharge_stage4_STC_t));
			
			agent_err = lib_lnt_recharge_stage3TO4_process(sockfd, &extra, &recharge_stage4_STC);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE3TO4;
				sprintf(result->s_info,"获取充值订单号失败,如尝试多次后,仍然失败,请找人工咨询处理--错误码:%02X,返回码:%02X", TASKSTEP_RECHARGE_STAGE3TO4, agent_err);
				emit signals_Result(result);
				goto Done;	
			}
			
			//lib_printf_v2("--------recharge order---------",(unsigned char *)&recharge_stage4_STC.order,sizeof(recharge_stage4_STC.order),16);
			//lib_printf_v2("--------recharge amount---------",(unsigned char *)&recharge_stage4_STC.recharge,sizeof(recharge_stage4_STC.recharge),16);

			#if 1
			/* 检查充值上限 */
			unsigned int amount = LNT_NTOHL(recharge_stage4_STC.recharge); //要充值金额
			if((ticket_blance + amount) > LNT_CPU_RECHARGE_LIMIT)
			{
				result->step = TASKSTEP_RECHARGE_STAGE3TO4;
				result->result = TASK_RESULT_OP_FAIL_FREE;
				sprintf(result->s_info,"该卡充值%02d.%02d元后会超过%d.%02d元余额上限,不允许会员充值操作,如有疑问,请找人工咨询处理,请退卡", amount / 100, amount % 100, 
								LNT_CPU_RECHARGE_LIMIT / 100, LNT_CPU_RECHARGE_LIMIT % 100);
				emit signals_Result(result);
				goto Done;
			}
			#endif		
			#if 1
			sprintf(result->s_info,"该充值订单号对应的充值金额为%02d.%02d元,正在执行充值操作,请耐心等待充值完成......", amount / 100, amount % 100);
			emit signals_Result(result);
			#endif
			taskStep = TASKSTEP_RECHARGE_STAGE5TO6; //获取订单成功后执行充值
		}

		/* 执行充值5-6阶段 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE5TO6) //DD11 充值第一阶段
		{
			emit_TaskRunStep(TaskRunStep_p3);   //开始会员充值[3]
			
			/* 信息查询 */
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
			ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 500);	
			if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
			{
				if(memcmp(SAVE_pid, qry_ticket.ticket.pid, 8) != 0) //卡号不符合
				{
					result->result = TASK_RESULT_OP_FAIL_FREE;
					result->step = TASKSTEP_RECHARGE_STAGE5TO6;
					sprintf(result->s_info,"会员充值失败,出现卡号不一致,请退卡");
					emit signals_Result(result);
					goto Done;
				}
			}

			bal = LNT_NTOHL(recharge_stage4_STC.recharge);
				
			result->result = TASK_RESULT_OP_LOOP;
			result->step = TASKSTEP_RECHARGE_STAGE5TO6;
			sprintf(result->s_info,"正在执行会员充值操作,充值金额为%d.%02d元,请不要移动卡片,并耐心等待......", bal / 100, bal % 100);
			emit signals_Result(result);
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Order: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", lib_pkt_sn_RO_get(), recharge_stage4_STC.order[0],recharge_stage4_STC.order[1],\
				recharge_stage4_STC.order[2],recharge_stage4_STC.order[3],recharge_stage4_STC.order[4],recharge_stage4_STC.order[5],recharge_stage4_STC.order[6],\
				recharge_stage4_STC.order[7],recharge_stage4_STC.order[8],recharge_stage4_STC.order[9]);
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Fee %d.%02d(Y)\n", lib_pkt_sn_RO_get(),  bal / 100, bal % 100);

			recharge_info_t recharge;
			memset(&recharge_stage2, 0, sizeof(lnt_packet_recharge_stage2_STC_t));
			recharge.apply_type = 0x00; //新协议只有0x00  2016-03-04

			memcpy(&recharge.recharge_amount, &recharge_stage4_STC.recharge, 4); //充值金额 [高位在前]
			memcpy(&recharge.order_num, &recharge_stage4_STC.order, 10); //订单号 

			agent_err = lib_lnt_recharge_stage5TO6_process(sockfd, &extra, &recharge, &recharge_stage2);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE5TO6;
				sprintf(result->s_info,"会员充值失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,返回码:%02X", TASKSTEP_RECHARGE_STAGE5TO6, agent_err);
				emit signals_Result(result);
				goto Done;	
			}     

			if(recharge_stage2.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE5TO6;

				if(recharge_stage2.retcode == 0xDD)
				{
					sprintf(result->s_info, "系统繁忙导致会员充值失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_OFF_STAGE13TO14, recharge_stage2.retcode);
				}else{
					sprintf(result->s_info,"会员充值失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_RECHARGE_STAGE5TO6, recharge_stage2.retcode);
				}
				
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_RECHARGE_STAGE7TO8;
		}
		
		/* 执行充值7-8阶段 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE7TO8)
		{
			memset(&recharge_stage4, 0, sizeof(lnt_packet_recharge_stage4_STC_t));
			
			agent_err = lib_lnt_recharge_stage7TO8_process(sockfd, &extra, &recharge_stage2, &recharge_stage4);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE7TO8;
				sprintf(result->s_info,"会员充值失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,返回码:%02X,状态码:%02X", TASKSTEP_RECHARGE_STAGE7TO8, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage4.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE7TO8;
				sprintf(result->s_info,"会员充值失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_RECHARGE_STAGE7TO8, recharge_stage4.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_RECHARGE_STAGE9TO10;
		}
			
		/* 执行充值9-10阶段 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE9TO10)
		{
			memset(&recharge_stage6, 0, sizeof(lnt_packet_recharge_stage6_STC_t));
			
			agent_err = lib_lnt_recharge_stage9TO10_process(sockfd, &extra, &recharge_stage4, &recharge_stage6);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE9TO10;
				sprintf(result->s_info,"会员充值失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,返回码:%02X,状态码:%02X", TASKSTEP_RECHARGE_STAGE9TO10, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage6.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE9TO10;
				sprintf(result->s_info,"会员充值失败,如尝试多次后还是失败,请找客服处理--错误码:%02X,信息码:%02X", TASKSTEP_RECHARGE_STAGE9TO10, recharge_stage6.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			taskStep = TASKSTEP_RECHARGE_STAGE11TO12;
		}
		
		/* 
		 * 执行充值11-12阶段 岭南通后台返回DD18报文后,就认为充值已经成功
		 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE11TO12)
		{
			memset(&recharge_stage8, 0, sizeof(lnt_packet_recharge_stage8_STC_t));
			
			agent_err = lib_lnt_recharge_stage11TO12_process(sockfd, &extra, &recharge_stage6, &recharge_stage8);
			if(agent_err != AGENT_ERR_OK)
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE11TO12;
				sprintf(result->s_info,"会员充值失败,请检查卡内余额,如有疑问请找人工咨询处理,请退卡--错误码:%02X,返回码:%02X,状态码:%02X", TASKSTEP_RECHARGE_STAGE11TO12, agent_err, extra.r_stat);
				emit signals_Result(result);
				goto Done;	
			}

			if(recharge_stage8.retcode != 0x00) //0x00正常
			{
				result->result = TASK_RESULT_OP_FAIL_FREE;
				result->step = TASKSTEP_RECHARGE_STAGE11TO12;
				sprintf(result->s_info,"会员充值失败,请检查卡内余额,如有疑问请找人工咨询处理,请退卡--错误码:%02X,信息码:%02X", TASKSTEP_RECHARGE_STAGE11TO12, recharge_stage8.retcode);
				emit signals_Result(result);
				goto Done;	
			}

			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Process: [DD18] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			taskStep = TASKSTEP_RECHARGE_STAGE13TO14;
		}

		/* 执行充值13-14阶段 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE13TO14)
		{
			extra.result = 0x00; 
			
			memset(&taskstep_err_code, 0, sizeof(struct task_err_code));
			memset(&recharge_stage10, 0, sizeof(lnt_packet_recharge_stage10_STC_t));

			/* 包含R_CPU_LOAD*/
			agent_err = lib_lnt_recharge_stage13TO14_process(sockfd, &extra, &recharge_stage8, &recharge_stage10, &r_cpu_load); //CPU圈存
			if(r_cpu_load != 0x00)  //说明充值写卡失败
			{
				taskstep_err_code.task_stat = extra.r_stat;
				extra.result = 0xff;
				taskstep_err_code.task_step = TASKSTEP_RECHARGE_STAGE13TO14;
				taskstep_err_code.task_ret = AGENT_ERR_UNKNOWN;

				taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
				goto DONE_TASKSTEP_RECHARGE_STAGE17TO18;	//上报充值结果
			}
			
			LIB_LNT_log_vsprintf("PKT SN:%d, Recharge Process: [R_CPU_LOAD] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD已经成功，说明充值已经成功
			{
				extra.result = 0x00;
				taskstep_err_code.task_ret = AGENT_ERR_OK;

				taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
				goto DONE_TASKSTEP_RECHARGE_STAGE17TO18;	//上报充值结果
			}

			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Recharge Process: [DD20] Ok\n", lib_pkt_sn_RO_get());//add by zjc
						
			taskStep = TASKSTEP_RECHARGE_STAGE15TO16;
		}

		/* 执行充值15-16阶段 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE15TO16)
		{	
			memset(&recharge_stage12, 0, sizeof(lnt_packet_recharge_stage12_STC_t));
			
			agent_err = lib_lnt_recharge_stage15TO16_process(sockfd, &extra, &recharge_stage10, &recharge_stage12);
			if((agent_err != AGENT_ERR_OK) || (recharge_stage10.retcode != 0x00)) // R_CPU_LOAD已经成功，说明充值已经成功
			{
				extra.result = 0x00;
				taskstep_err_code.task_ret = AGENT_ERR_OK;

				taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
				goto DONE_TASKSTEP_RECHARGE_STAGE17TO18;	//上报充值结果
			}
			
			LIB_LNT_log_vsprintf_debug(1,"PKT SN:%d, Recharge Process: [DD22] Ok\n", lib_pkt_sn_RO_get());//add by zjc

			
			taskStep = TASKSTEP_RECHARGE_STAGE17TO18;
		}

	DONE_TASKSTEP_RECHARGE_STAGE17TO18:
		/* 执行充值17-18阶段 */
		if(taskStep == TASKSTEP_RECHARGE_STAGE17TO18)
		{
			/*上报充值订单号和充值金额 zjc,2016-03-24*/
			memcpy(&extra.order, &recharge_stage4_STC.order, 10); //订单号 
			memcpy(&extra.recharge, &recharge_stage4_STC.recharge, 4); //充值金额 [高位在前]

			lnt_agent_off_card_stage24_STC_t off_card_stage24_STC;
			memset(&off_card_stage24_STC, 0, sizeof(lnt_agent_off_card_stage24_STC_t));
			memset(&retrans, 0, sizeof(agent_retransmission_data_t));

			agent_err = lib_lnt_recharge_stage17TO18_process(sockfd, &extra, &off_card_stage24_STC, &retrans);
			if(agent_err != AGENT_ERR_OK)
			{
				/* 充值结果上传失败，插入数据库,重传 [4]*/
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
				sprintf(result->s_info,"会员充值失败,请检查卡内余额,如有疑问请找人工咨询处理,请退卡--错误码:%02X, 状态码:%02X", taskstep_err_code.task_step, taskstep_err_code.task_stat);
				emit signals_Result(result);
				goto Done;
			}

			memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));

			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
			bal = qry_ticket.ticket.blance;


			emit_TaskRunStep(TaskRunStep_p4);	//注意充值成功[4]

			time(&end_times);
			consume_time = end_times - begin_times;
			
			result->result = TASK_RESULT_OP_LOOP_FREE;
			result->step = TASKSTEP_RECHARGE_STAGE17TO18;
			;(result->s_info,"会员充值成功,现在票卡余额为%d.%02d元,全过程耗时为%d秒,请退卡,返回码:%02X", bal / 100, bal % 100, consume_time, agent_err);
			emit signals_Result(result);
		}
	}


Done:
	LIB_LNT_log_close();
	if(sockfd > 0)
	{
		lib_lnt_on_card_close_to_agent(sockfd);  //关闭套接字
	}
	
	return;  //如果是正常处理,执行完毕后，直接返回
		
BlackListDeal:  //黑名单处理
		memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
		stat = LNT_MOC_ERROR_CARD_OP_FAIL;
		
		ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
		if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))  
		{
			lib_lnt_blacklist_record_ack_t blacklist_rec;
			memset(&blacklist_rec, 0, sizeof(lib_lnt_blacklist_record_ack_t));
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;

			memcpy(&(blacklist_rec.blacklist.lid), &(qry_ticket.ticket.lid), 8);  //逻辑卡号
			memcpy(&(blacklist_rec.blacklist.pid), &(qry_ticket.ticket.pid), 8);  //物理卡号
			lib_lnt_set_blacklist_flag(&blacklist_rec, &stat, 500);	//置黑操作
		}
		
		memset(&qry_ticket, 0, sizeof(lnt_qry_ticket_info_ack_t));
		stat = LNT_MOC_ERROR_CARD_OP_FAIL;
		
		ret = lib_lnt_qry_ticket_info(&qry_ticket, &stat, 300);
		if((ret > 0) && (stat == LNT_MOC_ERROR_CARD_NAMELIST))    //黑名单卡
		{
			if(taskdata.serviceType == CARD_SERVICE_ON)
			{
				memcpy(&(extra.pid), &(qry_ticket.ticket.pid), 8);	//物理卡号
				memcpy(&(extra.lid), &(qry_ticket.ticket.lid), 8);  //逻辑卡号
				
				lib_lnt_on_card_blacklist_process(sockfd, &extra);	//上报黑名单处理结果
			}
			else if(taskdata.serviceType == CARD_SERVICE_OFF)
			{
				memcpy(&(extra.pid), &(qry_ticket.ticket.pid), 8);	//物理卡号
				memcpy(&(extra.lid), &(qry_ticket.ticket.lid), 8);  //逻辑卡号
				
				lib_lnt_off_card_blacklist_process(sockfd, &extra);	//上报黑名单处理结果
			}
		}
			
		time(&end_times);
		consume_time = end_times - begin_times;
	#if 0
		result->result = TASK_RESULT_OP_LOOP_FREE;
		sprintf(result->s_info, "黑名单卡捕获成功,全过程耗时为%d秒,请退卡", consume_time);
		emit signals_Result(result);
	#endif
		LIB_LNT_log_close();
		if(sockfd > 0)
		{
			lib_lnt_on_card_close_to_agent(sockfd);  //关闭套接字
		}
}


