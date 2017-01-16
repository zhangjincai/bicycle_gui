#include <QtGui>
#include "basic_info_dlg.h"
#include "helper.h"
#include "lib_lnt.h"
#include "ndev_config_dlg.h"

extern int up_thr_runing; //读卡器升级状态 1为正在升级

/* 基本信息刷新时间  单位:秒 */
#define BINFO_REFRESH_PERIOD				(20)
#define BINFO_TIMER_SLEEP					(2)

extern lib_gui_t *g_gui;
extern lib_wl_t *g_wl;

BasicInfoDlg::BasicInfoDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

	refreshFlag = 1;
	unityFilePage = 1;
	
	memset(&bUnity_file_info, 0, sizeof(bUnity_file_info));
	
	__getPageConfig();

	pushButton_manualRefresh->installEventFilter(this);
	
	connect(pushButton_manualRefresh, SIGNAL(clicked()), this, SLOT(slots_manualRefresh()));

	QString string;
	timerCount = BINFO_REFRESH_PERIOD;
	string.setNum(timerCount);
	label_refreshPeriod->setText(string);
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(BINFO_TIMER_SLEEP * 1000);  //定时2秒
}

BasicInfoDlg::~BasicInfoDlg(void)
{
	
}

void BasicInfoDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_basicInfoShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_basicInfoShow->setText(tr("读取【基本信息】成功"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_TIMEOUT:
		{
			label_basicInfoShow->setText(tr("读取【基本信息】超时"));	
		}
		break;

		case DEV_W_CONFIG_RESULT_REFRESH:
		{
			label_basicInfoShow->setText(tr("刷新【基本信息】成功"));	
		}
		break;
		
	}	
}

void BasicInfoDlg::slots_manualRefresh(void)
{
	QString string;

	refreshFlag = 0;
	
	timerCount = BINFO_REFRESH_PERIOD;
	label_refreshPeriod->setText(string);

	unityFilePage++;
	if(unityFilePage > 3)
		unityFilePage = 1;

	if(__getPageConfig() > 0)
		__showConfigResult(DEV_W_CONFIG_RESULT_REFRESH);  //刷新成功

	refreshFlag = 1;

	
	
}

void BasicInfoDlg::slots_timerDone(void)
{
	QString string;

	if(refreshFlag == 1) //手动刷新时候,不执行自动刷新
	{
		string.setNum(timerCount);
		label_refreshPeriod->setText(string);
		timerCount = timerCount - BINFO_TIMER_SLEEP;
		if(timerCount == 0)
			slots_manualRefresh();

		if(timerCount <= 0)
			timerCount = BINFO_REFRESH_PERIOD;

		if(timerCount == (BINFO_REFRESH_PERIOD / 2))
			label_basicInfoShow->clear(); //清空信息显示
	}
}

void BasicInfoDlg::__allWidgetClear(void)
{
	label_ndevFwVer->clear();
	label_saeFwVer->clear();
	label_lntFwVer->clear();

	label_terminalName->clear();
	label_terminalNo->clear();
	label_registerTime->clear();
	label_connIpaddr->clear();
	label_connPort->clear();

	label_dialStat->clear();
	label_systemMode->clear();
	label_uimCardStat->clear();
	label_rssi->clear();
	label_fer->clear();

	label_centerConnStat->clear();
	label_lntRegStat->clear();


	label_totalBlacklistVer->clear();
	label_incBlacklistVer->clear();
	label_decBlacklistVer->clear();
	label_tempBlacklistVer->clear();

	label_nPowerStat->clear();
	label_nRtcStat->clear();
	label_nCanStat->clear();
	label_nCenterStat->clear();
	label_nWirelessStat->clear();
	label_nYctStat->clear();
	label_nGpsStat->clear();
	label_nKeyboardStat->clear();
	label_nExtMcuStat->clear();
	label_nTimeStat->clear();

	label_totalFileSeq->clear();
	label_incFileSeq->clear();
	label_decFileSeq->clear();
	label_tempFileSeq->clear();
	label_stakeFileSeq->clear();
	label_paraFileSeq->clear();

	label_totalDnl->clear();
	label_incDnl->clear();
	label_decDnl->clear();
	label_tempDnl->clear();
	label_stakeDnl->clear();
	label_paraDnl->clear();

	label_totalStat->clear();
	label_incStat->clear();
	label_decStat->clear();
	label_tempStat->clear();
	label_stakeStat->clear();
	label_paraStat->clear();

	label_totalStartTime->clear();
	label_incStartTime->clear();
	label_decStartTime->clear();
	label_tempStartTime->clear();
	label_stakeStartTime->clear();
	label_paraStartTime->clear();

	label_totalEndTime->clear();
	label_incEndTime->clear();
	label_decEndTime->clear();
	label_tempEndTime->clear();
	label_stakeEndTime->clear();
	label_paraEndTime->clear();
	
	

	label_basicInfoShow->clear();
}

int BasicInfoDlg::__getPageConfig(void)
{
	memset(&bPageConfig, 0, sizeof(struct gui_basic_info_page_config));
	retval = lib_gui_get_basic_page_config(g_gui, &bPageConfig, 1500);  //获取节点机配置数据  3000->1300
	if(retval == LIB_GUI_EOK)
	{
		__setPageConfig();
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);

	}
	else if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	retval =  lib_gui_unity_file_info_get(g_gui, &bUnity_file_info, 500);
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

void BasicInfoDlg::__setPageConfig(void)
{
	QString string;
	QDateTime startTime, endTime;
        int ret = -1;
		
	__allWidgetClear();

	/* 岭南通状态 */
	enum LNT_REGISTER_STAT lnt_reg_stat;
	lnt_reg_stat = lib_lnt_register_stat_get();
	 switch(lnt_reg_stat)
	 {
	 	case LNT_REGISTER_STAT_INIT:
		{
			label_lntRegStat->setStyleSheet("color:red;"); 
			label_lntRegStat->setText(tr("初始化"));
	 	}
		break;

		case LNT_REGISTER_STAT_ING:
		{
			label_lntRegStat->setStyleSheet("color:red;"); 
			label_lntRegStat->setText(tr("正在签到"));
	 	}
		break;

		case LNT_REGISTER_STAT_OK:
		{
			label_lntRegStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
			label_lntRegStat->setText(tr("签到成功"));
	 	}
		break;

		case LNT_REGISTER_STAT_RE:
		{
			label_lntRegStat->setStyleSheet("color:red;"); 
			label_lntRegStat->setText(tr("重新签到"));
	 	}
		break;
		
		case LNT_REGISTER_STAT_FAIL:
		{
			label_lntRegStat->setStyleSheet("color:red;"); 
			label_lntRegStat->setText(tr("签到失败"));
	 	}
		break;
	 }

	lib_lnt_register_proc_code_t lntRegProc;
	memset(&lntRegProc, 0, sizeof(lib_lnt_register_proc_code_t));

	lntRegProc = lib_lnt_register_proc_code_get();

	Helper::__IntToQString(lntRegProc.proc, string);
	label_lntProc->setText(string);

	Helper::__IntToQString(lntRegProc.fresult, string);
	label_lntFresult->setText(string);

	string.sprintf("%02X", lntRegProc.stat);
	label_lntStat->setText(string);

	string.clear();
	string.sprintf("%02X", lntRegProc.sw);
	label_lntSw->setText(string);
	

	/* 固件版本 */
	Helper::__CharToQString(bPageConfig.ndev_firmware_ver, string);
	label_ndevFwVer->setText(string);

	Helper::__CharToQString(bPageConfig.stake_firmware_ver, string);
	label_saeFwVer->setText(string);

	char version[32] = {0};
	unsigned char stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	lib_lnt_getversion_ack_t lnt_ver_ack;

	//printf("----------basic_info:up_thr_runing:%d\n",up_thr_runing);
#if 1
	#if CONFS_USING_READER_UPDATE//CONFS_USING_TEST_BY_ZJC
	if(up_thr_runing == 0) //读卡器升级期间不能操作读卡器!!!
		lib_lnt_get_version(&lnt_ver_ack, &stat, 200);
	#else
		lib_lnt_get_version(&lnt_ver_ack, &stat, 200);
	#endif
#endif
	//lib_lnt_get_version(&lnt_ver_ack, &stat, 200);
	if(stat == LNT_MOC_ERROR_NORMAL)
	{
		char *p = strchr((char *)lnt_ver_ack.version, ';');
		*p = '\0';
		strcpy(version, (char *)lnt_ver_ack.version);
		Helper::__CharToQString(version, string);
		label_lntFwVer->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_lntFwVer->setText(string);
		label_nYctStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nYctStat->setText(tr("正常"));
	}
	else
	{
		label_lntFwVer->setStyleSheet("color:red;"); 
		label_lntFwVer->setText(tr("00000000"));
		
		label_nYctStat->setStyleSheet("color:red;"); 
		label_nYctStat->setText(tr("异常"));
	}

	/* 终端信息 */
	QTextCodec *codec = QTextCodec::codecForLocale();
	QString s_gbk = codec->toUnicode(bPageConfig.terminal_name);  //显示中文,GBK
	label_terminalName->setText(s_gbk);

	#if 0
	Helper::__UshortToQString(bPageConfig.terminal_no, string);  //终端编号
	label_terminalNo->setText(string);
	#endif

	/* 终端编号 */
	string.sprintf("%05d-%X", bPageConfig.terminal_no, bPageConfig.terminal_no);
	label_terminalNo->setText(string);

	if(bPageConfig.register_time != 0)
	{
		QDateTime regDateTime = QDateTime::fromTime_t(bPageConfig.register_time);
		string = regDateTime.toString("yyyy-MM-dd hh:mm");
		label_registerTime->setText(string);
	}
	else
	{
		label_registerTime->setText(tr("0000-00-00 00:00"));
	}
	
	Helper::__CharToQString(bPageConfig.conn_ipaddr, string);  //中心连接地址
	label_connIpaddr->setText(string);

	Helper::__UshortToQString(bPageConfig.conn_port, string);  //连接端口
	label_connPort->setText(string);

	/* 最后一次上电时间 */
	QDateTime wlLastDialDateTime = QDateTime::fromTime_t(bPageConfig.last_time_power_on);
	string = wlLastDialDateTime.toString("yyyy-MM-dd hh:mm");
	label_lastPowerOn->setText(string);


	enum WL_DIAL_STAT dial_stat = lib_wl_dial_stat_get(g_wl, 500);  //拨号状态
	if(dial_stat == WL_DIAL_STAT_UNDIAL)
	{
		label_dialStat->setStyleSheet("color:red;"); 
		label_dialStat->setText(tr("未拨号"));
	}
	else if(dial_stat == WL_DIAL_STAT_DIALING)
	{
		label_dialStat->setStyleSheet("color:red;"); 
		label_dialStat->setText(tr("拨号中"));
	}
	else if(dial_stat == WL_DIAL_STAT_DIALOK)
	{
		label_dialStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_dialStat->setText(tr("已拨号"));
	}

	struct lib_wl_csq_sysinfo wl_csq_sysinfo;
	memset(&wl_csq_sysinfo, 0, sizeof(struct lib_wl_csq_sysinfo));
	
	if(lib_wl_csq_sysinfo_get(g_wl, &wl_csq_sysinfo, 500) == LIB_GUI_EOK)
	{
		switch(wl_csq_sysinfo.sys_mode)  //系统模式
		{
			case 0:
				label_systemMode->setStyleSheet("color:red;"); 
				label_systemMode->setText(tr("无服务"));
				break;

			case 2:
				label_systemMode->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
				label_systemMode->setText(tr("CDMA模式"));
				break;			

			case 4:
				label_systemMode->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
				label_systemMode->setText(tr("HDR模式"));
				break;

			case 8:
				label_systemMode->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
				label_systemMode->setText(tr("HYBRID"));
				break;

			case 9:
				label_systemMode->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
				label_systemMode->setText(tr("LTE"));
				break;
		}

		if(wl_csq_sysinfo.sim_state == 1)  //UIM卡状态
		{
			label_uimCardStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
			label_uimCardStat->setText(tr("有效"));
		}
		else if((wl_csq_sysinfo.sim_state == 255) ||(wl_csq_sysinfo.sim_state == 0))
		{
			label_uimCardStat->setStyleSheet("color:red;"); 
			label_uimCardStat->setText(tr("无效"));
		}

		Helper::__UshortToQString(wl_csq_sysinfo.rssi, string);  //信号强度
		label_rssi->setText(string);	

		Helper::__UshortToQString(wl_csq_sysinfo.fer, string);  //数据帧出错比例
		label_fer->setText(string);
	}

	/* 连接状态 */
	switch(bPageConfig.center_conn_stat)
	{
		case 0:
			label_centerConnStat->setStyleSheet("color:red;"); 
			label_centerConnStat->setText(tr("初始化"));
			break;
			
		case 1:
			label_centerConnStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
			label_centerConnStat->setText(tr("已连接"));
			break;

		case 2:
			label_centerConnStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
			label_centerConnStat->setText(tr("已注册"));
			break;

		case 3:
			label_centerConnStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
			label_centerConnStat->setText(tr("会话中"));
			break;
	}


	

	/* 泛文件版本 */
	Helper::__CharToQString(bPageConfig.total_blacklist_ver, string);
	label_totalBlacklistVer->setText(string);	

	Helper::__CharToQString(bPageConfig.inc_blacklist_ver, string);
	label_incBlacklistVer->setText(string);	

	Helper::__CharToQString(bPageConfig.dec_blacklist_ver, string);
	label_decBlacklistVer->setText(string);	

	Helper::__CharToQString(bPageConfig.temp_blacklist_ver, string);
	label_tempBlacklistVer->setText(string);	

	Helper::__CharToQString(bPageConfig.stake_para, string);
	label_stakeParaVer->setText(string);	

	/* 泛文件 */
	Helper::__UshortToQString(bPageConfig.use_total_seq, string);  
	label_totalBlacklistQuantity->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_inc_seq, string);  
	label_incBlacklistQuantity->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_dec_seq, string);  
	label_decBlacklistQuantity->setText(string);		
	
	Helper::__UshortToQString(bPageConfig.use_temp_seq, string);  
	label_tempBlacklistQuantity->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_fw_seq, string);  
	label_stakeFwQuantity->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_stake_para_seq, string);  
	label_stakeParaQuantity->setText(string);	

	////////////////////////////////////////////////////

	Helper::__UshortToQString(bPageConfig.use_total_db, string);  
	label_totalBlacklistDB->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_inc_db, string);  
	label_incBlacklistDB->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_dec_db, string);  
	label_decBlacklistDB->setText(string);		
	
	Helper::__UshortToQString(bPageConfig.use_temp_db, string);  
	label_tempBlacklistDB->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_fw_db, string);  
	label_stakeFwDB->setText(string);	

	Helper::__UshortToQString(bPageConfig.use_stake_para_db, string);  
	label_stakeParaDB->setText(string);	

	/* 节点机信息 */
	if(bPageConfig.nstatus.power == 1)  //电源电压
	{
		label_nPowerStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nPowerStat->setText(tr("正常"));
	}
	else 
	{
		label_nPowerStat->setStyleSheet("color:red;"); 
		label_nPowerStat->setText(tr("异常"));
	}

	if(bPageConfig.nstatus.rtc == 1)  //RTC状态
	{
		label_nRtcStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nRtcStat->setText(tr("正常"));
	}
	else 
	{
		label_nRtcStat->setStyleSheet("color:red;"); 
		label_nRtcStat->setText(tr("异常"));
	}

	if(bPageConfig.nstatus.can == 1)  //CAN总线状态
	{
		label_nCanStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nCanStat->setText(tr("正常"));
	}
	else 
	{
		label_nCanStat->setStyleSheet("color:red;"); 
		label_nCanStat->setText(tr("异常"));
	}

	if(bPageConfig.nstatus.center == 1)  //中心通讯状态
	{
		label_nCenterStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nCenterStat->setText(tr("正常"));
	}
	else 
	{
		label_nCenterStat->setStyleSheet("color:red;"); 
		label_nCenterStat->setText(tr("异常"));
	}

        /*2016-07-05 add by zjc*/
        struct gui_ndev_page_config nPageConfig;
        memset(&nPageConfig, 0, sizeof(struct gui_ndev_page_config));
        retval = lib_gui_get_ndev_page_config(g_gui, &nPageConfig, 2000);  //获取节点机配置数据

        //if(dial_stat == WL_DIAL_STAT_DIALOK)  //3G/4G网络状态
        if((nPageConfig.access_conf.using_wireless == 1) && (bPageConfig.nstatus.wireless == 1))  //2016-07-04 by zjc
        {
                label_nWirelessStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
                label_nWirelessStat->setText(tr("正常"));
        }
	else   
	{
		label_nWirelessStat->setStyleSheet("color:red;"); 
		label_nWirelessStat->setText(tr("异常"));
	}

	if(bPageConfig.nstatus.gps == 1)  //GPS状态
	{
		label_nGpsStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nGpsStat->setText(tr("正常"));
	}
	else 
	{
                //label_nGpsStat->setStyleSheet("color:red;");
                label_nGpsStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
                label_nGpsStat->setText(tr("搜索"));
	}

	if(bPageConfig.nstatus.keyboard == 1)  //键盘状态
	{
		label_nKeyboardStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nKeyboardStat->setText(tr("正常"));
	}
	else 
	{
		label_nKeyboardStat->setStyleSheet("color:red;"); 
		label_nKeyboardStat->setText(tr("异常"));
	}

	if(bPageConfig.nstatus.ext_mcu == 1)  //辅助CPU状态
	{
		label_nExtMcuStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nExtMcuStat->setText(tr("正常"));
	}
	else 
	{
		label_nExtMcuStat->setStyleSheet("color:red;"); 
		label_nExtMcuStat->setText(tr("异常"));
	}

	if(bPageConfig.nstatus.time== 1)  //时间同步状态
	{
		label_nTimeStat->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
		label_nTimeStat->setText(tr("正常"));
	}
	else 
	{
		label_nTimeStat->setStyleSheet("color:red;"); 
		label_nTimeStat->setText(tr("异常"));
	}

	/*
	 * 泛文件信息
	 */
	 int u_idx = 0;
	 switch(unityFilePage)
	 {
		case 1: //节点机下载泛文件
		{
			label_unityFileTitle->setText(tr("节点机下载泛文件"));

			//0
			u_idx = 0;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].file_seq));
			label_totalFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.ndev_ufile_st[u_idx].total));
			label_totalDnl->setText(string);

			if(bUnity_file_info.ndev_ufile_st[u_idx].status == 1)
				label_totalStat->setText(tr("正在下载"));
			else if(bUnity_file_info.ndev_ufile_st[u_idx].status == 2)
				label_totalStat->setText(tr("下载完成"));
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_totalStartTime->setText(string);
			}

			if(bUnity_file_info.ndev_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_totalEndTime->setText(string);
			}

			// 1
			u_idx = 1;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].file_seq));
			label_incFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.ndev_ufile_st[u_idx].total));
			label_incDnl->setText(string);

			if(bUnity_file_info.ndev_ufile_st[u_idx].status == 1)
				label_incStat->setText(tr("正在下载"));
			else if(bUnity_file_info.ndev_ufile_st[u_idx].status == 2)
				label_incStat->setText(tr("下载完成"));
				
			if(bUnity_file_info.ndev_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_incStartTime->setText(string);
			}
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_incEndTime->setText(string);
			}
				
			// 2
			u_idx = 2;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].file_seq));
			label_decFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.ndev_ufile_st[u_idx].total));
			label_decDnl->setText(string);

			if(bUnity_file_info.ndev_ufile_st[u_idx].status == 1)
				label_decStat->setText(tr("正在下载"));
			else if(bUnity_file_info.ndev_ufile_st[u_idx].status == 2)
				label_decStat->setText(tr("下载完成"));
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_decStartTime->setText(string);
			}
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_decEndTime->setText(string);
			}
				
			// 3
			u_idx = 3;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].file_seq));
			label_tempFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.ndev_ufile_st[u_idx].total));
			label_tempDnl->setText(string);

			if(bUnity_file_info.ndev_ufile_st[u_idx].status == 1)
				label_tempStat->setText(tr("正在下载"));
			else if(bUnity_file_info.ndev_ufile_st[u_idx].status == 2)
				label_tempStat->setText(tr("下载完成"));
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_tempStartTime->setText(string);
			}

			if(bUnity_file_info.ndev_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_tempEndTime->setText(string);
			}
				
			// 4
			u_idx = 4;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].file_seq));
			label_stakeFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.ndev_ufile_st[u_idx].total));
			label_stakeDnl->setText(string);

			if(bUnity_file_info.ndev_ufile_st[u_idx].status == 1)
				label_stakeStat->setText(tr("正在下载"));
			else if(bUnity_file_info.ndev_ufile_st[u_idx].status == 2)
				label_stakeStat->setText(tr("下载完成"));
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_stakeStartTime->setText(string);
			}
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_stakeEndTime->setText(string);
			}

			// 5
			u_idx = 5;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].file_seq));
			label_paraFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.ndev_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.ndev_ufile_st[u_idx].total));
			label_paraDnl->setText(string);

			if(bUnity_file_info.ndev_ufile_st[u_idx].status == 1)
				label_paraStat->setText(tr("正在下载"));
			else if(bUnity_file_info.ndev_ufile_st[u_idx].status == 2)
				label_paraStat->setText(tr("下载完成"));
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_paraStartTime->setText(string);
			}
			
			if(bUnity_file_info.ndev_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.ndev_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_paraEndTime->setText(string);
			}
		}
		break;

		case 2: //节点机广播泛文件
		{
			label_unityFileTitle->setText(tr("节点机广播泛文件"));

			
			//0
			u_idx = 0;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].file_seq));
			label_totalFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].total));
			label_totalDnl->setText(string);

			if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 1)
				label_totalStat->setText(tr("正在广播"));
			else if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 2)
				label_totalStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_totalStartTime->setText(string);
			}

			if(bUnity_file_info.broadcast_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_totalEndTime->setText(string);
			}

			// 1
			u_idx = 1;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].file_seq));
			label_incFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].total));
			label_incDnl->setText(string);

			if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 1)
				label_incStat->setText(tr("正在广播"));
			else if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 2)
				label_incStat->setText(tr("广播完成"));
				
			if(bUnity_file_info.broadcast_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_incStartTime->setText(string);
			}
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_incEndTime->setText(string);
			}

			// 2
			u_idx = 2;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].file_seq));
			label_decFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].total));
			label_decDnl->setText(string);

			if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 1)
				label_decStat->setText(tr("正在广播"));
			else if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 2)
				label_decStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_decStartTime->setText(string);
			}
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_decEndTime->setText(string);
			}
				
			// 3
			u_idx = 3;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].file_seq));
			label_tempFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].total));
			label_tempDnl->setText(string);

			if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 1)
				label_tempStat->setText(tr("正在广播"));
			else if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 2)
				label_tempStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_tempStartTime->setText(string);
			}

			if(bUnity_file_info.broadcast_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_tempEndTime->setText(string);
			}
				
			// 4
			u_idx = 4;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].file_seq));
			label_stakeFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].total));
			label_stakeDnl->setText(string);

			if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 1)
				label_stakeStat->setText(tr("正在广播"));
			else if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 2)
				label_stakeStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_stakeStartTime->setText(string);
			}
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_stakeEndTime->setText(string);
			}

			// 5
			u_idx = 5;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].file_seq));
			label_paraFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.broadcast_ufile_st[u_idx].total));
			label_paraDnl->setText(string);

			if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 1)
				label_paraStat->setText(tr("正在广播"));
			else if(bUnity_file_info.broadcast_ufile_st[u_idx].status == 2)
				label_paraStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_paraStartTime->setText(string);
			}
			
			if(bUnity_file_info.broadcast_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.broadcast_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_paraEndTime->setText(string);
			}	
		}
		break;

		case 3: //锁桩申请广播泛文件
		{
			label_unityFileTitle->setText(tr("锁桩申请广播泛文件"));
	
			//0
			u_idx = 0;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].file_seq));
			label_totalFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.stake_ufile_st[u_idx].total));
			label_totalDnl->setText(string);

			if(bUnity_file_info.stake_ufile_st[u_idx].status == 1)
				label_totalStat->setText(tr("正在广播"));
			else if(bUnity_file_info.stake_ufile_st[u_idx].status == 2)
				label_totalStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.stake_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_totalStartTime->setText(string);
			}

			if(bUnity_file_info.stake_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_totalEndTime->setText(string);
			}

			// 1
			u_idx = 1;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].file_seq));
			label_incFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.stake_ufile_st[u_idx].total));
			label_incDnl->setText(string);

			if(bUnity_file_info.stake_ufile_st[u_idx].status == 1)
				label_incStat->setText(tr("正在广播"));
			else if(bUnity_file_info.stake_ufile_st[u_idx].status == 2)
				label_incStat->setText(tr("广播完成"));
				
			if(bUnity_file_info.stake_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_incStartTime->setText(string);
			}
			
			if(bUnity_file_info.stake_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_incEndTime->setText(string);
			}

			// 2
			u_idx = 2;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].file_seq));
			label_decFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.stake_ufile_st[u_idx].total));
			label_decDnl->setText(string);

			if(bUnity_file_info.stake_ufile_st[u_idx].status == 1)
				label_decStat->setText(tr("正在广播"));
			else if(bUnity_file_info.stake_ufile_st[u_idx].status == 2)
				label_decStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.stake_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_decStartTime->setText(string);
			}
			
			if(bUnity_file_info.stake_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_decEndTime->setText(string);
			}
				
			// 3
			u_idx = 3;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].file_seq));
			label_tempFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.stake_ufile_st[u_idx].total));
			label_tempDnl->setText(string);

			if(bUnity_file_info.stake_ufile_st[u_idx].status == 1)
				label_tempStat->setText(tr("正在广播"));
			else if(bUnity_file_info.stake_ufile_st[u_idx].status == 2)
				label_tempStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.stake_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_tempStartTime->setText(string);
			}

			if(bUnity_file_info.stake_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_tempEndTime->setText(string);
			}
				
			// 4
			u_idx = 4;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].file_seq));
			label_stakeFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.stake_ufile_st[u_idx].total));
			label_stakeDnl->setText(string);

			if(bUnity_file_info.stake_ufile_st[u_idx].status == 1)
				label_stakeStat->setText(tr("正在广播"));
			else if(bUnity_file_info.stake_ufile_st[u_idx].status == 2)
				label_stakeStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.stake_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_stakeStartTime->setText(string);
			}
			
			if(bUnity_file_info.stake_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_stakeEndTime->setText(string);
			}

			// 5
			u_idx = 5;
			string.clear();
			string.sprintf("%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].file_seq));
			label_paraFileSeq->setText(string);

			string.clear();
			string.sprintf("%d/%d", ntohs(bUnity_file_info.stake_ufile_st[u_idx].div_seq), ntohs(bUnity_file_info.stake_ufile_st[u_idx].total));
			label_paraDnl->setText(string);

			if(bUnity_file_info.stake_ufile_st[u_idx].status == 1)
				label_paraStat->setText(tr("正在广播"));
			else if(bUnity_file_info.stake_ufile_st[u_idx].status == 2)
				label_paraStat->setText(tr("广播完成"));
			
			if(bUnity_file_info.stake_ufile_st[u_idx].start_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].start_time);
				string = startTime.toString("MM-dd hh:mm");
				label_paraStartTime->setText(string);
			}
			
			if(bUnity_file_info.stake_ufile_st[u_idx].end_time != 0)
			{
				string.clear();
				startTime = QDateTime::fromTime_t(bUnity_file_info.stake_ufile_st[u_idx].end_time);
				string = startTime.toString("MM-dd hh:mm");
				label_paraEndTime->setText(string);
			}	
		}
		break;
	 }

	
}

bool BasicInfoDlg::eventFilter(QObject *watched, QEvent *event)
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



