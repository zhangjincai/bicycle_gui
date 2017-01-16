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
			label_wirelessShow->setText(tr("��ȡ������Ϣ�ɹ�"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_TIMEOUT:
		{
			label_wirelessShow->setText(tr("��ȡ������Ϣ��ʱ"));	
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
	
	retval = lib_gui_get_ndev_access_pattern_config(g_gui, &access_config, 500);  //����ģʽ
	if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	if(access_config.using_wireless == 2) //ʹ������
	{
		__setPageConfig();
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);
	}

	wireless_dial_stat = lib_wl_dial_stat_get(g_wl, 500);  //����״̬

	retval = lib_wl_cgmr_get(g_wl, &wl_cgmr, 500);  //����汾
	if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval = lib_wl_hwver_get(g_wl, &wl_hwver, 500);  //Ӳ���汾
	if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);

	}
	
	retval = lib_wl_csq_sysinfo_get(g_wl, &wl_csq_sysinfo, 500);  //ϵͳ��Ϣ
	if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval = lib_wl_pppd_info_get(g_wl, &wl_pppd_info, 500);  //PPPD��Ϣ
	if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval = lib_wl_dial_time_get(g_wl, &wl_dial_time, 500);
	if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

    	memset(&gps_info, 0, sizeof(struct gui_gps_info));
    	retval = lib_gui_get_gps_info(g_gui, &gps_info, 500);
   	if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
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

	if(access_config.using_wireless == 2) //ʹ������
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
	
	/* ���뷽ʽ */
	if(access_config.using_wireless == 1)
		label_wlAccessPattern->setText(tr("����"));
	else if(access_config.using_wireless == 2)
		label_wlAccessPattern->setText(tr("����"));

	/* ����״̬ */
	if(wireless_dial_stat == WL_DIAL_STAT_UNDIAL)
		label_wlDialStat->setText(tr("δ����"));
	else if(wireless_dial_stat == WL_DIAL_STAT_DIALING)
		label_wlDialStat->setText(tr("������"));
	else if(wireless_dial_stat == WL_DIAL_STAT_DIALOK)
		label_wlDialStat->setText(tr("�Ѳ���"));

	/* UIM��״̬ */
	if(wl_csq_sysinfo.sim_state == 1)
		label_wlUimCardStat->setText(tr("��Ч"));
	else if((wl_csq_sysinfo.sim_state == 255) || (wl_csq_sysinfo.sim_state == 0))
		label_wlUimCardStat->setText(tr("��Ч"));
	else if(wl_csq_sysinfo.sim_state == 99)
		label_wlUimCardStat->setText(tr("Ƿ��"));

	/* �ź�ǿ�� */
	Helper::__UshortToQString(wl_csq_sysinfo.rssi, string);
	label_wlRssi->setText(string);	

	/* ����֡������� */
	Helper::__UshortToQString(wl_csq_sysinfo.fer, string);
	label_wlFer->setText(string);	
	
	/* ����״̬ */
	if(wl_csq_sysinfo.roam_status == 0)
		label_wlRoamStatus->setText(tr("������״̬"));
	else if(wl_csq_sysinfo.roam_status == 1)
		label_wlRoamStatus->setText(tr("����״̬"));

	/* ϵͳ����״̬ */
	switch(wl_csq_sysinfo.srv_status)
	{
		case 0:
		{
			label_wlSrvStatus->setText(tr("�޷���"));
		}
		break;

		case 1:
		{
			label_wlSrvStatus->setText(tr("�����Ʒ���"));
		}
		break;

		case 2:
		{
			label_wlSrvStatus->setText(tr("������Ч"));
		}
		break;

		case 3:
		{
			label_wlSrvStatus->setText(tr("�����Ƶ��������"));
		}
		break;

		case 4:
		{
			label_wlSrvStatus->setText(tr("ʡ�����˯״̬"));
		}
		break;
	}

	/* ϵͳ������ */
	switch(wl_csq_sysinfo.srv_domain)
	{
		case 0:
		{
			label_wlSrvDomain->setText(tr("�޷���"));
		}
		break;

		case 1:
		{
			label_wlSrvDomain->setText(tr("��CS����"));
		}
		break;

		case 2:
		{
			label_wlSrvDomain->setText(tr("��PS����"));
		}
		break;

		case 3:
		{
			label_wlSrvDomain->setText(tr("PS+CS����"));
		}
		break;

		case 4:
		{
			label_wlSrvDomain->setText(tr("CS, PS��δע��,��������״̬"));
		}
		break;		

		case 255:
		{
			label_wlSrvDomain->setText(tr("CDMA��֧��"));
		}
		break;
	}


	/* ϵͳģʽ */
	switch(wl_csq_sysinfo.sys_mode)
	{
		case 0:
		{
			label_wlSysMode->setText(tr("�޷���"));
		}
		break;

		case 2:
		{
			label_wlSysMode->setText(tr("CDMAģʽ"));
		}
		break;

		case 3:  //4G
		{
			label_wlSysMode->setText(tr("GSM/GPRS"));
		}
		break;
		
		case 4:
		{
			label_wlSysMode->setText(tr("HDRģʽ"));
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


	/* ʹ��ͨ�� */
	Helper::__UshortToQString(wl_pppd_info.using_channel, string);
	label_wlUsingChannel->setText(string);

	/* ʹ�ýӿ� */
	Helper::__CharToQString(wl_pppd_info.using_interface, string);
	label_wlUsingInterface->setText(string);

	/* ���ص�ַ */
	Helper::__CharToQString(wl_pppd_info.local_ip_address, string);
	label_wllLocalIpAddress->setText(string);	

	/* Զ�̵�ַ */
	Helper::__CharToQString(wl_pppd_info.remote_ip_address, string);
	label_wlRemoteIpAddress->setText(string);	

	/* ������ */
	Helper::__CharToQString(wl_pppd_info.primary_dns_address, string);
	label_wlPrimarynsAddress->setText(string);	

	/* ������ */
	Helper::__CharToQString(wl_pppd_info.secondary_dns_address, string);
	label_wlSecondaryDnsAddress->setText(string);		

	/* ����汾 */
	Helper::__CharToQString(wl_cgmr.softversion, string);
	label_wlCgmr->setText(string);	

	/* Ӳ���汾 */
	Helper::__CharToQString(wl_hwver.firmversion, string);
	label_wlHwver->setText(string);	

	/* ����ʱ�� */
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
                        //label_gpsStatus->setText(tr("δ��λ"));
                        label_gpsStatus->setText(tr("������")); //by zjc at 2016-10-14
       		}
       		else if(gps_info.gga.status==1)
          		 label_gpsStatus->setText(tr("�ǲ�ֶ�λ"));
      		 else if(gps_info.gga.status==2)
           		label_gpsStatus->setText(tr("��ֶ�λ"));
		
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
           		label_gpsNs_indicator->setText(tr("N(������)"));
       		else if(gps_info.gga.ns_indicator=='S')
            		label_gpsNs_indicator->setText(tr("S(�ϰ���)"));

       		if(gps_info.gga.ew_indicator=='E')
           		label_gpsEw_indicator->setText(tr("E(����)"));
       		if(gps_info.gga.ew_indicator=='W')
           		label_gpsEw_indicator->setText(tr("W(����)"));

       		Helper:: __IntToQString(gps_info.gga.satellite, string);
      	 	label_gpsSatellite->setText(string);

		#if CONFS_USING_TEST_BY_ZJC
			/* ���ݾ�γ����Ϣ add by zjc at 2016-10-18 */
			#if 0 //ȫ�ֱ�������
			memset(&gps_gga_backup, 0, sizeof(gps_gga_info_backup_t));
			gps_gga_backup.latitude = gps_info.gga.latitude;
			gps_gga_backup.ns_indicator = gps_info.gga.ns_indicator;
			gps_gga_backup.longitude = gps_info.gga.longitude;
			gps_gga_backup.ew_indicator = gps_info.gga.ew_indicator;
			
			#else //���浽�ı�����
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
		/* ��ȡ���ݾ�γ����Ϣ add by zjc at 2016-10-18 */
		#if 1
		FILE *gps_fp = NULL;
		int ret = -1, gps_flag = 0; //gps_flag:��ȡ��־ 0:��ȡʧ�ܣ�1:��ȡ�ɹ�
		gps_gga_info_backup_t gps_gga_backup;

		gps_fp = fopen(GPS_INFO_BACKUP_PATH, "rb");
		if(NULL != gps_fp)
		{
			ret = fread(&gps_gga_backup, sizeof(gps_gga_info_backup_t), 1, gps_fp);
			if(ret < 1) //�����ֽ���!
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
                    #if 1 //�ȷ����ʽ
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
                    label_gpsNs_indicator->setText(tr("N(������)"));
                    else if(gps_gga_backup.ns_indicator=='S')
                            label_gpsNs_indicator->setText(tr("S(�ϰ���)"));

                    if(gps_gga_backup.ew_indicator=='E')
                    label_gpsEw_indicator->setText(tr("E(����)"));
                    else if(gps_gga_backup.ew_indicator=='W')
                    label_gpsEw_indicator->setText(tr("W(����)"));
		}
		
	#endif  
	}

        if(gps_info.gga.status == 0)
        {
            label_gpsStatus->setText(tr("������")); //by zjc at 2016-10-14
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




