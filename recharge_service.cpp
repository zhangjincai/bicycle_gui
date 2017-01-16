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
	RET_ERROR_NORMAL = 0,   //����
	RET_ERROR_CARD_NOTAG = 1,  //û�п�
	RET_ERROR_NOVIP = 2,  //�ǻ�Ա 
	RET_ERROR_DP_INIT = 3,	//Ѻ���ʼ��
	RET_ERROR_OP_FAIL = 4, //����ʧ��
	RET_ERROR_OP_SUCCESS = 5,  //�����ɹ�
	RET_ERROR_OP_LOOP = 6, //��ѯ
	RET_ERROR_LNT_BAD = 7, //����ͨ�������� i
	RET_ERROR_NETWORK_DISCONNT = 8, //���粻ͨ
	RET_ERROR_LNT_NOT_REG = 9, //������δע��
	RET_ERROR_ID_NO = 10, //���֤����Ϊ��
	RET_ERROR_PHONE_NUMBER = 11, //�ֻ���������Ϊ��
	RET_ERROR_CENTRE_NOT_REG = 12,  //����δע��

        RET_ERROR_ORDER_NO = 13 //����������Ϊ��

};

RechargeServiceDlg::RechargeServiceDlg(QWidget *parent) : QDialog(parent)
{
        setupUi(this);
        setWindowFlags(Qt::FramelessWindowHint);

        pushButton_RechargeStart->installEventFilter(this);
        lineEdit_RechargeOrderNO->installEventFilter(this);

        confirmCnt = 0; //����ִ�г�ֵ��������

        ptn_start_f = 1;
        retError = RET_ERROR_NORMAL;
        timerCount = BINFO_RETURN_PERIOD;
        esc_quit_key_f = ECS_QUIT_ON;  //����ʹ��ESC�˳���

        pushButton_RechargeStart->setStyleSheet("background-color: rgb(224, 239, 198)");
        label_RechargeShow->clear();


        if(g_CardServiceFlag == CARD_SERVICE_RECHARGE) //��ֵ
        {
                label_RechargeTitle->setText(tr("��Ա��ֵ����"));
                pushButton_RechargeStart->setText(tr("ִ�г�ֵ����"));

                //pushButton_RechargeStart->setGeometry(420, 350, 151, 51); //���ÿؼ�λ��
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
    confirmCnt++;   //����ȷ�ϼ�������1
    qDebug("\n-----------confirmCnt:%d",confirmCnt);

#if 1
    //���������жԡ�ȷ�ϼ�����������
    if(confirmCnt == 1)
#endif
    {
	if(ptn_start_f == 1)
	{
		ptn_start_f = 2;

		QString string;
		struct TaskData taskdata;
		memset(&taskdata, 0, sizeof(struct TaskData));

                /* �жϳ�ֵ�����������Ƿ�Ϊ�� */
		#if 1
		QString s_lineEdit;
                if(g_CardServiceFlag == CARD_SERVICE_RECHARGE)
		{
                        s_lineEdit = lineEdit_RechargeOrderNO->text();
                        if(s_lineEdit.length() < 10)  //С�ںϷ�����
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
                    esc_quit_key_f = ECS_QUIT_OFF;  //��ֵ�����в�����ʹ��ESC�˳���
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

                        confirmCnt = 0; //ȷ�ϼ����¿�ʼ����
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

            esc_quit_key_f = ECS_QUIT_ON;  //����ʹ��ESC�˳���
            confirmCnt = 0; //ȷ�ϼ����¿�ʼ����
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

	        esc_quit_key_f = ECS_QUIT_ON;  //����ʹ��ESC�˳���
	        confirmCnt = 0; //ȷ�ϼ����¿�ʼ����
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

	/* ���������Ƿ���� */
	ret = lib_lnt_get_version(&ver, &stat, 200);
	if((ret < 0) || (stat != LNT_MOC_ERROR_NORMAL)) 
	{	 
		retError = RET_ERROR_LNT_BAD;   //�������쳣 
		timerCount = BINFO_RETURN_FAIL_PERIOD; 
		return -1;
	}

	/* ����������� */
	struct gui_ndev_access_pattern_config access_pattern_config;
	memset(&access_pattern_config, 0, sizeof(struct gui_ndev_access_pattern_config));
	ret = lib_gui_get_ndev_access_pattern_config(g_gui, &access_pattern_config, 300);
	if(ret == LIB_GUI_EOK)
	{
		//fprintf(stderr, "ON OFF CARD:access_pattern_config.using_wireless = %d\n", access_pattern_config.using_wireless);
		
		if(access_pattern_config.using_wireless == 1) //����
		{
			long long rx_bytes;
			long long tx_bytes;
			ret = lib_get_network_flow("ppp0", &rx_bytes,&tx_bytes);
			if(ret != LIB_GE_EOK)
			{
				retError = RET_ERROR_NETWORK_DISCONNT;  //�����쳣
				timerCount = BINFO_RETURN_FAIL_PERIOD; 
				return -1;
			}
		}
	}
	
	/* ���������Ƿ��Ѿ�ǩ�� */
	reg_stat = lib_lnt_register_stat_get();
	if(reg_stat != LNT_REGISTER_STAT_OK)
	{
		retError = RET_ERROR_LNT_NOT_REG; //������û��ǩ��
		timerCount = BINFO_RETURN_FAIL_PERIOD; 
		return -1;
	}
	
	/* ����Ƿ����ӵ����г����� */
	memset(&state, 0, sizeof(gui_access_state_t));
	if(lib_gui_access_state(g_gui, &state, 300) != LIB_GUI_EOK)  //����Ƿ��������ĳɹ�
	{
		retError = RET_ERROR_CENTRE_NOT_REG;  //δע��
		timerCount = BINFO_RETURN_FAIL_PERIOD; 
		return -1;
	}

	if(state.connect_stat != 3)  //NDEV_NETSTAT_SESSION
	{
		retError = RET_ERROR_NETWORK_DISCONNT;  //�����쳣
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
                        infoString.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������,����ȷ����[��ֵ������], \
                        \n\n�˶����������,��\"ִ�г�ֵ����\",���л�Ա��ֵ��", timerCount);
		}
		break;
		
		case RET_ERROR_LNT_BAD: //�������쳣
		{
                        infoString.sprintf("��ʾ:�������쳣,����ִ�г�ֵ����, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		}
		break;

		case RET_ERROR_LNT_NOT_REG: //������δǩ��
		{
                        infoString.sprintf("��ʾ:������δǩ��,����ִ�г�ֵ����, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		}
		break;
		
		case RET_ERROR_NETWORK_DISCONNT: //�����쳣
		{
                        infoString.sprintf("��ʾ:�����쳣,����ִ�г�ֵ����, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		}
		break;

		case RET_ERROR_CENTRE_NOT_REG: //δע��
		{
                        infoString.sprintf("��ʾ:���������쳣,����ִ�г�ֵ����, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		}
		break;

                case RET_ERROR_ORDER_NO:
                {
                        infoString.sprintf("��ֵ���������볤�Ȳ�����\n\nϵͳ������%02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���",  timerCount);
                }
                break;

		case RET_ERROR_OP_FAIL:
		{
			infoString.sprintf(taskResult.s_info);
			QString string;
			string.sprintf("\n\nϵͳ������%02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���",  timerCount);
			infoString.append(string);

			string.clear();
		}
		break;

		case RET_ERROR_OP_LOOP:
		{
			infoString.sprintf(taskResult.s_info);
			QString string;
			string.sprintf("\n\nϵͳ������%02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���",  timerCount);
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

	/* ��"ȷ��"���ƶ���� */
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






