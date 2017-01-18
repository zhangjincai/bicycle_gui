#ifndef __CONFIG_H__
#define __CONFIG_H__

#if 0
#define NEW_PROTOCOL				1	//�°汾�ĳ�ֵЭ��
#define	RETRANS_TRADE_RECORD		1	//���׼�¼�ش�
#define	UPLOAD_TRY_TIMES			3	//���׼�¼�ش�����
#define NEW_UPLOAD_ON_CARD_RESUALT	1 	//Ѻ���ʼ����Ѻ����ʧ�ܺ��ϱ��������
//add by zjc 2016-03-23
#endif

#define LNT_MOC_ERROR_UNKNOW			0xEE  //δ֪����  //add by zjc


#define MEMBER_COUNTS_LIMIT	100		//������������
#define MEMBER_DAYS_LIMIT	30		//������������
#define MEMBER_MONTHS_LIMIT	12		//������������
#define MEMBER_YEARS_LIMIT	5		//������������

#define MEMBER_LOWER_LIMIT	1		//������������������������������


enum UPE_TYPE			//��������
{
	UPE_TYPE_INIT = 0,
	UPE_TYPE_KERNEL,
	UPE_TYPE_FIRMWARE,
	UPE_TYPE_APPL,
	UPE_TYPE_LNT_ZM,
	UPE_TYPE_END
};


/*
 * 2016-07-26 ����һ�������ڴ�������bicycle_guiͨ��
 * 
 */
#define UPE_SHART_MEM_KEY		20160726	

/*
 * test by zjc
 */
#define CONFS_USING_TEST_BY_ZJC		1 //zjc������


/*
 * �������̼��������ܿ������ 1:���� 0:�ر�
 */
#define CONFS_USING_READER_UPDATE		1

/*
 * �ڵ���Զ����� 1:���� 0:�ر�
 */
#define CONFS_USING_AUTO_REBOOT		0

/* �ڵ���Զ�����ʱ�� */
#define REBOOT_HOUR		01 //ʱ
#define REBOOT_MINUTE	00 //��
#define REBOOT_SECOND	00 //��

/* ������Ϣ����·�� 2016-12-21 */
//#define	NET_INFO_BACKUP_PATH	"/opt/logpath/net_info.txt"

/*
 * ������Ϣ���� 1:���� 0:�ر�
 */
#define CONFS_USING_NET_FLOW_BACKUP		0



#endif
