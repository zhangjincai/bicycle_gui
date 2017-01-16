#include <QtGui>
#include <QDebug>
#include <QString>
#include <string.h>
#include <QKeyEvent>
#include "ndev_config_dlg.h"
#include "helper.h"
#include "lib_general.h"


extern lib_gui_t *g_gui;
extern int g_adminCardCtrlSet;

NdevConfigDlg::NdevConfigDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	FormHelper::__FormHideTitleBar(this);

	__getPageConfig();

	__controlSettings(g_adminCardCtrlSet);   //��������
	

	pushButton_ndevConfigSetup->installEventFilter(this);
	memset(&ctrl_time, 0, sizeof(gui_light_ctrl_time));

	connect(pushButton_ndevConfigSetup, SIGNAL(clicked()), this, SLOT(slots_ndevConfigSetup()));
	connect(checkBox_50kbps, SIGNAL(stateChanged(int)), this, SLOT(slots_canBaudRate50kbpsCheckBox(int)));
	connect(checkBox_125kbps, SIGNAL(stateChanged(int)), this, SLOT(slots_canBaudRate125kbpsCheckBox(int)));
	connect(checkBox_250kbps, SIGNAL(stateChanged(int)), this, SLOT(slots_canBaudRate250kbpsCheckBox(int)));
	connect(checkBox_500kbps, SIGNAL(stateChanged(int)), this, SLOT(slots_canBaudRate500kbpsCheckBox(int)));
	connect(checkBox_selectWireless, SIGNAL(stateChanged(int)), this, SLOT(slots_selectWirelessCheckBox(int)));
	connect(checkBox_selectWired, SIGNAL(stateChanged(int)), this, SLOT(slots_selectWiredCheckBox(int)));
}

NdevConfigDlg::~NdevConfigDlg(void)
{
	
}

void NdevConfigDlg::slots_ndevConfigSetup(void)
{
	int ret = LIB_GUI_ERROR;
	
	ret = __ftpSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;
	
	ret = __centerSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;
		
	ret = __wifiSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;
	
	ret = __localSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;

	ret = __nParameterSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;

	ret = __sParameterSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;

	ret = __accessPatternSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;	

	ret = __lntSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;	

	ret = __lightCtrlTimeSetConfig();
	if(ret == LIB_GUI_ERROR)
		return;	
}

void NdevConfigDlg::slots_canBaudRate50kbpsCheckBox(int state)
{
	if(state == Qt::Checked)
	{
		canBaudRateCheck = 1;
		checkBox_125kbps->setCheckState(Qt::Unchecked);
		checkBox_250kbps->setCheckState(Qt::Unchecked);
		checkBox_500kbps->setCheckState(Qt::Unchecked);
	}
}

void NdevConfigDlg::slots_canBaudRate125kbpsCheckBox(int state)
{
	if(state == Qt::Checked)
	{
		canBaudRateCheck = 2;
		checkBox_50kbps->setCheckState(Qt::Unchecked);
		checkBox_250kbps->setCheckState(Qt::Unchecked);
		checkBox_500kbps->setCheckState(Qt::Unchecked);
	}	
}

void NdevConfigDlg::slots_canBaudRate250kbpsCheckBox(int state)
{
	if(state == Qt::Checked)
	{
		canBaudRateCheck = 3;
		checkBox_50kbps->setCheckState(Qt::Unchecked);
		checkBox_125kbps->setCheckState(Qt::Unchecked);
		checkBox_500kbps->setCheckState(Qt::Unchecked);
	}	
}

void NdevConfigDlg::slots_canBaudRate500kbpsCheckBox(int state)
{
	if(state == Qt::Checked)
	{
		canBaudRateCheck = 4;
		checkBox_50kbps->setCheckState(Qt::Unchecked);
		checkBox_125kbps->setCheckState(Qt::Unchecked);
		checkBox_250kbps->setCheckState(Qt::Unchecked);
	}	
}

void NdevConfigDlg::slots_selectWirelessCheckBox(int state)
{
	if(state == Qt::Checked)
	{
		accessPatternCheck = 1;
		checkBox_selectWired->setCheckState(Qt::Unchecked);
	}
}

void NdevConfigDlg::slots_selectWiredCheckBox(int state)
{
	if(state == Qt::Checked)
	{
		accessPatternCheck = 2;
		checkBox_selectWireless->setCheckState(Qt::Unchecked);
	}	
}
	
void NdevConfigDlg::__allWidgetClear(void)
{
	lineEdit_localIpaddr->clear();
	lineEdit_localNetmask->clear();
	lineEdit_localGateway->clear();
	
	lineEdit_wifiIpaddr->clear();
	lineEdit_wifiNetmask->clear();
	lineEdit_wifiGateway->clear();

	lineEdit_loadServ1Ipaddr->clear();
	lineEdit_loadServ1Port->clear();
	lineEdit_loadServ2Ipaddr->clear();
	lineEdit_loadServ2Port->clear();

	lineEdit_ftpIpaddr->clear();
	lineEdit_ftpPort->clear();
	lineEdit_ftpUserName->clear();
	lineEdit_ftpUserPasswd->clear();

	lineEdit_loadSoTimeout->clear();
	lineEdit_centerHeartTime->clear();
	lineEdit_timerGateValue->clear();
	lineEdit_termId->clear();
	
	lineEdit_stakeHeartTime->clear();
	lineEdit_stakeQuantity->clear();

	checkBox_50kbps->setCheckState(Qt::Unchecked);
	checkBox_125kbps->setCheckState(Qt::Unchecked);
	checkBox_250kbps->setCheckState(Qt::Unchecked);
	checkBox_500kbps->setCheckState(Qt::Unchecked);

	checkBox_selectWireless->setCheckState(Qt::Unchecked);
	checkBox_selectWired->setCheckState(Qt::Unchecked);	

	label_ndevConfigShow->clear(); //״̬��ʾ
}

void NdevConfigDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_ndevConfigShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_ndevConfigShow->setText(tr("�ڵ����ȡ���óɹ�"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_TIMEOUT:
		{
			label_ndevConfigShow->setText(tr("�ڵ����ȡ���ó�ʱ"));	
		}
		break;

		case DEV_W_CONFIG_RESULT_OK:
		{
			label_ndevConfigShow->setText(tr("�ڵ��д�����óɹ�"));	
		}
		break;

		case DEV_W_CONFIG_RESULT_TIMEOUT:  //��ʱ
		{
			label_ndevConfigShow->setText(tr("�ڵ��д�����ó�ʱ"));	
		}
		break;

		case DEV_W_CONFIG_RESULT_NO_CHANGE:  //����δ�ı�
		{
			label_ndevConfigShow->setText(tr("�ڵ������δ�仯"));	
		}
		break;
		
		case DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS: //IP��ַ��ʽ����ȷ
		{
			label_ndevConfigShow->setText(tr("����IP��ַ��ʽ����ȷ"));	
		}
		break;
	}
}

int NdevConfigDlg::__getPageConfig(void)
{
	memset(&nPageConfig, 0, sizeof(struct gui_ndev_page_config));
	retval = lib_gui_get_ndev_page_config(g_gui, &nPageConfig, 2000);  //��ȡ�ڵ����������
	if(retval == LIB_GUI_EOK)
	{
		__setPageConfig();
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);

	}
	else if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval = lib_gui_light_ctrl_time_get(g_gui, &ctrl_time, 500);

	//lib_printf("CTRL TIME", (unsigned char *)&ctrl_time, sizeof(ctrl_time));
	
	if(retval == LIB_GUI_EOK)
	{
		memcpy(&t_ctrl_time, &ctrl_time, sizeof(t_ctrl_time));
		
		__setPageConfig();
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);

	}
	else if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	return retval;
}

void NdevConfigDlg::__setPageConfig(void)
{
	QString string;

	__allWidgetClear();

	/* ����IP��ַ */
	Helper::__CharToQString(nPageConfig.ip_conf.ipaddr, string);
	lineEdit_localIpaddr->setText(string);

	Helper::__CharToQString(nPageConfig.ip_conf.subnet_mask, string);
	lineEdit_localNetmask->setText(string);

	Helper::__CharToQString(nPageConfig.ip_conf.default_gateway, string);
	lineEdit_localGateway->setText(string);

	/* WIFI-IP��ַ */
	Helper::__CharToQString(nPageConfig.wifi_ip_conf.ipaddr, string);
	lineEdit_wifiIpaddr->setText(string);

	Helper::__CharToQString(nPageConfig.wifi_ip_conf.subnet_mask, string);
	lineEdit_wifiNetmask->setText(string);

	Helper::__CharToQString(nPageConfig.wifi_ip_conf.default_gateway, string);
	lineEdit_wifiGateway->setText(string);

	/* ���г����ĵ�ַ */
	Helper::__CharToQString(nPageConfig.center_ip_conf.load1_ipaddr, string);
	lineEdit_loadServ1Ipaddr->setText(string);
	Helper::__UshortToQString(nPageConfig.center_ip_conf.load1_port, string);
	lineEdit_loadServ1Port->setText(string);

	Helper::__CharToQString(nPageConfig.center_ip_conf.load2_ipaddr, string);
	lineEdit_loadServ2Ipaddr->setText(string);
	Helper::__UshortToQString(nPageConfig.center_ip_conf.load2_port, string);
	lineEdit_loadServ2Port->setText(string);

	/* FTP��ַ */
	Helper::__CharToQString(nPageConfig.ftp_conf.ipaddr, string);
	lineEdit_ftpIpaddr->setText(string);
	
	Helper::__UshortToQString(nPageConfig.ftp_conf.port, string);
	lineEdit_ftpPort->setText(string);

	Helper::__CharToQString(nPageConfig.ftp_conf.user_name, string);
	lineEdit_ftpUserName->setText(string);

	Helper::__CharToQString(nPageConfig.ftp_conf.user_password, string);
	lineEdit_ftpUserPasswd->setText(string);

	/* �ڵ���������� */
	Helper::__UshortToQString(nPageConfig.n_param_conf.load_timeout, string);
	lineEdit_loadSoTimeout->setText(string);

	Helper::__UshortToQString(nPageConfig.n_param_conf.heart_time, string);
	lineEdit_centerHeartTime->setText(string);

	//Helper::__UshortToQString(nPageConfig.n_param_conf.emerg_heart_time, string);
	//lineEdit_centerEmergHeartTime->setText(string);

	Helper::__UshortToQString(nPageConfig.n_param_conf.timer_gate_value, string);
	lineEdit_timerGateValue->setText(string);

	Helper::__UshortToQString(nPageConfig.n_param_conf.term_id, string);
	lineEdit_termId->setText(string);

	/* ��׮�������� */
	unsigned char m_canBaudRate = nPageConfig.s_param_conf.can_baud_rate;
	canBaudRateCheck = m_canBaudRate;
	switch(m_canBaudRate)
	{
		case 1:
		{
			checkBox_50kbps->setCheckState(Qt::Checked);
		}
		break;

		case 2:
		{
			checkBox_125kbps->setCheckState(Qt::Checked);
		}
		break;

		case 3:
		{
			checkBox_250kbps->setCheckState(Qt::Checked);
		}
		break;

		case 4:
		{
			checkBox_500kbps->setCheckState(Qt::Checked);
		}
		break;
	}

	Helper::__UshortToQString(nPageConfig.s_param_conf.heart_time, string);
	lineEdit_stakeHeartTime->setText(string);

	Helper::__UshortToQString(nPageConfig.s_param_conf.quantity, string);
	lineEdit_stakeQuantity->setText(string);

	/* ���뷽ʽ */
	unsigned char m_accessPattern = nPageConfig.access_conf.using_wireless;
	accessPatternCheck = m_accessPattern;
	switch(m_accessPattern)
	{
		case 1:
		{
			checkBox_selectWireless->setCheckState(Qt::Checked);
		}
		break;

		case 2:
		{
			checkBox_selectWired->setCheckState(Qt::Checked);
		}
		break;
	}

	/* ����ͨ���� */
	Helper::__CharToQString(nPageConfig.lnt_conf.ipaddr, string);
	lineEdit_lntIpaddr->setText(string);
	
	Helper::__UshortToQString(nPageConfig.lnt_conf.port, string);
	lineEdit_lntServ1Port->setText(string);



	/* ������� */
	string.clear();
	string.sprintf("%02d", ctrl_time.light_ctrl_time_enable[1]);
	lineEdit_lightCtrlTimeEnable0->setText(string);

	string.clear();
	string.sprintf("%02d", ctrl_time.light_ctrl_time_enable[0]);
	lineEdit_lightCtrlTimeEnable1->setText(string);

	string.clear();
	string.sprintf("%02d", ctrl_time.light_ctrl_time_disable[1]);
	lineEdit_lightCtrlTimeDisenable0->setText(string);

	string.clear();
	string.sprintf("%02d", ctrl_time.light_ctrl_time_disable[0]);
	lineEdit_lightCtrlTimeDisenable1->setText(string);
	
}

/*
 * FTP����
 */
int NdevConfigDlg::__ftpSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_ndev_ftp_config ftp_config;
	memset(&ftp_config, 0, sizeof(struct gui_ndev_ftp_config));

	QString string;
	char *str = NULL;
	unsigned short port = 0;

	string = lineEdit_ftpIpaddr->text();   //FTP��ַ
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	
	str = Helper::__QStringToChar(string);	
	strcpy(ftp_config.ipaddr, str);

	string = lineEdit_ftpPort->text();	//�˿�
	port = Helper::__QStringToUshort(string);
	ftp_config.port = port;

	string = lineEdit_ftpUserName->text();   //�û���
	str = Helper::__QStringToChar(string);
	strcpy(ftp_config.user_name, str);

	string = lineEdit_ftpUserPasswd->text();  //����
	str = Helper::__QStringToChar(string);
	strcpy(ftp_config.user_password, str);	

	if(memcmp(&ftp_config, &(nPageConfig.ftp_conf), sizeof(struct gui_ndev_ftp_config)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_set_ndev_ftp_config(g_gui, &ftp_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	//__showConfigResult(DEV_W_CONFIG_RESULT_NO_CHANGE);

	return LIB_GUI_EOK;
}


/*
 * ���г����ĵ�ַ����
 */
int NdevConfigDlg::__centerSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_ndev_center_ip_config center_config;
	memset(&center_config, 0, sizeof(struct gui_ndev_center_ip_config));

	QString string;
	char *str = NULL;
	unsigned short port = 0;

	string = lineEdit_loadServ1Ipaddr->text();   //��ַ
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(center_config.load1_ipaddr, str);

	string = lineEdit_loadServ1Port->text();	//�˿�
	port = Helper::__QStringToUshort(string);
	center_config.load1_port = port;

	string = lineEdit_loadServ2Ipaddr->text();   //��ַ
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(center_config.load2_ipaddr, str);

	string = lineEdit_loadServ2Port->text();	//�˿�
	port = Helper::__QStringToUshort(string);
	center_config.load2_port = port;	

	if(memcmp(&center_config, &(nPageConfig.center_ip_conf), sizeof(struct gui_ndev_center_ip_config)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_set_ndev_center_config(g_gui, &center_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	//__showConfigResult(DEV_W_CONFIG_RESULT_NO_CHANGE);

	return LIB_GUI_EOK;
}

/*
 * WIFI����
 */
int NdevConfigDlg::__wifiSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_ndev_wifi_ip_config wifi_config;
	memset(&wifi_config, 0, sizeof(struct gui_ndev_wifi_ip_config));

	QString string;
	char *str = NULL;

	string = lineEdit_wifiIpaddr->text();   //��ַ
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(wifi_config.ipaddr, str);

	string = lineEdit_wifiNetmask->text();   //����
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(wifi_config.subnet_mask, str);

	string = lineEdit_wifiGateway->text();   //����
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(wifi_config.default_gateway, str);
	
	if(memcmp(&wifi_config, &(nPageConfig.wifi_ip_conf), sizeof(struct gui_ndev_wifi_ip_config)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_set_ndev_wifi_config(g_gui, &wifi_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	//__showConfigResult(DEV_W_CONFIG_RESULT_NO_CHANGE);

	return LIB_GUI_EOK;
}

/*
 * ��������
 */
int NdevConfigDlg::__localSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_ndev_local_ip_config local_config;
	memset(&local_config, 0, sizeof(struct gui_ndev_local_ip_config));

	QString string;
	char *str = NULL;

	string = lineEdit_localIpaddr->text();   //��ַ
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(local_config.ipaddr, str);

	string = lineEdit_localNetmask->text();   //����
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(local_config.subnet_mask, str);

	string = lineEdit_localGateway->text();   //����
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(local_config.default_gateway, str);

	if(memcmp(&local_config, &(nPageConfig.ip_conf), sizeof(struct gui_ndev_local_ip_config)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_set_ndev_local_config_reboot(g_gui, &local_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	//__showConfigResult(DEV_W_CONFIG_RESULT_NO_CHANGE);	

	return LIB_GUI_EOK;
}

int NdevConfigDlg::__nParameterSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_ndev_parameter_config param_config;
	memset(&param_config, 0, sizeof(struct gui_ndev_parameter_config));

	QString string;
	unsigned short n = 0;

	string = lineEdit_loadSoTimeout->text();	 //���ؾ��ⳬʱʱ��
	n = Helper::__QStringToUshort(string);
	param_config.load_timeout = n;

	string = lineEdit_centerHeartTime->text();	 //�����������
	n = Helper::__QStringToUshort(string);
	param_config.heart_time = n;

	/*
	string = lineEdit_centerEmergHeartTime->text();	 //���Ľ����������
	n = Helper::__QStringToUshort(string);
	param_config.emerg_heart_time = n;
	*/

	string = lineEdit_timerGateValue->text();	 //ʱ������բֵ
	n = Helper::__QStringToUshort(string);

	if(n > 60)
		n = 60;

	if(n < 10)
		n = 10;
	
	param_config.timer_gate_value = n;

	
	string = lineEdit_termId->text();	 		//�ն˱��
	n = Helper::__QStringToUshort(string);
	param_config.term_id = n;

	if(memcmp(&param_config, &(nPageConfig.n_param_conf), sizeof(struct gui_ndev_parameter_config)) != 0)  //���øı�ʱ
	{
		//ret = lib_gui_set_ndev_parameter_config(g_gui, &param_config, 2000);
		ret = lib_gui_set_ndev_parameter_config_reboot(g_gui, &param_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	return LIB_GUI_EOK;
}

int NdevConfigDlg::__sParameterSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_stake_parameter_config param_config;
	memset(&param_config, 0, sizeof(struct gui_stake_parameter_config));

	QString string;
	unsigned short n = 0;	

	param_config.can_baud_rate = canBaudRateCheck;	//CAN������

	string = lineEdit_stakeHeartTime->text();	 //��׮�������
	n = Helper::__QStringToUshort(string);
	param_config.heart_time = n;

	string = lineEdit_stakeQuantity->text();	 //��׮����
	n = Helper::__QStringToUshort(string);
	if(n > 64)  //���64����׮
	{
		lineEdit_stakeQuantity->setText("64");
		param_config.quantity = 64;
	}
	else
		param_config.quantity = n;

	if(memcmp(&param_config, &(nPageConfig.s_param_conf), sizeof(struct gui_stake_parameter_config)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_set_stake_parameter_config(g_gui, &param_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	return LIB_GUI_EOK;
}

int NdevConfigDlg::__accessPatternSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_ndev_access_pattern_config access_config;
	memset(&access_config, 0, sizeof(struct gui_ndev_access_pattern_config));

	access_config.using_wireless = accessPatternCheck;  //���뷽ʽ

	if(memcmp(&access_config, &(nPageConfig.access_conf), sizeof(struct gui_ndev_access_pattern_config)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_set_ndev_access_pattern_config_reboot(g_gui, &access_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	return LIB_GUI_EOK;
}

int NdevConfigDlg::__lntSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_lnt_page_config lnt_config;
	memset(&lnt_config, 0, sizeof(struct gui_lnt_page_config));

	QString string;
	char *str = NULL;
	unsigned short port = 0;

	string = lineEdit_lntIpaddr->text();   //��ַ
	if(!Helper::__IsIpAddress(string))
	{
		__showConfigResult(DEV_W_CONFIG_RESULT_NOT_IP_ADDRESS);	
		return LIB_GUI_ERROR;
	}
	str = Helper::__QStringToChar(string);	
	strcpy(lnt_config.ipaddr, str);

	string = lineEdit_lntServ1Port->text();	//�˿�
	port = Helper::__QStringToUshort(string);
	lnt_config.port= port;

	if(memcmp(&lnt_config, &(nPageConfig.lnt_conf), sizeof(struct gui_lnt_page_config)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_set_lnt_config_reboot(g_gui, &lnt_config, 2000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}

	return LIB_GUI_EOK;	
}

int NdevConfigDlg::__lightCtrlTimeSetConfig(void)
{
	int ret = LIB_GUI_ERROR;
	struct gui_light_ctrl_time c_time;
	memset(&c_time, 0, sizeof(struct gui_light_ctrl_time));
	unsigned short s_val, s1_val;
	
	QString c_string;

	c_string.clear();
	c_string = lineEdit_lightCtrlTimeEnable1->text();
	s_val = Helper::__QStringToUshort(c_string);	
	if(s_val > 60)
		s1_val = 60;
	else
		s1_val = s_val;
	c_time.light_ctrl_time_enable[0] = (unsigned char)s1_val;
		
	c_string.clear();
	c_string = lineEdit_lightCtrlTimeEnable0->text();
	s_val =  Helper::__QStringToUshort(c_string);
	if(s_val > 24)
		s1_val = 24;
	else
		s1_val = s_val;
	c_time.light_ctrl_time_enable[1] = s1_val;
	
	c_string.clear();
	c_string = lineEdit_lightCtrlTimeDisenable1->text();
	s_val = Helper::__QStringToUshort(c_string);
	if(s_val > 60)
		s1_val = 60;
	else
		s1_val = s_val;
	c_time.light_ctrl_time_disable[0] = s1_val;

	c_string.clear();
	c_string = lineEdit_lightCtrlTimeDisenable0->text();
	s_val = Helper::__QStringToUshort(c_string);
	if(s_val > 24)
		s1_val = 24;
	else
		s1_val = s_val;
	c_time.light_ctrl_time_disable[1] = (unsigned char)s1_val;


		//lib_printf("t_ctrl_time", (unsigned char *)&t_ctrl_time, sizeof(t_ctrl_time));
		//lib_printf("c_time", (unsigned char *)&c_time, sizeof(c_time));
		
	if(memcmp(&t_ctrl_time, &(c_time), sizeof(struct gui_light_ctrl_time)) != 0)  //���øı�ʱ
	{
		ret = lib_gui_light_ctrl_time_set(g_gui, &c_time, 1000);
		if(ret == LIB_GUI_EOK)
		{
			__showConfigResult(DEV_W_CONFIG_RESULT_OK);
			return LIB_GUI_EOK;
		}
		__showConfigResult(DEV_W_CONFIG_RESULT_TIMEOUT);

		return LIB_GUI_ERROR;
	}
	

	return LIB_GUI_EOK;	
}

void NdevConfigDlg::__controlSettings(const int ctrl)
{
	if(ctrl > 0)
	{
		lineEdit_localIpaddr->setEnabled(TRUE);
		lineEdit_localNetmask->setEnabled(TRUE);
		lineEdit_localGateway->setEnabled(TRUE);

		lineEdit_wifiIpaddr->setEnabled(FALSE);
		lineEdit_wifiNetmask->setEnabled(FALSE);
		lineEdit_wifiGateway->setEnabled(FALSE);

		lineEdit_loadServ1Ipaddr->setEnabled(TRUE);
		lineEdit_loadServ1Port->setEnabled(TRUE);
		lineEdit_loadServ2Ipaddr->setEnabled(TRUE);
		lineEdit_loadServ2Port->setEnabled(TRUE);

		lineEdit_ftpIpaddr->setEnabled(TRUE);
		lineEdit_ftpPort->setEnabled(TRUE);
		lineEdit_ftpUserName->setEnabled(FALSE);
		lineEdit_ftpUserPasswd->setEnabled(FALSE);

		lineEdit_loadSoTimeout->setEnabled(TRUE);
		lineEdit_centerHeartTime->setEnabled(TRUE);
		lineEdit_timerGateValue->setEnabled(TRUE);
		lineEdit_termId->setEnabled(TRUE);

		checkBox_50kbps->setEnabled(TRUE);
		checkBox_125kbps->setEnabled(TRUE);
		checkBox_250kbps->setEnabled(TRUE);
		checkBox_500kbps->setEnabled(TRUE);
		lineEdit_stakeHeartTime->setEnabled(TRUE);
		lineEdit_stakeQuantity->setEnabled(TRUE);

		checkBox_selectWireless->setEnabled(TRUE);
		checkBox_selectWired->setEnabled(TRUE);

		lineEdit_lntIpaddr->setEnabled(TRUE);
		lineEdit_lntServ1Port->setEnabled(TRUE);

		lineEdit_lightCtrlTimeEnable0->setEnabled(TRUE);
		lineEdit_lightCtrlTimeEnable1->setEnabled(TRUE);
		lineEdit_lightCtrlTimeDisenable0->setEnabled(TRUE);
		lineEdit_lightCtrlTimeDisenable1->setEnabled(TRUE);

		pushButton_ndevConfigSetup->setEnabled(TRUE);
	}
	else
	{
		lineEdit_localIpaddr->setEnabled(FALSE);
		lineEdit_localNetmask->setEnabled(FALSE);
		lineEdit_localGateway->setEnabled(FALSE);

		lineEdit_wifiIpaddr->setEnabled(FALSE);
		lineEdit_wifiNetmask->setEnabled(FALSE);
		lineEdit_wifiGateway->setEnabled(FALSE);

		lineEdit_loadServ1Ipaddr->setEnabled(FALSE);
		lineEdit_loadServ1Port->setEnabled(FALSE);
		lineEdit_loadServ2Ipaddr->setEnabled(FALSE);
		lineEdit_loadServ2Port->setEnabled(FALSE);

		lineEdit_ftpIpaddr->setEnabled(FALSE);
		lineEdit_ftpPort->setEnabled(FALSE);
		lineEdit_ftpUserName->setEnabled(FALSE);
		lineEdit_ftpUserPasswd->setEnabled(FALSE);

		lineEdit_loadSoTimeout->setEnabled(FALSE);
		lineEdit_centerHeartTime->setEnabled(FALSE);
		lineEdit_timerGateValue->setEnabled(FALSE);
		lineEdit_termId->setEnabled(FALSE);

		checkBox_50kbps->setEnabled(FALSE);
		checkBox_125kbps->setEnabled(FALSE);
		checkBox_250kbps->setEnabled(FALSE);
		checkBox_500kbps->setEnabled(FALSE);
		lineEdit_stakeHeartTime->setEnabled(FALSE);
		lineEdit_stakeQuantity->setEnabled(FALSE);

		checkBox_selectWireless->setEnabled(FALSE);
		checkBox_selectWired->setEnabled(FALSE);

		lineEdit_lntIpaddr->setEnabled(FALSE);
		lineEdit_lntServ1Port->setEnabled(FALSE);

		lineEdit_lightCtrlTimeEnable0->setEnabled(FALSE);
		lineEdit_lightCtrlTimeEnable1->setEnabled(FALSE);
		lineEdit_lightCtrlTimeDisenable0->setEnabled(FALSE);
		lineEdit_lightCtrlTimeDisenable1->setEnabled(FALSE);

		pushButton_ndevConfigSetup->setEnabled(FALSE);

		label_ndevConfigShow->clear();
		label_ndevConfigShow->setText(tr("ע��:�ù���ֻ����\"��\"Ȩ��,��\"�˳�\"��������һ���˵�"));	
	}
}


void NdevConfigDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)
		close();

	if(e->key() == Qt::Key_Up)
		focusPreviousChild();

	if(e->key() == Qt::Key_Down)
		focusNextChild();

	

}

bool NdevConfigDlg::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == pushButton_ndevConfigSetup)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			pushButton_ndevConfigSetup->setStyleSheet("background-color: rgb(85, 170, 255)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_ndevConfigSetup->setStyleSheet("background-color: rgb(255, 255, 255)");
		}
	}	

	return QWidget::eventFilter(watched,event); 
}














