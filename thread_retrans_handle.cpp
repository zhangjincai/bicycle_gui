#include <QDebug>
#include "lib_general.h"
#include "lib_records.h"
#include "lib_lnt.h"
#include "lib_gui.h"
#include "lib_wireless.h"

#include "thread_retrans_handle.h"


extern lib_records_file_t *g_rec_file;
extern lib_gui_t *g_gui;
extern lib_wl_t *g_wl;


#define RETRANS_SLEEP_S				(10)
#define RETRANS_SLEEP_NEXT_TIME_S				(60)



ThreadRetransHandle::ThreadRetransHandle(void)
{
	
}

void ThreadRetransHandle::ThreadSleep(unsigned long secs)

{
	lib_sleep(secs);
}

void ThreadRetransHandle::mThreadSleep(unsigned long msecs)
{
	lib_msleep(msecs);
}

void ThreadRetransHandle::run()
{
	int ret = LIB_REC_ERROR;
	lib_lnt_config_t lnt_conf;
	lnt_agent_packet_head_t agent_pkt_hd;
	int count = 0;
	int sockfd = -1;
	int pkt_sn, pkt_len;
	unsigned char pkt_data[512] = {0};
	unsigned char txbuf[512] = {0};
	int tmp_pkt_sn;
	ThreadSleep(RETRANS_SLEEP_S / 2);

	
	//LIB_LNT_log_open();
#if 1
	while(1)
	{
		/* 开卡重传 */
		ret = lib_records_oncard_ack_count(g_rec_file, &count);  //查询有多少条未发开卡记录

		//fprintf(stderr, "ON CRAD count ret:%d, count:%d\n", ret, count);

		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //数据库有数据
			{
				while(1)  //循环发送数据
				{
					sockfd =  lib_lnt_on_card_connect_to_agent();  
					if(sockfd <= 0)
					{
						sockfd = -1;
						ThreadSleep(RETRANS_SLEEP_S);
						continue;
					}

					lib_lnt_get_config(&lnt_conf);
						
					//fprintf(stderr, "ON CARD Agent sockfd:%d, Delay ms:%d\n", sockfd, lnt_conf.delay_ms);
					
					ret = lib_records_oncard_ack_select_all_data(g_rec_file, &pkt_sn, &pkt_len, &pkt_data);
					if((ret == LIB_REC_OK) && (pkt_sn > 0))
					{
						lib_tcp_writen(sockfd, pkt_data, pkt_len);
						ret = lib_tcp_read_select(sockfd, txbuf, sizeof(txbuf), lnt_conf.delay_ms);
						if(ret > 0)
						{
							memset(&agent_pkt_hd, 0, sizeof(lnt_agent_packet_head_t));
							memcpy(&agent_pkt_hd, txbuf, sizeof(lnt_agent_packet_head_t));

							tmp_pkt_sn = pkt_sn;	//上传的报文序列号
							pkt_sn = LNT_NTOHL(agent_pkt_hd.sn);	//下发的报文序列号

							fprintf(stderr, "ON CARD pkt_sn:%d, sn:%d\n", tmp_pkt_sn, pkt_sn);

							if(tmp_pkt_sn == pkt_sn)	//报文序列号一致则重传成功，删除对应记录
							{
								lib_records_oncard_ack_delete_data(g_rec_file, pkt_sn); 
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans On Card Record OK\n\n", pkt_sn);
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //休眠,避免死循环

							break;  //跳出内while(1)
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //重传没响应则休眠,避免死循环
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //跳出内循环
					}
				}
			}
		}

		/* 消费重传 */
		ret = lib_records_reconsume_count(g_rec_file, &count);  //查询有多少条未发消费记录

		//fprintf(stderr, "RECONSUME count ret:%d, count:%d\n", ret, count);

		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //数据库有数据
			{
				while(1)  //循环发送数据
				{
					sockfd =  lib_lnt_on_card_connect_to_agent();  
					if(sockfd <= 0)
					{
						sockfd = -1;
						ThreadSleep(RETRANS_SLEEP_S);
						continue;
					}

					lib_lnt_get_config(&lnt_conf);
						
					//fprintf(stderr, "RECONSUME Agent sockfd:%d, Delay ms:%d\n", sockfd, lnt_conf.delay_ms);
					
					ret = lib_records_reconsume_select_all_data(g_rec_file, &pkt_sn, &pkt_len, &pkt_data);
					if((ret == LIB_REC_OK) && (pkt_sn > 0))
					{
						lib_tcp_writen(sockfd, pkt_data, pkt_len);
						ret = lib_tcp_read_select(sockfd, txbuf, sizeof(txbuf), lnt_conf.delay_ms);
						if(ret > 0)
						{
							memset(&agent_pkt_hd, 0, sizeof(lnt_agent_packet_head_t));
							memcpy(&agent_pkt_hd, txbuf, sizeof(lnt_agent_packet_head_t));

							tmp_pkt_sn = pkt_sn;
							pkt_sn = LNT_NTOHL(agent_pkt_hd.sn);

							fprintf(stderr, "RECONSUME pkt_sn:%d, sn:%d\n", tmp_pkt_sn, pkt_sn);

							if(tmp_pkt_sn == pkt_sn)	//报文序列号一致则重传成功，删除对应记录
							{
								lib_records_reconsume_delete_data(g_rec_file, pkt_sn); 
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans Consume Record OK\n", pkt_sn);	//交易记录重传成功
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //休眠,避免死循环

							break;
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //重传没响应则休眠,避免死循环
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //跳出内循环
					}
				}
			}
		}		

		/* 销卡重传 */
		count = 0;
		ret = lib_records_offcard_ack_count(g_rec_file, &count);  //查询有多少条未发销卡记录

		//fprintf(stderr, "OFF CRAD count ret:%d, count:%d\n", ret, count);
		
		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //数据库有数据
			{
				while(1)  //循环发送数据
				{
					sockfd =  lib_lnt_on_card_connect_to_agent();  
					if(sockfd <= 0)
					{
						sockfd = -1;
						ThreadSleep(RETRANS_SLEEP_S);
						continue;
					}

					lib_lnt_get_config(&lnt_conf);
						
					//fprintf(stderr, "OFF CARD Agent sockfd:%d, Delay ms:%d\n", sockfd, lnt_conf.delay_ms);
					
					ret = lib_records_offcard_ack_select_all_data(g_rec_file, &pkt_sn, &pkt_len, &pkt_data);
					if((ret == LIB_REC_OK) && (pkt_sn > 0))
					{
						lib_tcp_writen(sockfd, pkt_data, pkt_len);
						ret = lib_tcp_read_select(sockfd, txbuf, sizeof(txbuf), lnt_conf.delay_ms);
						if(ret > 0)
						{
							memset(&agent_pkt_hd, 0, sizeof(lnt_agent_packet_head_t));
							memcpy(&agent_pkt_hd, txbuf, sizeof(lnt_agent_packet_head_t));

							tmp_pkt_sn = pkt_sn;
							pkt_sn = LNT_NTOHL(agent_pkt_hd.sn);

							fprintf(stderr, "OFF CARD pkt_sn:%d, sn:%d\n", tmp_pkt_sn, pkt_sn);

							if(tmp_pkt_sn == pkt_sn)	//报文序列号一致则重传成功，删除对应记录
							{
								lib_records_offcard_ack_delete_data(g_rec_file, pkt_sn); 
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans Off Card Deposit Record OK\n", pkt_sn);
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //休眠,避免死循环

							break;
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //重传没响应则休眠,避免死循环
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //跳出内循环
					}
				}
			}
		}


		/* 充值重传 */
		count = 0;
		ret = lib_records_recharge_count(g_rec_file, &count);  //查询有多少条未发销卡记录

		//fprintf(stderr, "RECHARGE count ret:%d, count:%d\n", ret, count);
		
		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //数据库有数据
			{
				while(1)  //循环发送数据
				{
					sockfd =  lib_lnt_on_card_connect_to_agent();  
					if(sockfd <= 0)
					{
						sockfd = -1;
						ThreadSleep(RETRANS_SLEEP_S);
						continue;
					}

					lib_lnt_get_config(&lnt_conf);
						
					//fprintf(stderr, "Recharge Agent sockfd:%d, Delay ms:%d\n", sockfd, lnt_conf.delay_ms);
					
					ret = lib_records_recharge_select_all_data(g_rec_file, &pkt_sn, &pkt_len, &pkt_data);
					if((ret == LIB_REC_OK) && (pkt_sn > 0))
					{
						lib_tcp_writen(sockfd, pkt_data, pkt_len);
						ret = lib_tcp_read_select(sockfd, txbuf, sizeof(txbuf), lnt_conf.delay_ms);
						if(ret > 0)
						{
							memset(&agent_pkt_hd, 0, sizeof(lnt_agent_packet_head_t));
							memcpy(&agent_pkt_hd, txbuf, sizeof(lnt_agent_packet_head_t));

							tmp_pkt_sn = pkt_sn;
							pkt_sn = LNT_NTOHL(agent_pkt_hd.sn);

							fprintf(stderr, "Recharge pkt_sn:%d, sn:%d\n", tmp_pkt_sn, pkt_sn);

							if(tmp_pkt_sn == pkt_sn)	//报文序列号一致则重传成功，删除对应记录
							{
								lib_records_recharge_delete_data(g_rec_file, pkt_sn);
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans Recharge Record OK\n\n", pkt_sn);
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //休眠,避免死循环

							break;
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //重传没响应则休眠,避免死循环
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //跳出内循环
					}
				}
			}
		}

		
		ThreadSleep(RETRANS_SLEEP_S);	//数据库没有数据时每隔10秒查询一次

		if(sockfd > 0)
		{
			lib_lnt_on_card_close_to_agent(sockfd);
			sockfd = -1;
		}
	}
#endif
	//LIB_LNT_log_close();
}

