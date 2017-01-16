#include <QtGui>
#include <QDebug>
#include <QKeyEvent>
#include <QString>
#include <QThreadPool>
#include "helper.h"
#include "lib_general.h"
#include "recharge_service_dlg.h"
#include "ui_recharge_service_ui.h"


#define BINFO_RETURN_PERIOD					(120)
#define BINFO_RETURN_FAIL_PERIOD				(25)


#define ECS_QUIT_ON		1
#define ECS_QUIT_OFF		2


extern int g_CardServiceFlag;
extern lib_gui_t *g_gui;
	
enum RET_ERROR
{
	RET_ERROR_NORMAL = 0,   //正常
	RET_ERROR_CARD_NOTAG = 1,  //没有卡
	RET_ERROR_NOVIP = 2,  //非会员 
	RET_ERROR_DP_INIT = 3,	//押金初始化
	RET_ERROR_OP_FAIL = 4, //操作失败
	RET_ERROR_OP_SUCCESS = 5,  //操作成功
	RET_ERROR_OP_LOOP = 6, //轮询
	RET_ERROR_LNT_BAD = 7, //岭南通读卡器坏 i
	RET_ERROR_NETWORK_DISCONNT = 8, //网络不通
	RET_ERROR_LNT_NOT_REG = 9, //读卡器未注册
	RET_ERROR_ID_NO = 10, //身份证输入为空
	RET_ERROR_PHONE_NUMBER = 11, //手机号码输入为空
	RET_ERROR_CENTRE_NOT_REG = 12,  //中心未注册

        RET_ERROR_ORDER_NO = 13 //订单号输入为空

};

RechargeServiceDlg::RechargeServiceDlg(QWidget *parent) : QDialog(parent)
{
        setupUi(this);
        setWindowFlags(Qt::FramelessWindowHint);

        pushButton_RechargeStart->installEventFilter(this);
        lineEdit_RechargeOrderNO->installEventFilter(this);

        confirmCnt = 0; //按下执行充值操作次数

        ptn_start_f = 1;
        retError = RET_ERROR_NORMAL;
        timerCount = BINFO_RETURN_PERIOD;
        esc_quit_key_f = ECS_QUIT_ON;  //允许使用ESC退出键

        pushButton_RechargeStart->setStyleSheet("background-color: rgb(224, 239, 198)");
        label_RechargeShow->clear();


        if(g_CardServiceFlag == CARD_SERVICE_RECHARGE) //充值
        {
                label_RechargeTitle->setText(tr("会员充值服务"));
                pushButton_RechargeStart->setText(tr("执行充值操作"));

                //pushButton_RechargeStart->setGeometry(420, 350, 151, 51); //设置控件位置
        }

        checkEnvironment();
        slots_timerDone();

        connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
        timer.start(1000);

        connect(this, SIGNAL(signals_RechargeQuit()), this, SLOT(accept()));
        connect(pushButton_RechargeStart, SIGNAL(clicked()), this, SLOT(slots_RechargeStart()));
}

RechargeServiceDlg::~RechargeServiceDlg(void)
{
	
}


void RechargeServiceDlg::slots_RechargeStart(void)
{
    confirmCnt++;   //按下确认键次数加1
    qDebug("\n-----------confirmCnt:%d",confirmCnt);

#if 1
    //开卡过程中对“确认键”进行限制
    if(confirmCnt == 1)
#endif
    {
	if(ptn_start_f == 1)
	{
		ptn_start_f = 2;

		QString string;
		struct TaskData taskdata;
		memset(&taskdata, 0, sizeof(struct TaskData));

                /* 判断充值订单号输入是否为空 */
		#if 1
		QString s_lineEdit;
                if(g_CardServiceFlag == CARD_SERVICE_RECHARGE)
		{
                        s_lineEdit = lineEdit_RechargeOrderNO->text();
                        if(s_lineEdit.length() < 10)  //小于合法长度
			{
                                retError = RET_ERROR_ORDER_NO;
				timerCount = BINFO_RETURN_FAIL_PERIOD;
				ptn_start_f = 1;
				return;
			}
		}
		#endif
		
		taskdata.serviceType = g_CardServiceFlag;
                string = lineEdit_RechargeOrderNO->text();
		char *s = Helper::__QStringToChar(string);
                strcpy((char *)taskdata.order_number, s);

		QString s_tmp;
		string.clear();
                string = lineEdit_RechargeOrderNO->text();

		s_tmp.append(string);

		char *s_pn = Helper::__QStringToChar(s_tmp);
		unsigned char s_hex[32] = {0};
		lib_lnt_str_to_hex((unsigned char *)s_pn, (unsigned char *)s_hex, strlen(s_pn));

                memcpy(taskdata.order_number, s_hex, 10);
		
                //lib_printf("BCD:", taskdata.order_number, 10);
		
		OnOffCardTask * task = new OnOffCardTask(&taskdata);
		task->setAutoDelete(true);
		connect(task, SIGNAL(signals_Result(struct TaskResult *)), this, SLOT(slots_TaskResult(struct TaskResult *)), Qt::QueuedConnection);
	 	QThreadPool::globalInstance()->start(task);	


                #if 1
                if(confirmCnt > 0)
                #endif
                {
                    esc_quit_key_f = ECS_QUIT_OFF;  //充值过程中不允许使用ESC退出键
                }

		label_RechargeShow->clear();
	}
     }
}


void RechargeServiceDlg::slots_TaskResult(struct TaskResult *result)
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

            if(result->serviceType == CARD_SERVICE_RECHARGE)
            {
                switch(result->taskRunStep)
                {
                    case TaskRunStep_p0:
                    {
                        label_MemberRechargeProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/member_recharge_p0.png")));
                    }
                    break;

                    case TaskRunStep_p1:
                    {
                        label_MemberRechargeProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/member_recharge_p1.png")));
                    }
                    break;

                    case TaskRunStep_p2:
                    {
                        label_MemberRechargeProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/member_recharge_p2.png")));
                    }
                    break;

                    case TaskRunStep_p3:
                    {
                        label_MemberRechargeProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/member_recharge_p3.png")));
                    }
                    break;

                    case TaskRunStep_p4:
                    {
                        label_MemberRechargeProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/member_recharge_p4.png")));

                        confirmCnt = 0; //确认键重新开始计数
                    }
                }
            }

		free(result);
		result = NULL;	
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
            label_RechargeShow->setText(infoString);
		}
		break;

		case TASK_RESULT_OP_LOOP_FREE:
		{
			retError = RET_ERROR_OP_LOOP;
			timerCount = BINFO_RETURN_FAIL_PERIOD;
			memset(&taskResult, 0, sizeof(struct TaskResult));
			memcpy(&taskResult, result, sizeof(struct TaskResult));
			timer.start(1000); 	

			free(result);
			result = NULL;

            esc_quit_key_f = ECS_QUIT_ON;  //允许使用ESC退出键
            confirmCnt = 0; //确认键重新开始计数
		}
		break;
		
		case TASK_RESULT_OP_FAIL_FREE:
		{
			retError = RET_ERROR_OP_FAIL;
			timerCount = BINFO_RETURN_FAIL_PERIOD;
			memset(&taskResult, 0, sizeof(struct TaskResult));
			memcpy(&taskResult, result, sizeof(struct TaskResult));
			timer.start(1000); 	
			
			free(result);
			result = NULL;

	        esc_quit_key_f = ECS_QUIT_ON;  //允许使用ESC退出键
	        confirmCnt = 0; //确认键重新开始计数
	}
		break;
	}
	
	
	 

	  
	//qDebug("activeThreadCount:%d\n", QThreadPool::globalInstance()->activeThreadCount());


	 
	 ptn_start_f = 1;
}






int RechargeServiceDlg::checkEnvironment(void)
{
	int ret = -1;
	unsigned char stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	lib_lnt_getversion_ack_t ver;
	enum LNT_REGISTER_STAT reg_stat = LNT_REGISTER_STAT_INIT;
	gui_access_state_t state;

	/* 检查读卡器是否存在 */
	ret = lib_lnt_get_version(&ver, &stat, 200);
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

void RechargeServiceDlg::slots_timerDone(void)
{
	QString string;
	
	switch(retError)
	{
		case RET_ERROR_NORMAL:
		{
                        infoString.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上,并正确输入[充值订单号], \
                        \n\n核对输入无误后,按\"执行充值操作\",进行会员充值。", timerCount);
		}
		break;
		
		case RET_ERROR_LNT_BAD: //读卡器异常
		{
                        infoString.sprintf("提示:读卡器异常,不能执行充值操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		}
		break;

		case RET_ERROR_LNT_NOT_REG: //读卡器未签到
		{
                        infoString.sprintf("提示:读卡器未签到,不能执行充值操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		}
		break;
		
		case RET_ERROR_NETWORK_DISCONNT: //网络异常
		{
                        infoString.sprintf("提示:网络异常,不能执行充值操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		}
		break;

		case RET_ERROR_CENTRE_NOT_REG: //未注册
		{
                        infoString.sprintf("提示:连接中心异常,不能执行充值操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		}
		break;

                case RET_ERROR_ORDER_NO:
                {
                        infoString.sprintf("充值订单号输入长度不符合\n\n系统将会在%02d秒后自动返回上一级界面, 按[退出]键直接返回",  timerCount);
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

	}

        label_RechargeShow->setText(infoString);
	infoString.clear();


	if(timerCount <= 0)
	{
                emit signals_RechargeQuit();
	}
	
	timerCount--;
}

void RechargeServiceDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)
	{
		if(esc_quit_key_f == ECS_QUIT_ON)
                        emit signals_RechargeQuit();
	}

	if(e->key() == Qt::Key_Up)
		focusPreviousChild();

	if(e->key() == Qt::Key_Down)
		focusNextChild();
}

bool RechargeServiceDlg::eventFilter(QObject *watched, QEvent *event)
{
        if(watched == pushButton_RechargeStart)
	{
		if(event->type()==QEvent::FocusIn) 
		{
                        pushButton_RechargeStart->setStyleSheet("background-color: rgb(89, 182, 75)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
                        pushButton_RechargeStart->setStyleSheet("background-color: rgb(224, 239, 198)");
		}
	}	

#if 1
        if(watched == lineEdit_RechargeOrderNO)
	{
		if(event->type() == QEvent::KeyPress)
		{
			QKeyEvent *KeyEvent = static_cast<QKeyEvent *>(event);
			if(KeyEvent->key() == Qt::Key_Period)
			{
                                QString s_lineEdit = lineEdit_RechargeOrderNO->text();
				int s_len = s_lineEdit.length();
				
				s_lineEdit.replace(s_len, 1, 'X');
                                lineEdit_RechargeOrderNO->setText(s_lineEdit);

				return TRUE;
			}
		}
	}
#endif

	/* 按"确定"键移动光标 */
        if((watched == lineEdit_RechargeOrderNO))
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






