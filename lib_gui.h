#ifndef __LIB_GUI_H__
#define __LIB_GUI_H__

/*@*/
#ifdef __cplusplus
extern "C" {
#endif
/*@*/

#include "gui_header.h"

/*
 * LIB GUI��̬��汾��
 */
#define LIB_GUI_VERSION		"1.0.3"


/*
 * ���������붨��
 */
#define LIB_GUI_NULL				(NULL) 
#define LIB_GUI_EOK				(0)  //����
#define LIB_GUI_ERROR			(-1) //����
#define LIB_GUI_ETIMEOUT			(-2) //��ʱ
#define LIB_GUI_EFULL			(-3) //��
#define LIB_GUI_EEMPTY			(-4) //��
#define LIB_GUI_ENOMEM 			(-5) //�ڴ治��
#define LIB_GUI_EXCMEM			(-6) //�ڴ�Խ��
#define LIB_GUI_EBUSY			(-7) //æ
#define LIB_GUI_ERR_COMMAND     	(-8) //��֧�ֵ�����

#define LIB_GUI_TRUE				(1)
#define LIB_GUI_FALSE			(0)



enum GUI_CONNECT_MODE
{
	GUI_CONNECT_MODE_UNIX = 0,
	GUI_CONNECT_MODE_TCP,
	GUI_CONNECT_MODE_SERIAL
};

struct tcp_connect_mode
{
	char ipaddr[32];
	unsigned short port;
	unsigned int conn_nsec;
};

struct serial_connect_mode
{
	char pathname[32];
};

struct gui_setup
{
	int connect_mode;
	union{
		struct tcp_connect_mode tcp;
		struct serial_connect_mode serial;
	}s_un;
};



typedef struct gui lib_gui_t;


/*
 * �ɹ�����:LIB_GUI_EOK
 */

int lib_gui_version(lib_gui_t *gui, char *version, const unsigned int msec);  //��ȡGUI��汾

lib_gui_t *lib_gui_new(struct gui_setup *setup);
void lib_gui_destroy(lib_gui_t *gui);
int lib_gui_reconnect(lib_gui_t *gui);


int lib_gui_get_ndev_stat(lib_gui_t *gui, gui_ndev_status_t *stat, const unsigned int msec);
int lib_gui_get_sae_stat(lib_gui_t *gui, const unsigned char id, gui_sae_status_t *stat, const unsigned int msec);
int lib_gui_get_ndev_config(lib_gui_t *gui, struct gui_ndev_config *config, const unsigned int msec);
int lib_gui_get_stake_config(lib_gui_t *gui, struct gui_stake_config *config, const unsigned int msec);
int lib_gui_ndev_version(lib_gui_t *gui, char version[32], const unsigned int msec);  //��ȡ�ڵ���̼��汾


/*
 * ������Ϣҳ��
 */
  int lib_gui_get_basic_page_config(lib_gui_t *gui, struct gui_basic_info_page_config *config, const unsigned int msec);

/*
 * �ڵ������ҳ��
 */
int lib_gui_get_ndev_page_config(lib_gui_t *gui, struct gui_ndev_page_config *config, const unsigned int msec);  //��ȡ�ڵ��ҳ������
int lib_gui_get_ndev_access_pattern_config(lib_gui_t *gui, struct gui_ndev_access_pattern_config *config, const unsigned int msec);  //��ȡ������뷽ʽ

int lib_gui_set_ndev_ftp_config(lib_gui_t *gui, struct gui_ndev_ftp_config *config, const unsigned int msec);   //����FTP
int lib_gui_set_ndev_center_config(lib_gui_t *gui, struct gui_ndev_center_ip_config *config, const unsigned int msec);   //�������ĵ�ַ--���ؾ����������ַ
int lib_gui_set_ndev_wifi_config(lib_gui_t *gui, struct gui_ndev_wifi_ip_config *config, const unsigned int msec);   //����WIFI
int lib_gui_set_ndev_local_config_reboot(lib_gui_t *gui, struct gui_ndev_local_ip_config *config, const unsigned int msec);   //���ñ��ص�ַ
int lib_gui_set_ndev_parameter_config(lib_gui_t *gui, struct gui_ndev_parameter_config *config, const unsigned int msec); //�ڵ����������
int lib_gui_set_ndev_parameter_config_reboot(lib_gui_t *gui, struct gui_ndev_parameter_config *config, const unsigned int msec);  
int lib_gui_set_stake_parameter_config(lib_gui_t *gui, struct gui_stake_parameter_config *config, const unsigned int msec); //��׮��������
int lib_gui_set_ndev_access_pattern_config_reboot(lib_gui_t *gui, struct gui_ndev_access_pattern_config *config, const unsigned int msec); //�ڵ��������뷽ʽ
int lib_gui_set_lnt_config_reboot(lib_gui_t *gui, struct gui_lnt_page_config *config, const unsigned int msec);  //����ͨ����


/*
 * ��׮��Ϣҳ��
 */
  int lib_gui_get_stake_info_page_config(lib_gui_t *gui, struct gui_stake_info_page_config *config, const unsigned int msec);

/*
 * ��ȡ������Ϣҳ��
 */
 int lib_gui_get_ndev_info_page_config_stage1(lib_gui_t *gui, const unsigned int msec);
 int lib_gui_get_ndev_info_page_config_stage2(lib_gui_t *gui, struct gui_ndev_home_page_info *config, const unsigned int msec); //��ȡ������Ϣҳ��

/* ��׮ȫ����Ϣ */
int lib_gui_get_stake_all_info_page_config(lib_gui_t *gui, struct gui_stake_all_info_page_config *config, const unsigned int msec);

/* ��������ͨ������״̬ */
int lib_gui_set_lnt_card_status(lib_gui_t *gui, struct gui_lnt_card_status *config, const unsigned int msec);

/* ����ʹ�ü�¼ */
int lib_gui_set_admin_card_info(lib_gui_t *gui, gui_admin_card_info_t *info, const unsigned int msec);

/* �쳣����۷ѽ��׼�¼ */
int lib_gui_set_exception_handle_record(lib_gui_t *gui, gui_exception_handle_record_t *record, const unsigned int msec);

/* �쳣���� */
int lib_gui_except_handle_req(lib_gui_t *gui, gui_except_handle_req_t *req, const unsigned int msec);
int lib_gui_except_handle_ack(lib_gui_t *gui, gui_except_handle_ack_t *ack, const unsigned int msec);


/* �Ա���Ϣ */
int lib_gui_get_sae_comparison_status(lib_gui_t *gui, gui_sae_comparison_status_t comp[65], const unsigned int msec);



int lib_gui_access_state(lib_gui_t *gui, gui_access_state_t *access, const unsigned int msec);
int lib_gui_get_gps_info(lib_gui_t *gui, struct gui_gps_info *info, const unsigned int msec);

/* �⻹����¼��ѯ */
int lib_gui_rent_info_qry_req(lib_gui_t *gui, gui_rent_info_qry_req_t *req, const unsigned int msec);
int lib_gui_rent_info_qry_ack(lib_gui_t *gui, gui_rent_info_qry_ack_t *ack, const unsigned int msec);

int lib_gui_iboard_device_version(lib_gui_t *gui, gui_iboard_device_version_t *version, const unsigned int msec);
int lib_gui_light_ctrl_time_set(lib_gui_t *gui, gui_light_ctrl_time_t *time, const unsigned int msec);
int lib_gui_light_ctrl_time_get(lib_gui_t *gui, gui_light_ctrl_time_t *time, const unsigned int msec);

int lib_gui_unity_file_info_get(lib_gui_t *gui, gui_unity_file_info_t *file, const unsigned int msec);

int lib_gui_lnt_all_info_set(lib_gui_t *gui, gui_lnt_all_info_t *info, const unsigned int msec);

int lib_gui_delay_return_bicyle_req(lib_gui_t *gui, const unsigned int msec);
int lib_gui_delay_return_bicyle_ack(lib_gui_t *gui, gui_delay_return_bicycle_t *bicycle, const unsigned int msec);

/* ����������Ϣ��ѯ */
int lib_gui_nearby_site_info_qry_req(lib_gui_t *gui, gui_nearby_site_info_qry_req_t *req, const unsigned int msec);
int lib_gui_nearby_site_info_qry_ack(lib_gui_t *gui, gui_nearby_site_info_qry_ack_t *ack, const unsigned int msec);




/*@*/
#ifdef __cplusplus
}
#endif
#endif

