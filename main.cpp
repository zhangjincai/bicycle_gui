#include <QApplication>
#include <QDebug>
#include <QThreadPool>

#include <signal.h>
#include "bicycle_dlg.h"
#include "helper.h"
#include "lib_gui.h"
#include "lib_wireless.h"
#include "lib_general.h"
#include "lib_records.h"
#include "lib_lnt.h"
#include "thread_retrans_handle.h"

#include "lnt_register_task.h"


#define QT_LNT_IPADDR			"14.23.145.210"
#define QT_LNT_PORT				(7018)	
#define QT_LNT_SPON0			0x01  ////服务商代码
#define QT_LNT_SPON1			0x01
#define QT_LNT_CONPA			(0xE0)   //消费初始参数  0xE0:禁止M1钱包使用, 0x80:正常消费
#define QT_LNT_CVAD				(0x00)  //卡离线有效期启用
#define QT_LNT_USERID			(0x00)  //用户ID


#define REC_DB_PATHNAME		"/opt/records/records_file.db"

lib_gui_t *g_gui = NULL;
lib_wl_t *g_wl = NULL;
lib_records_file_t *g_rec_file = NULL;
char g_terminal_no[32];

extern int up_thr_runing; //读卡器升级状态 1为正在升级


static void __sigint(int sig)
{
	qDebug("GUI signal: %d\n", sig);
	
	//lib_gui_destroy(g_gui);
	//lib_wl_destroy(g_wl);
	//lib_lnt_release();

	//::exit(-1);  //直接退出程序

#ifdef UI_LOG_RUN
	closelog();
#endif
}

static void __init_signals(void)
{
	struct sigaction sa;

	signal(SIGPIPE, SIG_IGN); //管道关闭
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	
	signal(SIGTTOU, SIG_IGN); //后台进程写控制终端
	signal(SIGTTIN, SIG_IGN); //后台进程读控制终端
	signal(SIGTSTP, SIG_IGN); //终端挂起
	
	sa.sa_flags = 0;
   	sigaddset(&sa.sa_mask, SIGTERM); //终止进程
	sa.sa_handler = __sigint;
	sigaction(SIGTERM, &sa, NULL);
}

static void __terminal_no_file_get(char *terminal_no)
{
	FILE *fp = NULL;
	int s_len = 0;
	char s_terminal_no[64] = {0};

	fp = fopen(TERMINAL_NO_PATH, "rt");  /* 读文本 */
	if(fp != NULL)
	{
		if(fgets(s_terminal_no, sizeof(s_terminal_no), fp) != NULL)
		{
			fprintf(stderr, "GUI Terminal NO get: %s\n", s_terminal_no);
			
			s_len = strlen(s_terminal_no);
			strcpy(terminal_no, s_terminal_no);
			terminal_no[s_len] = '\0';
		}
		else
			strcpy(terminal_no, "65535");
		
		fclose(fp);
	}
}


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);	

	qDebug("gui software compiled time: %s, %s\n",__DATE__, __TIME__);
	
	//daemon(0, 1); //守护进程

	int err = -1;

	system("ulimit -s 7168");  //设置线程栈
	
	__init_signals();
	
	/* 初始化syslog日志*/
#ifdef UI_LOG_RUN
	char log_ident[64] = {0};
	char macaddr[8] = {0};
	unsigned char s_macaddr[16] = {0};
	
	lib_get_macaddr("eth1", macaddr);
	lib_hex_to_str((unsigned char *)macaddr, 6, s_macaddr);
	__terminal_no_file_get(g_terminal_no);
	
	sprintf(log_ident, "------[GUI]-[%s]:[%s]", s_macaddr, g_terminal_no);
	
	fprintf(stderr, "gui log ident: %s\n", log_ident);

	openlog(log_ident, LOG_NDELAY, LOG_DAEMON);
#endif

	SYS_LOG_NOTICE("gui software compiled time: %s, %s\n",__DATE__, __TIME__);

	struct gui_setup setup;
	memset(&setup, 0, sizeof(struct gui_setup));

#if 0
	setup.connect_mode = GUI_CONNECT_MODE_UNIX;
#else
	setup.connect_mode = GUI_CONNECT_MODE_TCP;
	strcpy(setup.s_un.tcp.ipaddr, "127.0.0.1");
	setup.s_un.tcp.port = 10086;
	setup.s_un.tcp.conn_nsec = 1;
#endif      

	/* 线程池设置 */
	QThreadPool::globalInstance()->setMaxThreadCount(3);  // 3个线程
	ThreadRetransHandle * task = new ThreadRetransHandle();
	task->setAutoDelete(true);
	QThreadPool::globalInstance()->start(task);

	g_gui = lib_gui_new(&setup);
	if(g_gui == LIB_GUI_NULL)
	{
		qDebug("GUI lib gui new failed!\n");
		SYS_LOG_ERR("GUI lib gui new failed!");
	}
	
	g_wl = lib_wl_new();
	if(g_wl == LIB_WL_NULL)   
	{
		qDebug("GUI lib wl new failed!\n");
		SYS_LOG_ERR("GUI lib wl new failed!");
	}
   
	/* 岭南通读卡器初始化 */
	unsigned char stat = LNT_MOC_ERROR_NORMAL;
	lib_lnt_config_t lnt_config;
	lib_lnt_parameter_set_req_t lnt_paraset_req;
	lib_lnt_core_init_req_t lnt_core_req;
	lib_lnt_core_init_ack_t lnt_core_ack;
	gui_ndev_config_t ndev_conf;

	memset(&lnt_config, 0, sizeof(lib_lnt_config_t));
	memset(&lnt_paraset_req, 0, sizeof(lib_lnt_parameter_set_req_t));
	memset(&lnt_core_req, 0, sizeof(lib_lnt_core_init_req_t));
	memset(&lnt_core_ack, 0, sizeof(lib_lnt_core_init_ack_t));
	memset(&ndev_conf, 0, sizeof(gui_ndev_config_t));
	  
	if(g_gui != NULL)  
	{
		err = lib_gui_get_ndev_config(g_gui, &ndev_conf, 3000); //500
		if(err != LIB_GUI_EOK)
		{
			qDebug("GUI lib_gui_get_ndev_config failed!\n");
			SYS_LOG_ERR("GUI lib_gui_get_ndev_config failed!");

			strcpy(lnt_config.ipaddr, QT_LNT_IPADDR);
			lnt_config.port = QT_LNT_PORT;
			lnt_config.spno[0] = QT_LNT_SPON0;
			lnt_config.spno[1] = QT_LNT_SPON1;
			memset(lnt_config.userid, QT_LNT_USERID, 16);
			lnt_config.conpa = QT_LNT_CONPA;
			lnt_config.cvad = QT_LNT_CVAD;
		}
		else
		{
			strcpy(lnt_config.ipaddr, lib_iaddr_to_saddr(ndev_conf.lnt_ipaddr));
			lnt_config.port = ndev_conf.lnt_port;

			fprintf(stderr, "%s\n", ndev_conf.appl_ver);
			fprintf(stderr, "ndev_conf.lnt_port=%d\n", ndev_conf.lnt_port);
			fprintf(stderr, "heart_time:%d\n", ndev_conf.heart_time);
				
			memcpy(lnt_config.spno, ndev_conf.lnt_spno, 2);
			memcpy(lnt_config.userid, ndev_conf.lnt_userid, 16);
			lnt_config.conpa = QT_LNT_CONPA; //ndev_conf.lnt_conpa;
			lnt_config.cvad = ndev_conf.lnt_cvad;

			memset(lnt_config.pki, 0xff, 4); //	PKI
			lib_get_macaddr("eth1", (char *)lnt_config.macaddr); //MAC
		}
	}

	strcpy(lnt_config.tty, "/dev/ttyO1");
	lnt_config.baudrate = 115200;
  

	lnt_config.delay_ms = 15000;  //延迟毫秒
	#if CONFS_USING_READER_UPDATE // CONFS_USING_TEST_BY_ZJC
	if(up_thr_runing == 0) //读卡器升级期间不能操作读卡器!!!
	#endif
		err = lib_lnt_init(&lnt_config);
	if(err == LIB_LNT_ERROR)
	{  
		qDebug("linnantong init failed!\n");
		SYS_LOG_ERR("linnantong init failed!\n");
	}
  
#if 0 
	/* 岭南通读卡器签到 */
        lnt_register_thread *lnt_reg_thread;

        lnt_reg_thread = new lnt_register_thread(&lnt_config);
        lnt_reg_thread->start();
#endif

	/* 创建记录数据库 */
	g_rec_file = lib_records_create(REC_DB_PATHNAME);

	
           
#if 0
	/* 参数设置 */
	lnt_paraset_req.conpa = QT_LNT_CONPA;  //正常消费
	lnt_paraset_req.cvad = QT_LNT_CVAD;   //不判断离线时间
	err = lib_lnt_set_parameter(&lnt_paraset_req, &stat, 100);
	if((err == LIB_LNT_ERROR) || (stat != LNT_MOC_ERROR_NORMAL))
	{
		qDebug("linnantong set parameter failed!\n");
		SYS_LOG_ERR("linnantong set parameter failed!\n");
	}	

	/* 核心模块初始化 */
	err = lib_lnt_core_init(&lnt_core_req, &lnt_core_ack, &stat, 100);
	if((err == LIB_LNT_ERROR) || (stat != LNT_MOC_ERROR_NORMAL))
	{
		qDebug("linnantong core init failed!\n");
		SYS_LOG_ERR("linnantong core init failed!\n");	
	}
#endif  


	//Helper::__SetGB2312Code(); //设置字体
	Helper::__SetGBKCode();

	BicycleDlg *dlg = new BicycleDlg();
	dlg->show();

	return app.exec();	
}




