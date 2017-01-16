#include <QtGui>
#include "wireless_dlg.h"
#include "helper.h"
#include "config.h"

extern lib_wl_t *g_wl;
extern lib_gui_t *g_gui;


//static gps_gga_info_backup_t gps_gga_backup;
#define GPS_INFO_BACKUP_PATH	"/opt/config/gps_info_backup.txt"

  
WirelessDlg::WirelessDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

	wireless_dial_stat = WL_DIAL_STAT_UNDIAL;
	
	__getPageConfig();

	pushButton_manualRefresh->installEventFilter(this);
	
	connect(pushButton_manualRefresh, SIGNAL(clicked()), this, SLOT(slots_manualRefresh()));
}

WirelessDlg::~WirelessDlg(void)
{
	
}

void WirelessDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_wirelessShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_wirelessShow->setText(tr("读取基本信息成功"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_TIMEOUT:
		{
			label_wirelessShow->setText(tr("读取基本信息超时"));	
		}
		break;

	}	
}

void WirelessDlg::slots_manualRefresh(void)
{
	__getPageConfig();
}

void WirelessDlg::__allWidgetClear(void)
{
	label_wlAccessPattern->clear();
	label_wlDialStat->clear();
	label_wlUimCardStat->clear();
	label_wlRssi->clear();
	label_wlFer->clear();
	label_wlRoamStatus->clear();
	label_wlSrvStatus->clear();
	label_wlSrvDomain->clear();
	label_wlSysMode->clear();
	label_wlUsingChannel->clear();
	label_wlUsingInterface->clear();
	label_wllLocalIpAddress->clear();
	label_wlRemoteIpAddress->clear();
	label_wlPrimarynsAddress->clear();
	label_wlSecondaryDnsAddress->clear();
	label_wlHwver->clear();
	label_wlCgmr->clear();
	label_wlLastDialTime->clear();
	label_wlLastDisconTime->clear();

	label_gpsLongitude->clear();
	label_gpsEw_indicator->clear();
	label_gpsLatitude->clear();
	label_gpsNs_indicator->clear();
	label_gpsTime->clear();
	label_gpsDate->clear();
	label_gpsStatus->clear();
	label_gpsSatellite->clear();
	
	label_wirelessShow->clear();
}

int WirelessDlg::__getPageConfig(void)
{
	wireless_dial_stat = 0;
	memset(&access_config, 0, sizeof(struct gui_ndev_access_pattern_config));
	memset(&wl_cgmr, 0, sizeof(struct lib_wl_cgmr));
	memset(&wl_hwver, 0, sizeof(struct lib_wl_hwver));
	memset(&wl_pppd_info, 0, sizeof(struct lib_wl_pppd_info));
	memset(&wl_csq_sysinfo, 0, sizeof(struct lib_wl_csq_sysinfo));
	memset(&wl_dial_time, 0, sizeof(struct lib_wl_dial_time));
	
	retval = lib_gui_get_ndev_access_pattern_config(g_gui, &access_config, 500);  //接入模式
	if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	if(access_config.using_wireless == 2) //使用有线
	{
		__setPageConfig();
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);
	}

	wireless_dial_stat = lib_wl_dial_stat_get(g_wl, 500);  //拨号状态

	retval = lib_wl_cgmr_get(g_wl, &wl_cgmr, 500);  //软件版本
	if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval = lib_wl_hwver_get(g_wl, &wl_hwver, 500);  //硬件版本
	if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);

	}
	
	retval = lib_wl_csq_sysinfo_get(g_wl, &wl_csq_sysinfo, 500);  //系统信息
	if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval = lib_wl_pppd_info_get(g_wl, &wl_pppd_info, 500);  //PPPD信息
	if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval = lib_wl_dial_time_get(g_wl, &wl_dial_time, 500);
	if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

    	memset(&gps_info, 0, sizeof(struct gui_gps_info));
    	retval = lib_gui_get_gps_info(g_gui, &gps_info, 500);
   	if(retval == LIB_GUI_ETIMEOUT) //读取超时
    	{
        	__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);

        	return retval;
    	}
	
	__setPageConfig();
	__showConfigResult(DEV_R_CONFIG_RESULT_OK);
	
	return retval;
}

void WirelessDlg::__setPageConfig(void)
{
	QString string;

	__allWidgetClear();

	if(access_config.using_wireless == 2) //使用有线
	{
		wireless_dial_stat = WL_DIAL_STAT_UNDIAL;
		wl_csq_sysinfo.sim_state = 255;
		wl_csq_sysinfo.rssi = 0;
		wl_csq_sysinfo.fer = 0;
		wl_csq_sysinfo.roam_status = 0;
		wl_csq_sysinfo.srv_status = 0;
		wl_csq_sysinfo.srv_domain = 0;
		wl_csq_sysinfo.sys_mode = 0;
		
		wl_pppd_info.using_channel = 0;
		strcpy(wl_pppd_info.using_interface, "no");
		strcpy(wl_pppd_info.local_ip_address, "0.0.0.0");
		strcpy(wl_pppd_info.remote_ip_address, "0.0.0.0");
		strcpy(wl_pppd_info.primary_dns_address, "0.0.0.0");
		strcpy(wl_pppd_info.secondary_dns_address, "0.0.0.0");

		strcpy(wl_cgmr.softversion, "no");
		strcpy(wl_hwver.firmversion, "no");

		wl_dial_time.last_dial_time = 0;
		wl_dial_time.last_discon_time = 0;
	}
	
	/* 接入方式 */
	if(access_config.using_wireless == 1)
		label_wlAccessPattern->setText(tr("无线"));
	else if(access_config.using_wireless == 2)
		label_wlAccessPattern->setText(tr("有线"));

	/* 拨号状态 */
	if(wireless_dial_stat == WL_DIAL_STAT_UNDIAL)
		label_wlDialStat->setText(tr("未拨号"));
	else if(wireless_dial_stat == WL_DIAL_STAT_DIALING)
		label_wlDialStat->setText(tr("拨号中"));
	else if(wireless_dial_stat == WL_DIAL_STAT_DIALOK)
		label_wlDialStat->setText(tr("已拨号"));

	/* UIM卡状态 */
	if(wl_csq_sysinfo.sim_state == 1)
		label_wlUimCardStat->setText(tr("有效"));
	else if((wl_csq_sysinfo.sim_state == 255) || (wl_csq_sysinfo.sim_state == 0))
		label_wlUimCardStat->setText(tr("无效"));
	else if(wl_csq_sysinfo.sim_state == 99)
		label_wlUimCardStat->setText(tr("欠费"));

	/* 信号强度 */
	Helper::__UshortToQString(wl_csq_sysinfo.rssi, string);
	label_wlRssi->setText(string);	

	/* 数据帧出错比例 */
	Helper::__UshortToQString(wl_csq_sysinfo.fer, string);
	label_wlFer->setText(string);	
	
	/* 漫游状态 */
	if(wl_csq_sysinfo.roam_status == 0)
		label_wlRoamStatus->setText(tr("非漫游状态"));
	else if(wl_csq_sysinfo.roam_status == 1)
		label_wlRoamStatus->setText(tr("漫游状态"));

	/* 系统服务状态 */
	switch(wl_csq_sysinfo.srv_status)
	{
		case 0:
		{
			label_wlSrvStatus->setText(tr("无服务"));
		}
		break;

		case 1:
		{
			label_wlSrvStatus->setText(tr("有限制服务"));
		}
		break;

		case 2:
		{
			label_wlSrvStatus->setText(tr("服务有效"));
		}
		break;

		case 3:
		{
			label_wlSrvStatus->setText(tr("有限制的区域服务"));
		}
		break;

		case 4:
		{
			label_wlSrvStatus->setText(tr("省电和深睡状态"));
		}
		break;
	}

	/* 系统服务域 */
	switch(wl_csq_sysinfo.srv_domain)
	{
		case 0:
		{
			label_wlSrvDomain->setText(tr("无服务"));
		}
		break;

		case 1:
		{
			label_wlSrvDomain->setText(tr("仅CS服务"));
		}
		break;

		case 2:
		{
			label_wlSrvDomain->setText(tr("仅PS服务"));
		}
		break;

		case 3:
		{
			label_wlSrvDomain->setText(tr("PS+CS服务"));
		}
		break;

		case 4:
		{
			label_wlSrvDomain->setText(tr("CS, PS均未注册,处于搜索状态"));
		}
		break;		

		case 255:
		{
			label_wlSrvDomain->setText(tr("CDMA不支持"));
		}
		break;
	}


	/* 系统模式 */
	switch(wl_csq_sysinfo.sys_mode)
	{
		case 0:
		{
			label_wlSysMode->setText(tr("无服务"));
		}
		break;

		case 2:
		{
			label_wlSysMode->setText(tr("CDMA模式"));
		}
		break;

		case 3:  //4G
		{
			label_wlSysMode->setText(tr("GSM/GPRS"));
		}
		break;
		
		case 4:
		{
			label_wlSysMode->setText(tr("HDR模式"));
		}
		break;

		case 5: //4G
		{
			label_wlSysMode->setText(tr("WCDMA"));
		}
		break;
  
		case 8:
		{
			label_wlSysMode->setText(tr("CDMA/HDR HYBRID"));
		}
		break;

		case 9: //4G
		{
			label_wlSysMode->setText(tr("LTE"));
		}
		break;
	}


	/* 使用通道 */
	Helper::__UshortToQString(wl_pppd_info.using_channel, string);
	label_wlUsingChannel->setText(string);

	/* 使用接口 */
	Helper::__CharToQString(wl_pppd_info.using_interface, string);
	label_wlUsingInterface->setText(string);

	/* 本地地址 */
	Helper::__CharToQString(wl_pppd_info.local_ip_address, string);
	label_wllLocalIpAddress->setText(string);	

	/* 远程地址 */
	Helper::__CharToQString(wl_pppd_info.remote_ip_address, string);
	label_wlRemoteIpAddress->setText(string);	

	/* 主域名 */
	Helper::__CharToQString(wl_pppd_info.primary_dns_address, string);
	label_wlPrimarynsAddress->setText(string);	

	/* 次域名 */
	Helper::__CharToQString(wl_pppd_info.secondary_dns_address, string);
	label_wlSecondaryDnsAddress->setText(string);		

	/* 软件版本 */
	Helper::__CharToQString(wl_cgmr.softversion, string);
	label_wlCgmr->setText(string);	

	/* 硬件版本 */
	Helper::__CharToQString(wl_hwver.firmversion, string);
	label_wlHwver->setText(string);	

	/* 拨号时间 */
	QDateTime wlLastDialDateTime = QDateTime::fromTime_t(wl_dial_time.last_dial_time);
	QDateTime wlLasDisconDateTime = QDateTime::fromTime_t(wl_dial_time.last_discon_time);

	string = wlLastDialDateTime.toString("yyyy-MM-dd hh:mm");
	label_wlLastDialTime->setText(string);

	string = wlLasDisconDateTime.toString("yyyy-MM-dd hh:mm");
	label_wlLastDisconTime->setText(string);

   	 if(gps_info.attr.gga == 1)
    	{
       		if(gps_info.gga.status == 0)
       		{
                        //label_gpsStatus->setText(tr("未定位"));
                        label_gpsStatus->setText(tr("搜索中")); //by zjc at 2016-10-14
       		}
       		else if(gps_info.gga.status==1)
          		 label_gpsStatus->setText(tr("非差分定位"));
      		 else if(gps_info.gga.status==2)
           		label_gpsStatus->setText(tr("差分定位"));
		
               #if 1
               int gps_deg1, gps_deg2, gps_min1, gps_min2;
               float gps_sec1, gps_sec2;
               QString gps_latitude_str, gps_longitude_str;
               int ret1, ret2;

               ret1 = Helper::gpsTransform(gps_info.gga.latitude, &gps_deg1, &gps_min1, &gps_sec1, gps_latitude_str);
               ret2 = Helper::gpsTransform(gps_info.gga.longitude, &gps_deg2, &gps_min2, &gps_sec2, gps_longitude_str);
               #endif

                Helper:: __DoubleToQStringV2(gps_info.gga.latitude, string);  //16-09-14 by zyp

                if(ret1 < 0)
                     label_gpsLatitude->setText(string);
                else
                      label_gpsLatitude->setText(gps_latitude_str);

                Helper:: __DoubleToQStringV2(gps_info.gga.longitude, string); //16-09-14 by zyp

                if(ret2 < 0)
                     label_gpsLongitude->setText(string);
                else
                    label_gpsLongitude->setText(gps_longitude_str);

       		if(gps_info.gga.ns_indicator=='N')
           		label_gpsNs_indicator->setText(tr("N(北半球)"));
       		else if(gps_info.gga.ns_indicator=='S')
            		label_gpsNs_indicator->setText(tr("S(南半球)"));

       		if(gps_info.gga.ew_indicator=='E')
           		label_gpsEw_indicator->setText(tr("E(东经)"));
       		if(gps_info.gga.ew_indicator=='W')
           		label_gpsEw_indicator->setText(tr("W(西经)"));

       		Helper:: __IntToQString(gps_info.gga.satellite, string);
      	 	label_gpsSatellite->setText(string);

		#if CONFS_USING_TEST_BY_ZJC
			/* 备份经纬度信息 add by zjc at 2016-10-18 */
			#if 0 //全局变量方法
			memset(&gps_gga_backup, 0, sizeof(gps_gga_info_backup_t));
			gps_gga_backup.latitude = gps_info.gga.latitude;
			gps_gga_backup.ns_indicator = gps_info.gga.ns_indicator;
			gps_gga_backup.longitude = gps_info.gga.longitude;
			gps_gga_backup.ew_indicator = gps_info.gga.ew_indicator;
			
			#else //保存到文本方法
			FILE *gps_fp = NULL;
			int ret = -1;
			
			gps_fp = fopen(GPS_INFO_BACKUP_PATH, "wb");
			if(NULL != gps_fp)
			{
				ret = fwrite(&gps_info.gga, sizeof(struct g_gps_gga), 1, gps_fp);
				if(ret < 1)
					fprintf(stderr, "write gps backup info failed!\n");
			}

			if(gps_fp != NULL)
			{
				fflush(gps_fp);
				fclose(gps_fp);
				gps_fp = NULL;
			}

			#endif
		#endif
   	 }
	else
	{
		label_gpsLongitude->clear();
		label_gpsEw_indicator->clear();  
		label_gpsLatitude->clear();
		label_gpsNs_indicator->clear();
		label_gpsStatus->clear();
		label_gpsSatellite->clear();

	#if CONFS_USING_TEST_BY_ZJC
		/* 读取备份经纬度信息 add by zjc at 2016-10-18 */
		#if 1
		FILE *gps_fp = NULL;
		int ret = -1, gps_flag = 0; //gps_flag:读取标志 0:读取失败，1:读取成功
		gps_gga_info_backup_t gps_gga_backup;

		gps_fp = fopen(GPS_INFO_BACKUP_PATH, "rb");
		if(NULL != gps_fp)
		{
			ret = fread(&gps_gga_backup, sizeof(gps_gga_info_backup_t), 1, gps_fp);
			if(ret < 1) //不是字节数!
			{
				gps_flag = 0;
				fprintf(stderr, "read gps backup info failed!\n");
			}
			else
				gps_flag = 1;
		}

		if(gps_fp != NULL)
		{
			//fflush(gps_fp);
			fclose(gps_fp);
			gps_fp = NULL;
		}
		#endif

		if(gps_flag == 1) 
		{
                    #if 1 //度分秒格式
                    int gps_deg1, gps_deg2, gps_min1, gps_min2;
                    float gps_sec1, gps_sec2;
                    QString gps_latitude_str, gps_longitude_str;
                    int ret1, ret2;

                    ret1 = Helper::gpsTransform(gps_gga_backup.latitude, &gps_deg1, &gps_min1, &gps_sec1, gps_latitude_str);
                    ret2 = Helper::gpsTransform(gps_gga_backup.longitude, &gps_deg2, &gps_min2, &gps_sec2, gps_longitude_str);
                    if(ret1 == 0)
                         label_gpsLatitude->setText(gps_latitude_str);

                    if(ret2 == 0)
                         label_gpsLongitude->setText(gps_longitude_str);

                    #else
                    Helper:: __DoubleToQStringV2(gps_gga_backup.latitude, string);
                    label_gpsLatitude->setText(string);

                    Helper:: __DoubleToQStringV2(gps_gga_backup.longitude, string);
                    label_gpsLongitude->setText(string);
                    #endif

                    if(gps_gga_backup.ns_indicator=='N')
                    label_gpsNs_indicator->setText(tr("N(北半球)"));
                    else if(gps_gga_backup.ns_indicator=='S')
                            label_gpsNs_indicator->setText(tr("S(南半球)"));

                    if(gps_gga_backup.ew_indicator=='E')
                    label_gpsEw_indicator->setText(tr("E(东经)"));
                    else if(gps_gga_backup.ew_indicator=='W')
                    label_gpsEw_indicator->setText(tr("W(西经)"));
		}
		
	#endif  
	}

        if(gps_info.gga.status == 0)
        {
            label_gpsStatus->setText(tr("搜索中")); //by zjc at 2016-10-14
        }

	
    	if(gps_info.attr.tm == 1)
    	{
		string.sprintf("%02d:%02d:%02d", gps_info.tm.tm_hour, gps_info.tm.tm_min, gps_info.tm.tm_sec);
		label_gpsTime->setText(string);

		string.sprintf("%d-%02d-%02d", gps_info.tm.tm_year, gps_info.tm.tm_mon, gps_info.tm.tm_mday);
		label_gpsDate->setText(string);
    	}
	else
	{
		label_gpsTime->clear();
		label_gpsDate->clear();
	}
}


bool WirelessDlg::eventFilter(QObject *watched, QEvent *event)
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




