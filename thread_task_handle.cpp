#include <QDebug>
#include "lib_general.h"
#include "lib_lnt.h"
#include "lib_gui.h"
#include "lib_serialnumber.h"
#include "thread_task_handle.h"

extern lib_gui_t *g_gui;

#define TTH_DBG								1

#define CHECK_EXCEPTION_PROCESS_RESAULT 	1 //检查异常处理写卡是否成功

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
			ExceptionHandle(TaskResult);  //异常处理
		}
		break;
	}
}

/*
 * 异常处理流程
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
		if(threadTaskStep == EX_TASK_STEP_LNT_DEVICE)  //读卡器查询
		{
#ifdef TTH_DBG
			qDebug("----------------EX_TASK_STEP_LNT_DEVICE---%d-------------\n", EX_TASK_STEP_LNT_DEVICE);
#endif		
			timerCount = 0;
			retval = LIB_LNT_ERROR;
			stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_getversion_ack_t ver;
			retval = lib_lnt_get_version(&ver, &stat, 300);
			if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL))  //没有读卡器
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
				TaskResult->step = EX_TASK_STEP_LNT_DEVICE;
				sprintf(TaskResult->s_info, "提示:读卡器异常,不能执行异常处理操作");
				
				emit signals_ExceptionResult(TaskResult);
				return -1;
			}
			
			threadTaskStep = EX_TASK_STEP_REGISTER;
		}

		if(threadTaskStep == EX_TASK_STEP_REGISTER) //网络注册
		{
#ifdef TTH_DBG
			qDebug("----------------EX_TASK_STEP_REGISTER---%d-------------\n", EX_TASK_STEP_REGISTER);
#endif
			timerCount = 0;
			gui_access_state_t state;
			memset(&state, 0, sizeof(gui_access_state_t));

			/* 检查无线网络 */
			struct gui_ndev_access_pattern_config access_pattern_config;
			memset(&access_pattern_config, 0, sizeof(struct gui_ndev_access_pattern_config));
			retval = lib_gui_get_ndev_access_pattern_config(g_gui, &access_pattern_config, 500);
			if(retval == LIB_GUI_EOK)
			{
				if(access_pattern_config.using_wireless == 1) //无线
				{
					long long rx_bytes;
					long long tx_bytes;
					retval = lib_get_network_flow("ppp0", &rx_bytes,&tx_bytes);
					if(retval != LIB_GE_EOK)
					{
						TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
						TaskResult->step = EX_TASK_STEP_REGISTER;
						sprintf(TaskResult->s_info,  "提示:网络异常,不能执行异常处理操作");
						
						emit signals_ExceptionResult(TaskResult);
						return -1;
					}
				}
			}

			if(lib_gui_access_state(g_gui, &state, 500) == LIB_GUI_EOK)  //检查是否连接中心成功
			{
				if(state.connect_stat != 3)  //NDEV_NETSTAT_SESSION
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_REGISTER;
					sprintf(TaskResult->s_info,  "提示:连接中心异常,不能执行异常处理操作");
					
					emit signals_ExceptionResult(TaskResult);
					return -1;
				}
			}

			timerCount = threadTaskData.refresh_period - 1 - (8);
			threadTaskStep = EX_TASK_STEP_TICKET_QRY;
		}

		if(threadTaskStep == EX_TASK_STEP_TICKET_QRY) //询卡
		{
#ifdef TTH_DBG
			if(timerCount ==  threadTaskData.refresh_period - 1 - (8))
				qDebug("----------------EX_TASK_STEP_TICKET_QRY---%d-------------\n", EX_TASK_STEP_TICKET_QRY);
#endif
			memset(&lnt_qry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));
			retval = lib_lnt_qry_ticket_info(&lnt_qry_ticket, &stat, 300);
			if((retval > 0) && (stat == LNT_MOC_ERROR_CARD_NOTAG))  //没有卡
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_SHOW_LOOP;
				TaskResult->step = EX_TASK_STEP_TICKET_QRY;
				sprintf(TaskResult->s_info, "提示:请在%02d秒内,把卡放置到指定刷卡区域上", timerCount--);
				emit signals_ExceptionResult(TaskResult);

				mThreadSleep(800);

				if(timerCount == 0)
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_TICKET_QRY;
					sprintf(TaskResult->s_info, "提示:检测不到卡片");
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				
				continue;
			}
			else  if((retval > 0) && (stat == LNT_MOC_ERROR_M1))
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
				TaskResult->step = EX_TASK_STEP_TICKET_QRY;
				sprintf(TaskResult->s_info,"提示:不支持该类型卡操作");
				emit signals_ExceptionResult(TaskResult);
				return -1;
			}
			else if((retval > 0) && (stat == LNT_MOC_ERROR_NORMAL))
			{
				memcpy(ex_pid, lnt_qry_ticket.ticket.pid, 8);
				timerCount = 0;
				pmflag_t pf;
				memcpy(&pf, &(lnt_qry_ticket.ticket.pmflag), 1);  //押金标志
                                if(lnt_qry_ticket.ticket.pursetype != 0x02)  //非CPU卡
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_TICKET_QRY;
					sprintf(TaskResult->s_info,"提示:不支持该类型卡操作");
					emit signals_ExceptionResult(TaskResult);
					return -1;
				}
				
				if((lnt_qry_ticket.ticket.pmflag == 0xff) || (pf.pflag != 1))  
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
					TaskResult->step = EX_TASK_STEP_TICKET_QRY;
					sprintf(TaskResult->s_info,"提示:不支持非会员卡操作");
					emit signals_ExceptionResult(TaskResult);
					return -1;
				}

				threadTaskStep = EX_TASK_STEP_RENT_INFO_QRY;
			}
			else if(stat == LNT_MOC_ERROR_CARD_NAMELIST)  //黑名单卡
			{
				TaskResult->result = THREAD_TASK_RESULT_OP_FAIL_FREE;
				TaskResult->step = EX_TASK_STEP_LNT_DEVICE;
				sprintf(TaskResult->s_info, "提示:该卡是黑名单卡,不能做异常处理");
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
				if(lnt_rent_ack.rent.rent_flag == 0x00) //已还车
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_RENT_INFO_QRY;
					sprintf(TaskResult->s_info,"提示:未检测到卡异常"); 
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
			lib_SN_except_read32(&m_serialNumber);  //流水号
			e_hndl_req.sn = LNT_HTONL(m_serialNumber);
			memcpy(&e_hndl_req.pid, &lnt_qry_ticket.ticket.pid, 8); //物理卡号
			memcpy(&e_hndl_req.ticket, &lnt_qry_ticket.ticket, 42);  //拷贝票卡信息
			memcpy(&e_hndl_req.rentinfo, &(lnt_rent_ack.rent), 36);  //租还车信息

			//qDebug("m_serialNumber=%d\n", m_serialNumber);

            retval = lib_gui_except_handle_req(g_gui, &e_hndl_req, 300); //从中心获取异常信息
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
				sprintf(TaskResult->s_info, "提示:异常处理进行中,请勿移动卡片并耐心等待 %02d秒,并仔细留意屏幕上的提示", timerCount--);
				emit signals_ExceptionResult(TaskResult);

				mThreadSleep(800);
				
				if(timerCount == 0)
				{
					doneFree = 2; //超时
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

		if(threadTaskStep == EX_TASK_STEP_EHANDLE_START)  //开始做异常处理
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
			sprintf(TaskResult->s_info, "异常处理状态:"); //for test
			strcat(TaskResult->s_info, (const char*)e_hndl_ack.info); //for test
			emit signals_ExceptionResult(TaskResult);

			lib_sleep(1);
			//----------------for test------------------
		#endif

			switch(e_hndl_ack.status)
			{
				case GUI_EX_HDL_NORMAL: //正常
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					sprintf(TaskResult->s_info,"提示:该卡无异常或异常记录未上传,请稍后再试或者找人工处理\n");
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_NOT_CARD:  //还车未刷卡
				{
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* 查询租还车信息 */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"提示:还车未刷卡异常处理过程中查询票卡信息失败,请重试");						doneFree = 1;
						goto Done;
					}
					
					/* 设置租还车信息 */
					lib_lnt_rent_info_t rent_info;
					memcpy(&rent_info.rent, &(lnt_rent_ack.rent), sizeof(struct rent_info));

					rent_info.id = 0x68;
					rent_info.length = sizeof(lib_rent_info_t );
					rent_info.rent.rent_flag = 0;  //修改为已还车
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					retval = lib_lnt_set_rent_info(&rent_info, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"提示:还车未刷卡异常处理过程中写卡失败,请重试");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}
					
					/* 消费写卡 */
					lib_lnt_consume_req_t lnt_cse;
					lib_lnt_trade_record_ack_t lnt_trade_rec;
					
					memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lnt_cse.amount = f_fee;
					lnt_cse.fee = f_fee;
					lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
					memcpy(lnt_cse.phyiscalid, lnt_qry_ticket.ticket.pid, 8);
					lnt_cse.aty = LNT_ATY_RETURN; //收押金
					rstat = LNT_MOC_ERROR_CARD_OP_FAIL;
					
					retval = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &rstat, &f_type);
					if(retval != LNT_EOK)
					{
						if(f_type == LNT_GET_TRANS)  
						{
							stat = LNT_MOC_ERROR_CARD_OP_FAIL;
							memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
							retval = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //防止读取消费记录失败
							if((retval <  0) || (stat != LNT_MOC_ERROR_NORMAL))
							{
								doneFree = 1;
								goto Done;
							}
						}
						else	//消费失败 2016-11-01
						{
                            sprintf(TaskResult->s_info,"提示:还车未刷卡异常处理失败,请重试");
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
						
						lib_gui_set_exception_handle_record(g_gui, &ex_record, 500);  //上传消费记录
					
						//sprintf(TaskResult->s_info,"提示:还车未刷卡异常处理成功,扣除%02d.%02d元", f_fee / 100, f_fee % 100);
					}

					
				#if CHECK_EXCEPTION_PROCESS_RESAULT
					/* 增加写卡是否成功的判断 add by zjc at 2016-08-12 */
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* 查询租还车信息 */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if(lnt_rent_ack.rent.rent_flag != 0) 
					{
						sprintf(TaskResult->s_info,"提示:还车未刷卡异常处理时写卡失败,请重试或找人工处理");
						goto Done;
					}
				#endif
				
					sprintf(TaskResult->s_info,"提示:还车未刷卡异常处理成功,扣除%02d.%02d元", f_fee / 100, f_fee % 100);

					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_NOT_COST:  //欠费
				{
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* 查询租还车信息 */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"提示:欠费异常处理过程中查询票卡信息失败,请重试");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}
					
					/* 设置租还车信息 */
					lib_lnt_rent_info_t rent_info;
					memcpy(&rent_info.rent, &(lnt_rent_ack.rent), sizeof(struct rent_info));

					rent_info.id = 0x68;
					rent_info.length = sizeof(lib_rent_info_t );
					rent_info.rent.rent_flag = 0;  //修改为已还车
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					retval = lib_lnt_set_rent_info(&rent_info, &stat, 200);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"提示:欠费异常处理过程中写卡失败,请重试");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}

					/* 消费写卡 */
					lib_lnt_consume_req_t lnt_cse;
					lib_lnt_trade_record_ack_t lnt_trade_rec;
					
					memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lnt_cse.amount = f_fee;
					lnt_cse.fee = f_fee; //扣费
					lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
					memcpy(lnt_cse.phyiscalid, lnt_qry_ticket.ticket.pid, 8);
					lnt_cse.aty = LNT_ATY_RETURN; //收押金
					rstat = LNT_MOC_ERROR_CARD_OP_FAIL;
					
					retval = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &rstat, &f_type);
					if(retval != LNT_EOK)
					{
						if(f_type == LNT_GET_TRANS)  
						{
							stat = LNT_MOC_ERROR_CARD_OP_FAIL;
							memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
							retval = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //防止读取消费记录失败
							if((retval <  0) || (stat != LNT_MOC_ERROR_NORMAL))
							{
								doneFree = 1;
								goto Done;
							}
						}
						else	//消费失败 2016-05-17
						{
                            sprintf(TaskResult->s_info,"提示:欠费异常处理失败,请重试");
							goto Done;
						}
					}

					if(f_fee > 0) //增加费用判断 add by zjc at 2016-11-01
					{
						//printf("--------------------upload exception record----------------\n");

						memset(&ex_record, 0, sizeof(gui_exception_handle_record_t));

						ex_record.sn = LNT_HTONL(m_serialNumber);  //序列号
						memcpy(&(ex_record.pid), &(lnt_qry_ticket.ticket.pid), sizeof(ex_record.pid));
						memcpy(&(ex_record.record), &(lnt_trade_rec.record), sizeof(ex_record.record));
						lib_gui_set_exception_handle_record(g_gui, &ex_record, 500);  //上传消费记录
					}
					
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
                    sprintf(TaskResult->s_info,"提示:欠费异常处理成功,扣除%02d.%02d元", f_fee / 100, f_fee % 100);
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_NOT_DEPOSIT: //未交押金
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					sprintf(TaskResult->s_info,"提示:异常处理失败,非会员卡");
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;
			
				case GUI_EX_HDL_NOT_BIKE: //刷卡未取车
				{
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* 查询租还车信息 */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"提示:刷卡未取车异常处理过程中查询票卡信息失败,请重试");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}
					
					/* 设置租还车信息 */
					lib_lnt_rent_info_t rent_info;
					memcpy(&rent_info.rent, &(lnt_rent_ack.rent), sizeof(struct rent_info));

					rent_info.id = 0x68;
					rent_info.length = sizeof(lib_rent_info_t );
					rent_info.rent.rent_flag = 0;  //修改为已还车
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					retval = lib_lnt_set_rent_info(&rent_info, &stat, 300);
					if((retval < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
					{
						sprintf(TaskResult->s_info,"提示:刷卡未取车异常处理过程中写卡失败,请重试");						doneFree = 1;
						doneFree = 1;
						goto Done;
					}

					/* 消费写卡 */
					lib_lnt_consume_req_t lnt_cse;
					lib_lnt_trade_record_ack_t lnt_trade_rec;
					
					memset(&lnt_cse, 0, sizeof(lib_lnt_consume_req_t));
					memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
					
					lnt_cse.amount = f_fee;
					lnt_cse.fee = f_fee;
					lib_lnt_utils_time_bcd_yymmddhhMMss(lnt_cse.time);
					memcpy(lnt_cse.phyiscalid, lnt_qry_ticket.ticket.pid, 8);
					lnt_cse.aty = LNT_ATY_RETURN; //收押金
					rstat = LNT_MOC_ERROR_CARD_OP_FAIL;
					
					retval = lib_lnt_consume_and_get_record(&lnt_cse, &lnt_trade_rec, &rstat, &f_type);
					if(retval != LNT_EOK)
					{
						if(f_type == LNT_GET_TRANS)  
						{
							stat = LNT_MOC_ERROR_CARD_OP_FAIL;
							memset(&lnt_trade_rec, 0, sizeof(lib_lnt_trade_record_ack_t));
							retval = lib_lnt_get_trade_record(&lnt_trade_rec, &stat, 400);  //防止读取消费记录失败
							if((retval <  0) || (stat != LNT_MOC_ERROR_NORMAL))
							{
								doneFree = 1;
								goto Done;
							}
						}
						else	//消费失败 2016-05-17
						{
                            sprintf(TaskResult->s_info,"提示:刷卡未取车异常处理失败,请重试");
							goto Done;
						}
					}

					if(f_fee > 0)
					{
						memset(&ex_record, 0, sizeof(gui_exception_handle_record_t));
						ex_record.sn = LNT_HTONL(m_serialNumber);  //序列号
						memcpy(&(ex_record.pid), &(lnt_qry_ticket.ticket.pid), sizeof(ex_record.pid));
						memcpy(&(ex_record.record), &(lnt_trade_rec.record), sizeof(ex_record.record));

						//printf("--------------------upload exception record----------------\n");
						lib_gui_set_exception_handle_record(g_gui, &ex_record, 500);  //上传消费记录
					
						//sprintf(TaskResult->s_info,"提示:刷卡未取车异常处理成功,扣除%02d.%02d元", f_fee / 100, f_fee % 100);
					}

				#if CHECK_EXCEPTION_PROCESS_RESAULT
					/* 增加写卡是否成功的判断 add by zjc at 2016-08-19 */
					memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
					memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));

					/* 查询租还车信息 */
					stat = LNT_MOC_ERROR_CARD_OP_FAIL;
					lnt_rent_req.id = 0x68;
					retval = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &stat, 300);
					if(lnt_rent_ack.rent.rent_flag != 0) 
					{
						sprintf(TaskResult->s_info,"提示:刷卡未取车异常处理时写卡失败,请重试或找人工处理");
						goto Done;
					}
				#endif
				
					sprintf(TaskResult->s_info,"提示:刷卡未取车异常处理成功,扣除%02d.%02d元", f_fee / 100, f_fee % 100);
					
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					emit signals_ExceptionResult(TaskResult);

					return 0;
				}
				break;

				case GUI_EX_HDL_MANUAL: //后台异常
				{
					TaskResult->result = THREAD_TASK_RESULT_OP_FREE;
					TaskResult->step = EX_TASK_STEP_EHANDLE_START;
					sprintf(TaskResult->s_info,"提示:服务异常,请找人工处理");
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
			sprintf(TaskResult->s_info,"提示:异常处理超时");
		else
			sprintf(TaskResult->s_info,"提示:异常处理失败");
		
		emit signals_ExceptionResult(TaskResult);
	
		return 0;
	}
	
	return 0;
}


