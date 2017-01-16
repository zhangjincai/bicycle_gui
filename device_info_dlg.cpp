#include <QtGui>
#include <QDebug>
#include "device_info_dlg.h"
#include "helper.h"
#include "lib_lnt.h"
#include "lib_general.h"

#include "gui_header.h"

extern lib_gui_t *g_gui;
extern int up_thr_runing; //读卡器升级状态 1为正在升级

#define	NET_INFO_BACKUP_PATH	"/opt/logpath/net_info.txt"
extern pthread_mutex_t g_net_mutex;

DeviceInfoDlg::DeviceInfoDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

	m_ret = LIB_LNT_ERROR;
	m_stat = 0;
	memset(&lnt_ver_ack, 0, sizeof(lib_lnt_getversion_ack_t));
	memset(&ndev_config, 0, sizeof(struct gui_ndev_config));
	memset(&lnt_config, 0, sizeof(lib_lnt_config_t));
	memset(&iboard_version, 0, sizeof(gui_iboard_device_version_t));
	
	__allWidgetClear();
	__getPageConfig();

	pushButton_manualRefresh->installEventFilter(this);
		
	connect(pushButton_manualRefresh, SIGNAL(clicked()), this, SLOT(slots_manualRefresh()));
}


DeviceInfoDlg::~DeviceInfoDlg(void)
{
	
}

void DeviceInfoDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_deviceInfoShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_deviceInfoShow->setText(tr("读取设备信息成功"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_TIMEOUT:
		{
			label_deviceInfoShow->setText(tr("读取设备信息超时"));	
		}
		break;

		case DEV_W_CONFIG_RESULT_REFRESH:
		{
			label_deviceInfoShow->setText(tr("刷新设备信息成功"));	
		}
		break;
		
	}	
}

void DeviceInfoDlg::slots_manualRefresh(void)
{
	if(__getPageConfig() >= 0)
		__showConfigResult(DEV_W_CONFIG_RESULT_REFRESH);  //刷新成功
	else
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
}

void DeviceInfoDlg::__allWidgetClear(void)
{
	label_lntFwVer->clear();
	label_lntSoftVer->clear();
	label_bootSoftVer->clear();
	label_kernelSoftVer->clear();
	label_rootfsSoftVer->clear();
	label_fwSoftVer->clear();
	label_applSoftVer->clear();
	label_eth0MAC->clear();
	label_eth1MAC->clear();
	
	label_MemTotal->clear();
	label_MemFree->clear();
	label_Buffers->clear();
	label_Cached->clear();
	label_CommitLimit->clear();
	label_Committed_AS->clear();

	
}

int DeviceInfoDlg::__getPageConfig(void)
{
	int retval = LIB_LNT_ERROR;
	QString string;
	char version[32] = {0};

	__allWidgetClear();

	memset(&lnt_ver_ack.version, 0, sizeof(lib_lnt_getversion_ack_t));
#if 1
	#if CONFS_USING_READER_UPDATE//#if CONFS_USING_TEST_BY_ZJC
	if(up_thr_runing == 0) //读卡器升级期间不能操作读卡器!!!
		m_ret = lib_lnt_get_version(&lnt_ver_ack, &m_stat, 100);
	#else
		m_ret = lib_lnt_get_version(&lnt_ver_ack, &m_stat, 100);
	#endif
#endif
	//m_ret = lib_lnt_get_version(&lnt_ver_ack, &m_stat, 100);
	if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_NORMAL))
	{
		string.clear();
		char version[32] = {0};
		strcpy(version, (char *)lnt_ver_ack.version);
		version[24] = '\0';
		Helper::__CharToQString(version, string);
		label_lntFwVer->setText(string);

		string.clear();
		retval = LIB_LNT_EOK;
	}

	lib_lnt_get_config(&lnt_config);  //读取岭南通配置
	
        lib_lnt_get_firmware_version(version); //liblnt.a版本
	Helper::__CharToQString(version, string);
	label_lntSoftVer->setText(string);

	retval = lib_gui_get_ndev_config(g_gui, &ndev_config, 500);
	if(retval == LIB_GUI_EOK)
	{
		__setPageConfig();
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);

	}
	else if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

#if 0
	if(retval == LIB_LNT_EOK)
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);

	if(retval == LIB_LNT_ETIMEOUT)
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
#endif

	retval = lib_gui_iboard_device_version(g_gui, &iboard_version, 1000);
	if(retval == LIB_GUI_EOK)
	{
		__setPageConfig();
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);

	}
	else if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}


	return retval;
}


void DeviceInfoDlg::__setPageConfig(void)
{
	QString string;
	string.clear();

        //设备固件版本
	Helper::__CharToQString(ndev_config.boot_ver, string);
	label_bootSoftVer->setText(string);

	Helper::__CharToQString(ndev_config.kernel_ver, string);
	label_kernelSoftVer->setText(string);

	Helper::__CharToQString(ndev_config.rootfs_ver, string);
	label_rootfsSoftVer->setText(string);	

	Helper::__CharToQString(ndev_config.fw_ver, string);
	label_fwSoftVer->setText(string);

	Helper::__CharToQString(ndev_config.appl_ver, string);
	label_applSoftVer->setText(string);

        //硬件信息
	char macaddr[32] = {0};
	lib_get_macaddr("eth0", macaddr);
	string.clear();
	string.sprintf("%02x.%02x.%02x.%02x.%02x.%02x", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	label_eth0MAC->setText(string);

	lib_get_macaddr("eth1", macaddr);
	string.clear();
	string.sprintf("%02x.%02x.%02x.%02x.%02x.%02x", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	label_eth1MAC->setText(string);

	struct proc_meminfo meminfo;
	memset(&meminfo, 0, sizeof(struct proc_meminfo));
    lib_get_proc_meminfo(&meminfo); //cat /proc/meminfo
	
	string.clear();
	string.sprintf("%02d KB", meminfo.MemTotal);
	label_MemTotal->setText(string);

	string.clear();
	string.sprintf("%02d KB", meminfo.MemFree);
	label_MemFree->setText(string);

	string.clear();
	string.sprintf("%02d KB", meminfo.Buffers);
	label_Buffers->setText(string);

	string.clear();
	string.sprintf("%02d KB", meminfo.Cached);
	label_Cached->setText(string);

	string.clear();
	string.sprintf("%02d KB", meminfo.CommitLimit);
	label_CommitLimit->setText(string);

	string.clear();
	string.sprintf("%02d KB", meminfo.Committed_AS);
	label_Committed_AS->setText(string);

	/* 岭南通读卡器信息 */
	string.clear();
	string.sprintf("%02x%02x%02x%02x ", lnt_config.pki_RO[0],lnt_config.pki_RO[1],lnt_config.pki_RO[2],lnt_config.pki_RO[3]);
        label_lntPKI->setText(string); //由pki_r改为pki_RO，修复读卡器签到失败查询不到PKI问题 add by zjc at 2016-10-13
	
	string.clear();
	string.sprintf("%02x%02x%02x%02x", lnt_config.psam[0], lnt_config.psam[1], lnt_config.psam[2], lnt_config.psam[3]);
	label_lntPSAM->setText(string);

	/* 接口板信息 */
	string.clear();
	string.sprintf("%04d", ntohs(iboard_version.vendor_encoding));
	label_vendorEncoding->setText(string);

	string.clear();
	string.sprintf("%x", iboard_version.device_type);
	label_deviceType->setText(string);

	string.clear();
	string.sprintf("%04d", ntohs(iboard_version.product_batch));
	label_productBatch->setText(string);

	string.clear();
	string.sprintf("%04d", ntohs(iboard_version.hardware_version));
	label_hardwareVersion->setText(string);

	string.clear();
	string.sprintf("%02d%02d%02d%02d%02d%02d%02d%02d", iboard_version.software_version[0], iboard_version.software_version[1], \
		iboard_version.software_version[2], iboard_version.software_version[3], \
		iboard_version.software_version[4], iboard_version.software_version[5], \
		iboard_version.software_version[6], iboard_version.software_version[7]);
	
	label_softwareVersion->setText(string);

#if 1
	/* 网卡信息显示 2016-12-21 */
	FILE *net_fd = NULL;
	int ret = -1; 
	gui_net_info_backup_t net_info_backup;
	float r_bytes = 0, t_bytes = 0, tt_bytes = 0;
	unsigned char r_M_flag = 0, t_M_flag = 0, tt_M_flag = 0;

	memset(&net_info_backup, 0, sizeof(gui_net_info_backup_t));
	
	net_fd = fopen(NET_INFO_BACKUP_PATH, "rb");
	if(NULL != net_fd)
	{
		ret = fread(&net_info_backup, sizeof(gui_net_info_backup_t), 1, net_fd);
		if(ret < 1) 
		{
			fprintf(stderr, "----------------read net backup info failed!\n");
			SYS_LOG_DEBUG("ERROR:read net backup info failed!");
		}

		fclose(net_fd);
		net_fd = NULL;
	}
	else    
	{
		fprintf(stderr, "----------------read net backup info failed!\n");
		SYS_LOG_DEBUG("ERROR:open net backup info for read failed!");
	}

	printf("----2----read: rx_bytes:%lld, tx_bytes:%lld, total_bytes:%lld\n",\
		net_info_backup.rx_bytes, net_info_backup.tx_bytes, net_info_backup.total_bytes);
	
	r_bytes = (float)net_info_backup.rx_bytes/1024; //以K为单位
	t_bytes = (float)net_info_backup.tx_bytes/1024;
	tt_bytes = (float)net_info_backup.total_bytes/1024;
	
	if(r_bytes > 1024.0)
	{       
		r_bytes = r_bytes/1024; //以M为单位
		r_M_flag = 1;
	}
	else
		r_M_flag = 0;
	
	if(t_bytes > 1024.0)  
	{
		t_bytes = t_bytes/1024;
		t_M_flag = 1;
	}   
	else
		t_M_flag = 0;
	
	if(tt_bytes > 1024.0)
	{
		tt_bytes = tt_bytes/1024; //以M为单位
		tt_M_flag = 1;
	}
	else
		tt_M_flag = 0;

	//printf("--------r_M_flag:%d, t_M_flag:%d, tt_M_flag:%d\n", r_M_flag, t_M_flag, tt_M_flag);
	//printf("--------r_bytes:%f, t_bytes:%f, tt_bytes:%f\n", r_bytes, t_bytes, tt_bytes);
	if((tt_M_flag == 1) && (tt_bytes > (1.9 * 1024))) //大于1.9G则红色提醒
	{
		label_total_bytes->setStyleSheet("color:red;");
	}
	
	if(r_M_flag == 1) //接收大于1M
	{
		string.clear();
		string.sprintf("%lld(%.3fM)", net_info_backup.rx_bytes, r_bytes);
		label_rx_bytes->setText(string);
	}
	else
	{
		string.clear();
		string.sprintf("%lld(%.3fK)", net_info_backup.rx_bytes, r_bytes);
		label_rx_bytes->setText(string);
	}
	
	if(t_M_flag == 1) //发送大于1M
	{
		string.clear();
		string.sprintf("%lld(%.3fM)", net_info_backup.tx_bytes, t_bytes);
		label_tx_bytes->setText(string);
	}
	else
	{
		string.clear();
		string.sprintf("%lld(%.3fK)", net_info_backup.tx_bytes, t_bytes);
		label_tx_bytes->setText(string);
	}

	if(tt_M_flag == 1) //总共大于1M
	{
		string.clear();
		string.sprintf("%lld(%.3fM)", net_info_backup.total_bytes, tt_bytes);
		label_total_bytes->setText(string);
	}
	else
	{
		string.clear();
		string.sprintf("%lld(%.3fK)", net_info_backup.total_bytes, tt_bytes);
		label_total_bytes->setText(string);
	}
#endif
}


bool DeviceInfoDlg::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == pushButton_manualRefresh)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			pushButton_manualRefresh->setStyleSheet("background-color: rgb(85, 170, 255)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_manualRefresh->setStyleSheet("background-color: rgb(255, 255, 255)");
		}
	}	
	
	return QWidget::eventFilter(watched,event); 
}


