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
		/* �����ش� */
		ret = lib_records_oncard_ack_count(g_rec_file, &count);  //��ѯ�ж�����δ��������¼

		//fprintf(stderr, "ON CRAD count ret:%d, count:%d\n", ret, count);

		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //���ݿ�������
			{
				while(1)  //ѭ����������
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

							tmp_pkt_sn = pkt_sn;	//�ϴ��ı������к�
							pkt_sn = LNT_NTOHL(agent_pkt_hd.sn);	//�·��ı������к�

							fprintf(stderr, "ON CARD pkt_sn:%d, sn:%d\n", tmp_pkt_sn, pkt_sn);

							if(tmp_pkt_sn == pkt_sn)	//�������к�һ�����ش��ɹ���ɾ����Ӧ��¼
							{
								lib_records_oncard_ack_delete_data(g_rec_file, pkt_sn); 
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans On Card Record OK\n\n", pkt_sn);
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //����,������ѭ��

							break;  //������while(1)
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //�ش�û��Ӧ������,������ѭ��
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //������ѭ��
					}
				}
			}
		}

		/* �����ش� */
		ret = lib_records_reconsume_count(g_rec_file, &count);  //��ѯ�ж�����δ�����Ѽ�¼

		//fprintf(stderr, "RECONSUME count ret:%d, count:%d\n", ret, count);

		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //���ݿ�������
			{
				while(1)  //ѭ����������
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

							if(tmp_pkt_sn == pkt_sn)	//�������к�һ�����ش��ɹ���ɾ����Ӧ��¼
							{
								lib_records_reconsume_delete_data(g_rec_file, pkt_sn); 
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans Consume Record OK\n", pkt_sn);	//���׼�¼�ش��ɹ�
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //����,������ѭ��

							break;
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //�ش�û��Ӧ������,������ѭ��
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //������ѭ��
					}
				}
			}
		}		

		/* �����ش� */
		count = 0;
		ret = lib_records_offcard_ack_count(g_rec_file, &count);  //��ѯ�ж�����δ��������¼

		//fprintf(stderr, "OFF CRAD count ret:%d, count:%d\n", ret, count);
		
		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //���ݿ�������
			{
				while(1)  //ѭ����������
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

							if(tmp_pkt_sn == pkt_sn)	//�������к�һ�����ش��ɹ���ɾ����Ӧ��¼
							{
								lib_records_offcard_ack_delete_data(g_rec_file, pkt_sn); 
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans Off Card Deposit Record OK\n", pkt_sn);
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //����,������ѭ��

							break;
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //�ش�û��Ӧ������,������ѭ��
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //������ѭ��
					}
				}
			}
		}


		/* ��ֵ�ش� */
		count = 0;
		ret = lib_records_recharge_count(g_rec_file, &count);  //��ѯ�ж�����δ��������¼

		//fprintf(stderr, "RECHARGE count ret:%d, count:%d\n", ret, count);
		
		if(ret == LIB_REC_OK)
		{
			if(count > 0)  //���ݿ�������
			{
				while(1)  //ѭ����������
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

							if(tmp_pkt_sn == pkt_sn)	//�������к�һ�����ش��ɹ���ɾ����Ӧ��¼
							{
								lib_records_recharge_delete_data(g_rec_file, pkt_sn);
								//LIB_LNT_log_vsprintf("PKT SN:%d, Retrans Recharge Record OK\n\n", pkt_sn);
							}
							
							if(sockfd > 0)
							{
								lib_lnt_on_card_close_to_agent(sockfd);
								sockfd = -1;
							}
							
							ThreadSleep(1);   //����,������ѭ��

							break;
						}
						
						ThreadSleep(RETRANS_SLEEP_NEXT_TIME_S);   //�ش�û��Ӧ������,������ѭ��
					}
					else
					{
						if(sockfd > 0)
						{
							lib_lnt_on_card_close_to_agent(sockfd);
							sockfd = -1;
						}
						break;  //������ѭ��
					}
				}
			}
		}

		
		ThreadSleep(RETRANS_SLEEP_S);	//���ݿ�û������ʱÿ��10���ѯһ��

		if(sockfd > 0)
		{
			lib_lnt_on_card_close_to_agent(sockfd);
			sockfd = -1;
		}
	}
#endif
	//LIB_LNT_log_close();
}

