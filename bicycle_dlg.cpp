#include <QtGui>
#include "bicycle_dlg.h"
#include "helper.h"
#include "lib_lnt.h"
#include "lib_serialnumber.h"
#include "lib_general.h"


#include "qrcode.h"
#include "myqrencode.h"
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <string.h>

#include "config.h"


#define NDEV_INFO_STAGE1_PERIOD			(8)
#define NDEV_INFO_STAGE2_PERIOD			(4)	
#define WARN_WAIT_TIME				(3)
#define NDEV_LNT_PERIOD				(15)

      
extern lib_gui_t *g_gui;


#define USING_ADMIN             1 //管理卡限制
#define USING_ON_OFF_CARD       1 //开销卡功能
#define USING_GIF_SHOW          1 //节点机动画

#define USING_QR_CODE           1 //二维码显示
#define USING_QR_RENT_BIKE      1 //网点租车二维码
 
     
#if 1
#define GIF_PATH_NAME			"image/nDemo.gif"  //目标板路径，烧写用
#else
#define GIF_PATH_NAME			"/mnt/mount/nDemo.gif" //挂载路径，挂载用
#endif  
    
#define GIF_TIME_COUNT			20     
  
int g_adminCardCtrlSet;  
        
#if CONFS_USING_READER_UPDATE
/* --------------------岭南通读卡器升级用-------------------- */
static void *lnt_update_thread(void *arg);
lnt_firmware_update_result_t update_result; //岭南通读卡器升级结果
int upe_mid;
int volatile up_thr_runing = 0; 
/* --------------------岭南通读卡器升级用-------------------- */
#endif  
    
static void  __lnt_firmware_version_put(char version[24]);
//static int  __lnt_firmware_version_fgets(char version[24]);


#define	NET_INFO_BACKUP_PATH	"/opt/logpath/net_info.txt" //备份网卡信息
pthread_mutex_t g_net_mutex = PTHREAD_MUTEX_INITIALIZER;

      
BicycleDlg::BicycleDlg(QWidget *parent) : QDialog(parent)
{  
	setupUi(this);        
	setWindowFlags(Qt::FramelessWindowHint);	  
   
        gifTimeCount = 0;  
	gif_current_frame = 0;     
	lntTimeCount = 0;  
	netCount = 0;
	net_info_read_flag = 0;
	timerCount = NDEV_INFO_STAGE1_PERIOD;

	lntAdminCardType = LNT_ADMIN_CARD_TYPE_NOT;  
	g_adminCardCtrlSet = 0;   
		   
	__allWidgetClear();   
  
        //fprintf(stderr, "BicycleDlg RUN\n");

        label_gifShow->setVisible(false);

    #if USING_GIF_SHOW  
        p_movie = new QMovie(GIF_PATH_NAME);
        label_gifShow->setVisible(true);
        label_gifShow->setMovie(p_movie);  
        p_movie->start();   
    #endif  

        label_QR_code->setVisible(false);

    #if 0
        label_weixinAPP->setGeometry(831,310,140,140);
        label_3->setGeometry(841,454,121,20);
    #endif

#if USING_QR_RENT_BIKE  //测试租车二维码
        label_QR_code->setVisible(true);
  
        QString QrString;

        memcpy(&site_QR_code, "http://bk.ucpiao.com/q?348C8556C3892DE6", strlen("http://bk.ucpiao.com/q?348C8556C3892DE6"));	//二维码数据初始化
        //printf("\n-----------site_QR_code_old:%s\n\n", site_QR_code);

        QrString.sprintf("%s",site_QR_code);
        myQrcode = new MyQrenCode(QrString,this);
        myQrcode->move(475, 145);
        myQrcode->setFixedSize(85,85);	//设置二维码固定大小

        myQrcode->show();
#endif

		#if 1 //屏蔽微信公众号二维码 2016-11-25
		label_weixinNO->setVisible(false);
        label_2->setVisible(false);
		#endif
		
#if !USING_QR_CODE   //关掉所有二维码显示
         label_weixinNO->setVisible(false);
         label_2->setVisible(false);

         label_weixinAPP->setVisible(false);
         label_3->setVisible(false);

         label_AliPayAPP->setVisible(false);
         label_4->setVisible(false);

         label_iosAPP->setVisible(false);
         label_5->setVisible(false);

         label_QR_code->setVisible(false);

         label_6->setVisible(false); //主界面技术支持
#endif
	connect(this, SIGNAL(signals_entryMangageDlg()), this, SLOT(slots_entryMangageDlg()));  //运营管理
	connect(this, SIGNAL(signals_entryInfoQueryDlg()), this, SLOT(slots_entryInfoQueryDlg())); //信息查询
	//connect(this, SIGNAL(signals_entryOperationGuideDlg()), this, SLOT(slots_entryOperationGuideDlg())); //操作指南
	connect(this, SIGNAL(signals_entryExcepionHandleDlg()), this, SLOT(slots_entryExcepionHandleDlg()));  //异常处理

#if USING_ON_OFF_CARD
	connect(this, SIGNAL(signals_entryMemberServiceDlg()), this, SLOT(slots_entryMemberServiceDlg()));  //会员服务
#endif

	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);  //定时1秒

    connect(&timer1, SIGNAL(timeout()), this, SLOT(slots_timer1Done()));//羊城通充值提示

	
#if CONFS_USING_READER_UPDATE//CONFS_USING_TEST_BY_ZJC //读卡器升级结果通知初始化
	memset(&update_result, 0, sizeof(update_result));
	update_result.type = UPE_TYPE_LNT_ZM;
	update_result.result = 0;
	
	//通过共享内存通知bicycle进程升级结果
	upe_mid = 0;
	retval = lib_share_mem_init(&upe_mid, UPE_SHART_MEM_KEY, sizeof(lnt_firmware_update_result_t) + 16);
	lib_share_mem_write(upe_mid, 0, (char *)&update_result, sizeof(lnt_firmware_update_result_t));
#endif

}

BicycleDlg::~BicycleDlg(void)
{
	
}

/* 运营管理界面 */
void BicycleDlg::slots_entryMangageDlg(void)
{
	//unsigned char stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	lib_lnt_qry_ticket_info_ack_t lnt_qry_ticket;
	lib_lnt_rent_info_req_t lnt_rent_req;
	lib_lnt_rent_info_ack_t lnt_rent_ack;
	gui_admin_card_info_t card_info;
		
	memset(&lnt_qry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));
	memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
	memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));
	memset(&card_info, 0, sizeof(gui_admin_card_info_t));

#if USING_GIF_SHOW
	p_movie->stop();
#endif

	if(timer.isActive())
	{
		timer.stop();
	}

#if USING_ADMIN//检查管理卡
	lntAdminCardType = lib_lnt_is_admin_card();
	if((lntAdminCardType == LNT_ADMIN_CARD_TYPE_NOT) || (lntAdminCardType == LNT_ADMIN_CARD_UNVALID))  //非管理卡
	{
		if(lntAdminCardType == LNT_ADMIN_CARD_UNVALID)
		{
			g_adminCardCtrlSet = 2; //管理卡无效
			
			#if 1 //过期管理卡提示
			warnTimeCount1 = 0;

            label_homePageWarn->setText(tr("温馨提示：该管理卡已过期!!!"));

            if(timer1.isActive())
            {
                timer1.stop();
            }

            timer1.start(1000);

            //return;
			#endif
		}
		
		warnTimeCount = 0;
		label_homePageLogo->setVisible(true);
		timer.start(1000); 
		return;
	}
	else if(lntAdminCardType == LNT_ADMIN_CARD_TYPE_RW)
		g_adminCardCtrlSet = 1;
	else if(lntAdminCardType == LNT_ADMIN_CARD_TYPE_RD)
		g_adminCardCtrlSet = 0;   
	
#else
	g_adminCardCtrlSet = 1;
#endif

#if 0
	lib_SN_admin_card_read32(&card_info.sn); //流水号
	lib_get_systime_bcd(card_info.time);  //使用时间
	lib_gui_set_admin_card_info(g_gui, &card_info, 300);  
#endif

	m_pManageDlg = new ManageDlg;
	m_pManageDlg->exec();

	if(m_pManageDlg != NULL)
	{
		delete m_pManageDlg;
		m_pManageDlg = NULL;
	}

	label_homePageWarn->clear();
	label_homePageLogo->setVisible(false);

#if USING_GIF_SHOW
	p_movie->start();
#endif
		
	timer.start(1000); 
}

/* 信息查询界面 */
void BicycleDlg::slots_entryInfoQueryDlg(void)
{
	label_homePageWarn->clear();
	label_homePageLogo->setVisible(false);


#if USING_GIF_SHOW
	p_movie->stop();
#endif

	if(timer.isActive())
	{
		timer.stop();
	}
	
	m_pInfoQryDlg = new InfoQueryDlg;
	m_pInfoQryDlg->exec();

	if(m_pInfoQryDlg != NULL)
	{
		delete m_pInfoQryDlg;
		m_pInfoQryDlg = NULL;
	}


#if USING_GIF_SHOW
	p_movie->start();
#endif

	timer.start(1000); 
}

/* 操作指南界面 */
void BicycleDlg::slots_entryOperationGuideDlg(void)
{
	label_homePageWarn->clear();
	label_homePageLogo->setVisible(false);

#if USING_GIF_SHOW
	p_movie->stop();
#endif

	if(timer.isActive())
	{
		timer.stop();
	}
	
	m_pOperGuideDlg = new OperationGuideDlg;
	m_pOperGuideDlg->exec();

	if(m_pOperGuideDlg != NULL)
	{
		delete m_pOperGuideDlg;
		m_pOperGuideDlg = NULL;
	}


#if USING_GIF_SHOW
	p_movie->start();
#endif

	timer.start(1000); 	
}

/* 异常处理界面 */
void BicycleDlg::slots_entryExcepionHandleDlg(void)
{
	label_homePageWarn->clear();
	label_homePageLogo->setVisible(false);


#if USING_GIF_SHOW
	p_movie->stop();
#endif

	if(timer.isActive())
	{
		timer.stop();
	}
	
	m_pExcepHndDlg = new ExceptionHandleDlg;
	m_pExcepHndDlg->exec();

	if(m_pExcepHndDlg != NULL)
	{
		delete m_pExcepHndDlg;
		m_pExcepHndDlg = NULL;
	}


#if USING_GIF_SHOW
	p_movie->start();
#endif

	timer.start(1000); 	
}

/* 会员服务界面 */
void BicycleDlg::slots_entryMemberServiceDlg(void)
{
	label_homePageWarn->clear();
	label_homePageLogo->setVisible(false);


#if USING_GIF_SHOW
	p_movie->stop();
#endif

	if(timer.isActive())
	{
		timer.stop();
	}
	
	m_pMemServDlg = new MemberServiceDlg;
	m_pMemServDlg->exec();

	if(m_pMemServDlg != NULL)
	{
		delete m_pMemServDlg;
		m_pMemServDlg = NULL;
	}


#if USING_GIF_SHOW
	p_movie->start();
#endif

	timer.start(1000); 	
}


void BicycleDlg::slots_timerDone(void)
{
	static int f_lnt_ver = 1;
	static lib_lnt_getversion_ack_t lnt_ver_ack;
	QString string;
	static int ret = -1;

	
#if CONFS_USING_AUTO_REBOOT
	/* 节点机自动重启时间设置 add by zjc at 2016-11-02 */
	QTime device_Time = QTime::currentTime();
	int hour = device_Time.hour();
	int minute = device_Time.minute();
	int second = device_Time.second();

	printf("\n------------------ system time:[%02d:%02d:%02d] ------------------\n", hour, minute, second);

	if((hour == REBOOT_HOUR) && (minute == REBOOT_MINUTE) && (second == REBOOT_SECOND))
	//if(((hour % 2) == 0) && (minute == 0) && (second == 0))
	{
		printf("system auto reboot...\n");

		SYS_LOG_DEBUG("System Auto Reboot");

		system("/mnt/firmware/reboot_wdt");
	}	
#endif

#if 1
	/* 重启系统前保存网卡信息 2016-12-21 */
	netCount++;
	if(netCount == 2)
	{
		FILE *net_fd = NULL;
		long long rx_bytes = 0, tx_bytes = 0;
		static long long rx_bytes_pre = 0, tx_bytes_pre = 0;	
		gui_net_info_backup_t net_info_backup;
		int err = 0;

		memset(&net_info_backup, 0, sizeof(gui_net_info_backup_t)); //must!
		
		net_fd = fopen(NET_INFO_BACKUP_PATH, "rb");  
		if(NULL != net_fd)
		{
			err = fread(&net_info_backup, sizeof(gui_net_info_backup_t), 1, net_fd);
			if(err == 1)
			{
				net_info_read_flag = 1;
			}
			else 
			{
				fprintf(stderr, "------------read net backup info failed!\n");
				SYS_LOG_DEBUG("ERROR:read net backup info failed!");
				
				net_info_read_flag = 0;
			}    
	
			fclose(net_fd);
			net_fd = NULL;
		}
		else
		{
			fprintf(stderr, "------------open net backup info failed!\n");
			SYS_LOG_DEBUG("ERROR:open net backup info for read failed!");

			net_info_read_flag = 0;
		}
	
		//printf("----1----read: rx_bytes:%lld, tx_bytes:%lld, total_bytes:%lld\n",\
		//net_info_backup.rx_bytes, net_info_backup.tx_bytes, net_info_backup.rx_bytes + net_info_backup.tx_bytes);

		//获取网络接入方式
		struct gui_ndev_page_config nPageConfig;
		memset(&nPageConfig, 0, sizeof(struct gui_ndev_page_config));
		
		retval = lib_gui_get_ndev_page_config(g_gui, &nPageConfig, 2000); 
        if(nPageConfig.access_conf.using_wireless == 1)
        {
        	//printf("---------network:wireless\n");
			lib_get_network_flow("ppp0", &rx_bytes, &tx_bytes); //无线
		}
		else
		{
        	//printf("---------network:wire\n");
			lib_get_network_flow("eth1", &rx_bytes, &tx_bytes); //有线
		}
			
		//printf("------1------rx_bytes:%lld, 	tx_bytes:%lld\n", rx_bytes, tx_bytes);
		//printf("--------rx_bytes_pre:%lld, tx_bytes_pre:%lld\n", rx_bytes_pre, tx_bytes_pre);
		//接收计数
		if(rx_bytes > rx_bytes_pre) //本次>上次，加增量
		{
			printf("\n----------------(rx_bytes - rx_bytes_pre):%lld\n", (rx_bytes - rx_bytes_pre));
			net_info_backup.rx_bytes += (rx_bytes - rx_bytes_pre);
		}  
		else if((rx_bytes < rx_bytes_pre) && (rx_bytes > 0)) //重新拨号网卡重新开始计数
		{
			printf("\n----------------(redial, rx_bytes):%lld\n", rx_bytes);
			net_info_backup.rx_bytes += rx_bytes;
		}
		//发送计数
		if(tx_bytes > tx_bytes_pre)
		{
			printf("\n----------------(tx_bytes - tx_bytes_pre):%lld\n\n", (tx_bytes - tx_bytes_pre));
			net_info_backup.tx_bytes += (tx_bytes - tx_bytes_pre);
		}
		else if((tx_bytes < tx_bytes_pre) && (tx_bytes > 0)) 
		{
			printf("\n----------------(redial, tx_bytes):%lld\n", tx_bytes);
			net_info_backup.tx_bytes += tx_bytes;
		}
		
		net_info_backup.total_bytes = net_info_backup.rx_bytes + net_info_backup.tx_bytes;
	   
		//printf("----1----to write: rx_bytes:%lld, tx_bytes:%lld, total_bytes:%lld\n",\
			//net_info_backup.rx_bytes, net_info_backup.tx_bytes, net_info_backup.total_bytes);

		//每个月1号统计清零
		time_t timep;  
	    struct tm *p_tm;             
		
	    timep = time(NULL);  
	    p_tm = localtime(&timep);  
		
	    //printf("========== %d-%d-%d ", (p_tm->tm_year+1900), (p_tm->tm_mon+1), p_tm->tm_mday);  
   		//printf(" %d:%d:%d ==========\n", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);  

		if((p_tm->tm_mday == 1) && (p_tm->tm_hour == 0) && (p_tm->tm_min == 0) && (p_tm->tm_sec < 3))
		{
			net_info_backup.rx_bytes = 0;
			net_info_backup.tx_bytes = 0;
			net_info_backup.total_bytes = 0;

			SYS_LOG_DEBUG("Start of the month, clear the flow count!");
		}

		//第一次写入或者不是第一次写入但是前面读取成功才写入,防止读取失败时写入错误数据
		if((net_info_read_flag == 1) || ((access(NET_INFO_BACKUP_PATH, F_OK) != 0) && (net_info_read_flag == 0)))	
		{
			net_fd = fopen(NET_INFO_BACKUP_PATH, "wb");
			if(NULL != net_fd)
			{
				err = fwrite(&net_info_backup, sizeof(gui_net_info_backup_t), 1, net_fd);
				if(err != 1)
				{
					fprintf(stderr, "-------------write net backup info failed!\n");
					SYS_LOG_DEBUG("ERROR:write net backup info failed!");
				}

				fflush(net_fd);
				fclose(net_fd);
				net_fd = NULL;
			}
			else
			{
				fprintf(stderr, "-------------open net backup info failed!\n");
				SYS_LOG_DEBUG("ERROR:open net backup info for write failed!");
			}
		}
			
		//备份上次数据
		rx_bytes_pre = rx_bytes;
		tx_bytes_pre = tx_bytes;	
		
		netCount = 0;
	}
#endif

	QDateTime deviceTime = QDateTime::currentDateTime();
	string = deviceTime.toString("yyyy-MM-dd hh:mm:ss");
	label_devcieTime->setText(string); //主界面时间显示

	lntTimeCount++;
	if(lntTimeCount == NDEV_LNT_PERIOD)  //岭南通信息
	{
		memset(&lntAllInfo, 0, sizeof(struct gui_lnt_all_info));

		lib_lnt_get_firmware_version((char *)&(lntAllInfo.lib_version));		   //库版本

		if(f_lnt_ver == 1)
		{		
			unsigned char stat;
			memset(&lnt_ver_ack.version, 0, sizeof(lib_lnt_getversion_ack_t));
			
			ret = lib_lnt_get_version(&lnt_ver_ack, &stat, 200); 
			if(ret != LNT_ERROR) //获取成功才保存 2017-01-12
			{
				strcpy((char *)lntAllInfo.hw_version, (char *)lnt_ver_ack.version);  //读卡器版本

				#if 1
				/* 保存岭南通读卡器固件版本，供向中心签到用 add by zjc at 2016-08-17 */
				char version[32] = {0}; 		
				memcpy(version, lnt_ver_ack.version, sizeof(version));
				version[24] = '\0';
				//printf("Bicycle_gui, version:%s\n", version);
				
				__lnt_firmware_version_put(version); //保存最新的岭南通读卡器版本
				#endif  
		
				f_lnt_ver = 0;
			}
		}
		else
		{
			strcpy((char *)lntAllInfo.hw_version, (char *)lnt_ver_ack.version);  //读卡器版本
		}
		
		lntAllInfo.register_stat = lib_lnt_register_stat_get();  //注册状态

		lib_lnt_config_t config;
		lib_lnt_get_config(&config);  
		memcpy(lntAllInfo.pki_r, config.pki_r, 4);  //PSAM卡号
		memcpy(lntAllInfo.psam, config.psam, 4);  //PKI

		lntAllInfo.pkt_sn_RO = lib_pkt_sn_RO_get();  //报文序列号

		lib_lnt_register_proc_code_t proc_code = lib_lnt_register_proc_code_get();
		lntAllInfo.proc = proc_code.proc;		//执行步骤
		lntAllInfo.fresult = proc_code.fresult;	//执行结果
		lntAllInfo.stat = proc_code.stat;		//返回状态
		lntAllInfo.sw = proc_code.sw;			//返回码
		lntAllInfo.time = proc_code.time;		//执行时间
		lntAllInfo.agent_err = proc_code.agent_err;	//代理错误码
		
		lib_gui_lnt_all_info_set(g_gui, &lntAllInfo, 200);
		
		lntTimeCount = 0;
	}
	
	if(timerCount == NDEV_INFO_STAGE1_PERIOD)
	{
		__getPageConfig(1);
	}

	if(timerCount == NDEV_INFO_STAGE2_PERIOD)
	{
		__getPageConfig(2);

		#if 0
		int __frameCount = p_movie->frameCount();
		int __loopCount = p_movie->loopCount();
		int __nextFrameDelay = p_movie->nextFrameDelay();
		int __speed = p_movie->speed();

		fprintf(stderr, "__currentFrameNumber: %d, __frameCount: %d, __loopCount: %d, __nextFrameDelay: %d, __speed: %d\n", __currentFrameNumber, __frameCount, \
			__loopCount, __nextFrameDelay, __speed);
		#endif


		#if 0

		int __currentFrameNumber = p_movie->currentFrameNumber();
		fprintf(stderr, "__currentFrameNumber: %d, gif_current_frame: %d\n", __currentFrameNumber, gif_current_frame);

		update();
		
		if(gif_current_frame == __currentFrameNumber)
		{
			p_movie->stop();
			p_movie->start();

			gif_current_frame = 0;
		}
		
		gif_current_frame = __currentFrameNumber;
		#endif
	}
		
	timerCount--;
	if(timerCount == 0)
		timerCount = NDEV_INFO_STAGE1_PERIOD;  //

	/* 刷新管理卡 */
	warnTimeCount++;
	if(warnTimeCount == WARN_WAIT_TIME)
	{
		label_homePageWarn->clear();
		label_homePageLogo->setVisible(false);
	}

	gifTimeCount++;
	if(gifTimeCount == GIF_TIME_COUNT)
	{

#if USING_GIF_SHOW
		int __MovieState = p_movie->state();
		//fprintf(stderr, "__MovieState: %d, gifTimeCount: %d\n", __MovieState, gifTimeCount);
		
		if(__MovieState != QMovie::Running)
		{
			p_movie->start();
		}
#endif

		gifTimeCount = 0;
	}
}

void BicycleDlg::slots_timer1Done(void)
{
    /* 羊城通充值提示 */
    warnTimeCount1++;
    if(warnTimeCount1 == WARN_WAIT_TIME)
    {
            label_homePageWarn->clear();

            timer1.stop();
    }
}

void BicycleDlg::__allWidgetClear(void)
{
	label_terminalName->clear();
	label_terminalNo->clear();
	label_devcieTime->clear();

	label_stakeProportion->clear();
	label_homePageWarn->clear();
	label_homePageLogo->setVisible(false);
}

int BicycleDlg::__getPageConfig(const unsigned int stage)
{		
	if(stage == 1)
	{
		retval = LIB_GUI_ERROR;
		retval = lib_gui_get_ndev_info_page_config_stage1(g_gui, 300);
	}
	else if(stage == 2)
	{
		memset(&home_page_info, 0, sizeof(struct gui_ndev_home_page_info));
		if(retval == LIB_GUI_EOK)
		{
			retval = lib_gui_get_ndev_info_page_config_stage2(g_gui, &home_page_info, 400);  //1000->400
			if(retval == LIB_GUI_EOK)
			{
				//如果网点数据有变则保存下来              
				if(memcmp(&site_QR_code, &home_page_info.site_QR_code, sizeof(home_page_info.site_QR_code)))
				{
					QR_code_flag = 1;	//数据有变

					//printf("----------------home_page_info.site_QR_code:%s\n", home_page_info.site_QR_code);
					
					memcpy(&site_QR_code, &home_page_info.site_QR_code, sizeof(home_page_info.site_QR_code));
				}
				else   
				{    
					QR_code_flag = 0; 
				}   

				//printf("----------reader update status:%d\n", lib_get_update_status());

				#if CONFS_USING_READER_UPDATE
				fprintf(stderr, "--------Bicycle_gui, lnt_firmware_update_flag:%d\n", home_page_info.lnt_firmware_update_flag);
				//fprintf(stderr, "------------gui:lnt_firmware_path:%s\n", home_page_info.lnt_firmware_path);
				if(home_page_info.lnt_firmware_update_flag == 1) //有读卡器固件需要升级
				{
					#if 0
					//执行固件升级
					printf("--------start update reader ...\n");
					retval = lib_lnt_reader_update(home_page_info.lnt_firmware_path);
					printf("-------gui:update_result:%d\n", update_result.result);

					memset(&update_result, 0, sizeof(update_result));
					update_result.type = UPE_TYPE_LNT_ZM;
					update_result.result = retval;
					memcpy(&update_result.ftp_local_path, &home_page_info.lnt_firmware_path, sizeof(update_result.ftp_local_path));

					//通过共享内存通知bicycle进程升级结果
					retval = lib_share_mem_init(&upe_mid, UPE_SHART_MEM_KEY, sizeof(lnt_firmware_update_result_t) + 16);
					lib_share_mem_write(upe_mid, 0, (char *)&update_result, sizeof(lnt_firmware_update_result_t));
					
					//升级完通知bicycle进程清除固件信息
					//lib_gui_clr_lnt_firmware_info(g_gui, &update_result, 200); //retval:升级结果 1:成功,2:失败

					#else
					char lnt_firmware_path[96]; //用于字符串分割，防止分割后修改源字符串
					memcpy(&lnt_firmware_path, &home_page_info.lnt_firmware_path, sizeof(lnt_firmware_path));

					char *buf = NULL, *pre_buf = NULL;
					const char *delim = "_";//分割符
					buf = strtok((char *)&lnt_firmware_path,delim);
					while(NULL != buf){
						//printf("%s\n",buf);
						pre_buf = buf;//保存分割出的字符串,取最后一个分割出的字符串
						buf = strtok(NULL,delim);
					}
					//printf("-----------new version:%s\n", pre_buf);

					#if 1
					lnt_getversion_ack_t version;
					unsigned char stat = -1;
					if(up_thr_runing == 0) //防止干扰正在升级的线程!
						lib_lnt_get_version(&version, &stat, 100);
					//printf("-----------old version:%s\n", version.version);

					if((strncmp((char *)&version.version, pre_buf, 8)) && pre_buf != NULL)
					{
						printf("Reader version is not same, need to update ...\n");
						//enum UPDATE_STATUS up_stat = lib_get_update_status();
						//printf("---------reader update status:%d, up_thr_runing:%d\n", up_stat, up_thr_runing);
						
						if(up_thr_runing == 0) //已有升级线程运行则不创建升级线程
						{
							pthread_t lnt_update_thr;
							   
							lib_normal_thread_create(&lnt_update_thr, lnt_update_thread, &home_page_info); //读卡器升级线程
							up_thr_runing = 1;	
						}
					}
					else
					{
						printf("Reader version is same, don't need to update ...\n");

						update_result.type = UPE_TYPE_LNT_ZM;
						update_result.result = 3; //用于通知upgrade_app进程不需要升级读卡器,防止upgrade_app进程不必要的一直读升级结果
						lib_share_mem_write(upe_mid, 0, (char *)&update_result, sizeof(lnt_firmware_update_result_t));
					}	
					#endif
					#endif
				}   
				#endif
				__setPageConfig();
			}
		}

	#if 1
		#if CONFS_USING_READER_UPDATE//#if CONFS_USING_TEST_BY_ZJC
		if(up_thr_runing == 0) //读卡器升级期间不能操作读卡器!!!
		#endif
	#endif
		{
			/* 岭南通读卡器状态 */
			int ret = -1;
			unsigned char stat = LNT_MOC_ERROR_CARD_OP_FAIL;
			lib_lnt_getversion_ack_t lnt_ver_ack;
			gui_lnt_card_status_t lnt_card_stat;
			memset(&lnt_card_stat, 0, sizeof(gui_lnt_card_status_t));
			
			ret = lib_lnt_get_version(&lnt_ver_ack, &stat, 200); //干扰读卡器升级2
			if((ret > 0) && (stat == LNT_MOC_ERROR_NORMAL))
			{
				lnt_card_stat.lnt_card_stat = 1;
			}
			else  
			{
				lnt_card_stat.lnt_card_stat = 0;
			}
			
			lib_gui_set_lnt_card_status(g_gui,  &lnt_card_stat, 200);
		}
	}
	
#if 0
    printf("\n----site_QR_code_old:%s\n", site_QR_code);
    printf("\nhome_page_info.site_QR_code:%s\n", home_page_info.site_QR_code);

    //lib_printf("site_QR_code",(unsigned char *)home_page_info.site_QR_code,32);
#endif

	return retval;
}

void BicycleDlg::__setPageConfig(void)
{
    QString string, QrString;

#if USING_QR_RENT_BIKE  //测试租车二维码

    label_QR_code->setVisible(true);

    if(QR_code_flag == 1)   
    {
        printf("\n-----------site_QR_code_new:%s\n\n", site_QR_code);
	
        QrString.sprintf("%s",site_QR_code);
        myQrcode = new MyQrenCode(QrString,this);
        myQrcode->move(475, 145);
        myQrcode->setFixedSize(85,85);	//设置二维码固定大小

        myQrcode->show();
    }
#endif    

	/* 终端名称 */
	QTextCodec *codec = QTextCodec::codecForLocale();
	QString s_gbk = codec->toUnicode(home_page_info.terminal_name);  //显示中文,GBK
	label_terminalName->setText(s_gbk);

	/* 终端编号 */
	#if 0
	string.sprintf("ID:%d-%02X", home_page_info.terminal_no, home_page_info.terminal_no);
	#endif
	string.sprintf("%05d", home_page_info.terminal_no);
	
	label_terminalNo->setText(string);

	/* 网络类型 */
	if(home_page_info.network_type == 1)
		label_wireSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/wireless.png")));
	else if(home_page_info.network_type == 2)
		label_wireSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/wired.png")));

	/* 3G信号 */
	switch(home_page_info.signal_level)
	{
		case WL_SIGNAL_LEVEL0:
		{
			label_3gSignalLevel->setPixmap(QPixmap(QString::fromUtf8(":/image/signal_level0.png")));
		}
		break;

		case WL_SIGNAL_LEVEL1:
		{
			label_3gSignalLevel->setPixmap(QPixmap(QString::fromUtf8(":/image/signal_level1.png")));
		}
		break;

		case WL_SIGNAL_LEVEL2:
		{
			label_3gSignalLevel->setPixmap(QPixmap(QString::fromUtf8(":/image/signal_level2.png")));
		}
		break;

		case WL_SIGNAL_LEVEL3:
		{
			label_3gSignalLevel->setPixmap(QPixmap(QString::fromUtf8(":/image/signal_level3.png")));
		}
		break;

		case WL_SIGNAL_LEVEL4:
		{
			label_3gSignalLevel->setPixmap(QPixmap(QString::fromUtf8(":/image/signal_level4.png")));
		}
		break;

		case WL_SIGNAL_LEVEL5:
		{
			label_3gSignalLevel->setPixmap(QPixmap(QString::fromUtf8(":/image/signal_level5.png")));
		}
		break;
	}

	/* WIFI */
	if(home_page_info.wifi_status == 0)
		label_wifiSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/wifi_inaction.png")));
	else if(home_page_info.wifi_status == 1)
		label_wifiSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/wifi_active.png")));

	/* gps */
	if(home_page_info.gps_status == 0)
		label_gpsSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/gps_inactive.png")));
	else if(home_page_info.gps_status == 1)
		label_gpsSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/gps_active.png")));

	/* center */
	if(home_page_info.center_status == 0)
		label_centerSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/center_inactive.png")));
	else if(home_page_info.center_status == 1)
		label_centerSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/center_active.png")));
	
	/* yct */
	#if 0
	if(home_page_info.yct_status == 0)
		label_yctSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/yct_inactive.png")));
	else if(home_page_info.center_status == 1)
		label_yctSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/yct_active.png")));
	#endif

	/* lnt */
	enum LNT_REGISTER_STAT reg_stat = lib_lnt_register_stat_get();
	if(reg_stat == LNT_REGISTER_STAT_OK)
		label_yctSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/yct_active.png")));
	else
		label_yctSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/yct_inactive.png")));


	
	/* canbus */
	if(home_page_info.canbus_status== 0)
		label_canbusSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/canbus_inactive.png")));
	else if(home_page_info.canbus_status == 1)
		label_canbusSignal->setPixmap(QPixmap(QString::fromUtf8(":/image/canbus_active.png")));

	/* 锁桩签到 */
	string.sprintf("%d/%d", home_page_info.stake_online_num, home_page_info.stake_total_num);
	label_stakeProportion->setText(string);

	
}

void BicycleDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  //屏蔽ESC按键
		return;

#if 0
	if(e->key() == Qt::Key_1)   //操作指南
		emit signals_entryOperationGuideDlg();
#endif

	if(e->key() == Qt::Key_2) //信息查询
		emit signals_entryInfoQueryDlg();

	if(e->key() == Qt::Key_3)  //异常处理
		emit signals_entryExcepionHandleDlg();

	if(e->key() == Qt::Key_1) //会员服务
		emit signals_entryMemberServiceDlg();
	
	if(e->key() == Qt::Key_5) //运营管理
		emit signals_entryMangageDlg();

#if 1
        if(e->key() == Qt::Key_4) //羊城通充值
        {
            warnTimeCount1 = 0;

            label_homePageWarn->setText(tr("温馨提示：[羊城通充值]功能暂未开通,请关注后续更新......"));

            if(timer1.isActive())
            {
                timer1.stop();
            }

            timer1.start(1000);

            return;
        }
#endif
	
	return;
}
   

#if CONFS_USING_READER_UPDATE//CONFS_USING_TEST_BY_ZJC
static void *lnt_update_thread(void *arg)
{
	char retval = -1, up_cnt = 0;
	struct lnt_firmware_update_result result;  
	memset(&result, 0, sizeof(result));
	
	struct gui_ndev_home_page_info home_page_info = *(struct gui_ndev_home_page_info *)arg;
	//printf("---------update file:%s\n", home_page_info.lnt_firmware_path);
	
	printf("Start update reader ...\n");

	while(1)
	{
		up_cnt++; //升级次数
		//printf("---------up_cnt:%d\n", up_cnt);
		  
		//执行固件升级
		retval = lib_lnt_reader_update(home_page_info.lnt_firmware_path); //创建线程升级发包要慢点!
		printf("Bicycle_gui, update_result:%d\n", retval);

		update_result.type = UPE_TYPE_LNT_ZM;
		update_result.result = retval;
		memcpy(&update_result.ftp_local_path, &home_page_info.lnt_firmware_path, sizeof(home_page_info.lnt_firmware_path));
		lib_share_mem_write(upe_mid, 0, (char *)&update_result, sizeof(lnt_firmware_update_result_t));

		if(retval == 1) //升级成功
		{
			lib_lnt_getversion_ack_t lnt_ver_ack;
			unsigned char m_stat;
			char version[32] = {0}; 		

			memset(&lnt_ver_ack, 0, sizeof(lnt_ver_ack));
			lib_lnt_get_version(&lnt_ver_ack, &m_stat, 100);
			memcpy(version, lnt_ver_ack.version, sizeof(version));
			version[24] = '\0';
			//printf("-------version:%s\n", version);
			
			__lnt_firmware_version_put(version); //保存最新的岭南通读卡器版本
		}
		#if 0
		result.result = retval;
		lib_gui_lnt_update_result_notify(g_gui, &result, 100); //通知bicycle进程读卡器升级结果
		#endif
		
		lib_sleep(8); //等待upgrade_app读取升级结果,防止还没读取到升级结果就再次创建线程升级!
		break; 
	}

	up_thr_runing = 0; //防止升级失败后进入不了升级线程!!
	return lib_thread_exit((void *)NULL);
	//return NULL;
}
#endif

#if 1
/* 保存读卡器固件版本号到文本 add by zjc at 2016-08-08 */
void  __lnt_firmware_version_put(char version[24])
{
	FILE *fp = NULL;

	fp = fopen(LNT_FIRMWARE_PATH, "wt"); /* 写文本 */
	if(fp != NULL)
	{
		if(fputs(version, fp) != EOF)
		{    
			fprintf(stderr, "Bicycle_gui Lnt Firmware Version put: %s\n", version);

			fflush(fp);
		}

		fclose(fp);
	}
}
#endif

