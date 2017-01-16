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

extern int memberTypeCheck;  //������Ա���ͱ�ʶ

	
enum RET_ERROR
{
	RET_ERROR_NORMAL = 0,   //����
	RET_ERROR_CARD_NOTAG = 1,  //û�п�
	RET_ERROR_NOVIP = 2,  //�ǻ�Ա 
	RET_ERROR_DP_INIT = 3,	//Ѻ���ʼ��
	RET_ERROR_OP_FAIL = 4, //����ʧ��
	RET_ERROR_OP_SUCCESS = 5,  //�����ɹ�
	RET_ERROR_OP_LOOP = 6, //��ѯ
	RET_ERROR_LNT_BAD = 7, //����ͨ�������� 
	RET_ERROR_NETWORK_DISCONNT = 8, //���粻ͨ
	RET_ERROR_LNT_NOT_REG = 9, //������δע��
	RET_ERROR_ID_NO = 10, //���֤����Ϊ��
	RET_ERROR_PHONE_NUMBER = 11, //�ֻ���������Ϊ��
	RET_ERROR_CENTRE_NOT_REG = 12,  //����δע��
};

OnOffCardServiceDlg::OnOffCardServiceDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	
	
	pushButton_OnOffCardStart->installEventFilter(this);
	lineEdit_OnOffCardIdNO->installEventFilter(this);
	lineEdit_OnOffCardPhoneNumber->installEventFilter(this);
        lineEdit_memberCount->installEventFilter(this);
		
        confirmCnt = 0; //�������������´���


	ptn_start_f = 1;
	retError = RET_ERROR_NORMAL;
	timerCount = BINFO_RETURN_PERIOD;
	esc_quit_key_f = ECS_QUIT_ON;  //����ʹ��ESC�˳���

	pushButton_OnOffCardStart->setStyleSheet("background-color: rgb(224, 239, 198)");
	label_onOffCardShow->clear();
	
        //qDebug("g_CardServiceFlag:%d\n",g_CardServiceFlag);

        pushButton_OnOffCardStart->setText(tr("ִ�п�ͨ����"));

        if(g_CardServiceFlag == CARD_SERVICE_ON_NORMAL)
	{
		label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));
		
                label_memberCount->setVisible(false);   //��ͨ����Աʱ�����������ɼ�
                lineEdit_memberCount->setVisible(false);
                label_memberCnt->setVisible(false);

                label_OnOffCardTitle->setText(tr("��ͨ��Ա��ͨ����"));
	}
        else if(g_CardServiceFlag == CARD_SERVICE_ON_TIME)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/time_count.png")));

                label_OnOffCardTitle->setText(tr("�ο���Ա��ͨ����"));
                label_memberCnt->setText(tr("��"));
        }
        else if(g_CardServiceFlag == CARD_SERVICE_ON_DAY)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/day_count.png")));

                label_OnOffCardTitle->setText(tr("�쿨��Ա��ͨ����"));
                label_memberCnt->setText(tr("��"));

        }
        else if(g_CardServiceFlag == CARD_SERVICE_ON_MONTH)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/month_count.png")));

                label_OnOffCardTitle->setText(tr("�¿���Ա��ͨ����"));
                label_memberCnt->setText(tr("��"));

        }
        else if(g_CardServiceFlag == CARD_SERVICE_ON_YEAR)
        {
                label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p0.png")));

                label_memberCount->setVisible(true);
                lineEdit_memberCount->setVisible(true);
                label_memberCnt->setVisible(true);

                label_memberCount->setPixmap(QPixmap(QString::fromUtf8(":/image/year_count.png")));

                label_OnOffCardTitle->setText(tr("�꿨��Ա��ͨ����"));
                label_memberCnt->setText(tr("��"));
        }


        else if(g_CardServiceFlag == CARD_SERVICE_OFF) //����
	{         
		label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p0.png")));
			
		label_OnOffCardTitle->setText(tr("��Աע������"));	
		pushButton_OnOffCardStart->setText(tr("ִ��ע������"));

		label_OnOffCardPhoneNumber->setVisible(false);
		lineEdit_OnOffCardPhoneNumber->setVisible(false);

                //����ʱ�����������ɼ�
                label_memberCount->setVisible(false);
                lineEdit_memberCount->setVisible(false);
                label_memberCnt->setVisible(false);

                pushButton_OnOffCardStart->setGeometry(420, 380, 151, 51); //���ÿؼ�λ��
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
        unsigned short unuse_cnt = 0; //ʣ�����������������������

        confirmCnt++;   //����ȷ�ϼ�������1
        qDebug("\n-----------confirmCnt:%d",confirmCnt);

        qDebug("g_CardServiceFlag:%d\n",g_CardServiceFlag);

#if 1   //���ʣ�����������ʱ��,��Ӱ���������!
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
                //����ͨģ�鴮�ڳ�ʼ��
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
    /*//�����������жԡ�ȷ�ϼ�����������
     if(((confirmCnt <= 1) && ((g_CardServiceFlag == CARD_SERVICE_ON_NORMAL) || (g_CardServiceFlag == CARD_SERVICE_ON_TIME) \
        || (g_CardServiceFlag == CARD_SERVICE_ON_DAY) || (g_CardServiceFlag == CARD_SERVICE_ON_MONTH) \
        || (g_CardServiceFlag == CARD_SERVICE_ON_YEAR))) \
        || ((unuse_cnt > 0) && (confirmCnt <= 2) && (g_CardServiceFlag == CARD_SERVICE_OFF)) \
        || ((unuse_cnt == 0) && (confirmCnt == 1) && (g_CardServiceFlag == CARD_SERVICE_OFF))) //����(���������Ƿ���δʹ����Ĵ���)
     */
      //�������Ϊ��ʱһֱ��������ʾ���Ե����� by zjc at 2016-10-27
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

		/* �ж����֤�͵绰���������Ƿ�Ϊ�� */
		#if 1
		QString s_lineEdit;
		//if(g_CardServiceFlag == CARD_SERVICE_ON)
		if(g_CardServiceFlag != CARD_SERVICE_OFF) //�����������Ϊ���ж�ʧЧ������ by zjc at 2016-10-27
		{
			s_lineEdit = lineEdit_OnOffCardIdNO->text();
			printf("-------------IdNO len:%d\n", s_lineEdit.length());
			if(s_lineEdit.length() < 15)  //С�ںϷ�����
			{
				retError = RET_ERROR_ID_NO;
				timerCount = BINFO_RETURN_FAIL_PERIOD;
				ptn_start_f = 1;
				return;
			}

			s_lineEdit.clear();
			s_lineEdit = lineEdit_OnOffCardPhoneNumber->text();
			printf("-------------PhoneNumber len:%d\n", s_lineEdit.length());
			if(s_lineEdit.length() < 11) //С�ںϷ�����
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
			if(s_lineEdit.length() < 15)  //С�ںϷ�����
			{
				retError = RET_ERROR_ID_NO;
				timerCount = BINFO_RETURN_FAIL_PERIOD;
				ptn_start_f = 1;
				return;
			}
		}
		#endif
		
		taskdata.serviceType = g_CardServiceFlag;
                string = lineEdit_OnOffCardIdNO->text(); //���֤����
		char *s = Helper::__QStringToChar(string);
		strcpy((char *)taskdata.id_card, s);  

		QString s_tmp;
		string.clear();
                string = lineEdit_OnOffCardPhoneNumber->text(); //�ֻ�����

		s_tmp.append('0');
		s_tmp.append(string);

		char *s_pn = Helper::__QStringToChar(s_tmp);
		unsigned char s_hex[32] = {0};
		lib_lnt_str_to_hex((unsigned char *)s_pn, (unsigned char *)s_hex, strlen(s_pn));

		memcpy(taskdata.phone_number, s_hex, 6);
		
		//lib_printf("BCD:", taskdata.phone_number, 6);

#if 1
                ///////////////����ͨ��Ա�������� 2016-04-25 add by zjc//////////
                unsigned short cnt;
                string.clear();
                string = lineEdit_memberCount->text();
                cnt = Helper::__QStringToUshort(string);

                //qDebug("-----------member_type:%d\n",memberTypeCheck);
                //qDebug("-----------member_typeCount:%d\n",cnt);

                taskdata.member_type = memberTypeCheck; //��Ա���ͣ�Ʊ�ֱ�ǣ�
                memcpy(&taskdata.member_typeCount, &cnt, 2); //����������������������
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
            || ((unuse_cnt > 0) && (confirmCnt > 1) && (g_CardServiceFlag == CARD_SERVICE_OFF))) //����(���������Ƿ���δʹ����Ĵ���)
    #else
        if(confirmCnt != 1)
    #endif
        {
                esc_quit_key_f = ECS_QUIT_OFF;  //�����������в�����ʹ��ESC�˳���
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

                                case TaskRunStep_p4: //�������һ��
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/on_card_p4.png")));
                                        confirmCnt = 0; //ȷ�ϼ����¿�ʼ����
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

                                case TaskRunStep_p5:    //�������һ��
				{
					label_OnOffCarProcShow->setPixmap(QPixmap(QString::fromUtf8(":/image/off_card_p5.png")));
                                        confirmCnt = 0; //ȷ�ϼ����¿�ʼ����
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

			if(result->memberCount != 0xFFFF) //������ʾȷ������������ģ���ʾȷ�����������û�ȷ�Ϻ��������ȥ add by zjc at 2016-11-01 
				confirmCnt = 0; //ȷ�ϼ����¿�ʼ����
			
            esc_quit_key_f = ECS_QUIT_ON;  //����ʹ��ESC�˳���
							
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

                        esc_quit_key_f = ECS_QUIT_ON;  //����ʹ��ESC�˳���
                        confirmCnt = 0; //ȷ�ϼ����¿�ʼ����
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

	/* ���������Ƿ���� */
	ret = lib_lnt_get_version(&ver, &stat, 200); //ע���������Ŷ���������!!!
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

void OnOffCardServiceDlg::slots_timerDone(void)
{
        //qDebug("-----------g_CardServiceFlag:%d\n",g_CardServiceFlag);
	QString string;
	
	switch(retError)
	{
		case RET_ERROR_NORMAL:
		{
                    if(g_CardServiceFlag == CARD_SERVICE_ON_NORMAL)
                            infoString.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������,����ȷ����[���֤����]��[�ֻ�����], \
                           \n\n�˶����������,��\"ִ�п�ͨ����\" ,���л�Ա��ͨ��", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_TIME)
                            infoString.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������,����ȷ����[��������]��[���֤����]��[�ֻ�����], \
                           \n\n�˶����������,��\"ִ�п�ͨ����\" ,���л�Ա��ͨ��", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_DAY)
                            infoString.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������,����ȷ����[��������]��[���֤����]��[�ֻ�����], \
                           \n\n�˶����������,��\"ִ�п�ͨ����\" ,���л�Ա��ͨ��", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_MONTH)
                            infoString.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������,����ȷ����[��������]��[���֤����]��[�ֻ�����], \
                           \n\n�˶����������,��\"ִ�п�ͨ����\" ,���л�Ա��ͨ��", timerCount);
                    else if(g_CardServiceFlag == CARD_SERVICE_ON_YEAR)
                            infoString.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������,����ȷ����[��������]��[���֤����]��[�ֻ�����], \
                           \n\n�˶����������,��\"ִ�п�ͨ����\" ,���л�Ա��ͨ��", timerCount);

                    else if(g_CardServiceFlag == CARD_SERVICE_OFF)
                                        infoString.sprintf("��ʾ:����%02d����,�ѿ����õ�ָ��ˢ��������,����ȷ����[���֤����], \
                                        \n\n�˶����������,��\"ִ��ע������\" ,���л�Աע����", timerCount);
                }
		break;
		
		case RET_ERROR_LNT_BAD: //�������쳣
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:�������쳣,����ִ�п�������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:�������쳣,����ִ����������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		}
		break;

		case RET_ERROR_LNT_NOT_REG: //������δǩ��
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:������δǩ��,����ִ�п�������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:������δǩ��,����ִ����������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);
		}
		break;
		
		case RET_ERROR_NETWORK_DISCONNT: //�����쳣
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:�����쳣,����ִ�п�������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);	
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:�����쳣,����ִ����������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);	
		}
		break;

		case RET_ERROR_CENTRE_NOT_REG: //δע��
		{
                        if(g_CardServiceFlag != CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:���������쳣,����ִ�п�������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);	
			else if(g_CardServiceFlag == CARD_SERVICE_OFF)
				infoString.sprintf("��ʾ:���������쳣,����ִ����������, %02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���", timerCount);	
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

		case RET_ERROR_ID_NO:
		{
			infoString.sprintf("���֤�������볤�Ȳ�����\n\nϵͳ������%02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���",  timerCount);
		}
		break;

		case RET_ERROR_PHONE_NUMBER:
		{
			infoString.sprintf("�ֻ��������볤�Ȳ�����\n\nϵͳ������%02d����Զ�������һ������, ��[�˳�]��ֱ�ӷ���",  timerCount);
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

	/* ��"ȷ��"���ƶ���� */
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



