#ifndef __LNT_H__
#define __LNT_H__

#include <arpa/inet.h>

#include <time.h>


#define LNT_NULL					(NULL) 
#define LNT_EOK					(0)  //����
#define LNT_ERROR				(-1) //����
#define LNT_ETIMEOUT				(-2) //��ʱ
#define LNT_EFULL				(-3) //��
#define LNT_EEMPTY				(-4) //��
#define LNT_ENOMEM 				(-5) //�ڴ治��
#define LNT_EXCMEM				(-6) //�ڴ�Խ��
#define LNT_EBUSY				(-7) //æ
#define LNT_NOT_SUPPORT			(-8) //��֧�ָ�ָ��

#define LNT_ADMIN_SET_ENALBE	(2)
#define LNT_TRUE				(1)
#define LNT_FALSE				(0)

#define LNT_CHAR_MAX			0xff
#define LNT_SHORT_MAX			0xffff
#define LNT_INT_MAX				0xffffffff

#define LNT_NTOHS(n)				ntohs(n)	
#define LNT_HTONS(n)				htons(n)
#define LNT_NTOHL(n)				ntohl(n)
#define LNT_HTONL(n)				htonl(n)


#if 0
/* ��־���� */
//#define LNT_LOG_RUN

#ifdef LNT_LOG_RUN
#include <syslog.h>
#define SYS_LOG_EMERG(fmt, args...) 		syslog(LOG_EMERG, fmt, ##args)
#define SYS_LOG_ALERT(fmt, args...) 			syslog(LOG_ALERT, fmt, ##args)
#define SYS_LOG_CRIT(fmt, args...) 			syslog(LOG_CRIT, fmt, ##args)
#define SYS_LOG_ERR(fmt, args...) 			syslog(LOG_ERR, fmt, ##args)
#define SYS_LOG_WARNING(fmt, args...) 		syslog(LOG_WARNING, fmt, ##args) 
#define SYS_LOG_NOTICE(fmt, args...)  		syslog(LOG_NOTICE, fmt, ##args)
#define SYS_LOG_INFO(fmt, args...) 			syslog(LOG_INFO, fmt, ##args)
#define SYS_LOG_DEBUG(fmt, args...) 		syslog(LOG_DEBUG, fmt, ##args)
#else
#define SYS_LOG_EMERG(fmt, args...) 		fprintf(stderr, fmt, ##args)
#define SYS_LOG_ALERT(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_CRIT(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_ERR(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_WARNING(fmt, args...) 		fprintf(stderr, fmt, ##args)
#define SYS_LOG_NOTICE(fmt, args...) 		fprintf(stderr, fmt, ##args)
#define SYS_LOG_INFO(fmt, args...) 			fprintf(stderr, fmt, ##args)
#define SYS_LOG_DEBUG(fmt, args...) 		fprintf(stderr, fmt, ##args)
#endif
#endif

/*
* ����ͨģ�������
*/
#define LNT_MOC_ERROR_NORMAL					0x00  //��������
#define LNT_MOC_ERROR_SAM_EMPTY				0x60 //û�а�װSAM��
#define LNT_MOC_ERROR_INIT						0x61 //SAM����ʼ�������δ��ʼ��
#define LNT_MOC_ERROR_PIN						0x62 //SAM������PIN����
#define LNT_MOC_ERROR_READ						0x65 //SAM��������
#define LNT_MOC_ERROR_WRITE					0x66 //SAM��д����
#define LNT_MOC_ERROR_CARD_NOTAG				0x80  //û�п�
#define LNT_MOC_ERROR_CARD_AUTH				0x83  //��֤��Ƭ����
#define LNT_MOC_ERROR_CARD_OP_FAIL			0x89  //��Ƭ����ʧ��
#define LNT_MOC_ERROR_CARD_UNKOWN			0x90  //������ͨ��
#define LNT_MOC_ERROR_CARD_OUTOFDATE			0x91  //��Ƭ������Ч��
#define LNT_MOC_ERROR_CARD_NAMELIST			0x94   //������
#define LNT_MOC_ERROR_CARD_OWEPTY			0x95   //Ǯ����
#define LNT_MOC_ERROR_CARD_OVERLIMIT			0x96   //Ǯ����������
#define LNT_MOC_ERROR_CARD_PURSE_UNUSED		0x97   //Ǯ��δ����
#define LNT_MOC_ERROR_TS_PENDING				0xB0   //���ײ�����;�ж�
#define LNT_MOC_ERROR_LASTTIME_TRADE_FAIL		0xD4   //�ϴν���δ�ɹ�
#define LNT_MOC_ERROR_INVALID_PARAM			0xE3	   //��������
#define LNT_MOC_ERROR_M1						0xD3   //M1��

#define LNT_MOC_ERROR_LILIN_NOT_CPU_CARD		0xFE	   //��CPU��
#define LNT_MOC_ERROR_LILIN_NOT_MEMBER_CARD	0xFF   //�ǻ�Ա��

#define LNT_MOC_ERROR_UNKNOW	0xEE   	//δ֪����


/*
 * �豸��Ӧ״̬��
 */
#define LNT_SW_ERROR_NORMAL					0x9000 //�ɹ�ִ��,�޴���
#define LNT_SW_ERROR_LENGTH					0x6700 //���Ȳ���
#define LNT_SW_ERROR_INS						0x6D00 //INS����
#define LNT_SW_ERROR_CLA						0x6E00 //CLA����
#define LNT_SW_ERROR_DATA						0x6F00 //������Ч
#define LNT_SW_ERROR_AUTH						0x6300 //��֤ʧ��

/*
 * ��Ϣ����ձ�
 */
 #define LNT_CINFO_ERROR_ORDER					0xA0    //��ǰ��������һ��
 #define LNT_CINFO_ERROR_PKEY					0xA1    //�����ն��Ҳ�����Կ
 #define LNT_CINFO_ERROR_CRC					0xA2    //����У���벻��ȷ
 #define LNT_CINFO_ERROR_TICKET_TYPE			0xA3	   //��ѯ����Ǯ�����Ͳ�ȷ��
 #define LNT_CINFO_ERROR_ECORRECT				0xA4	   //����ʧ��
 #define LNT_CINFO_ERROR_CORRECT				0xA5	   //�����ɹ�
 #define LNT_CINFO_ERROR_APP_TYPE				0xA6   //Ӧ�����Ͳ�ȷ��
 #define LNT_CINFO_ERROR_ACC_BAL				0xA7   //��ֵ�˺����Ϊ��
 #define LNT_CINFO_ERROR_CARD					0xAA   //����������
 #define LNT_CINFO_ERROR_NOT_REGISTER			0xBB   //�����ն�δǩ��
 #define LNT_CINFO_ERROR_PKT_LEN				0xCC	  //���������ر��ĳ����쳣
 #define LNT_CINFO_ERROR_SYS_BUSY				0xDD   //ϵͳ��æ
 #define LNT_CINFO_ERROR_ESYS					0xEE   //��ϵͳ���ؽ������
 #define LNT_CINFO_ERROR_ORDER_NOT_EXIST		0x96   //��Ӧ����������
 #define LNT_CINFO_ERROR_ORDER_BAL				0x97   //��Ӧ�����Ľ�һ��
 #define LNT_CINFO_ERROR_ORDER_NO				0x98   //��Ӧ�����Ŀ��Ų�һ��


/*
 * ����ͨ����ֵ����
 */
#define LNT_CPU_RECHARGE_LIMIT					(100000)  //1000Ԫ



/*
 * ����ͨǩ��״̬
 */
enum LNT_REGISTER_STAT
{
	LNT_REGISTER_STAT_INIT = 0,  //��ʼ��
	LNT_REGISTER_STAT_ING,   //����ǩ��
	LNT_REGISTER_STAT_OK,  //�Ѿ�ǩ��
	LNT_REGISTER_STAT_RE, //����ǩ��
	LNT_REGISTER_STAT_FAIL //ǩ��ʧ��
};

struct lnt_register_proc_code
{
	unsigned char proc:4;     //ִ�в���
	unsigned char fresult:4;  //ִ�н��
	unsigned char stat;      	//����״̬
	unsigned short sw;  	//������
	time_t time;  		//ִ��ʱ��
	unsigned char agent_err; //���������
}__attribute__((packed));
typedef struct lnt_register_proc_code lib_lnt_register_proc_code_t;

struct lnt_member_proc_code
{
	unsigned char proc;     //ִ�в���
	unsigned char fresult;  //ִ�н��
	unsigned char stat;      	//����״̬
	unsigned short sw;  	//������
}__attribute__((packed));
typedef struct lnt_member_proc_code lnt_member_proc_code_t;

/*
 * ǩ������
 */
enum LNT_REGISTER_PROC
{
	LNT_REGISTER_PROC_QUERTICKET = 1, //��ѯƱ����Ϣ
	LNT_REGISTER_PROC_CONN,   //��������
	LNT_REGISTER_PROC_DD01,
	LNT_REGISTER_PROC_DD02,
	LNT_REGISTER_PROC_R_INIT,
	LNT_REGISTER_PROC_DD03,
	LNT_REGISTER_PROC_DD04,
	LNT_REGISTER_PROC_R_AC_LOGIN_1,
	LNT_REGISTER_PROC_DD05,
	LNT_REGISTER_PROC_DD06,
	LNT_REGISTER_PROC_R_AC_LOGIN_2,
	LNT_REGISTER_PROC_DD07,
	LNT_REGISTER_PROC_DD08,
	LNT_REGISTER_PROC_END //����
};

/*
 * ����������
 */
enum LNT_MEMBER_PROC
{
	LNT_MEMBER_PROC_DEPOSIT_INIT = 1,
	LNT_MEMBER_PROC_CONN,   //��������
	LNT_MEMBER_PROC_DD73,
	LNT_MEMBER_PROC_DD74,
	LNT_MEMBER_PROC_QUERTICKET,
	LNT_MEMBER_PROC_DEPOSIT_PROCESS
};


/*
 * ��������
 */
enum LNT_ON_CARD_PROC
{
	LNT_ON_CARD_PROC_QUERTICKET = 1,  //��ѯƱ����Ϣ
	
	
};




/*
 * ����ִ�н��
 */
enum LNT_FRESULT
{
	LNT_FRESULT_E_OK =  0,  //ȫ���ɹ�
	LNT_FRESULT_E_CRC,  //У��ʧ��
	LNT_FRESULT_E_NET,  //�������
	LNT_FRESULT_E_STEP, //�������
	LNT_FRESULT_E_UART, //���ڳ���
	LNT_FRESULT_E_STAT, //״̬����
	LNT_FRESULT_E_PID, //�����Ų���
	LNT_FRESULT_E_MEMBER, //�Ѿ��ǻ�Ա��
	LNT_FRESULT_E_RETCODE  //���������
};

/*
 * ����,������־
 */
enum LNT_MEMBER_OP
{
	LNT_MEMBER_OP_REGISTER = 0x01,   //����
	LNT_MEMBER_OP_CANCEL = 0x02, //����
};






/******************************************************************
*						����ͨ������ָ��
********************************************************************/
/*
* ����ͨ������ָ��--���ݹ������г�
*/
#define LNT_GETVERSION				0x60 	//��ȡ����汾
#define LNT_CORE_INIT				0xD2		//����ģ���ʼ��
#define LNT_QRY_TICKET_INFO			0xD3        //��ѯ����Ϣ
#define LNT_QRY_RENT_INFO			0xD9         //��ѯ�⻹��Ϣ
#define LNT_SET_RENT_INFO			0xDA		//�����⻹��Ϣ
#define LNT_CONSUME					0xD4         //����
#define LNT_SET_BL_FLAG				0xD5         //����������
#define LNT_GET_HIS_RECORD			0xD6         //�����ʷ���׼�¼
#define LNT_SET_PARAM				0xD7         //��������
#define LNT_GET_TRANS				0xD8         //��ȡ���׼�¼

#define LNT_DEPOSIT_INIT				0xE3         //Ѻ���ʼ��
#define LNT_DEPOSIT_PROCESS			0xE4        //Ѻ����

#define LNT_READER_APDU 			0x47       //APDUָ��


/*
 * ����ͨ����ģ���ֵЭ��
 */
#define LNT_R_CMD					0x40
#define LNT_R_CLA					0xBB
#define LNT_R_INIT_INS				0x10
#define LNT_R_AC_LOGIN_1_INS		0xC8

#define LNT_R_CMD_UPDATE			0xFF //�������̼�����ר��!




/*
 * Ѻ���־
 */
struct deposit_sign
{
	unsigned char rfu1:2;
	unsigned char type:2;
	unsigned char foshan:1;
	unsigned char guangzhou:1;
	unsigned char rfu2:2;
}__attribute__((packed));
typedef struct deposit_sign deposit_sign_t;

struct pmflag
{
	unsigned char rfu:7;
	unsigned char pflag:1;
}__attribute__((packed));
typedef struct pmflag pmflag_t;


/*
 * Ʊ����Ϣ����˵��
 */
struct ticket_info
{
	unsigned int blance;         		//���
	unsigned char pid[8];			//������(��M1��ǰ��4��0)
	unsigned char lid[8];               		//�߼�����
	unsigned short ttype;			//��Ƭ����
	unsigned char ctim[4];			//��ֵʱ��(YYYYMMDD)
	unsigned char pursetype;		//Ǯ������(0x01:M1Ǯ��, 0x02:CPUǮ��)
	unsigned char areaid;			//�������
	unsigned char areacardtype[2];	//���򿨴���
	unsigned char areacarddate[4];	//����������Ч��
	unsigned char dflag;			//���ݷ�ɽѺ���־
	unsigned char dflagother[4];		//���������Ѻ���־
	unsigned short pcode;			//��Ӫ�̱���
	unsigned char pmflag;			//��Ӫ�̻�Ա��־
}__attribute__((packed));
typedef struct ticket_info ticket_info_t;

/*
 * ���׼�¼
 */
struct trade_record
{
	unsigned char rectype; 			//��¼����
	unsigned char areaid;			//��Ƭ�������
	unsigned char ctype[2];			//������
	unsigned char areacardtype[2];	//��������
	unsigned int opid:24;			//����Ա����,3�ֽ�
	unsigned char curtransdevid[6];	//���ν����豸���
	unsigned int offlinetranssn; 		////�ѻ�������ˮ
	unsigned char curtransdtime[7];	//���ν�������ʱ��
	unsigned char lid[8];               		//�߼�����
	unsigned char pid[8];			//������(��M1��ǰ��4��0)
	unsigned int amount;			//���׽��
	unsigned int priceofticket;		//Ʊ��
	unsigned int blance;         		//���׺������
	unsigned char ttype;	      		//��������
	unsigned char addittype;		//���ӽ�������
	unsigned short rechargettimes;	//��ֵ���״���
	unsigned short consumettimes;      //���ѽ��״���
	unsigned char lasttdevid[6];		//�ϴν����豸���
	unsigned char lasttdtime[7];		//�ϴν�������ʱ��
	unsigned int authcode;			//������֤��
	unsigned char rfu[13];			//����
}__attribute__((packed));
typedef struct trade_record trade_record_t;

/*
 * ��������¼
 */
struct blacklist_record
{
	unsigned char pid[8];			//������
	unsigned char lid[8];			//�߼�����
}__attribute__((packed));
typedef struct blacklist_record blacklist_record_t;

/*
 * M1��ʷ��¼
 */
struct m1_history_record
{
	unsigned short spid;          //�����̴���
	unsigned char ttype;	      //��������
	unsigned int amount:24;     //���׽��(��λ��ǰ)
	unsigned char samid[4]; 	      //SAM����
	unsigned char dtime[6];        //����ʱ��(YYMMDDHHMMSS)
}__attribute__((packed));
typedef struct m1_history_record m1_history_record_t; 

/*
 * CPU��ʷ��¼
 */
struct cpu_history_record
{
	unsigned short spid;          //�������(��λ��ǰ
	unsigned int ovd:24; 	    //͸֧�޶�(��λ��ǰ
	unsigned int amount;    		//���׽��(��λ��ǰ)
	unsigned char ttype;	      //��������
	unsigned char tid[6];	      //�ն˻����
	unsigned char date[4]; 	      //��������(XXMMDD)
	unsigned char time[3];        //����ʱ��(HHMMSS)
}__attribute__((packed));
typedef struct cpu_history_record cpu_history_record_t; 

#if 0
/*
 * �⻹����Ϣ
 */
struct rent_info
{
	unsigned char app_locker;        	//Ӧ��������־
	unsigned short provider;		//����Ӫ�˴���
	unsigned char version_no:4;		//�汾���--��4λ
	unsigned char rent_flag:4;		//�⻹����־--��4λ
	unsigned char member_level:4;	//��Ա����--��4λ
	unsigned char member_type:4;	//��Ա����--��4λ
	unsigned char ticket_type;		//Ʊ�ֱ��
	unsigned short accum;			//����/����/����
	unsigned short cur_accum;		//�������Ѵ���/����/����
	unsigned short cur_deal_sn;		//���ν�����ˮ
	unsigned char start_time[3];		//����ʱ��   			YYMMDD
	unsigned char  cur_deal_date[5];	//���ν�������ʱ��   YYMMDDHHMM
	unsigned char site_no[6]; 		//վ����
	unsigned char lock_no[4];		//��׮���
	unsigned char bike_no[4];		//���г����
	unsigned short accu_time_of_day; //�����ۼ�ʱ��
}__attribute__((packed));
typedef struct rent_info rent_info_t;
#endif



/*
 * �⻹����Ϣ
 */
enum TICKET_FLAG
{
	TICKET_FLAG_ORDINARG = 0x00,  //��ͨ��
	TICKET_FLAG_TIME = 0x01,	//�ο�
	TICKET_FLAG_DAY = 0x02, //�쿨
	TICKET_FLAG_MON = 0x03, //�¿�
	TICKET_FLAG_YEAR = 0x04, //�꿨
	TICKET_FLAG_SPECIAL = 0x05, //ר��
	
	TICKET_FLAG_OLD = 0xfd, //ԭ�ͻ�
	TICKET_FLAG_ADMIN = 0xfe,  //����
};

struct rent_flag
{
	unsigned char b0:1;
	unsigned char b1:1;
	unsigned char b2:1;
	unsigned char b3:1;
};


struct accum
{
	unsigned short accum;			//����/����/����/����/ԭ�ͻ��Żݴ���[A8]	
	unsigned short cur_accum;		//�������ѵĻ���/���� [A9]
}__attribute__((packed));

struct rent_info
{
	unsigned char app_locker;        	//Ӧ��������־  [A1]
	unsigned short provider;		//����Ӫ�˴��� [A2]

	unsigned char version_no:4;		//�汾���--��4λ [A4]
	unsigned char rent_flag:4;		//�⻹����־--��4λ [A3]
	unsigned char area_flag:4;		//������--��4λ [A6]
	unsigned char free_time:4;		//���ʱ��--��4λ [A5]
	unsigned char ticket_flag;		//Ʊ�ֱ�� [A7]
	union{
		struct accum acc;
		unsigned char special_phy[4];  // A[8] A[9]
	}s_un;
	unsigned short cur_deal_sn;		//���ν�����ˮ [A10]
	unsigned char start_time[3];		//����ʱ��  YYMMDD [A11]
	unsigned char  cur_deal_date[5];	//���ν�������ʱ��   YYMMDDHHMM [A12]
	unsigned char site_no[6]; 		//վ���� [A13]
	unsigned char lock_no[4];		//��׮��� [A14]
	unsigned char bike_no[4];		//���г���� [A15]
	unsigned short points;	 		//����  [A16]
}__attribute__((packed));
typedef struct rent_info rent_info_t;





/*
 * APDU(������������APDUָ��)
 */
struct lnt_r_apdu_req
{
	unsigned char len;//ָ���
	unsigned char cmd[32];//APDUָ��
}__attribute__((packed));
typedef struct lnt_r_apdu_req lnt_r_apdu_req_t;

/*
 * ��������APDUָ��ص�Ӧ��
 */
struct lnt_r_apdu_ack
{
	unsigned char snum;	//ָ�����
	unsigned char len;	//���ݳ���
	unsigned char data[128];//����
}__attribute__((packed));
typedef struct lnt_r_apdu_ack lnt_r_apdu_ack_t;


/*
 * ��������Ӧ����
 */
struct lnt_reader_ack
{
	unsigned char head;	
	unsigned char len;	
	unsigned char cmd;
	unsigned char stat;
	unsigned char data[128];//����
	unsigned char checksum;
}__attribute__((packed));
typedef struct lnt_reader_ack lnt_reader_ack_t;







/*
* ��ȡ����汾
*/
struct lnt_getversion_ack
{
	unsigned char version[24]; 		//ASCII
}__attribute__((packed));
typedef struct lnt_getversion_ack lnt_getversion_ack_t;

/*
 * ����ģ���ʼ��
 */
struct lnt_core_init_req
{
	unsigned char spno[2];
}__attribute__((packed));
typedef struct lnt_core_init_req lnt_core_init_req_t;

struct lnt_core_init_ack
{
	unsigned char psam[4];
}__attribute__((packed));
typedef struct lnt_core_init_ack lnt_core_init_ack_t;

/*
 * ��ѯƱ����Ϣ
 */
 struct lnt_qry_ticket_info_ack
{
	unsigned char length;
	struct ticket_info ticket;
}__attribute__((packed));
typedef  struct lnt_qry_ticket_info_ack lnt_qry_ticket_info_ack_t;

/*
 * ��ѯ�⻹����Ϣ
 */
struct lnt_rent_info_req
{
	unsigned char id;
}__attribute__((packed));
typedef struct lnt_rent_info_req lnt_rent_info_req_t;

struct lnt_rent_info_ack
{
	unsigned char length;
	struct rent_info rent;
}__attribute__((packed));
typedef struct lnt_rent_info_ack lnt_rent_info_ack_t;

struct lnt_rent_info
{
	unsigned char id;
	unsigned char length;
	struct rent_info rent;
}__attribute__((packed));
typedef struct lnt_rent_info lnt_rent_info_t;


/*
 * ����
 */
#define LNT_ATY_RENT			0x11		//�⳵
#define LNT_ATY_RETURN		0x17		//����
struct lnt_consume_req
{
	unsigned int amount;		//���׽��(��λ��ǰ)
	unsigned int fee;			//Ӧ�ս��(��λ��ǰ)
	unsigned char time[6];		//YYMMDDHHMMSS
	unsigned char phyiscalid[8];	//������
	unsigned char aty;			//���ӽ�������
}__attribute__((packed));
typedef struct lnt_consume_req lnt_consume_req_t;

/*
 * ��������
 */
struct lnt_parameter_set_req
{
	unsigned char conpa;
	unsigned char cvad;
}__attribute__((packed));
typedef struct lnt_parameter_set_req lnt_parameter_set_req_t;

/*
 * ����������
 */
struct lnt_blacklist_record_ack
{
	struct blacklist_record blacklist;
}__attribute__((packed));
typedef struct lnt_blacklist_record_ack lnt_blacklist_record_ack_t;

/*
 * �����ʷ���׼�¼
 */
struct lnt_history_record_ack
{
	unsigned char ticket_type;
	union{
		struct m1_history_record m1_his;
		struct cpu_history_record cpu_his;
	}s_un;
}__attribute__((packed));
typedef struct lnt_history_record_ack lnt_history_record_ack_t;


/*
 * ��ȡ��������
 */
struct lnt_trade_record_ack
{
	struct trade_record record;
}__attribute__((packed));
typedef struct lnt_trade_record_ack lnt_trade_record_ack_t;








/*
 * *******************  ����ͨ����ģ���ֵЭ�� *************************
 */
/*
 * R_INIT(������ʼ��)
 */
struct lnt_r_init_ack
{
	unsigned char pki[4];   	 //PKI������
	unsigned char fw_ver[8];  //�������̼��汾
	unsigned short sw;    	 //״̬��
}__attribute__((packed));
typedef struct lnt_r_init_ack lnt_r_init_ack_t;

/*
 * R_AC_LOGIN_1(ǩ������1)
 */
struct lnt_r_ac_login_1_ack
{
	unsigned char login1_data[8+128];
	unsigned short sw;
}__attribute__((packed));
typedef struct lnt_r_ac_login_1_ack lnt_r_ac_login_1_ack_t;

/*
 * R_AC_LOGIN_2(ǩ������2)
 */
struct lnt_r_ac_login_2_req
{
	unsigned char login2_data[8+128];
}__attribute__((packed));
typedef struct lnt_r_ac_login_2_req lnt_r_ac_login_2_req_t;

struct lnt_r_ac_login_2_ack
{
	unsigned char rfu[46];
	unsigned char login2_data[8+128];
	unsigned short sw;
}__attribute__((packed));
typedef struct lnt_r_ac_login_2_ack lnt_r_ac_login_2_ack_t;

/*
 * R_PUB_QRY_HIS(��ѯ��������Ϣ)
 */
struct card_status_16bytes 
{
	unsigned short spid;          //�����̴���
	unsigned char ttype;	      //��������			//0x17:��ͨ����,0x14:��ֵ,0x13:��������
	unsigned int amount:24;     //���׽��(��λ��ǰ)
	unsigned char samid[4]; 	      //SAM����
	unsigned char dtime[6];        //����ʱ��(YYMMDDHHMMSS)
}__attribute__((packed));

struct card_status_18bytes 
{
	unsigned int amount;    		//���׽��(��λ��ǰ)
	unsigned char ttype_ident;  //�������ͱ�ʶ
	unsigned char tid[6];	      //�ն˻����
	unsigned char rfu[2];	      //����
	unsigned char date[4]; 	      //��������(XXMMDD)
	unsigned char ttype;	      //��������     //0x17:��ͨ����,0x14:��ֵ,0x13:��������
}__attribute__((packed));

struct lnt_r_pub_qry_his_ack
{
	unsigned char sak;
	unsigned char card_status;
	union{
		struct card_status_16bytes b16;
		struct card_status_18bytes b18;
	}s_un;
}__attribute__((packed));
typedef struct lnt_r_pub_qry_his_ack lnt_r_pub_qry_his_ack_t;

/*
 * R_PUB_QRY_PHYSICS_INFO(��ѯ��������Ϣ)
 */
struct lnt_r_pub_qry_physics_info_NoEncry_ack  //������
{
	unsigned char data[51];
	unsigned short sw;
}__attribute__((packed));
typedef struct lnt_r_pub_qry_physics_info_NoEncry_ack lnt_r_pub_qry_physics_info_NoEncry_ack_t;

struct lnt_r_pub_qry_physics_info_Encry_ack  //����
{
	unsigned char data[80];
	unsigned short sw;
}__attribute__((packed));
typedef struct lnt_r_pub_qry_physics_info_Encry_ack lnt_r_pub_qry_physics_info_Encry_ack_t;

/*
 * R_PUB_SET_READCRADINFO(���ö�����+����Ϣ)
 */
 struct lnt_r_pub_set_readcardinfo_req
{
	unsigned char data[8 + 72];
}__attribute__((packed));
typedef  struct lnt_r_pub_set_readcardinfo_req lnt_r_pub_set_readcardinfo_req_t;

 struct lnt_r_pub_set_readcardinfo_ack
{
	unsigned char data[72];   //�������͵�Ʊ�����ز�ͬ
	unsigned short sw;
}__attribute__((packed));
typedef  struct lnt_r_pub_set_readcardinfo_ack lnt_r_pub_set_readcardinfo_ack_t;

/*
 * R_CPU_LOAD_INIT(CPUǮ��Ȧ���ʼ��)
 */
struct lnt_r_cpu_load_init_req
{
	unsigned char data[93];///
}__attribute__((packed));
typedef  struct lnt_r_cpu_load_init_req lnt_r_cpu_load_init_req_t;

struct lnt_r_cpu_load_init_ack
{
	unsigned char data[104];
	unsigned short sw;
}__attribute__((packed));
typedef  struct lnt_r_cpu_load_init_ack lnt_r_cpu_load_init_ack_t;

/*
 * R_CPU_LOAD(CPU Ǯ��Ȧ��)
 */
struct lnt_r_cpu_load_req
{
	unsigned char data[61];///
}__attribute__((packed));
typedef  struct lnt_r_cpu_load_req lnt_r_cpu_load_req_t;

struct lnt_r_cpu_load_ack
{
	unsigned char data[80];
	unsigned short sw;
}__attribute__((packed));
typedef  struct lnt_r_cpu_load_ack lnt_r_cpu_load_ack_t;

/*
 * R_M1_LOAD(M1 ��ֵ)
 */
struct lnt_r_m1_load_req
{
	unsigned char data[128];///
}__attribute__((packed));
typedef  struct lnt_r_m1_load_req lnt_r_m1_load_req_t;

struct lnt_r_m1_load_ack
{
	unsigned char data[32];
	unsigned short sw;
}__attribute__((packed));
typedef  struct lnt_r_m1_load_ack lnt_r_m1_load_ack_t;

/*
 * R_M1_CPU_QRY(M1_CPU Ǯ����ѯ)
 */
struct lnt_r_m1_cpu_qry_req
{
	unsigned char data[128]; 
}__attribute__((packed));
typedef  struct lnt_r_m1_cpu_qry_req lnt_r_m1_cpu_qry_req_t;

struct lnt_r_m1_cpu_qry_ack
{
	unsigned char data[72];
	unsigned short sw;
}__attribute__((packed));
typedef  struct lnt_r_m1_cpu_qry_ack lnt_r_m1_cpu_qry_ack_t;




































/*
 * R_DEPOSIT_INIT(Ѻ���ʼ��)
 */
struct lnt_r_deposit_init_req
{
	unsigned char area;          		//�������
	unsigned char dtype;         		//Ѻ������
	unsigned char member_op;  	  	//����������־
	unsigned short spno;      	 		//��Ӫ�̱���
	unsigned char member_mark;	   //��Ա��־
}__attribute__((packed));
typedef struct lnt_r_deposit_init_req lnt_r_deposit_init_req_t;

struct lnt_r_deposit_init_ack
{
	unsigned char data[176];
}__attribute__((packed));
typedef struct lnt_r_deposit_init_ack lnt_r_deposit_init_ack_t;

/*
 * R_DEPOSIT_PROCESS(Ѻ����)
 */
struct lnt_r_deposit_process_req
{
	unsigned char data[152];
}__attribute__((packed));
typedef struct lnt_r_deposit_process_req lnt_r_deposit_process_req_t;

struct lnt_r_deposit_process_ack
{
	unsigned char sw;        			//��Ƭ������
}__attribute__((packed));
typedef struct lnt_r_deposit_process_ack lnt_r_deposit_process_ack_t;











/******************************************************************
*						����ͨ����ָ��
********************************************************************/
/*
 * ǩ������ָ��
 */
#define LNT_REGISTER_STAGE1				0xDD01
#define LNT_REGISTER_STAGE2				0xDD02
#define LNT_REGISTER_STAGE3				0xDD03
#define LNT_REGISTER_STAGE4				0xDD04
#define LNT_REGISTER_STAGE5				0xDD05
#define LNT_REGISTER_STAGE6				0xDD06
#define LNT_REGISTER_STAGE7				0xDD07
#define LNT_REGISTER_STAGE8				0xDD08

/*
 * ����,����
 */
#define LNT_MEMBER_REGISTER				0xDD73
#define LNT_MEMBET_CANCEL				0xDD74

/*
 * ��ֵ����ָ��
 */
#define LNT_RECHARGE_STAGE1				0xDD11
#define LNT_RECHARGE_STAGE2				0xDD12
#define LNT_RECHARGE_STAGE3				0xDD13
#define LNT_RECHARGE_STAGE4				0xDD14
#define LNT_RECHARGE_STAGE5				0xDD15
#define LNT_RECHARGE_STAGE6				0xDD16
#define LNT_RECHARGE_STAGE7				0xDD17
#define LNT_RECHARGE_STAGE8				0xDD18
#define LNT_RECHARGE_STAGE9				0xDD19
#define LNT_RECHARGE_STAGE10			0xDD20
#define LNT_RECHARGE_STAGE11			0xDD21
#define LNT_RECHARGE_STAGE12			0xDD22


struct lnt_packet_head
{
	unsigned short head;
	unsigned char version;
	unsigned short length;
}__attribute__((packed));
typedef struct lnt_packet_head lnt_packet_head_t;


/*
 * ǩ���׶�1
 */
 struct lnt_packet_register_stage1_CTS
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];	  //PKI����
	unsigned char ilen;           //��Ϣ����
	unsigned char INFO[0];      //��Ϣ
	unsigned int crc32;         //У����
}__attribute__((packed));
typedef  struct lnt_packet_register_stage1_CTS lnt_packet_register_stage1_CTS_t;

/*
 * ǩ���׶�2
 */
 struct lnt_packet_register_stage2_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[5];  	//��Ϣ  [R_INIT REQ]
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_register_stage2_STC lnt_packet_register_stage2_STC_t;

/*
 * ǩ���׶�3: R-INIT����
 */
 struct lnt_packet_register_stage3_CTS    
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];	  //PKI����
	unsigned char ilen;           //��Ϣ����
	unsigned char INFO[14];      //��Ϣ [R-INIT ACK]
	unsigned int crc32;         //У����	
}__attribute__((packed));
typedef  struct lnt_packet_register_stage3_CTS lnt_packet_register_stage3_CTS_t;

/*
 * ǩ���׶�4
 */
 struct lnt_packet_register_stage4_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[5];  	//��Ϣ [R_AC_LOGIN_1 REQ]
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_register_stage4_STC lnt_packet_register_stage4_STC_t;


/*
 * ǩ���׶�5
 */
 struct lnt_packet_register_stage5_CTS
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[8+128+2];  	//��Ϣ  [R_AC_LOGIN_1 ACK]
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_register_stage5_CTS lnt_packet_register_stage5_CTS_t;

/*
 * ǩ���׶�6
 */
 struct lnt_packet_register_stage6_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[5+8+128];  	//��Ϣ[R_AC_Login2 REQ]
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_register_stage6_STC lnt_packet_register_stage6_STC_t;

/*
 * ǩ���׶�7
 */
 struct lnt_packet_register_stage7_CTS
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[46+8+128+2];  	//��Ϣ [R_AC_Login2 ACK]
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_register_stage7_CTS lnt_packet_register_stage7_CTS_t;

/*
 * ǩ���׶�8
 */
 struct lnt_packet_register_stage8_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[0];  	//��Ϣ 
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_register_stage8_STC lnt_packet_register_stage8_STC_t;




 struct lnt_member_process_stage1_CTS
 {
 	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[176]; //��Ϣ  [R_DEPOSIT_INIT]
	unsigned int crc32;  	//У����
 }__attribute__((packed));
typedef  struct lnt_member_process_stage1_CTS lnt_member_process_stage1_CTS_t;

 struct lnt_member_process_stage2_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[152];  	//��Ϣ  [R_DEPPSIT_PROCESS]
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_member_process_stage2_STC lnt_member_process_stage2_STC_t;




/**************************��ֵ****************************/

struct recharge_info
{
	unsigned char apply_type;//Ӧ������
	unsigned char recharge_amount[4];//��ֵ���
	unsigned char order_num[10];//������
}__attribute__((packed));
typedef struct recharge_info recharge_info_t;

struct lnt_packet_recharge_stage1_CTS
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];	  //PKI����
	unsigned char ilen;           //��Ϣ����
	struct recharge_info INFO;      //��Ϣ
	unsigned int crc32;         //У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage1_CTS lnt_packet_recharge_stage1_CTS_t;

/*
 * ��ֵ�׶�2
 */
struct lnt_packet_recharge_stage2_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[29];  	//��Ϣ
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage2_STC lnt_packet_recharge_stage2_STC_t;

/*
 * ��ֵ�׶�3 
 */
struct lnt_packet_recharge_stage3_CTS     //113byte
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];	  //PKI����
	unsigned char ilen;           //��Ϣ����
	//union info{
	//	unsigned char Encry_INFO[80+2];//����
	//	unsigned char NoEncry_INFO[51+2];//������
	//}INFO;

	union info{
		unsigned char Encry_INFO[80+2];//����
	}INFO;

	unsigned int crc32;         //У����	
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage3_CTS  lnt_packet_recharge_stage3_CTS_t;

/*
 * ��ֵ�׶�4
 */
 struct lnt_packet_recharge_stage4_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[8+72+5];  	//��Ϣ
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage4_STC lnt_packet_recharge_stage4_STC_t;


/*
 * ��ֵ�׶�5
 */
 struct lnt_packet_recharge_stage5_CTS 
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[72+2];  	//��Ϣ ///
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage5_CTS  lnt_packet_recharge_stage5_CTS_t;

/*
 * ��ֵ�׶�6
 */
 struct lnt_packet_recharge_stage6_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[93];  	/// 
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage6_STC lnt_packet_recharge_stage6_STC_t;

/*
 * ��ֵ�׶�7
 */
 struct lnt_packet_recharge_stage7_CTS 
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[104+2];   
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage7_CTS  lnt_packet_recharge_stage7_CTS_t;
  

/*
 * ��ֵ�׶�8
 */
 struct lnt_packet_recharge_stage8_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[61];   //��Ϣ///
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage8_STC lnt_packet_recharge_stage8_STC_t;


/*
 * ��ֵ�׶�9
 */
 struct lnt_packet_recharge_stage9_CTS
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[80+2];  	//��Ϣ
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage9_CTS lnt_packet_recharge_stage9_CTS_t;
  

/*
 * ��ֵ�׶�10
 */
 struct lnt_packet_recharge_stage10_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	unsigned char INFO[128];   //��Ϣ///
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage10_STC lnt_packet_recharge_stage10_STC_t;

/*
 * ��ֵ�׶�11
 */
 struct lnt_packet_recharge_stage11_CTS
{
	unsigned char userid[16];   //�û�ID
	unsigned char appstep;	    //Ӧ�ò���
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	//unsigned char INFO[128];  	//��Ϣ
	unsigned char INFO[74];  	//��Ϣ
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage11_CTS lnt_packet_recharge_stage11_CTS_t;
  
/*
 * ��ֵ�׶�12
 */
struct recharge_msg
{
	unsigned char recharge_record_id[10];//��ֵ��¼ID
	unsigned char pki_num[4];//PKI ����
	unsigned char logic_num[8];//�߼�����
	unsigned char recharge_amount[4];//��ֵ���
	unsigned char balance[4];//���׺����
}__attribute__((packed));

struct lnt_packet_recharge_stage12_STC
{
	unsigned char userid[16];   //�û�ID
	unsigned char indcode;    //����ָʾ��
	unsigned char retcode;   //������Ϣ��
	unsigned char pki[4];    	//PKI����
	unsigned char ilen;     	//��Ϣ����
	struct recharge_msg INFO;
	unsigned int crc32;  	//У����
}__attribute__((packed));
typedef  struct lnt_packet_recharge_stage12_STC lnt_packet_recharge_stage12_STC_t;





/******************************************************************
*						���������ָ��
********************************************************************/
/*
 * ����ǩ������
 */
#define AGENT_REGISTER_STAGE1				0xDD01
#define AGENT_REGISTER_STAGE2				0xDD02
#define AGENT_REGISTER_STAGE3				0xDD03
#define AGENT_REGISTER_STAGE4				0xDD04
#define AGENT_REGISTER_STAGE5				0xDD05
#define AGENT_REGISTER_STAGE6				0xDD06
#define AGENT_REGISTER_STAGE7				0xDD07
#define AGENT_REGISTER_STAGE8				0xDD08

/*
 * ��������
 */
#define AGENT_ON_CARD_STAGE1				0xBB01  //[������Ա����]
#define AGENT_ON_CARD_STAGE2				0xBB02

#define AGENT_ON_CARD_STAGE3				0xBB10 //[���Ѽ�¼�ϱ�]
#define AGENT_ON_CARD_STAGE4				0xBB11

#define AGENT_ON_CARD_STAGE5				0xDD73 //[�޸�Ѻ���־]
#define AGENT_ON_CARD_STAGE6				0xDD74

#define AGENT_ON_CARD_STAGE7				0xBB20  //[��������ϱ�]
#define AGENT_ON_CARD_STAGE8				0xBB21

/*
 * ��������(������ƽ̨�۷�)
 */
#define AGENT_ON_CARD_OTHER_CONSUME_STAGE1				0xBB07  //[������Ա����]
#define AGENT_ON_CARD_OTHER_CONSUME_STAGE2				0xBB08

#define AGENT_ON_CARD_OTHER_CONSUME_STAGE3				0xDD73 //[�޸�Ѻ���־]
#define AGENT_ON_CARD_OTHER_CONSUME_STAGE4				0xDD74

#define AGENT_ON_CARD_OTHER_CONSUME_STAGE5				0xBB22  //[��������ϱ�]
#define AGENT_ON_CARD_OTHER_CONSUME_STAGE6				0xBB23

/*
 * ��������
 */
#define AGENT_OFF_CARD_STAGE1				0xBB03  //[������Ա����]
#define AGENT_OFF_CARD_STAGE2				0xBB04  //��Ϣ����2�ֽ�

#define AGENT_OFF_CARD_STAGE3				0xBB30  //��Ϣ����2�ֽ�  [�˻���ѯ]
#define AGENT_OFF_CARD_STAGE4				0xBB31  //��Ϣ����2�ֽ�

#define AGENT_OFF_CARD_STAGE5				0xBB34  //��Ϣ����2�ֽ� [��ȡ������]
#define AGENT_OFF_CARD_STAGE6				0xBB35

#define AGENT_OFF_CARD_STAGE7				0xDD73  //[�޸�Ѻ���־]
#define AGENT_OFF_CARD_STAGE8				0xDD74

#define AGENT_OFF_CARD_STAGE9				0xBB32  //[��������ϱ�]
#define AGENT_OFF_CARD_STAGE10				0xBB33

#define AGENT_OFF_CARD_STAGE11				0xDD11  //[��ֵ����]
#define AGENT_OFF_CARD_STAGE12				0xDD12
#define AGENT_OFF_CARD_STAGE13				0xDD13
#define AGENT_OFF_CARD_STAGE14				0xDD14
#define AGENT_OFF_CARD_STAGE15				0xDD15
#define AGENT_OFF_CARD_STAGE16				0xDD16
#define AGENT_OFF_CARD_STAGE17				0xDD17
#define AGENT_OFF_CARD_STAGE18				0xDD18 	//(�ù���ִ����Ϻ�����ͨ��̨�ͻ���Ϊ��ֵ�ɹ���,ʧ�ܵĻ�����Ҫ�˹�����)
#define AGENT_OFF_CARD_STAGE19				0xDD19
#define AGENT_OFF_CARD_STAGE20				0xDD20
#define AGENT_OFF_CARD_STAGE21				0xDD21
#define AGENT_OFF_CARD_STAGE22				0xDD22

#define AGENT_OFF_CARD_STAGE23				0xBB40	//[��ֵ����ϱ�]
#define AGENT_OFF_CARD_STAGE24				0xBB41



/*
 * ��ֵ����
 */
#define AGENT_RECHARGE_STAGE1				0xBB05  //[��ֵ����]
#define AGENT_RECHARGE_STAGE2				0xBB06   

#define AGENT_RECHARGE_STAGE3				0xBB34  //[��ȡ������]  
#define AGENT_RECHARGE_STAGE4				0xBB35 

#define AGENT_RECHARGE_STAGE5				0xDD11  //[��ֵ����]
#define AGENT_RECHARGE_STAGE6				0xDD12
#define AGENT_RECHARGE_STAGE7				0xDD13
#define AGENT_RECHARGE_STAGE8				0xDD14
#define AGENT_RECHARGE_STAGE9				0xDD15
#define AGENT_RECHARGE_STAGE10				0xDD16
#define AGENT_RECHARGE_STAGE11				0xDD17
#define AGENT_RECHARGE_STAGE12				0xDD18  //(�ù���ִ����Ϻ�����ͨ��̨�ͻ���Ϊ��ֵ�ɹ���,ʧ�ܵĻ�����Ҫ�˹�����)
#define AGENT_RECHARGE_STAGE13				0xDD19
#define AGENT_RECHARGE_STAGE14				0xDD20
#define AGENT_RECHARGE_STAGE15				0xDD21
#define AGENT_RECHARGE_STAGE16				0xDD22

#define AGENT_RECHARGE_STAGE17				0xBB40   //[��ֵ����ϱ�]
#define AGENT_RECHARGE_STAGE18				0xBB41 











/*
 * ��Ա������
 */
enum AGENT_APPLY_RESULT
{
	AGENT_APPLY_RESULT_BLACKLIST = 0x01,					//������
	AGENT_APPLY_RESULT_ID_CARD_INVALID = 0x02,				//���֤��ʽ��Ч
	AGENT_APPLY_RESULT_PHONE_NUMBER_INVALID = 0x03,		//�ֻ������ʽ��Ч
	AGENT_APPLY_RESULT_ALLOW_ON_CARD = 0x10,				//������
	AGENT_APPLY_RESULT_CONSUME_NOT_ON_CARD = 0x011,		//�����ѣ�δ����
	AGENT_APPLY_RESULT_ID_CRAD_AMOUNT_TO_MUCH = 0x12,	//���֤����������������
	AGENT_APPLY_RESULT_ON_CARD = 0x13,						//�ѿ���
	AGENT_APPLY_RESULT_FIND_MANUAL = 0x14,					//���˹�����
	AGENT_APPLY_RESULT_NOT_BALANCE = 0x15					//����
};




#define AGENT_ATTR_BIT0_PASS(c)			((c) | (1 << 0))
#define AGENT_ATTR_BIT0_NOT_PASS(c)		((c) &= ~(1 << 0))
#define AGENT_ATTR_BIT1_SUP(c)			((c) | (1 << 1))
#define AGENT_ATTR_BIT1_NOT_SUP(c)		((c) &= ~(1 << 1))




#define AGENT_ATTR_NOT_PASS			0x00   //��͸��
#define AGENT_ATTR_PASS					0x01   //͸��
#define AGENT_ATTR_SUP					0x02   //����


/*
 * �����������Ӧ
 */
enum AGENT_ERR
{
	AGENT_ERR_OK = 0x00,   				//�ɹ�
	AGENT_ERR_LNT_OFFLINE = 0xE0,	   	//����ͨ���ز�ͨ
	AGENT_ERR_BICYCLE_OFFLINE	= 0xE1,	  //���г����ز�ͨ
	AGENT_ERR_LNT_TIMEOUT	= 0xE2,	   //����ͨ���س�ʱ
	AGENT_ERR_BICYCLE_TIMEOUT = 0xE3,   //���г����س�ʱ
	AGENT_ERR_PKT_CRC = 0xE4,	  //����У���볬ʱ
	AGENT_ERR_SYS_BUSY = 0xE5,   //ϵͳ��æ
	AGENT_ERR_NOT_BALANCE	= 0xE6,   //�˺�����	 


	AGENT_ERR_CINFO_ERROR_ORDER=0xA0,    //��ǰ��������һ��
	AGENT_ERR_CINFO_ERROR_PKEY=0xA1,    //�����ն��Ҳ�����Կ
	AGENT_ERR_CINFO_ERROR_TICKET_TYPE=0xA3,	   //��ѯ����Ǯ�����Ͳ�ȷ��
	AGENT_ERR_CINFO_ERROR_ECORRECT=0xA4,	   //����ʧ��
	AGENT_ERR_CINFO_ERROR_CORRECT=0xA5,	   //�����ɹ�
	AGENT_ERR_CINFO_ERROR_APP_TYPE=0xA6,   //Ӧ�����Ͳ�ȷ��
	AGENT_ERR_CINFO_ERROR_ACC_BAL=0xA7,   //��ֵ�˺����Ϊ��
	AGENT_ERR_CINFO_ERROR_CARD=0xAA,   //����������
	AGENT_ERR_CINFO_ERROR_NOT_REGISTER=0xBB,   //�����ն�δǩ��
	AGENT_ERR_CINFO_ERROR_PKT_LEN=0xCC,	  //���������ر��ĳ����쳣
	AGENT_ERR_CINFO_ERROR_SYS_BUSY=0xDD,   //ϵͳ��æ
	AGENT_ERR_CINFO_ERROR_ESYS=0xEE,   //��ϵͳ���ؽ������
 	AGENT_ERR_CINFO_ERROR_ORDER_NOT_EXIST=0x96,   //��Ӧ����������
	AGENT_ERR_CINFO_ERROR_ORDER_BAL=0x97,   //��Ӧ�����Ľ�һ��
	AGENT_ERR_CINFO_ERROR_ORDER_NO=0x98,   //��Ӧ�����Ŀ��Ų�һ��



	AGENT_ERR_CRC = 0xFD, //CRC����
	AGENT_ERR_NOT_DATA = 0xFE, //���ղ�������
	AGENT_ERR_UNKNOWN = 0xFF  //δ֪����
};





#define AGENT_HEADER			0x55aa 
#define AGENT_ONCR_ILEN			255
#define AGENT_OFFCR_ILEN		512


enum AGENT_TYPE
{
	AGENT_LNT = 0x01,        //����ͨ
	AGENT_UNIONPAY = 0x02, //����
	AGENT_ALIPAY = 0x03,  //֧����
	AGENT_WECHAT = 0x04,  //΢��

	AGENT_RECHARGE = 0x05,  //��ֵ
	AGENT_OTHER_CONSUME = 0x06,  //������ƽ̨�۷�
};

/*
 * �ش����� : ���ڲ��뵽���ݿ�
 */
struct agent_retransmission_data
{
	unsigned short cmd;  			//����
	unsigned char pid[8];  			//Ʊ��������
	unsigned int process_pkt_sn; 	//���̱�����ˮ��
	unsigned int local_pkt_sn;  		//���ر�����ˮ��

	int pkt_len;  //���ĳ���
	unsigned char pkt_data[512];  //��������
}__attribute__((packed));
typedef struct agent_retransmission_data agent_retransmission_data_t;


/*
 * ADPU����
 */
struct adpu_str
{
	unsigned int s_len;
	unsigned char s_str[512];
}__attribute__((packed));
typedef struct adpu_str adpu_str_t;


/*
 * ��������
 */
 struct agent_extra_data
{
	unsigned int process_pkt_sn; 	//���̱�����ˮ��
	unsigned char pid[8];			//Ʊ��������
	unsigned char lid[8];                     //Ʊ���߼�����
	unsigned char macaddr[6];		//�����ַ
	unsigned char pki[4];			//PKI
	unsigned int local_pkt_sn;  		//���ر�����ˮ��
	unsigned char indcode[8];    		//����ָʾ��

	unsigned char order[10]; 		//������   //add by zjc 2016-03-24
	unsigned int recharge;  		//��ֵ���	 //add by zjc 2016-03-24
	
	unsigned char result;			//���
	unsigned char consume_number[10];	//������ƽ̨�۷�ƾ֤��

	unsigned char toreader_step;    //step
	unsigned char toreader_pki[4];     //PKI
	unsigned char toreader_serial[8];	//������ˮ��	
	adpu_str_t str; //ADPU


	unsigned char r_stat;		//���ض�����״̬��
}__attribute__((packed));
typedef  struct agent_extra_data agent_extra_data_t;




/*
 * �˻���ѯ����
 */ 
 struct agent_toreader_array
{
	unsigned char pki[4];				//PKI����
	unsigned char serial[8];				//������ˮ��	
	unsigned char toreader_array[512];	//����������ָ��
}__attribute__((packed));
typedef  struct agent_toreader_array agent_toreader_array_t;

 struct agent_reqinfo_array
{
	unsigned char pki[4];			//PKI����
	unsigned char serial[8];			//������ˮ��	
	unsigned char step; 			//������ָ��ִ�е���һ��
	unsigned char reqinfo_array[512];	//���������ص�Ӧ��
}__attribute__((packed));
typedef  struct agent_reqinfo_array agent_reqinfo_array_t;






/*
 * ������ͷ��
 */
struct lnt_agent_packet_head
{
	unsigned short head;   //ͷ��
	unsigned char type;	     //����
	unsigned short cmd;    //����
	unsigned int sn;        //�������к�
	unsigned char attr;      //����
	unsigned short length; //���ĳ���
}__attribute__((packed));
typedef struct lnt_agent_packet_head lnt_agent_packet_head_t;

/*
 * ��������
 */
struct agent_attribute   
{
	unsigned char pass:1;			//����͸��
	unsigned char supplement:1;		//���ݲ���
	unsigned char rfu[6];			//����
}__attribute__((packed));
typedef struct agent_attribute agent_attribute_t;

/*
 * ����ע��
 */
 struct agent_register
{
	unsigned char pki[4];
	unsigned char macaddr[6];
	unsigned char infolen;
}__attribute__((packed));
typedef  struct agent_register agent_register_t;



/*
 * �������̽׶�1(��Ա��������)
 */
  struct lnt_agent_on_card_stage1_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char id_card[36];		//���֤����
	unsigned char phone_number[6];  //�ֻ�����
	
	unsigned char member_type;	        //��Ա����
	unsigned short member_count;		//���������������������� 2016-04-14 add by zjc
	
	struct ticket_info ticket_info;         //Ʊ����Ϣ
	struct rent_info rent_info;             //�⻹����Ϣ
	unsigned char time[6];                    //ʱ�� 
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_stage1_CTS lnt_agent_on_card_stage1_CTS_t;

/*
 * ������ƽ̨�۷ѿ������̽׶�1(��Ա��������)
 */
struct lnt_agent_on_card_otherConsume_stage1_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	struct ticket_info ticket_info;         //Ʊ����Ϣ
	unsigned char consume_number[10];	//������ƽ̨�۷�ƾ֤��
	unsigned char time[6];                    //ʱ�� 
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_otherConsume_stage1_CTS lnt_agent_on_card_otherConsume_stage1_CTS_t;


/*
 * �������̽׶�2(��Ա����Ӧ��)
 */
 struct lnt_agent_on_card_stage2_STC
{
	unsigned char pid[8];		//Ʊ��������
	unsigned char retinfo; 		//��Ϣ������
	
	unsigned char member_type;	        //��Ա���� 2016-04-14 add by zjc
	unsigned short member_count;		//����������������������
	
	unsigned int fee;	   		//Ӧ�ս��
	unsigned char apply_result;  //������
}__attribute__((packed)); 
typedef struct lnt_agent_on_card_stage2_STC lnt_agent_on_card_stage2_STC_t;

/*
 * �������̽׶�3(���׼�¼�ϱ�)
 */
struct lnt_agent_on_card_stage3_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int trade_sn;			//������ˮ��
	struct trade_record trecord;         //���׼�¼
	unsigned char time[6];                    //ʱ�� 
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_stage3_CTS lnt_agent_on_card_stage3_CTS_t;


/*
 * �������̽׶�4(���׼�¼�ϱ�Ӧ��)
 */
 struct lnt_agent_on_card_stage4_STC
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int trade_sn;			//������ˮ��
}__attribute__((packed)); 
typedef struct lnt_agent_on_card_stage4_STC lnt_agent_on_card_stage4_STC_t;

/*
 * �������̽׶�5(��������)
 */
struct lnt_agent_on_card_stage5_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int trade_sn;			//������ˮ��
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_ONCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_stage5_CTS lnt_agent_on_card_stage5_CTS_t;

/*
 * �������̽׶�6(����Ӧ��)
 */
struct lnt_agent_on_card_stage6_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_ONCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_stage6_STC lnt_agent_on_card_stage6_STC_t;


/*
 * �������̽׶�7(��������ϱ�)
 */
struct lnt_agent_on_card_stage7_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int pkt_sn;			//������ˮ��
	unsigned char indcode[8];    		//����ָʾ��
	unsigned char result;			//���
	unsigned char time[6];                    //ʱ�� 
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_stage7_CTS lnt_agent_on_card_stage7_CTS_t;

/*
 * ������ƽ̨�۷ѵĿ������̽׶�5(��������ϱ�)
 */
struct lnt_agent_on_card_otherConsume_stage5_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int pkt_sn;			//������ˮ��
	unsigned char indcode[8];    	//����ָʾ��
	unsigned char result;			//���
	unsigned char consume_number[10];	//������ƽ̨�۷�ƾ֤��
	unsigned char time[6];          //ʱ�� 
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_otherConsume_stage5_CTS lnt_agent_on_card_otherConsume_stage5_CTS_t;


/*
 * �������̽׶�8(��������ϱ�Ӧ��)
 */
struct lnt_agent_on_card_stage8_STC
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int pkt_sn;			//������ˮ��
	unsigned char result;			//���
}__attribute__((packed)); 
typedef  struct lnt_agent_on_card_stage8_STC lnt_agent_on_card_stage8_STC_t;


/////////////////////////////////////////��������/////////////////////////////

/*
 * �������̽׶�1(��Ա��������)
 */
struct lnt_agent_off_card_stage1_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char id_card[36];		//���֤����
	unsigned char phone_number[6];  //�ֻ�����
	struct ticket_info ticket_info;         //Ʊ����Ϣ
	struct rent_info rent_info;             //�⻹����Ϣ
	unsigned char time[6];                    //ʱ�� 	
}__attribute__((packed)); 
typedef struct lnt_agent_off_card_stage1_CTS lnt_agent_off_card_stage1_CTS_t;


/*
 * �������̽׶�2(��Ա����Ӧ��)
 */
struct lnt_agent_off_card_stage2_STC
{
	unsigned char pid[8];	//Ʊ��������
	unsigned char retinfo; //��Ϣ������
	unsigned char apply_result;  //������	
	unsigned short agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef struct lnt_agent_off_card_stage2_STC lnt_agent_off_card_stage2_STC_t;


/*
 * �������̽׶�3(�˻���ѯҵ������)
 */
struct lnt_agent_off_card_stage3_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned short agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 	
}__attribute__((packed)); 
typedef struct lnt_agent_off_card_stage3_CTS lnt_agent_off_card_stage3_CTS_t;


/*
 * �������̽׶�4(�˻���ѯҵ��ر�)
 */
struct lnt_agent_off_card_stage4_STC
{
	unsigned char pid[8];	//Ʊ��������
	unsigned char retinfo; //��Ϣ������
	unsigned short agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 	
}__attribute__((packed)); 
typedef struct lnt_agent_off_card_stage4_STC lnt_agent_off_card_stage4_STC_t;

/*
 * �������̽׶�5(��ֵ����������)
 */
struct lnt_agent_off_card_stage5_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned short agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 	
}__attribute__((packed)); 
typedef struct lnt_agent_off_card_stage5_CTS lnt_agent_off_card_stage5_CTS_t;


/*
 * �������̽׶�6(�˻���ѯҵ��ر�)
 */
struct lnt_agent_off_card_stage6_STC
{
	unsigned char pid[8];	//Ʊ��������
	unsigned char retinfo; //��Ϣ������
	unsigned char order[10]; //������
	unsigned int recharge;  //��ֵ���
	unsigned char apply_result;  //������	
}__attribute__((packed)); 
typedef struct lnt_agent_off_card_stage6_STC lnt_agent_off_card_stage6_STC_t;


/*
 * �������̽׶�7(��������)
 */
struct lnt_agent_off_card_stage7_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
		}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage7_CTS lnt_agent_off_card_stage7_CTS_t;

/*
 * �������̽׶�8(����Ӧ��)
 */
struct lnt_agent_off_card_stage8_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage8_STC lnt_agent_off_card_stage8_STC_t;

/*
 * �������̽׶�9(��������ϱ�)
 */
struct lnt_agent_off_card_stage9_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int pkt_sn;			//������ˮ��
	unsigned char indcode[8];    		//����ָʾ��
	unsigned char result;			//���
	unsigned char time[6];                    //ʱ�� 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage9_CTS lnt_agent_off_card_stage9_CTS_t;

/*
 * �������̽׶�10(��������ϱ�Ӧ��)
 */
struct lnt_agent_off_card_stage10_STC
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int pkt_sn;			//������ˮ��
	unsigned char result;			//���
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage10_STC lnt_agent_off_card_stage10_STC_t;


/*
 * �������̽׶�11
 */
struct lnt_agent_off_card_stage11_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage11_CTS lnt_agent_off_card_stage11_CTS_t;

/*
 * �������̽׶�12
 */
struct lnt_agent_off_card_stage12_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage12_STC lnt_agent_off_card_stage12_STC_t;

/*
 * �������̽׶�13
 */
struct lnt_agent_off_card_stage13_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI���ź�
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage13_CTS lnt_agent_off_card_stage13_CTS_t;

/*
 * �������̽׶�14
 */
struct lnt_agent_off_card_stage14_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage14_STC lnt_agent_off_card_stage14_STC_t;


/*
 * �������̽׶�15
 */
struct lnt_agent_off_card_stage15_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage15_CTS lnt_agent_off_card_stage15_CTS_t;

/*
 * �������̽׶�16
 */
struct lnt_agent_off_card_stage16_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage16_STC lnt_agent_off_card_stage16_STC_t;


/*
 * �������̽׶�17
 */
struct lnt_agent_off_card_stage17_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage17_CTS lnt_agent_off_card_stage17_CTS_t;

/*
 * �������̽׶�18
 */
struct lnt_agent_off_card_stage18_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage18_STC lnt_agent_off_card_stage18_STC_t;


/*
 * �������̽׶�19
 */
struct lnt_agent_off_card_stage19_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage19_CTS lnt_agent_off_card_stage19_CTS_t;

/*
 * �������̽׶�20
 */
struct lnt_agent_off_card_stage20_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage20_STC lnt_agent_off_card_stage20_STC_t;

/*
 * �������̽׶�21
 */
struct lnt_agent_off_card_stage21_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage21_CTS lnt_agent_off_card_stage21_CTS_t;

/*
 * �������̽׶�22
 */
struct lnt_agent_off_card_stage22_STC
{
	unsigned char pid[8];				//Ʊ��������
	unsigned char retinfo; 				//��Ϣ������
	unsigned char agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage22_STC lnt_agent_off_card_stage22_STC_t;


/*
 * �������̽׶�23(��ֵ����ϱ�) 
 */
struct lnt_agent_off_card_stage23_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int pkt_sn;			//������ˮ��
	unsigned char indcode[8];    		//����ָʾ��

	unsigned char order[10];		//������   //add by zjc 2016-03-24
	unsigned int recharge;			//��ֵ���	 //add by zjc 2016-03-24
	
	unsigned char result;			//���
	unsigned char time[6];                    //ʱ�� 
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage23_CTS lnt_agent_off_card_stage23_CTS_t;

/*
 * �������̽׶�24(��ֵ����ϱ�Ӧ��)
 */
struct lnt_agent_off_card_stage24_STC
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned int pkt_sn;			//������ˮ��
	unsigned char result;			//���
}__attribute__((packed)); 
typedef  struct lnt_agent_off_card_stage24_STC lnt_agent_off_card_stage24_STC_t;


/*
 * ����
 * Key(16):���ͨ�����ţ�8��+���ͨ�߼��ţ�8������(8)��A6(1)+A7(2)+A9(3)+A12(2) TAC=3DES(���ģ�key)
 */
struct lnt_admin_36bytes
{
	unsigned char A1;   		 //Ӧ��������־
	unsigned short A2;  		 //����Ӫ�˴���
	unsigned char A3:4;		//�⻹����־
	unsigned char A4:4;		//�汾���
	unsigned char A5;			//����
	unsigned char A6;			//Ʊ�ֱ��  0XFE��Ա������������
	unsigned short A7;			//����
	unsigned char A8[4];		//����
	unsigned int A9:24;			//����ʱ��
	unsigned char A10_TAC_KEY[8]; //Tac
	unsigned char A11[11];			//����
	unsigned short A12;			//Ƭ��
}__attribute__((packed)); 
typedef struct lnt_admin_36bytes lnt_admin_36bytes_t;

struct lnt_admin_card_info_req
{
	unsigned char id;
}__attribute__((packed));
typedef struct lnt_admin_card_info_req lnt_admin_card_info_req_t;

struct lnt_admin_card_info_ack
{
	unsigned char length;
	struct lnt_admin_36bytes admin_36bytes;
}__attribute__((packed));
typedef struct lnt_admin_card_info_ack lnt_admin_card_info_ack_t;

struct lnt_admin_card_info
{
	unsigned char id;
	unsigned char length;
	struct lnt_admin_36bytes admin_36bytes;
}__attribute__((packed));
typedef struct lnt_admin_card_info lnt_admin_card_info_t;

/*
 * ������������
 */
struct admin_card_encry_data
{
	unsigned char A6;
	unsigned short A7;
	unsigned int A9:24;
	unsigned short A12;
}__attribute__((packed));
typedef struct admin_card_encry_data admin_card_encry_data_t;



/*
 * ��������
 */
 enum LNT_ADMIN_CARD_TYPE
{
	LNT_ADMIN_CARD_TYPE_NOT = 1,
	LNT_ADMIN_CARD_TYPE_RD,
	LNT_ADMIN_CARD_TYPE_RW,
	LNT_ADMIN_CARD_VALID,
	LNT_ADMIN_CARD_UNVALID,
};

/*
 * Ʊ����Ч��(�Ƿ񳬳���Ч��) add by zjc at 2016-12-06
 */
 enum LNT_CARD_VALID_FLAG
{
	LNT_CARD_VALID = 1,
	LNT_CARD_UNVALID,
};

/* 
* Ʊ�ֱ�ǣ������жϷ���ͨ������Ч�� add by zjc at 2016-12-06
*/
#define MEMBER_TYPE_NORMAL	0x00
#define MEMBER_TYPE_TIMES	0x01
#define MEMBER_TYPE_DAYS	0x02
#define MEMBER_TYPE_MONTHS	0x03
#define MEMBER_TYPE_YEARS	0x04
#define MEMBER_TYPE_ADMIN	0xFE





unsigned int lnt_crc32(unsigned char *buffer, unsigned int size);



/////////////////////////////////////////��ֵ����///////////////////////////////////////
/*
 * ��ֵ���̽׶�1(��ֵ��������)
 */
struct lnt_agent_recharge_stage1_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	struct ticket_info ticket_info; //Ʊ����Ϣ
	unsigned char order[10]; 		//������
	unsigned char time[6];          //ʱ�� 	
}__attribute__((packed)); 
typedef struct lnt_agent_recharge_stage1_CTS lnt_agent_recharge_stage1_CTS_t;


/*
 * ��ֵ���̽׶�2(��ֵ����Ӧ��)
 */
struct lnt_agent_recharge_stage2_STC
{
	unsigned char pid[8];						//Ʊ��������
	unsigned char retinfo; 						//��Ϣ������
	unsigned char apply_result;  				//������	
	unsigned short agent_ilen;     				//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 
}__attribute__((packed)); 
typedef struct lnt_agent_recharge_stage2_STC lnt_agent_recharge_stage2_STC_t;

/*
 * ��ֵ���̽׶�3(��ֵ����������)
 */
struct lnt_agent_recharge_stage3_CTS
{
	unsigned char pid[8];			//Ʊ��������
	unsigned char macaddr[6];		//MAC��ַ
	unsigned char pki[4];			//PKI����
	unsigned short agent_ilen;     			//��Ϣ����
	unsigned char agent_info[AGENT_OFFCR_ILEN]; //��Ϣ 	
}__attribute__((packed)); 
typedef struct lnt_agent_recharge_stage3_CTS lnt_agent_recharge_stage3_CTS_t;


/*
 * ��ֵ���̽׶�4(��ֵ�����Żر�)
 */
struct lnt_agent_recharge_stage4_STC
{
	unsigned char pid[8];	//Ʊ��������
	unsigned char retinfo; //��Ϣ������
	unsigned char order[10]; //������
	unsigned int recharge;  //��ֵ���
	unsigned char apply_result;  //������	
}__attribute__((packed)); 
typedef struct lnt_agent_recharge_stage4_STC lnt_agent_recharge_stage4_STC_t;


/******************************************************************
*	   ����ͨ�������̼����� add by zjc at 2016-07-22
********************************************************************/

/*
* ����״̬����
*/
struct update_stat_set{
	unsigned char mode;//ģʽ
	unsigned short total_packets;//�����ݰ���,��"ģʽ"Ϊ0x02���޴���
}__attribute__((packed));
typedef struct update_stat_set update_stat_set_t;

/*
* Ԥ������Ӧ
*/
struct pre_update_ack{
	unsigned int next_position;
}__attribute__((packed));
typedef struct pre_update_ack pre_update_ack_t;

/*
* ������Ӧ
*/
struct update_ack{
	unsigned char retcode[5];
}__attribute__((packed));
typedef struct update_ack update_ack_t;



#endif


