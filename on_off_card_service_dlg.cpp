#include <QtGui>
#include <QDebug>
#include <QKeyEvent>
#include <QString>
#include <QThreadPool>
#include "helper.h"
#include "lib_general.h"
#include "on_off_card_service_dlg.h"
#include "ui_on_off_card_service_ui.h"

#include "lib_lnt.h"
#include "lnt.h"


#define BINFO_RETURN_PERIOD					(120)
#define BINFO_RETURN_FAIL_PERIOD				(25)


#define ECS_QUIT_ON		1
#define ECS_QUIT_OFF		2

#define CONFIRM_KEY_ON		1
#define CONFIRM_KEY_OFF		2


extern int g_CardServiceFlag;
extern lib_gui_t *g_gui;

extern int memberTypeCheck;  //开卡会员类型标识

	
enum RET_ERROR
{
	RET_ERROR_NORMAL = 0,   //正常
	RET_ERROR_CARD_NOTAG = 1,  //没有卡
	RET_ERROR_NOVIP = 2,  //非会员 
	RET_ERROR_DP_INIT = 3,	//押金初始化
	RET_ERROR_OP_FAIL = 4, //操作失败
	RET_ERROR_OP_SUCCESS = 5,  //操作成功
	RET_ERROR_OP_LOOP = 6, //轮询
	RET_ERROR_LNT_BAD = 7, //岭南通读卡器坏 
	RET_ERROR_NETWORK_DISCONNT = 8, //网络不通
	RET_ERROR_LNT_NOT_REG = 9, //读卡器未注册
	RET_ERROR_ID_NO = 10, //身份证输入为空
	RET_ERROR_PHONE_NUMBER = 11, //手机号码输入为空
	RET_ERROR_CENTRE_NOT_REG = 12,  //中心未注册
};

OnOffCardServiceDlg::OnOffCardServiceDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	
	
	pushButton_OnOffCardStart->installEventFilter(this);
	lineEdit_OnOffCardIdNO->installEventFilter(this);
	lineEdit_OnOffCardPhoneNumber->installEventFilter(this);
        lineEdit_memberCount->installEventFilter(this);
		
        confirmCnt = 0; //开销卡按键按下次数


	ptn_start_f = 1;
	retError = RET_ERROR_NORMAL;
	timerCount = BINFO_RETURN_PERIOD;
	esc_quit_key_f = ECS_QUIT_ON;  //允许使用ESC退出键

	pushButton_OnOffCardStart->setStyleSheet("background-color: rgb(224, 239, 198)");
	label_onOffCardShow->clear();
	
        //qDebug("g_CardServiceFlag:%d\n",g_CardServiceFlag);

        pushButton_OnOffCardStart->setText(tr("执行开通操作"));

        if(g_CardServiceFlag == CARD_SERVICE_ON_NORMAL)
	{
		label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));
		
                label_memberCount->setVisible(false);   //普通卡会员时开卡次数不可见
                lineEdit_memberCount->setVisible(false);
                label_memberCnt->setVisible(false);

                label_OnOffCardTitle->setText(tr("普通会员开通服务"));
	}
        else if(g_CardServiceFlag == CARD_SERVICE_ON_TIME)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/time_count.png")));

                label_OnOffCardTitle->setText(tr("次卡会员开通服务"));
                label_memberCnt->setText(tr("次"));
        }
        else if(g_CardServiceFlag == CARD_SERVICE_ON_DAY)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/day_count.png")));

                label_OnOffCardTitle->setText(tr("天卡会员开通服务"));
                label_memberCnt->setText(tr("天"));

        }
        else if(g_CardServiceFlag == CARD_SERVICE_ON_MONTH)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/month_count.png")));

                label_OnOffCardTitle->setText(tr("月卡会员开通服务"));
                label_memberCnt->setText(tr("月"));

        }
        else if(g_CardServiceFlag == CARD_SERVICE_ON_YEAR)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/year_count.png")));

                label_OnOffCardTitle->setText(tr("年卡会员开通服务"));
                label_memberCnt->setText(tr("年"));
        }


        else if(g_CardServiceFlag == CARD_SERVICE_OFF) //销卡
	{         
		label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p0.png")));
			
		label_OnOffCardTitle->setText(tr("会员注销服务"));	
		pushButton_OnOffCardStart->setText(tr("执行注销操作"));

		label_OnOffCardPhoneNumber->setVisible(false);
		lineEdit_OnOffCardPhoneNumber->setVisible(false);

                //销卡时开卡次数不可见
                label_memberCount->setVisible(false);
                lineEdit_memberCount->setVisible(false);
                label_memberCnt->setVisible(false);

                pushButton_OnOffCardStart->setGeometry(420, 380, 151, 51); //设置控件位置
        }

	checkEnvironment();
	slots_timerDone();
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);

	connect(this, SIGNAL(signals_OnOffCardQuit()), this, SLOT(accept()));
	connect(pushButton_OnOffCardStart, SIGNAL(clicked()), this, SLOT(slots_OnOffCardStart()));
}

OnOffCardServiceDlg::~OnOffCardServiceDlg(void)
{
	
}





void OnOffCardServiceDlg::slots_OnOffCardStart(void)
{
        unsigned short unuse_cnt = 0; //剩余次数、天数、月数、年数

        confirmCnt++;   //按下确认键次数加1
        qDebug("\n-----------confirmCnt:%d",confirmCnt);

        qDebug("g_CardServiceFlag:%d\n",g_CardServiceFlag);

#if 1   //检查剩余次数，销卡时用,会影响后面流程!
        if((g_CardServiceFlag == CARD_SERVICE_OFF) && (confirmCnt <= 2))
        {
            unsigned char stat = 0xEE;
            int ret = -1;
            lnt_rent_info_req_t rent_info_req;
            lnt_rent_info_ack_t rent_info_ack;
            memset(&rent_info_req, 0, sizeof(lnt_rent_info_req_t));
            memset(&rent_info_ack, 0, sizeof(lnt_rent_info_ack_t));

            rent_info_req.id = 0x68;
            ret = lib_lnt_qry_rent_info(&rent_info_req, &rent_info_ack, &stat, 300);
            if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL))
            {
                printf("--------qry_rent_info failed!\n");
            #if 1
                //岭南通模块串口初始化
                lib_lnt_config_t config;
                memset(&config,0,sizeof(lib_lnt_config_t));
                strcpy((char *)config.tty,"/dev/ttyO1");
                config.baudrate = 115200;

                ret = lnt_init(&config);
                if(ret < 0){
                        fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
                }
            #endif
            }

            unuse_cnt = LNT_NTOHS(rent_info_ack.rent.s_un.acc.accum);
            qDebug("---------unuse_memberCnt:%d\n", unuse_cnt);
        }
        //qDebug("============unuse_memberCnt:%d\n", unuse_cnt);

#endif
#if 1
    /*//开销卡过程中对“确认键”进行限制
     if(((confirmCnt <= 1) && ((g_CardServiceFlag == CARD_SERVICE_ON_NORMAL) || (g_CardServiceFlag == CARD_SERVICE_ON_TIME) \
        || (g_CardServiceFlag == CARD_SERVICE_ON_DAY) || (g_CardServiceFlag == CARD_SERVICE_ON_MONTH) \
        || (g_CardServiceFlag == CARD_SERVICE_ON_YEAR))) \
        || ((unuse_cnt > 0) && (confirmCnt <= 2) && (g_CardServiceFlag == CARD_SERVICE_OFF)) \
        || ((unuse_cnt == 0) && (confirmCnt == 1) && (g_CardServiceFlag == CARD_SERVICE_OFF))) //销卡(还需区分是否有未使用完的次数)
     */
      //解决输入为空时一直按开卡提示不对的问题 by zjc at 2016-10-27
     if((g_CardServiceFlag != CARD_SERVICE_NOT) && (g_CardServiceFlag != CARD_SERVICE_RECHARGE))
#endif //(confirmCnt <= 2)
    {
        if(ptn_start_f == 1)
	{
		ptn_start_f = 2;

		QString string;
		struct TaskData taskdata;
		memset(&taskdata, 0, sizeof(struct TaskData));

                taskdata.confimeCnt = confirmCnt;

		/* 判断身份证和电话号码输入是否为空 */
		#if 1
		QString s_lineEdit;
		//if(g_CardServiceFlag == CARD_SERVICE_ON)
		if(g_CardServiceFlag != CARD_SERVICE_OFF) //解决开卡输入为空判断失效的问题 by zjc at 2016-10-27
		{
			s_lineEdit = lineEdit_OnOffCardIdNO->text();
			printf("-------------IdNO len:%d\n", s_lineEdit.length());
			if(s_lineEdit.length() < 15)  //小于合法长度
			{
				retError = RET_ERROR_ID_NO;
				timerCount = BINFO_RETURN_FAIL_PERIOD;
				ptn_start_f = 1;
				return;
			}

			s_lineEdit.clear();
			s_lineEdit = lineEdit_OnOffCardPhoneNumber->text();
			printf("-------------PhoneNumber len:%d\n", s_lineEdit.length());
			if(s_lineEdit.length() < 11) //小于合法长度
			{
				retError = RET_ERROR_PHONE_NUMBER;
				timerCount = BINFO_RETURN_FAIL_PERIOD;
				ptn_start_f = 1;
				return;
			}
		}
		
		if(g_CardServiceFlag == CARD_SERVICE_OFF)
		{
			s_lineEdit = lineEdit_OnOffCardIdNO->text();
			printf("-------------IdNO len:%d\n", s_lineEdit.length());
			if(s_lineEdit.length() < 15)  //小于合法长度
			{
				retError = RET_ERROR_ID_NO;
				timerCount = BINFO_RETURN_FAIL_PERIOD;
				ptn_start_f = 1;
				return;
			}
		}
		#endif
		
		taskdata.serviceType = g_CardServiceFlag;
                string = lineEdit_OnOffCardIdNO->text(); //身份证号码
		char *s = Helper::__QStringToChar(string);
		strcpy((char *)taskdata.id_card, s);  

		QString s_tmp;
		string.clear();
                string = lineEdit_OnOffCardPhoneNumber->text(); //手机号码

		s_tmp.append('0');
		s_tmp.append(string);

		char *s_pn = Helper::__QStringToChar(s_tmp);
		unsigned char s_hex[32] = {0};
		lib_lnt_str_to_hex((unsigned char *)s_pn, (unsigned char *)s_hex, strlen(s_pn));

		memcpy(taskdata.phone_number, s_hex, 6);
		
		//lib_printf("BCD:", taskdata.phone_number, 6);

#if 1
                ///////////////非普通会员开卡参数 2016-04-25 add by zjc//////////
                unsigned short cnt;
                string.clear();
                string = lineEdit_memberCount->text();
                cnt = Helper::__QStringToUshort(string);

                //qDebug("-----------member_type:%d\n",memberTypeCheck);
                //qDebug("-----------member_typeCount:%d\n",cnt);

                taskdata.member_type = memberTypeCheck; //会员类型（票种标记）
                memcpy(&taskdata.member_typeCount, &cnt, 2); //次数、天数、月数、年数
#endif

		OnOffCardTask * task = new OnOffCardTask(&taskdata);
		task->setAutoDelete(true);
		connect(task, SIGNAL(signals_Result(struct TaskResult *)), this, SLOT(slots_TaskResult(struct TaskResult *)), Qt::QueuedConnection);
	 	QThreadPool::globalInstance()->start(task);	

    #if 1
        if(((confirmCnt > 0) && ((g_CardServiceFlag == CARD_SERVICE_ON_NORMAL) || (g_CardServiceFlag == CARD_SERVICE_ON_TIME) \
            || (g_CardServiceFlag == CARD_SERVICE_ON_DAY) || (g_CardServiceFlag == CARD_SERVICE_ON_MONTH) \
            || (g_CardServiceFlag == CARD_SERVICE_ON_YEAR))) \
            || ((unuse_cnt == 0) && (confirmCnt > 0) && (g_CardServiceFlag == CARD_SERVICE_OFF)) \
            || ((unuse_cnt > 0) && (confirmCnt > 1) && (g_CardServiceFlag == CARD_SERVICE_OFF))) //销卡(还需区分是否有未使用完的次数)
    #else
        if(confirmCnt != 1)
    #endif
        {
                esc_quit_key_f = ECS_QUIT_OFF;  //开销卡过程中不允许使用ESC退出键
        }

		label_onOffCardShow->clear();
	}
    }
}

void OnOffCardServiceDlg::slots_TaskResult(struct TaskResult *result)
{
//	qDebug("result:%02X\n", result->result);
//	qDebug("step:%02X\n", result->step);
//	qDebug("%s\n", result->s_info);

	if(result == NULL)
	{
		SYS_LOG_ALERT("%s: result is NULL\n", __FUNCTION__);
		return;
	}

	if(result->type == 0x88)
	{
		//qDebug("serviceType: %d\n", result->serviceType);
		//qDebug("taskRunStep: %d\n", result->taskRunStep);
		//qDebug("taskRunStat: %d\n", result->taskRunStat);

                if(result->serviceType == CARD_SERVICE_ON_NORMAL || result->serviceType == CARD_SERVICE_ON_TIME	\
                   || result->serviceType == CARD_SERVICE_ON_DAY || result->serviceType == CARD_SERVICE_ON_MONTH \
                   || result->serviceType == CARD_SERVICE_ON_YEAR)
		{
			switch(result->taskRunStep)
			{
				case TaskRunStep_p0:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));
				}
				break;
					
				case TaskRunStep_p1:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p1.png")));
				}
				break;

				case TaskRunStep_p2:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p2.png")));
				}
				break;

				case TaskRunStep_p3:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p3.png")));
				}
				break;

                                case TaskRunStep_p4: //开卡最后一步
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p4.png")));
                                        confirmCnt = 0; //确认键重新开始计数
                                }
				break;

				case TaskRunStep_p5:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p5.png")));
				}
				break;
			}			
		}
		else if(result->serviceType == CARD_SERVICE_OFF)
		{
			switch(result->taskRunStep)
			{
				case TaskRunStep_p0:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p0.png")));
				}
				break;
				
				case TaskRunStep_p1:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p1.png")));
				}
				break;

				case TaskRunStep_p2:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p2.png")));
				}
				break;

				case TaskRunStep_p3:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p3.png")));
				}
				break;

				case TaskRunStep_p4:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p4.png")));
				}
				break;

                                case TaskRunStep_p5:    //销卡最后一步
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p5.png")));
                                        confirmCnt = 0; //确认键重新开始计数
                                }
				break;
				
				case TaskRunStep_p6:
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p6.png")));
				}
				break;
			}
		}

                if(result != NULL)
                {
                    free(result);
                    result = NULL;
                }

		return;
	}

	switch(result->result)
	{
		case TASK_RESULT_OP_LOOP:
		{
			if(timer.isActive())
				timer.stop();

			retError = RET_ERROR_OP_LOOP;
			infoString.sprintf(result->s_info);
			label_onOffCardShow->setText(infoString);
		}
		break;

		case TASK_RESULT_OP_LOOP_FREE:
		{
			retError = RET_ERROR_OP_LOOP;
			timerCount = BINFO_RETURN_FAIL_PERIOD;
			memset(&taskResult, 0, sizeof(struct TaskResult));
			memcpy(&taskResult, result, sizeof(struct TaskResult));
			timer.start(1000); 	

			if(result->memberCount != 0xFFFF) //不是提示确认销卡后到这里的，提示确认销卡的则用户确认后继续走下去 add by zjc at 2016-11-01 
				confirmCnt = 0; //确认键重新开始计数
			
            esc_quit_key_f = ECS_QUIT_ON;  //允许使用ESC退出键
							
            if(result != NULL)
            {
                free(result);
                result = NULL;
            }
		}
		break;
		
		case TASK_RESULT_OP_FAIL_FREE:
		{
			retError = RET_ERROR_OP_FAIL;
			timerCount = BINFO_RETURN_FAIL_PERIOD;
			memset(&taskResult, 0, sizeof(struct TaskResult));
			memcpy(&taskResult, result, sizeof(struct TaskResult));
			timer.start(1000); 	
			
                        if(result != NULL)
                        {
                            free(result);
                            result = NULL;
                        }

                        esc_quit_key_f = ECS_QUIT_ON;  //允许使用ESC退出键
                        confirmCnt = 0; //确认键重新开始计数
                }
		break;
	}
	
	
	 

	  
	//qDebug("activeThreadCount:%d\n", QThreadPool::globalInstance()->activeThreadCount());


	 
	 ptn_start_f = 1;
}













int OnOffCardServiceDlg::checkEnvironment(void)
{
	int ret = -1;
	unsigned char stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	lib_lnt_getversion_ack_t ver;
	enum LNT_REGISTER_STAT reg_stat = LNT_REGISTER_STAT_INIT;
	gui_access_state_t state;

	/* 检查读卡器是否存在 */
	ret = lib_lnt_get_version(&ver, &stat, 200); //注意这里会干扰读卡器升级!!!
	if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
	{	 
		retError = RET_ERROR_LNT_BAD;   //读卡器异常 
		timerCount = BINFO_RETURN_FAIL_PERIOD; 
		return -1;
	}

	/* 检查无线网络 */
	struct gui_ndev_access_pattern_config access_pattern_config;
	memset(&access_pattern_config, 0, sizeof(struct gui_ndev_access_pattern_config));
	ret = lib_gui_get_ndev_access_pattern_config(g_gui, &access_pattern_config, 300);
	if(ret == LIB_GUI_EOK)
	{
		//fprintf(stderr, "ON OFF CARD:access_pattern_config.using_wireless = %d\n", access_pattern_config.using_wireless);
		
		if(access_pattern_config.using_wireless == 1) //无线
		{
			long long rx_bytes;
			long long tx_bytes;
			ret = lib_get_network_flow("ppp0", &rx_bytes,&tx_bytes);
			if(ret != LIB_GE_EOK)
			{
				retError = RET_ERROR_NETWORK_DISCONNT;  //网络异常
				timerCount = BINFO_RETURN_FAIL_PERIOD; 
				return -1;
			}
		}
	}
	
	/* 检查读卡器是否已经签到 */
	reg_stat = lib_lnt_register_stat_get();
	if(reg_stat != LNT_REGISTER_STAT_OK)
	{
		retError = RET_ERROR_LNT_NOT_REG; //读卡器没有签到
		timerCount = BINFO_RETURN_FAIL_PERIOD; 
		return -1;
	}
	
	/* 检查是否连接到自行车中心 */
	memset(&state, 0, sizeof(gui_access_state_t));
	if(lib_gui_access_state(g_gui, &state, 300) != LIB_GUI_EOK)  //检查是否连接中心成功
	{
		retError = RET_ERROR_CENTRE_NOT_REG;  //未注册
		timerCount = BINFO_RETURN_FAIL_PERIOD; 
		return -1;
	}

	if(state.connect_stat != 3)  //NDEV_NETSTAT_SESSION
	{
		retError = RET_ERROR_NETWORK_DISCONNT;  //网络异常
		timerCount = BINFO_RETURN_FAIL_PERIOD; 
		return -1;
	}


	retError = RET_ERROR_NORMAL;
	return 0;
}

void OnOffCardServiceDlg::slots_timerDone(void)
{
        //qDebug("-----------g_CardServiceFlag:%d\n",g_CardServiceFlag);
	QString string;
	
	switch(retError)
	{
		case RET_ERROR_NORMAL:
		{
                    if(g_CardServiceFlag == CARD_SERVICE_ON_NORMAL)
                            infoString.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上,并正确输入[身份证号码]和[手机号码], \
                           \n\n核对输入无误后,按\"执行开通操作\" ,进行会员开通。", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_TIME)
                            infoString.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上,并正确输入[开卡次数]、[身份证号码]和[手机号码], \
                           \n\n核对输入无误后,按\"执行开通操作\" ,进行会员开通。", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_DAY)
                            infoString.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上,并正确输入[开卡天数]、[身份证号码]和[手机号码], \
                           \n\n核对输入无误后,按\"执行开通操作\" ,进行会员开通。", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_MONTH)
                            infoString.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上,并正确输入[开卡月数]、[身份证号码]和[手机号码], \
                           \n\n核对输入无误后,按\"执行开通操作\" ,进行会员开通。", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_YEAR)
                            infoString.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上,并正确输入[开卡年数]、[身份证号码]和[手机号码], \
                           \n\n核对输入无误后,按\"执行开通操作\" ,进行会员开通。", timerCount);

                    else if(g_CardServiceFlag == CARD_SERVICE_OFF)
                                        infoString.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上,并正确输入[身份证号码], \
                                        \n\n核对输入无误后,按\"执行注销操作\" ,进行会员注销。", timerCount);
                }
		break;
		
		case RET_ERROR_LNT_BAD: //读卡器异常
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("提示:读卡器异常,不能执行开卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("提示:读卡器异常,不能执行销卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		}
		break;

		case RET_ERROR_LNT_NOT_REG: //读卡器未签到
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("提示:读卡器未签到,不能执行开卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("提示:读卡器未签到,不能执行销卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		}
		break;
		
		case RET_ERROR_NETWORK_DISCONNT: //网络异常
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("提示:网络异常,不能执行开卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);	
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("提示:网络异常,不能执行销卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);	
		}
		break;

		case RET_ERROR_CENTRE_NOT_REG: //未注册
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("提示:连接中心异常,不能执行开卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);	
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("提示:连接中心异常,不能执行销卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);	
		}
		break;

		case RET_ERROR_OP_FAIL:
		{
			infoString.sprintf(taskResult.s_info);
			QString string;
			string.sprintf("\n\n系统将会在%02d秒后自动返回上一级界面, 按[退出]键直接返回",  timerCount);
			infoString.append(string);

			string.clear();
		}
		break;

		case RET_ERROR_OP_LOOP:
		{
			infoString.sprintf(taskResult.s_info);
			QString string;
			string.sprintf("\n\n系统将会在%02d秒后自动返回上一级界面, 按[退出]键直接返回",  timerCount);
			infoString.append(string);

			string.clear();
		}
		break;

		case RET_ERROR_ID_NO:
		{
			infoString.sprintf("身份证号码输入长度不符合\n\n系统将会在%02d秒后自动返回上一级界面, 按[退出]键直接返回",  timerCount);
		}
		break;

		case RET_ERROR_PHONE_NUMBER:
		{
			infoString.sprintf("手机号码输入长度不符合\n\n系统将会在%02d秒后自动返回上一级界面, 按[退出]键直接返回",  timerCount);
		}
		break;
	}

	label_onOffCardShow->setText(infoString);
	infoString.clear();


	if(timerCount <= 0)
	{
		emit signals_OnOffCardQuit();
	}
	
	timerCount--;
}


void OnOffCardServiceDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)
	{
		if(esc_quit_key_f == ECS_QUIT_ON)
			emit signals_OnOffCardQuit();
	}

        else if(e->key() == Qt::Key_Up)
		focusPreviousChild();

        else if(e->key() == Qt::Key_Down)
		focusNextChild();
}

bool OnOffCardServiceDlg::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == pushButton_OnOffCardStart)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			pushButton_OnOffCardStart->setStyleSheet("background-color: rgb(89, 182, 75)");

		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_OnOffCardStart->setStyleSheet("background-color: rgb(224, 239, 198)");
		}
	}	

        if(watched == lineEdit_OnOffCardIdNO)
	{
		if(event->type() == QEvent::KeyPress)
		{
			QKeyEvent *KeyEvent = static_cast<QKeyEvent *>(event);
			if(KeyEvent->key() == Qt::Key_Period)
			{
				QString s_lineEdit = lineEdit_OnOffCardIdNO->text();
				int s_len = s_lineEdit.length();
				
				s_lineEdit.replace(s_len, 1, 'X');
				lineEdit_OnOffCardIdNO->setText(s_lineEdit);

				return TRUE;
			}
		}
	}

	/* 按"确定"键移动光标 */
        if((watched == lineEdit_memberCount) || (watched == lineEdit_OnOffCardIdNO) || (watched == lineEdit_OnOffCardPhoneNumber))
	{
		if(event->type() == QEvent::KeyPress)
		{
			QKeyEvent *KeyEvent = static_cast<QKeyEvent *>(event);
			
			if(KeyEvent->key() == Qt::Key_Space)
				focusNextChild();
		}
	}



	return QWidget::eventFilter(watched,event); 
}



