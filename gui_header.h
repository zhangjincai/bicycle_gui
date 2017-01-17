#ifndef __GUI_HEADER_H__
#define __GUI_HEADER_H__


struct gui_bit_status
{
	unsigned int except_ack:1;
	unsigned int rfu:31;
}__attribute__((packed));
typedef struct gui_bit_status gui_bit_status_t;


/*
 * ��汾
 */
 struct gui_version
{
	char ver[32];
}__attribute__((packed));
typedef  struct gui_version gui_version_t;

/*
 * �ڵ����汾
 */
 struct gui_ndev_version
{
	char ver[32];
}__attribute__((packed));
typedef  struct gui_ndev_version gui_ndev_version_t;

/*
 * �ڵ��״̬  1:����, 0:�쳣
 */
struct gui_ndev_status
{
	unsigned short power:1;				//��Դ��ѹ״̬
	unsigned short rtc:1;				//RTC״̬
	unsigned short can:1;				//CAN����״̬
	unsigned short center:1;				//������ͨѶ״̬
	unsigned short wireless:1;			// 3G/4G����״̬
	unsigned short yct:1;				//���ͨ������
	unsigned short gps:1;				//gpsģ��״̬
	unsigned short keyboard:1;			//���̰�״̬
	unsigned short ext_mcu:1;			//����CPU��״̬
	unsigned short time:1;				//ʱ��ͬ��״̬
	unsigned short rfu:6;				//����
}__attribute__((packed));
typedef struct gui_ndev_status gui_ndev_status_t;

/*
 * ��׮״̬  1:����, 0:�쳣
 */
struct gui_sae_status
{
	unsigned short in:1;		//�����ջؼ��
	unsigned short out:1;		//����������
	unsigned short bike:1;		//�Ƿ��г�
	unsigned short op:1;		//�����ɹ�
	unsigned short rc531:1;		//RC531��Ӧ
	unsigned short lock:1;		//��������Ӧ
	unsigned short time:1;		//ʱ��ͬ��״̬
	unsigned short volt:1;		//��Դ��ѹ״̬
	unsigned short rtc:1;		//RTC״̬
	unsigned short can:1;		//CAN����״̬
	unsigned short rfu:6;		//����
}__attribute__((packed));
typedef struct gui_sae_status gui_sae_status_t;

/*
 * �ڵ���豸����
 */
struct gui_ndev_config
{
	char boot_ver[32];				//UBOOT �汾
	char kernel_ver[32];			//kernel�汾
	char rootfs_ver[32];			//rootfs�汾
	char fw_ver[32]; 				//firmware�汾
	char appl_ver[32];				//appl�汾
	
	unsigned int local_ip;			//����IP
	unsigned int local_submask;		//������������
	unsigned int local_gateway;		//��������
	unsigned int local_dns;			//������ַ

	unsigned int wifi_ip;			//WIFI ����IP
	unsigned int wifi_submask;		//WIFI ��������
	unsigned int wifi_gateway;		//WIFI ����
	unsigned int wifi_dns;			//WIFI ������ַ
	
	unsigned char using_wireless;   //ʹ����������   {1:���� 2:����}
	
	unsigned int load_server1_ip;		//���ؾ��������1 IP ��ַ
	unsigned int load_server2_ip;		//���ؾ��������2 IP ��ַ
	unsigned short load_server1_port;	//���ؾ���1�˿�
	unsigned short load_server2_port;	//���ؾ���2�˿�
	unsigned short load_timeout;			//���ؾ��ⳬʱʱ��(��)
		
	unsigned int ftp_ip;
	unsigned short ftp_port;
	char ftp_username[32];
	char ftp_password[32];

	unsigned short heart_time;		//��������ʱ��
	unsigned short emerg_heart_time; //���Ľ�������ʱ��
	unsigned char timer_gate_value;  //ʱ������բֵ
	unsigned short term_id; 		//�ն˱��
	unsigned short area_info;       	//������Ϣ
	
	unsigned int flow_3g;   		// 3G����
	unsigned short first_level_3g;  // 3G������һբֵ
	unsigned short second_level_3g; // 3G�����ڶ�բֵ

	unsigned int lnt_ipaddr;   //����ͨ����IP
	unsigned short lnt_port;  //����ͨ���Ķ˿�
	unsigned char lnt_userid[16];  //����ͨ�û�ID
	unsigned char lnt_spno[2]; //�����̴���
	unsigned char lnt_conpa; //���ѳ�ʼ������
	unsigned char lnt_cvad; //��������Ч������	

	unsigned char light_ctrl_time_enable[2];  //hhmm
	unsigned char light_ctrl_time_disable[2];  //hhmm
	

	unsigned char rfu[252];       //���� {-4:light_ctrl_time_enable, light_ctrl_time_disable}
}__attribute__((packed));
typedef struct gui_ndev_config gui_ndev_config_t;

/*
 * ׮���豸����
 */
struct gui_stake_config
{
	unsigned short heart_time;       		//׮������ʱ��
	char firmware_uptime[7]; 			//�̼�����ʱ��
	unsigned char can_baud_rate;		//CAN������
	unsigned short quantity;   			//��׮����


	unsigned char rfu[128];       //����
}__attribute__((packed));
typedef struct gui_stake_config gui_stake_config_t;














/*
 * ������Ϣҳ������
 */
struct gui_basic_info_page_config
{
	char ndev_firmware_ver[32];  //�ڵ���汾
	char stake_firmware_ver[32];  //׮�汾
	char yct_firmware_ver[32];	//���ͨ�汾
		
	char terminal_name[50];	//�ն�����
	unsigned short terminal_no;	//�ն˱��
	unsigned int register_time;	//ע��ʱ��
	char conn_ipaddr[32];	//�������ӵ�ַ
	unsigned short conn_port; //���Ӷ˿�
	unsigned int last_time_power_on; //���һ���ϵ�ʱ��

	unsigned int dial_stat;		//����״̬
	unsigned char	system_mode;	//ϵͳģʽ
	unsigned char	uim_card_stat;	//UIM��״̬
	unsigned char rssi;	//�ź�ǿ��
	unsigned char fer;	//�ŵ���֡��

	unsigned char center_conn_stat;	//���г���������
	unsigned char load_bal_stat;       //���ؾ���״̬

	char total_blacklist_ver[9];	//����������
	char inc_blacklist_ver[9];	//����������
	char dec_blacklist_ver[9];	//����������
	char temp_blacklist_ver[9];  //��ʱ������
	char stake_para[9];   //��׮����
	char unionpay_key[9];  //������Կ

	unsigned short use_total_seq;
	unsigned short use_inc_seq;
	unsigned short use_dec_seq;
	unsigned short use_temp_seq;
	unsigned short use_fw_seq;
	unsigned short use_stake_para_seq;
	unsigned short use_unionpay_key_seq;
	
	unsigned char use_total_db;			
	unsigned char use_inc_db;
	unsigned char use_dec_db;
	unsigned char use_temp_db;
	unsigned char use_fw_db;
	unsigned char use_stake_para_db;
	unsigned char use_unionpay_key_db;
	
	gui_ndev_status_t nstatus; //�ڵ��״̬��Ϣ
}__attribute__((packed));
typedef struct gui_basic_info_page_config gui_basic_info_page_config_t;


/*
 * ����IP��ַ
 */
struct gui_ndev_local_ip_config
{
	char ipaddr[16];
	char subnet_mask[16];
	char default_gateway[16];
}__attribute__((packed));
typedef struct gui_ndev_local_ip_config gui_ndev_local_ip_config_t;

/*
 * WIFI-IP��ַ
 */
struct gui_ndev_wifi_ip_config
{
	char ipaddr[16];
	char subnet_mask[16];
	char default_gateway[16];
}__attribute__((packed));
typedef struct gui_ndev_wifi_ip_config gui_ndev_wifi_ip_config_t;

/*
 * ���г����ĵ�ַ
 */
struct gui_ndev_center_ip_config
{
	char load1_ipaddr[16];
	unsigned short load1_port;
	char load2_ipaddr[16];
	unsigned short load2_port;
}__attribute__((packed));
typedef struct gui_ndev_center_ip_config gui_ndev_center_ip_config_t;

/*
 * FTP��ַ
 */
struct gui_ndev_ftp_config
{
	char ipaddr[16];
	unsigned short port;
	char user_name[16];
	char user_password[16];
}__attribute__((packed));
typedef struct gui_ndev_ftp_config gui_ndev_ftp_config_t;

/*
 * �ڵ����������
 */
struct gui_ndev_parameter_config
{
	unsigned short load_timeout;
	unsigned short heart_time;
	unsigned short emerg_heart_time;
	unsigned char timer_gate_value;  //ʱ������բֵ
	unsigned short term_id;
}__attribute__((packed));
typedef struct gui_ndev_parameter_config gui_ndev_parameter_config_t;

/*
 * ��׮��������
 */
struct gui_stake_parameter_config
{
	unsigned char can_baud_rate;
	unsigned short heart_time; 
	unsigned short quantity; 	
}__attribute__((packed));
typedef struct gui_stake_parameter_config gui_stake_parameter_config_t;

/*
 * ������뷽ʽ
 */
struct gui_ndev_access_pattern_config
{
	unsigned char using_wireless;   //ʹ����������
}__attribute__((packed));
typedef struct gui_ndev_access_pattern_config gui_ndev_access_pattern_config_t;

/*
 * ����ͨ��������
 */
struct gui_lnt_page_config
{
	char ipaddr[16];   //����ͨ����IP
	unsigned short port;  //����ͨ���Ķ˿�
	unsigned char userid[16];  //����ͨ�û�ID
	unsigned char spno[2]; //�����̴���
	unsigned char conpa; //���ѳ�ʼ������
	unsigned char cvad; //��������Ч������	
}__attribute__((packed));
typedef struct gui_lnt_page_config gui_lnt_page_config_t;


/*
 * ��ȡ�ڵ��ҳ������
 */
struct gui_ndev_page_config
{
	struct gui_ndev_local_ip_config ip_conf;
	struct gui_ndev_wifi_ip_config wifi_ip_conf;
	struct gui_ndev_center_ip_config center_ip_conf;
	struct gui_ndev_ftp_config ftp_conf;
	struct gui_ndev_parameter_config n_param_conf;
	struct gui_stake_parameter_config s_param_conf;
	struct gui_ndev_access_pattern_config access_conf;
	struct gui_lnt_page_config lnt_conf;
}__attribute__((packed));
typedef struct gui_ndev_page_config gui_ndev_page_config_t;

/*
 * ��ȡ��׮��Ϣҳ��
 */
struct gui_stake_info_page_config
{
	unsigned char s_stat[17];	//��׮ȫ��״̬
	unsigned char s_bicycle_exist_stat[17]; //���Ƿ���׮״̬
	unsigned char max;     	//֧�������׮��
	unsigned char online;    	//������׮��
	unsigned char quantity;	//��׮����
	unsigned short status[64];  //�����׮״̬
}__attribute__((packed));
typedef struct gui_stake_info_page_config gui_stake_info_page_config_t;


/* �ڵ����ҳ��ʾ��Ϣ */
struct gui_ndev_home_page_info
{
	char terminal_name[50];   		//վ������
	unsigned short terminal_no;		//�ն˱��
	unsigned short network_type:2;	//��������
	unsigned short signal_level:3;	// 3/4G�źŵȼ�
	unsigned short wifi_status:1;         //wifi״̬
	unsigned short gps_status:1;         //gps״̬
	unsigned short center_status:1;    //��������״̬
	unsigned short yct_status:1;         //�������ͨ����״̬
	unsigned short canbus_status:1;  //can����״̬
	unsigned char stake_online_num;  //��׮������
	unsigned char stake_total_num; //��׮����
	
	unsigned char site_QR_code[128];   //վ���ά��  //32->128 2016-11-25
	
	#if 1 //CONFS_USING_READER_UPDATE
	unsigned char lnt_firmware_update_flag:1;		 //����ͨ�̼��������,1:���¹̼���Ҫ����,0:û���¹̼���Ҫ����
	char lnt_firmware_path[96]; 	 //����ͨ�����������̼�·��
	#endif
}__attribute__((packed));
typedef struct gui_ndev_home_page_info gui_ndev_home_page_info_t;



/* ��׮��Ϣͳ�� */
struct gui_sae_info
{
	unsigned char healthy_flag;			//��־ 1:���� 0:��Ч
	unsigned char send_hb_times;		//��������ʱ��һ,���ճɹ����һ	
	unsigned char id;					//��׮���-- ʮ����
	unsigned short status;				//׮��״̬
	unsigned char s_psam[7];			//PSAM�����
	unsigned char s_id[4];				//��׮���
	unsigned char s_ver[9];				//��׮�汾��
	unsigned char s_lock_id[4];			//���ر��
	unsigned char s_lock_ver[9];			//���ذ汾��
	unsigned char s_sn[7];				//��׮��ˮ��
	unsigned char s_total_bl_ver[9];		//�����������汾
	unsigned char s_inc_bl_ver[9];		//�����������汾
	unsigned char s_dec_bl_ver[9];		//�����������汾
	unsigned char s_temp_bl_ver[9];		//��ʱ�������汾
	unsigned char s_stake_para[9];           //��׮����
	unsigned char s_unionpay_key[9];     //��������
	unsigned char s_time[13]; 			//��׮ʱ��
	unsigned int s_reg_time;			//ǩ��ʱ��
	unsigned char s_phy_sn[9];			//��׮������
}__attribute__((packed));
typedef struct gui_sae_info gui_sae_info_t;

struct gui_stake_all_info_page_config
{
	unsigned char quantity;                              //��׮����
	struct gui_sae_info info[65];			//��׮��Ϣ
}__attribute__((packed));
typedef struct gui_stake_all_info_page_config gui_stake_all_info_page_config_t;

/* ����ͨ������״̬ */
struct gui_lnt_card_status
{
	unsigned char lnt_card_stat:1;
}__attribute__((packed));
typedef struct gui_lnt_card_status gui_lnt_card_status_t;

/*
 * ����ʹ�ü�¼
 */
struct gui_admin_card_info
{
	unsigned int sn;			//��ˮ��
	unsigned char pid[8];              //������
	unsigned char ticket[42];		//Ʊ����Ϣ
	unsigned char rentinfo[36];	//�⻹����¼
	unsigned char time[6]; 		 //YYMMDDHHmmSS
}__attribute__((packed));
typedef struct gui_admin_card_info gui_admin_card_info_t;

/*
 * �쳣����۷ѽ��׼�¼
 */
 struct gui_exception_handle_record
{
	unsigned int sn;			//��ˮ��
	unsigned char pid[8];              //������
	unsigned char record[90];      //���׼�¼
}__attribute__((packed));
typedef struct gui_exception_handle_record gui_exception_handle_record_t;


struct gui_except_handle_req
{
	unsigned int sn;
	unsigned char pid[8];
	unsigned char ticket[42];
	unsigned char rentinfo[36];
}__attribute__((packed));
typedef struct gui_except_handle_req gui_except_handle_req_t;



#define GUI_EX_HDL_NORMAL				0x00   //����(���쳣����)
#define GUI_EX_HDL_NOT_CARD			0x01   //����δˢ��
#define GUI_EX_HDL_NOT_COST			0x02  //Ƿ��
#define GUI_EX_HDL_NOT_DEPOSIT			0x03  //δ��Ѻ��
#define GUI_EX_HDL_NOT_BIKE				0x04 //ˢ��δȡ��
#define GUI_EX_HDL_MANUAL				0xFF   //��̨�쳣(�˹����ѯ)



struct gui_except_handle_ack
{
	unsigned int sn;
	unsigned char pid[8];
	unsigned char status;
	unsigned int fee; //�۷�
	unsigned char info[48];
}__attribute__((packed));
typedef struct gui_except_handle_ack gui_except_handle_ack_t;


/*
 * ��׮�ȶ�״̬
 */
 struct gui_sae_comparison_status
{
	unsigned char fw:1;  //�̼�
	unsigned char bl_total:1; //����������
	unsigned char bl_inc:1; //����������
	unsigned char bl_dec:1;  //����������
	unsigned char bl_temp:1;  //��ʱ������
	unsigned char stake_para:1;  //��׮����
	unsigned char unionpay_key:1;  //������Կ
	unsigned char rfu:1;
}__attribute__((packed));
typedef struct gui_sae_comparison_status gui_sae_comparison_status_t;


/*
 * ���뷽ʽ������״̬
 */
 struct gui_access_state
{
	unsigned char access;   //���뷽ʽ
	unsigned int connect_stat;  //����״̬
}__attribute__((packed));
typedef  struct gui_access_state gui_access_state_t;



/*
 * GPS
 */
 struct g_gps_attr
{
	unsigned char gga:1;
	unsigned char tm:1;
	unsigned char rfu:6;
}__attribute__((packed));
typedef struct g_gps_attr g_gps_attr_t;

struct g_gps_tm 
{
	int tm_sec;       /* �� �C ȡֵ����Ϊ[0,59] */
	int tm_min;       /* �� - ȡֵ����Ϊ[0,59] */
	int tm_hour;      /* ʱ - ȡֵ����Ϊ[0,23] */
	int tm_mday;      /* һ�����е����� - ȡֵ����Ϊ[1,31] */
	int tm_mon;       /* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
	int tm_year;      /* ��ݣ���ֵ����ʵ����ݼ�ȥ1900 */
	int tm_wday;      /* ���� �C ȡֵ����Ϊ[0,6]������0���������죬1��������һ���Դ����� */
	int tm_yday;      /* ��ÿ���1��1�տ�ʼ������ �C ȡֵ����Ϊ[0,365]������0����1��1�գ�1����1��2�գ��Դ����� */
	
	int tm_isdst;   /*����ʱ��ʶ����ʹ������ʱ��tm_isdstΪ������ʹ������ʱ��tm_isdstΪ0�����˽����ʱ��tm_isdstΪ��*/ 
};

struct g_gps_gga
{
	double latitude;  //γ�ȣ���ʽΪ ddmm.mmmmm���ȷָ�ʽ��
	unsigned char ns_indicator;  //γ�Ȱ���N �� S����γ����γ��
	double longitude; //���ȣ���ʽΪ dddmm.mmmmm���ȷָ�ʽ��
	unsigned char ew_indicator; //���Ȱ���E �� W��������������
	int status; // GPS ״̬��0=δ��λ��1=�ǲ�ֶ�λ��2=��ֶ�λ  
	int satellite; //����ʹ�õ����ڶ�λ������������00~12�� 
};

struct gui_gps_info
{
	struct g_gps_attr attr;
	
	struct g_gps_tm tm;
	struct g_gps_gga gga;
};
typedef struct gui_gps_info gui_gps_info_t;


/*
 * �⻹����¼��ѯ
 */
struct gui_rent_info_qry_req
{
	unsigned char pid[8];		//Ʊ��������
	unsigned char item;       //��¼����
}__attribute__((packed));
typedef struct gui_rent_info_qry_req gui_rent_info_qry_req_t;

struct gui_rent_info_qry_ack
{
	unsigned char pid[8];		//Ʊ��������
	unsigned char item;       		//��¼����
	unsigned short length;   //��¼����
	unsigned char record[2048];  //��¼
}__attribute__((packed));
typedef struct gui_rent_info_qry_ack gui_rent_info_qry_ack_t;

struct gui_rent_info_fmt		//���ü�¼��ʽ
{ 
	unsigned char bike_NO[32];		//���г����
	unsigned char rent_name[64];	//�⳵������
	unsigned char rent_time[32];		//�⳵ʱ��
	unsigned char return_name[64];	//����������
	unsigned char return_time[32];	//����ʱ��
	unsigned char used_time[16];		//����ʱ��
	unsigned char fee[16];			//�۷�
}__attribute__((packed));
typedef struct gui_rent_info_fmt gui_rent_info_fmt_t;


/*
 * �ڴ���Ϣ
 */
struct gui_proc_meminfo
{
	unsigned char mac0[6];
	unsigned char mac1[6];
	
	unsigned int MemTotal;
	unsigned int MemFree;
	unsigned int Buffers;
	unsigned int Cached;
	unsigned int SwapCached;
	unsigned int Active;
	unsigned int Inactive;
	unsigned int Active_anon;
	unsigned int Inactive_anon;
	unsigned int Active_file;
	unsigned int Inactive_file;
	unsigned int Unevictable;
	unsigned int Mlocked;
	unsigned int SwapTotal;
	unsigned int SwapFree;
	unsigned int Dirty;
	unsigned int Writeback;
	unsigned int AnonPages;
	unsigned int Mapped;
	unsigned int Shmem;
	unsigned int Slab;
	unsigned int SReclaimable;
	unsigned int SUnreclaim;
	unsigned int KernelStack;
	unsigned int PageTables;
	unsigned int NFS_Unstable;
	unsigned int Bounce;
	unsigned int WritebackTmp;
	unsigned int CommitLimit;
	unsigned int Committed_AS;
	unsigned int VmallocTotal;
	unsigned int VmallocUsed;
	unsigned int VmallocChunk;
};
typedef struct gui_proc_meminfo gui_proc_meminfo_t;

/*
 * �ӿڰ�汾��Ϣ
 */
struct gui_iboard_device_version
{
	unsigned short vendor_encoding; //��Ӧ�̱���
	unsigned char device_type;  //�豸����
	unsigned char rfu[5];   //����
	unsigned char device_sn[16];  //�豸���к�
	unsigned short product_batch; //��Ʒ����
	unsigned short hardware_version;  //Ӳ���汾��
	unsigned char software_version[8]; //����汾��	
}__attribute__((packed));
typedef struct gui_iboard_device_version gui_iboard_device_version_t;

/*
 * �������
 */
struct gui_light_ctrl_time
{
	unsigned char light_ctrl_time_enable[2];  //hhmm
	unsigned char light_ctrl_time_disable[2];  //hhmm
}__attribute__((packed));
typedef struct gui_light_ctrl_time gui_light_ctrl_time_t;


/*
 * ���ļ���Ϣ
 */
struct gui_unity_file_struct
{
	unsigned short file_seq;		 //�ļ����
	unsigned short total;		//�ָ�����
	unsigned short div_seq;		//�ָ����

	int status;			//״̬	1:��������, 2:���ڹ㲥
	time_t start_time;  		 //��ʼ����ʱ��
	time_t end_time;		//��������ʱ��
}__attribute__((packed));

struct gui_unity_file_info
{
	struct gui_unity_file_struct ndev_ufile_st[6];  //�ڵ������
	struct gui_unity_file_struct broadcast_ufile_st[6];  //�㲥���ļ�
	struct gui_unity_file_struct stake_ufile_st[6];  //��׮����  
}__attribute__((packed));
typedef struct gui_unity_file_info gui_unity_file_info_t;

/*
 * ����ͨ��Ϣ
 */
struct gui_lnt_all_info
{
	unsigned char lib_version[32];   //��汾
	unsigned char hw_version[32];  //�������汾

	unsigned int register_stat;   //ע��״̬
	unsigned char pki_r[4];  //PSAM����
	unsigned char psam[4];   //PKI
	
	unsigned int pkt_sn_RO; //�������к�
	
	unsigned char proc:4;     //ִ�в���
	unsigned char fresult:4;  //ִ�н��
	unsigned char stat;      	//����״̬
	unsigned short sw;  	//������
	time_t time;  		//ִ��ʱ��
	unsigned char agent_err; //���������
}__attribute__((packed));
typedef struct gui_lnt_all_info gui_lnt_all_info_t;

/*
 * �ӳٻ���
 */
 struct gui_delay_return_bicycle
{
	int quantity;	 			//��׮����
	int register_count;		//ע����׮��
	int bicycle_online_count;   //�г���׮��
}__attribute__((packed));
typedef  struct gui_delay_return_bicycle gui_delay_return_bicycle_t;
 

struct lnt_firmware_update_result
{
	char type;
	char result;  //0:��ʼ����1:�ɹ���2:ʧ��
	char ftp_local_path[96];
}__attribute__((packed));
typedef struct lnt_firmware_update_result lnt_firmware_update_result_t;

/* ����gps��Ϣ */
struct gps_gga_info_backup
{
	double latitude;  //γ�ȣ���ʽΪ ddmm.mmmmm���ȷָ�ʽ��
	unsigned char ns_indicator;  //γ�Ȱ���N �� S����γ����γ��
	double longitude; //���ȣ���ʽΪ dddmm.mmmmm���ȷָ�ʽ��
	unsigned char ew_indicator; //���Ȱ���E �� W��������������
	int status; // GPS ״̬��0=δ��λ��1=�ǲ�ֶ�λ��2=��ֶ�λ  
	int satellite; //����ʹ�õ����ڶ�λ������������00~12�� 
};
typedef struct gps_gga_info_backup gps_gga_info_backup_t;

/*
 * ����������Ϣ��ѯ add by zjc at 2016-11-03
 */
struct gui_nearby_site_info_qry_req
{
	unsigned short siteNo;		//�����ն˱��
	unsigned char gpsInfo[32];       //���㾭γ����Ϣ
}__attribute__((packed));
typedef struct gui_nearby_site_info_qry_req gui_nearby_site_info_qry_req_t;

struct gui_nearby_site_info_qry_ack
{
	unsigned char siteNo[2];		//�����ն˱��
	unsigned char item;       		//������Ϣ����
	unsigned short length;   //������Ϣ����
	unsigned char info[2048];  //������Ϣ����
}__attribute__((packed));
typedef struct gui_nearby_site_info_qry_ack gui_nearby_site_info_qry_ack_t;

struct gui_nearby_site_info_fmt		//����������Ϣ��ʽ
{ 
	unsigned char siteName[64];		//��������
	unsigned char distance[16];	//����
	unsigned char bikes[16];	//���г���
	unsigned char stacks[16];	//��׮��
}__attribute__((packed));
typedef struct gui_nearby_site_info_fmt gui_nearby_site_info_fmt_t;

/* end of ����������Ϣ��ѯ */

/* ������Ϣ 2016-12-21 */
struct gui_net_info_backup
{
	long long rx_bytes;
	long long tx_bytes;
	long long total_bytes;
};
typedef struct gui_net_info_backup gui_net_info_backup_t;


#endif

